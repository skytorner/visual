# Agent Instructions — visual / AV Sampler

Le dépôt `skytorner/visual` conserve le POC audio JUCE/C++17 et développe AV Sampler comme instrument visuel MIDI natif. AV Sampler est le produit prioritaire; son AU macOS a été validé manuellement dans Logic pour le parcours actuel. Le dépôt prépare maintenant une première cohorte fermée; aucun package AV Sampler distribuable n'est encore établi. Préserver le plugin audio et ses identifiants existants.

## Lire avant de coder (sources versionnées)

1. `docs/project/VISUAL_PRODUCT.md`
2. `docs/project/VISUAL_DECISIONS.md`
3. `docs/project/VISUAL_ARCHITECTURE.md`
4. `docs/project/VISUAL_BACKLOG.md`
5. `docs/project/VISUAL_QA_CHECKLIST.md`
6. `docs/project/VISUAL_CODEX_CHATGPT_WORKFLOW.md`
7. `docs/project/VISUAL_RELEASE_LOG.md`

Le répertoire `.codex/` est **local et ignoré par Git**. S'il existe, ses fichiers `project-memory.md`, `next-actions.md`, `progress.md` et ses skills peuvent être lus en complément; s'il manque, poursuivre normalement avec les documents ci-dessus. Aucun ticket ne doit dépendre de son existence.

## Règles techniques

- Une tâche = une branche = vérifications adaptées = PR = docs de suivi.
- Préférer les corrections ciblées à une réécriture et appliquer SOLID/DRY/KISS pragmatiquement.
- Conserver JUCE natif/C++/CMake et les identifiants plugin/paramètres pour le MVP sauf décision explicite.
- `processBlock()` : pas de mutex, I/O, chargement de média, allocation dynamique ou opération UI; ne pas changer le son.
- Tester la régression des projets DAW, des paramètres, de la stéréo et du chargement d'image.
- Ne pas prétendre que `AudioReactiveImagePOC` reçoit du MIDI; seul `AVSampler` est configuré pour le MIDI. Les validations hôte rapportées concernent AV Sampler dans Logic; elles ne qualifient pas les autres DAW ni VST3.
- N'annoncer PASS qu'avec une preuve et distinguer manuels, outils et non exécutés.

## Hygiène / livraison

- Pas de secrets, `.env`, dumps, captures de credentials, ZIP ou artefacts de build dans Git.
- Fin de tâche : résumé, branche/PR, fichiers modifiés, commandes réellement exécutées, résultats, risques et tests humains restant à faire.
- Mettre à jour `docs/project/VISUAL_BACKLOG.md` et les décisions/QA/release log concernées.
