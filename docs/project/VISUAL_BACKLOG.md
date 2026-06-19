# Visual Backlog

Status terms:

- `Done`: implemente et verifie.
- `Needs verification`: existe probablement mais demande une preuve.
- `Open`: pas encore traite.
- `Blocked`: attend une decision ou un element externe.

## Active Tickets

### VISUAL-001 - Definir la vision produit et le stack cible

- Status: Open
- Owner type: Human + Codex
- Priority: P0
- Type: Product / Architecture
- Blocks work: Yes
- Goal: Decider ce que ce depot doit construire et avec quel stack.
- Tasks:
  - Decrire le produit cible.
  - Identifier les utilisateurs et workflows principaux.
  - Choisir le stack initial.
  - Definir les non-objectifs.
  - Creer les tickets d'implementation.
- Acceptance criteria:
  - `docs/project/VISUAL_DECISIONS.md` contient les decisions de base.
  - `.codex/project-memory.md` contient la memoire durable.
  - `VISUAL-002+` sont actionnables.

### VISUAL-002 - Installer les skills projet definitifs

- Status: Open
- Owner type: Codex
- Priority: P1
- Type: Workflow
- Blocks work: No
- Goal: Adapter les skills locaux au stack retenu.
- Tasks:
  - Lister les skills requis.
  - Ecrire ou adapter les `SKILL.md`.
  - Supprimer les skills inutiles.
  - Verifier que `AGENTS.md` et les skills sont coherents.
- Acceptance criteria:
  - `.codex/skills/` reflete le projet reel.
  - Les agents savent quels fichiers lire avant de travailler.

