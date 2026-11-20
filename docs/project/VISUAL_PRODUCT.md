# AV Sampler — Vision produit et périmètre

Date de cadrage : 2026-11-20. Dépôt de travail : `skytorner/visual`.

## Vision

AV Sampler est un instrument visuel pour les musiciens et beatmakers travaillant dans un DAW. Il transforme des événements musicaux en réactions graphiques. Le but est d'obtenir un résultat visuel dès la première minute, sans quitter l'environnement musical.

L'approche produit est **progressive** et repose sur le POC JUCE/C++ déjà fonctionnel.

### Phase A — Audio Reactive MVP (priorité actuelle)

Parcours : piste audio/instrument produisant du son -> plugin d'effet audio -> analyse RMS/peak/envelope -> animation d'une image PNG/JPG -> sauvegarde/réouverture dans le projet DAW.

Fonctions existantes à préserver :
- Analyse audio RMS, peak et enveloppe; audio passthrough.
- Image unique chargée localement, transformations scale/rotation/opacity et six contrôles existants.
- Sauvegarde des paramètres et du chemin d'image; message explicite lorsque l'image n'est plus accessible.
- Plusieurs instances indépendantes dans un projet; AU/VST3/Standalone générés par CMake.

Périmètre MVP :
- Stabilisation de l'entrée mono et stéréo sans altérer le signal.
- Tests reproductibles du traitement DSP, paramètres et cycle de vie du plugin.
- Vérification de la cohérence de l'enveloppe selon taille de buffer et sample rate; correction ciblée si le test démontre un problème.
- Packaging/documentation suffisamment robustes pour une bêta macOS limitée.
- Conserver le rendu natif JUCE tant qu'il est adéquat.

Non-objectifs de ce MVP :
- Réception MIDI native, mapping note/pad, 4/16 pads, MIDI Learn.
- Export vidéo synchronisé, FFT/bandes, onset/beat detection, scènes, marketplace, VJ stage, Web/cloud.
- TouchDesigner comme runtime, WebView React/TypeScript, moteur GPU/shaders et nouveau backend.
- Compatibilité Windows ou avec tous les DAW revendiquée sans preuve.

### Phase B — MIDI et pads (après stabilisation)

Cible : Note On/Off, velocity, mapping de notes vers 4 pads, images distinctes et réglages indépendants. Le mode Audio Reactive reste disponible. Valider d'abord un vertical slice MIDI DAW -> événement -> rendu, sans déclarer que le POC actuel traite déjà le MIDI.

### Phases ultérieures (à requalifier selon usages)

16 pads, scènes/presets riches, export vidéo, FFT, animations avancées, éventuellement Web/desktop. TouchDesigner est acceptable comme laboratoire créatif externe, non comme dépendance obligatoire du plugin.

## Utilisateur et proposition de valeur

Utilisateur initial : beatmaker/producteur sur macOS, notamment Logic Pro. Premier succès : charger une image sur une piste, écouter et constater un rendu qui suit le son sans changer la musique.

Distribution ciblée : POC local -> MVP stable -> bêta macOS sur quelques DAW effectivement validés -> éventuel élargissement Windows/cross-platform.

## Définition de fini de l'Audio Reactive MVP

- Insertion sur pistes mono et stéréo des hôtes ciblés, sans modification intentionnelle du son.
- Les paramètres et le comportement visuel se restaurent dans la limite des règles de média documentées.
- Les problèmes connus d'image manquante donnent un état propre et récupérable.
- Tests automatiques DSP et validation AU/VST3 de base passent; QA manuelle pertinente documentée.
- Aucun crash, lock ou opération de fichiers dans le thread audio; avertissements et limitations connus explicités.
- Les validations sont faites sur la version exacte distribuée, sans confondre POC historique et nouveau build.

## Média : décision temporaire

Le POC enregistre le **chemin**, pas les pixels de l'image. Une image déplacée ou supprimée produit un message et peut être remplacée. C'est acceptable provisoirement pour une bêta limitée avec limitation explicite; la portabilité des projets entre machines et l'embarquement/recopie des images sont différés, et ne doivent pas être présentés comme résolus.

## Source de vérité

Les décisions durables sont dans `VISUAL_DECISIONS.md`, les tickets dans `VISUAL_BACKLOG.md`, les preuves et limites des tests dans `VISUAL_QA_CHECKLIST.md`. Les contenus locaux `.codex/` peuvent aider Codex mais ne font pas autorité pour un clone neuf.
