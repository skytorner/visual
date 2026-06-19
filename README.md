# visual

Depot prepare pour un workflow Codex / ChatGPT avec memoire projet, pilotage par tickets, skills locaux et exports de contexte.

## Structure de pilotage

- `AGENTS.md`: consignes a lire par les agents.
- `.codex/project-memory.md`: decisions durables, principes et limites du projet.
- `.codex/progress.md`: journal compact de ce qui a ete fait.
- `.codex/next-actions.md`: backlog actif et prochaines actions.
- `.codex/archive-next-actions.md`: archive des tickets termines.
- `.codex/skills/`: skills locaux propres au projet.
- `.codex/tools/export-chatgpt-context.sh`: export local du contexte utile pour ChatGPT.
- `docs/project/`: couche de pilotage projet, release, decisions et QA.

## Export ChatGPT

Exporter le contexte leger:

```bash
bash .codex/tools/export-chatgpt-context.sh
```

Exporter un ticket precis:

```bash
bash .codex/tools/export-chatgpt-context.sh --ticket VISUAL-001
```

Exporter le contexte complet de pilotage:

```bash
bash .codex/tools/export-chatgpt-context.sh --full
```

Le resultat est genere dans `.codex/shareout/latest/`. Ne jamais y ajouter de secrets.

