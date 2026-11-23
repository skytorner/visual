# Visual Decisions

Décisions durables et état de leur validation. Dernier cadrage : 2026-11-20.

## Organisation du dépôt (conservée)

- Répertoire : `skytorner/visual`, précurseur technique du produit **AV Sampler**.
- Préfixe de tickets : `VISUAL-`; une tâche = une branche = des vérifications = une revue PR = suivi documentaire.
- `docs/project/` est la **source de vérité versionnée** (vision, architecture, décisions, backlog, QA, workflow, historique).
- `.codex/` contient éventuellement les mémoires/skills/outils/exports locaux et **n'est pas versionné**; un clone vierge doit fonctionner sans ce répertoire.
- Secrets, `.env`, logs, captures de credentials, `build/`, `dist/` et archives binaires ne sont pas committés.
- Scripts reproductibles, sources C++, CMake, README, AGENTS.md, release notes et docs projet sont versionnés.

## Product & Architecture Decisions — 2026-11-20

- **D-001 — Historique (supersédé le 2026-11-23 par D-011) :** Audio Reactive MVP d'abord; MIDI et pads après stabilisation. Le POC audio reste conservé mais AV Sampler vidéo MIDI est désormais prioritaire.
- **D-002 — Réutiliser le POC :** conserver JUCE/C++17/CMake et l'interface native; pas de réécriture, ni WebView, TS, React, GPU, TouchDesigner dans le MVP.
- **D-003 — Cibles :** macOS et Logic Pro 13 d'abord; REAPER est également testé manuellement. AU et VST3 existent; Windows n'est pas certifié.
- **D-004 — Compatibilité DAW :** préserver IDs plugin/paramètres, comportement et état; refactorings minimaux avec non-régression.
- **D-005 — Audio safe du POC historique :** pas de blocage ni chargement média sur le thread audio; passthrough sans modification intentionnelle; gérer mono/stéréo.
- **D-006 — Validation proportionnée :** tests unitaires DSP + pluginval (niveau 5, AU et VST3), QA manuelle ciblée; bounce offline et endurance lourde différés.
- **D-007 — Médias :** enregistrer les chemins reste acceptable provisoirement pour une bêta limitée; message d'erreur/relink confirmés. Pas de promesse de portabilité multi-machine avant une autre solution.
- **D-008 — MIDI (mis à jour par D-011/D-014) :** vérifier les événements MIDI dans l'instrument AU cible avant généralisation; les validations audio historiques ne prouvent pas leur réception.
- **D-009 — Qualité :** SOLID/DRY/KISS appliqués pragmatiquement; découper uniquement sous pression d'un besoin/test; dette intentionnelle documentée.
- **D-010 — Déploiement :** packages macOS/Windows compilables selon platform, mais distribution officielle soumise aux tests du build exact, signature/notarisation macOS à qualifier.

## Historique du POC v1.0.0

Le POC `AudioReactiveImagePOC` contient actuellement RMS, peak, envelope, image PNG/JPG, transformations, paramètres et état basé sur chemin. Le JUCE tag actuellement épinglé est `8.0.8`. Formats CMake : VST3, Standalone, AU sur macOS. Il conserve le bundle `com.visual.audioreactiveimagepoc` et les identifiants existants.

## Risques / compromis non résolus

- Mono non supporté : P0.
- Lissage dépendant des blocs potentiellement non constant temporellement : test puis correction si nécessaire.
- Image externe manquante : défaut géré, mais déplacement de projet non portable.
- Pluginval AU annonce `v0.1.0`, VST3 annonce `v1.0.0` : ne pas supposer qu'ils proviennent du même build.
- Dans le log, le validateur VST3 Steinberg n'était pas configuré. `pluginval` a bien réussi mais n'équivaut pas à un passage du validateur externe.
- Signature/notarisation et qualification Windows/DAW non vérifiées; aucune revendication de compatibilité générale.

Voir `VISUAL_PRODUCT.md`, `VISUAL_ARCHITECTURE.md` et `VISUAL_QA_CHECKLIST.md`.

## Changement de direction produit — 2026-11-23

- **D-011 — Priorité vidéo MIDI (autorisation explicite utilisateur) :** AV Sampler devient d'abord un instrument visuel natif inspiré d'un sampler vidéo. Phase 1 : AU macOS, Logic Pro, une vidéo continue, quatre Cue Points et quatre notes du piano roll. UX avancée, effets MIDI et validation produit restent séparés.
- **D-012 — Identités séparées :** conserver `AudioReactiveImagePOC`, ses paramètres et sauvegardes. AV Sampler utilise un nouvel identifiant/plugin/bundle; aucune migration implicite.
- **D-013 — Backend vidéo du POC :** commencer par `juce::VideoComponent`/`juce_video` et le backend natif macOS. Les API documentent chargement, lecture et seek, mais la latence et l'intégration dans la vue AU doivent être mesurées avant de figer le choix.
- **D-014 — Temps réel MIDI :** le callback audio ne fait que mapper NoteOn fixe et publier les cues vers l'UI sans verrou ni allocation. Le thread message exécute les seeks. NoteOff ignoré; Cue valide le plus récent prioritaire.
- **D-015 — État média :** persister chemin, quatre positions et boucle dans l'état JUCE. La vidéo reste un fichier externe, dont l'absence est récupérable.
- **D-016 — Timeline phase 2 :** conserver A–D comme identifiants de Cue sur la timeline, afficher les notes MIDI sur les cartes utilisateur (C1/D1/E1/F1 dans Logic pour 36/38/40/41), et réutiliser le state existant. La précision clavier est nominalement 1/30 s tant que la fréquence d'images source n'est pas disponible.
