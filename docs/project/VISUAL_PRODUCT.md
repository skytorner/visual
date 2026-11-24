# AV Sampler — Vision produit et périmètre

Date de cadrage : 2026-11-23. Dépôt : `skytorner/visual`.

## Vision

**AV Sampler — Play video like an instrument.** AV Sampler est un instrument visuel natif JUCE pour les musiciens et beatmakers travaillant dans un DAW. Les notes MIDI déclenchent des Cue Points d'une vidéo locale; le DAW enregistre et rejoue la performance. Le premier produit ne génère pas de son.

La direction évolue volontairement du POC audio vers un sampler vidéo MIDI natif. Le plugin audio existant est conservé comme cible distincte pendant la transition; ses identifiants, paramètres et projets ne changent pas.

## Phase 1 — Native MIDI Video POC (priorité actuelle)

Parcours : instrument AU dans Logic Pro -> vidéo locale -> lecture continue et boucle -> quatre Cue Points configurables -> notes MIDI du piano roll qui déclenchent les Cue Points.

Critère de réussite : une région de quatre notes jouée dans Logic déclenche successivement les quatre moments configurés de la vidéo.

Une seule vidéo et une seule tête de lecture. Ne pas revendiquer une précision frame-perfect ni une faible latence de seek sans mesures sur des codecs et hôtes représentatifs.

## POC audio conservé

Le plugin `AudioReactiveImagePOC` conserve l'analyse RMS/peak/envelope, le passthrough, le rendu d'image et ses contrôles. Son identité et son comportement restent séparés d'AV Sampler; il n'est pas la cible produit prioritaire.

## Phase 2 — Video Sampler UX

VISUAL-010 fournit la timeline interactive pour déplacer les quatre Cue Points; VISUAL-011 conserve l'état de session et permet de relier un média; VISUAL-012 organise l'instrument en barre globale, aperçu, timeline, emplacements FX désactivés et quatre pads assignés. Les 16 pads, le mapping éditable, MIDI Learn et les effets actifs restent des étapes ultérieures.

## Phase 3 — MIDI Effects

VISUAL-013 active Speed via MIDI CC1 (0,5×–1,5×; 64 = 1×). VISUAL-014a remplace l'affichage vidéo par le rendu Metal; la lecture, les Cue Points, le Loop et CC1 ont été confirmés dans Logic. VISUAL-014b ajoute RGB Split contrôlé par vélocité, avec enveloppe transitoire et activation persistée; l'utilisateur confirme que l'effet fonctionne et reste fluide en boucle. Les autres effets, modes de pad, 16 pads et MIDI Learn restent ultérieurs.

## Phase 4 — Validation produit

Tester sur un beat réel la vidéo, la région MIDI, les Cue Points, un effet déclenché par note et un effet piloté par contrôleur. Mesurer la réactivité avant d'annoncer des propriétés temporelles.

## Hors périmètre initial

Polyphonie vidéo, plusieurs lecteurs, moteur 3D, export vidéo, séquenceur MIDI interne, synchronisation frame-perfect, marketplace et gestion de plugins tiers.

## Utilisateur et distribution

Utilisateur initial : beatmaker/producteur sur macOS, notamment Logic Pro. Premier succès : programmer quatre notes dans une région MIDI et rejouer la performance visuelle correspondante.

Distribution ciblée : POC macOS/AU -> validation Logic Pro -> VST3 et autres DAW qualifiés ensuite -> éventuel élargissement multiplateforme.

## Média

Le plugin sauvegarde la référence (chemin) de la vidéo, pas ses pixels. Si le fichier est absent, afficher un état récupérable et permettre de relier le fichier déplacé ou sélectionner un remplacement sans modifier les Cue timings. Le projet hôte conserve aussi l'état Loop, l'état Play/Pause demandé et le Cue sélectionné; les anciens états sans préférence de lecture démarrent la vidéo à l'ouverture de l'éditeur. Ne jamais promettre la portabilité du média entre machines.

## Source de vérité

Décisions durables : `VISUAL_DECISIONS.md`; tickets : `VISUAL_BACKLOG.md`; preuves de tests : `VISUAL_QA_CHECKLIST.md`.
