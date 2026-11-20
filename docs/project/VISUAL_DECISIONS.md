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

- **D-001 — Produit progressif :** Audio Reactive MVP d'abord; MIDI et pads après stabilisation. Un effet audio visuel, pas un sampler audio ou un MIDI effect pour la phase A.
- **D-002 — Réutiliser le POC :** conserver JUCE/C++17/CMake et l'interface native; pas de réécriture, ni WebView, TS, React, GPU, TouchDesigner dans le MVP.
- **D-003 — Cibles :** macOS et Logic Pro 13 d'abord; REAPER est également testé manuellement. AU et VST3 existent; Windows n'est pas certifié.
- **D-004 — Compatibilité DAW :** préserver IDs plugin/paramètres, comportement et état; refactorings minimaux avec non-régression.
- **D-005 — Audio safe :** pas de blocage ni chargement media sur le thread audio; passthrough sans modification intentionnelle; gérer mono/stéréo.
- **D-006 — Validation proportionnée :** tests unitaires DSP + pluginval (niveau 5, AU et VST3), QA manuelle ciblée; bounce offline et endurance lourde différés.
- **D-007 — Médias :** enregistrer les chemins reste acceptable provisoirement pour une bêta limitée; message d'erreur/relink confirmés. Pas de promesse de portabilité multi-machine avant une autre solution.
- **D-008 — MIDI futur :** Note On/Off, velocity, mapping de 4 pads à vérifier en spike DAW avant généralisation 16 pads. La réactivité audio existante n'est pas la preuve de prise en charge MIDI.
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
