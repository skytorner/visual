# AV Sampler — Architecture actuelle

Cadrage : 2026-11-24. `AVSampler` est un instrument vidéo MIDI JUCE natif. `AudioReactiveImagePOC` reste une cible séparée, conservée pour ses projets audio existants.

## Stack et identités

- JUCE 8.0.8, C++17 et CMake.
- `AudioReactiveImagePOC` conserve ses identifiants, paramètres, formats et état historiques.
- AV Sampler garde sa propre identité de plugin. Le prototype ajoute les formats AU + VST3 sur macOS et VST3 sur Windows; seul l'AU macOS dans Logic est actuellement qualifié.
- Interface JUCE native. Le renderer macOS utilise AVFoundation et Metal/MetalKit. Le prototype Windows enveloppe `juce::VideoComponent` pour lecture/transport; son build x64 passe dans GitHub Actions (run `45435924725`), mais le chargement et le comportement dans REAPER restent à qualifier dans VISUAL-017. Cette vue ne donne pas accès aux frames pour RGB Split et Flash; ces effets sont signalés indisponibles sur ce prototype.

## Flux actuel AV Sampler

```text
DAW / MIDI
  -> AVSamplerAudioProcessor::processBlock (instrument silencieux)
  -> notes 36/38/40/41 -> Cue 1..4; CC1 -> Speed; vélocité NoteOn -> triggers RGB Split + Flash + Zoom Punch
  -> atomiques bornés vers l'éditeur
  -> timer éditeur sur le thread message
       -> transport/seek/speed de VideoFramePlayerComponent
       -> triggers des enveloppes RGB Split, Flash et Zoom Punch côté renderer
  -> macOS: AVFoundation AVPlayerItemVideoOutput -> CVPixelBuffer
             -> CVMetalTextureCache -> MTKView / shader RGB Split + Flash + Zoom Punch
  -> Windows: JUCE VideoComponent / playback backend (effets visuels indisponibles)

État JUCE sauvegardé dans le projet hôte : chemin média, quatre Cue timings,
Loop, Play/Pause demandé, Cue sélectionné, valeur CC1, activation RGB Split, Flash et Zoom Punch.
```

## MIDI, threads et rendu

- `processBlock()` ne charge/décode pas de média, ne seek pas, ne fait pas d'I/O et n'appelle pas l'UI. Il traite les événements MIDI et publie les valeurs/événements nécessaires via des atomiques.
- NoteOff, vélocité zéro et notes non assignées ne déclenchent pas de Cue/RGB. Si plusieurs cues arrivent avant le prochain tick éditeur, la mailbox ne retient que le plus récent.
- L'éditeur exécute les opérations de transport sur le thread message. La lecture, le seek, la vitesse et le rendu vidéo ne tournent pas dans le callback audio.
- Le RGB Split décale rouge et bleu en directions opposées, garde le vert centré et borne les UV. La vélocité pilote l'intensité; l'enveloppe Attack/Hold/Release est transitoire. L'activation est persistée.
- Flash mélange l'image vers le blanc, au maximum 65 %. Le NoteOn partagé avec RGB Split fournit la vélocité; durée fixe Attack/Hold/Release 5/40/180 ms. Activation persistée, désactivée par défaut, hors état ancien si la propriété est absente. Rendu Metal macOS uniquement.
- Zoom Punch transforme les UV autour du centre de l'image avant l'échantillonnage; l'échelle varie de 1,0× à 1,12× selon la vélocité et revient à 1,0× avec la même enveloppe 5/40/180 ms. Le même buffer de constantes Metal porte décalage RGB, mélange Flash et échelle Zoom. Activation persistée, désactivée par défaut dans les anciennes sessions. Rendu Metal macOS uniquement; l'utilisateur confirme que l'effet fonctionne dans Logic.
- CC1 pilote Speed indépendamment de RGB Split. Mapping courant : 0,5× à 1,5×; l'utilisateur confirme dans Logic les extrêmes, la neutralité à 64 et save/reload. Refaire sur le candidat de distribution identifié par SHA.
- La fluidité de lecture en boucle est confirmée à l'usage par l'utilisateur. Aucune mesure de latence MIDI-image, framerate ou coût CPU/GPU n'est disponible.

## Média, timeline et session

- Un seul lecteur et une seule vidéo à la fois. `CueTimelineComponent` et `AssignedPadsComponent` représentent les mêmes quatre états du `ProjectState`; les pads et la timeline synchronisent leur sélection sans déclencher de NoteOn.
- Cue notes : MIDI 36/38/40/41, présentées C1/D1/E1/F1 dans le Logic testé. L'appellation d'octave varie entre hôtes.
- Le média reste externe; le projet conserve son chemin, pas les octets vidéo. Les anciens états sans préférence Play/Pause démarrent en lecture à l'ouverture de l'éditeur; les autres nouveaux champs ont des valeurs par défaut documentées par les tests.
- L'état média absent est récupérable depuis l'éditeur. L'utilisateur confirme le relink après déplacement avec préservation des Cue timings et du Loop dans Logic; refaire sur l'artefact candidat de distribution.
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
2. Le seul parcours actuellement qualifié dans un hôte reste l'AU macOS dans Logic. Le pipeline compile le VST3 Windows en CI (run `45435924725`); cela ne valide pas son chargement/transport MIDI dans REAPER, QA reportée après stabilisation des fonctions macOS.
3. Le projet hôte conserve les paramètres/session; le fichier vidéo demeure externe.
4. Un média absent ou illisible doit rester récupérable sans crash; valider relink/remplacement dans la version candidate de distribution.
5. Ne pas modifier l'identité ni l'état du POC audio.
6. Ne pas annoncer synchronisation sample/frame-accurate, latence garantie ou coût de rendu sans mesure dédiée.
