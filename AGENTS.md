# Agent Instructions

Ce depot utilise un workflow de pilotage inspire de `real-mvp`.

Avant tout travail de fond, lire dans cet ordre:

1. `.codex/project-memory.md`
2. `.codex/next-actions.md`
3. `.codex/progress.md`
4. `docs/project/VISUAL_BACKLOG.md`
5. `docs/project/VISUAL_RELEASE_LOG.md`
6. `docs/project/VISUAL_DECISIONS.md`

Regles de travail:

- Une tache = une branche = une verification = une mise a jour des documents de suivi.
- Conserver les decisions durables dans `.codex/project-memory.md` ou `docs/project/VISUAL_DECISIONS.md`.
- Conserver le journal d'avancement dans `.codex/progress.md`.
- Garder `.codex/next-actions.md` centre sur les prochaines actions executables.
- Archiver les tickets termines dans `.codex/archive-next-actions.md`.
- Ne jamais mettre de secrets, `.env`, dumps, cles privees ou captures de credentials dans Git ou `.codex/shareout/`.
- Les outils, recherches, exports ChatGPT, scripts QA et skills locaux restent sous `.codex/` sauf s'ils sont necessaires au runtime du produit.

Fin de tache attendue:

- Resumer la tache traitee.
- Lister les fichiers modifies.
- Donner les commandes de verification executees et leur resultat.
- Indiquer les risques restants et les actions QA humaines.
- Mettre a jour les fichiers de pilotage concernes.

