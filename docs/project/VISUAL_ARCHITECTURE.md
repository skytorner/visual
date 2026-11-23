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

État JUCE : chemin vidéo, positions Cue, boucle
```

- `processBlock()` ne décode ni ne charge un média, ne seek pas et n'appelle pas l'UI. Les actions vidéo sont exécutées sur le thread message.
- NoteOff n'arrête pas le lecteur. Notes non assignées ignorées. Le Cue valide le plus récent est prioritaire.
- Une seule vidéo, un lecteur et une tête de lecture. Les Cue peuvent être réglés depuis la position courante; pas encore de timeline éditable.
- Lecture continue, boucle et restauration passent par le composant JUCE. Le média reste externe; chemin absent récupérable.
- La documentation JUCE expose chargement, lecture et seek sur macOS, mais ne garantit ni latence de seek, ni frame présentation, ni rendu fiable dans chaque vue AU. Mesurer en Logic avant de figer le backend.
- Granularité effective = blocs MIDI du DAW + ordonnanceur UI + comportement du décodeur/codec; aucune promesse frame-perfect ou sample-accurate.
- La Phase 1 utilise une mailbox atomique latest-cue : si plusieurs cues arrivent avant le tick UI suivant, seul le plus récent est appliqué. Le séquencement rapide et les pertes éventuelles sont à évaluer en Logic; un tampon fixe sera un ticket séparé si cette limite gêne le scénario réel.
- VISUAL-010 ajoute `CueTimelineComponent` comme représentation/interactions de timeline; il réutilise l'état des quatre Cue Points existants via `setCuePosition`, sans état de Cue parallèle ni commande de seek lors d'un drag. Les flèches ajustent le Cue sélectionné par pas nominal de 1/30 s (Maj = 10 pas), car `juce::VideoComponent` ne fournit pas de frame rate du média dans l'API utilisée.
- Les cartes gardent les marqueurs A–D sur la timeline et affichent les noms de note attendus dans Logic pour les notes fixes actuelles: C1/D1/E1/F1. Aucun pad/mapping supplémentaire n'est introduit.

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
4. État sauvegardé via les callbacks JUCE; référence vidéo externe, Cue Points et loop restaurés.
5. Erreur de média : fichier absent ou illisible ne fait pas planter le plugin et reste remplaçable.
6. Ne pas modifier l'identité ni l'état du POC audio.

## Évolutions hors Phase 1

Timeline 16 pads, mapping éditable/MIDI Learn, CC, vélocité et comportements Hold/Toggle/One Shot seront ajoutés après validation du routage MIDI -> vidéo dans Logic. Ne pas introduire d'architecture générique avant un besoin concret.
