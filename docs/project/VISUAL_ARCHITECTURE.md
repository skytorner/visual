# AV Sampler — Architecture minimale

Cadrage : 2026-11-23. L'ancien plugin audio est conservé; AV Sampler est une cible instrument JUCE distincte.

## Stack et identité

- JUCE 8.0.8, C++17 et CMake restent la stack.
- `AudioReactiveImagePOC` garde ses identifiants, paramètres, formats et état historiques.
- AV Sampler utilise une nouvelle identité et commence comme instrument AU macOS. La qualification VST3/Standalone et des autres hôtes est ultérieure.
- Interface JUCE native; pas de WebView, React, TouchDesigner ou nouveau runtime.

## Architecture Phase 1

```text
Logic Pro (piano roll / MIDI)
  -> AVSamplerAudioProcessor::processBlock (instrument, sortie audio silencieuse)
  -> mapping NoteOn: MIDI 36, 38, 40, 41 -> Cue 1..4
  -> publication bornée sans verrou/allocation vers l'éditeur
  -> timer UI consomme le Cue, seek puis reprend la vidéo
  -> juce::VideoComponent / module juce_video / backend vidéo natif macOS

État JUCE : chemin vidéo, positions Cue, boucle, lecture demandée, Cue sélectionné
```

- `processBlock()` ne décode ni ne charge un média, ne seek pas et n'appelle pas l'UI. Les actions vidéo sont exécutées sur le thread message.
- NoteOff n'arrête pas le lecteur. Notes non assignées ignorées. Le Cue valide le plus récent est prioritaire.
- Une seule vidéo, un lecteur et une tête de lecture. `CueTimelineComponent` est la vue éditable des quatre Cue existants; elle n'introduit pas d'état de Cue parallèle.
- Lecture continue, boucle et restauration passent par le composant JUCE. Le média reste externe; chemin absent récupérable.
- La documentation JUCE expose chargement, lecture et seek sur macOS, mais ne garantit ni latence de seek, ni frame présentation, ni rendu fiable dans chaque vue AU. Mesurer en Logic avant de figer le backend.
- Granularité effective = blocs MIDI du DAW + ordonnanceur UI + comportement du décodeur/codec; aucune promesse frame-perfect ou sample-accurate.
- La Phase 1 utilise une mailbox atomique latest-cue : si plusieurs cues arrivent avant le tick UI suivant, seul le plus récent est appliqué. Le séquencement rapide et les pertes éventuelles sont à évaluer en Logic; un tampon fixe sera un ticket séparé si cette limite gêne le scénario réel.
- VISUAL-010 ajoute `CueTimelineComponent` comme représentation/interactions de timeline; il réutilise l'état des quatre Cue Points existants via `setCuePosition`, sans état de Cue parallèle ni commande de seek lors d'un drag. Les flèches ajustent le Cue sélectionné par pas nominal de 1/30 s (Maj = 10 pas), car `juce::VideoComponent` ne fournit pas de frame rate du média dans l'API utilisée.
- Les cartes gardent les marqueurs A–D sur la timeline et affichent les noms de note attendus dans Logic pour les notes fixes actuelles: C1/D1/E1/F1. Aucun pad/mapping supplémentaire n'est introduit.
- VISUAL-011 compose l'éditeur en en-tête session/média, commandes de transport, aperçu vidéo et timeline/Cues. L'état reste un seul `ProjectState` dans le preset/projet du DAW, sans fichier de session parallèle.
- Les nouvelles propriétés `playbackRequested` et `selectedCueIndex` sont rétrocompatibles: absence de propriété lecture dans un ancien état signifie autoplay et sélection Cue 1. Un chemin média absent reste mémorisé; le relink change seulement le chemin et préserve les Cue timings absolus, Loop et préférence de lecture.
- VISUAL-012 garde les responsabilités d'éditeur dans JUCE natif. `CueTimelineComponent` ne dessine plus les cartes de notes; `AssignedPadsComponent` lit le même `ProjectState` et sélectionne le même index sans envoyer de NoteOn ni seek. `FxControlBar` est une vue sans callback ou propriété moteur. `SamplerIconButton` ne commande que Play/Pause, Loop et Eject. Le composant vidéo JUCE centre la taille native dans son rectangle avec `RectanglePlacement::centred`, sans étirement.
- Eject ferme le `VideoComponent` et vide seulement `videoPath`. Cue positions, Loop, playback demandé et Cue sélectionné sont inchangés; remplacer ou relier un média garde le comportement de VISUAL-011.
- VISUAL-013 ajoute CC1 comme contrôle fixe de Speed. `processBlock()` met à jour un atomique (aucun accès UI ou vidéo); le timer éditeur applique la vitesse via `VideoComponent::setPlaySpeed()` sur le thread message et l'état CC est inclus dans `ProjectState`. La vitesse est également posée après chargement asynchrone du média.
- Inspection VISUAL-014 : AV Sampler utilise `juce::VideoComponent(false)`. Sur macOS, JUCE encapsule le lecteur dans un `NSViewComponent` et affiche par `AVPlayerLayer`; l'API publique de `VideoComponent` n'expose que transport, durée/taille, position, vitesse et volume. Le module JUCE épinglé ne fournit pas à AV Sampler un flux de CVPixelBuffer/texture, et le projet n'a pas de pipeline Metal existant. RGB Split requiert donc un chemin d'images explicite, pas un shader posé sur la vue actuelle.
- VISUAL-014a doit établir l'accès aux frames AVFoundation et leur présentation Metal dans un composant JUCE macOS, en gardant transport et session. VISUAL-014b ajoutera seulement ensuite le shader RGB et son enveloppe de vélocité. Aucun frame processing GPU/CPU n'est ajouté dans `processBlock()`.
- VISUAL-014b conserve `AVPlayerItemVideoOutput`/`CVMetalTextureCache` et applique RGB Split dans le fragment shader Metal. `processBlock()` ne publie que le dernier velocity NoteOn assigné sous forme atomique compacte, sans I/O ni allocation; le timer éditeur déclenche l'enveloppe côté renderer. Le shader échantillonne les trois canaux avec clamp UV, sur une amplitude max de 1,5 % de la largeur. Le booléen d'activation est persisté dans `ProjectState`; intensité et horodatage d'enveloppe sont transitoires. CC1 ne change pas.

## Plugin audio existant

```text
DAW (effet audio) -> processBlock() -> RMS / Peak / Envelope -> atomics
                  -> éditeur JUCE timer -> image / transformations
État APVTS + imagePath -> projet DAW
```

- MIDI explicitement désactivé; l'effet réagit à l'audio, pas aux notes.
- Préserver le passthrough, les IDs, les paramètres et le chargement d'image.
- Les fichiers `PluginProcessor.*` et `PluginEditor.*` restent distincts du nouveau sampler pour limiter le risque de régression.

## Invariants temps réel et persistance

1. Aucun mutex, I/O, décodage vidéo, seek bloquant, allocation dynamique ou appel UI dans `processBlock()`.
2. Routage MIDI par mapping fixe et communication audio -> UI bornée/non bloquante; métriques séparées pour demande et arrivée visible.
3. Instance isolation : lecteur, vidéo et état indépendants.
4. État sauvegardé via les callbacks JUCE; référence vidéo externe, Cue Points, Loop, préférence de lecture et Cue sélectionné restaurés.
5. Erreur de média : fichier absent ou illisible ne fait pas planter le plugin et reste remplaçable.
6. Ne pas modifier l'identité ni l'état du POC audio.

## Évolutions hors Phase 1

Timeline 16 pads, mapping éditable/MIDI Learn, vélocité hors RGB Split et comportements Hold/Toggle/One Shot restent hors scope. Speed utilise le CC1 fixe de VISUAL-013; le RGB Split velocity dépend de la preuve de rendu VISUAL-014a avant son implémentation.
