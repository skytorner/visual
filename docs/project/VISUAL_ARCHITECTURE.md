# AV Sampler — Architecture actuelle

Cadrage : 2026-11-24. `AVSampler` est un instrument vidéo MIDI JUCE natif. `AudioReactiveImagePOC` reste une cible séparée, conservée pour ses projets audio existants.

## Stack et identités

- JUCE 8.0.8, C++17 et CMake.
- `AudioReactiveImagePOC` conserve ses identifiants, paramètres, formats et état historiques.
- AV Sampler a sa propre identité de plugin et est actuellement configuré comme instrument AU macOS uniquement. VST3/Standalone, Windows et autres hôtes ne sont pas qualifiés pour AV Sampler.
- Interface JUCE native. Le lecteur vidéo macOS utilise AVFoundation et la présentation utilise Metal/MetalKit.

## Flux actuel AV Sampler

```text
DAW / MIDI
  -> AVSamplerAudioProcessor::processBlock (instrument silencieux)
  -> notes 36/38/40/41 -> Cue 1..4; CC1 -> Speed; vélocité NoteOn -> RGB trigger
  -> atomiques bornés vers l'éditeur
  -> timer éditeur sur le thread message
       -> transport/seek/speed de VideoFramePlayerComponent
       -> trigger de l'enveloppe RGB côté renderer
  -> AVFoundation AVPlayerItemVideoOutput
  -> CVPixelBuffer -> CVMetalTextureCache -> MTKView / shader RGB Split

État JUCE sauvegardé dans le projet hôte : chemin média, quatre Cue timings,
Loop, Play/Pause demandé, Cue sélectionné, valeur CC1 et activation RGB Split.
```

## MIDI, threads et rendu

- `processBlock()` ne charge/décode pas de média, ne seek pas, ne fait pas d'I/O et n'appelle pas l'UI. Il traite les événements MIDI et publie les valeurs/événements nécessaires via des atomiques.
- NoteOff, vélocité zéro et notes non assignées ne déclenchent pas de Cue/RGB. Si plusieurs cues arrivent avant le prochain tick éditeur, la mailbox ne retient que le plus récent.
- L'éditeur exécute les opérations de transport sur le thread message. La lecture, le seek, la vitesse et le rendu vidéo ne tournent pas dans le callback audio.
- Le RGB Split décale rouge et bleu en directions opposées, garde le vert centré et borne les UV. La vélocité pilote l'intensité; l'enveloppe Attack/Hold/Release est transitoire. L'activation est persistée.
- CC1 pilote Speed indépendamment de RGB Split. Mapping courant : 0,5× à 1,5×; les extrêmes ont été confirmés en Logic, neutralité à 64/save-reload restent à vérifier explicitement.
- La fluidité de lecture en boucle est confirmée à l'usage par l'utilisateur. Aucune mesure de latence MIDI-image, framerate ou coût CPU/GPU n'est disponible.

## Média, timeline et session

- Un seul lecteur et une seule vidéo à la fois. `CueTimelineComponent` et `AssignedPadsComponent` représentent les mêmes quatre états du `ProjectState`; les pads et la timeline synchronisent leur sélection sans déclencher de NoteOn.
- Cue notes : MIDI 36/38/40/41, présentées C1/D1/E1/F1 dans le Logic testé. L'appellation d'octave varie entre hôtes.
- Le média reste externe; le projet conserve son chemin, pas les octets vidéo. Les anciens états sans préférence Play/Pause démarrent en lecture à l'ouverture de l'éditeur; les autres nouveaux champs ont des valeurs par défaut documentées par les tests.
- L'état média absent est récupérable depuis l'éditeur. Le relink/remplacement est conçu pour préserver les Cue timings, Loop et préférences, mais le scénario utilisateur complet de déplacement/relink reste à confirmer dans la checklist de release.
- Les cues peuvent être déplacés sur la timeline. Le nudging clavier utilise un pas nominal de 1/30 s (Maj = 10 pas), car la cadence source n'est pas exposée par l'API projet.

## Historique d'architecture — décisions supersédées

- VISUAL-009 à VISUAL-013 utilisaient `juce::VideoComponent` pour la présentation et la vitesse. Cette description est historique; VISUAL-014a l'a remplacée par `VideoFramePlayerComponent` et le chemin AVFoundation/Metal ci-dessus.
- L'investigation VISUAL-014 avait conclu que `VideoComponent` encapsulait un `AVPlayerLayer` sans exposer les frames. Cette limite explique le spike, mais ne décrit plus le renderer sur `main`.
- L'interdiction GPU de l'ancienne direction Audio Reactive est supersédée pour AV Sampler par VISUAL-014a/b. Elle reste sans effet sur le plugin audio historique.

## Plugin audio historique

```text
DAW (effet audio) -> PluginProcessor::processBlock() -> RMS/Peak/Envelope -> atomics
                  -> éditeur JUCE timer -> image et transformations
État APVTS + imagePath -> projet DAW
```

- MIDI n'est pas utilisé par `AudioReactiveImagePOC`.
- Préserver son passthrough, ses IDs, paramètres, état et chargement d'image.
- Les fichiers `PluginProcessor.*`/`PluginEditor.*` restent séparés du sampler.

## Invariants et qualification restant à faire

1. Aucun mutex, I/O, décodage, seek bloquant, allocation dynamique ou appel UI dans `processBlock()`.
2. Le produit actuellement qualifié est l'AU macOS dans Logic; ne pas généraliser à d'autres hôtes, formats ou plateformes.
3. Le projet hôte conserve les paramètres/session; le fichier vidéo demeure externe.
4. Un média absent ou illisible doit rester récupérable sans crash; valider relink/remplacement dans la version candidate de distribution.
5. Ne pas modifier l'identité ni l'état du POC audio.
6. Ne pas annoncer synchronisation sample/frame-accurate, latence garantie ou coût de rendu sans mesure dédiée.
