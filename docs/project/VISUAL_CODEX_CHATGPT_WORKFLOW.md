# Visual Codex / ChatGPT Workflow

Ce document definit la boucle legere entre Codex, ChatGPT et le proprietaire humain du projet `visual`.

## Objective

Faire avancer le projet ticket par ticket avec un etat facile a relire et a partager. Codex implemente et met a jour le depot. ChatGPT peut aider a prioriser, relire un export de contexte, preparer le prochain prompt ou organiser la QA. Le proprietaire humain garde les decisions produit, UX, infrastructure, credentials et release.

## Roles

### Codex

- Traite un ticket a la fois.
- Lit la memoire et les documents de pilotage avant de changer le depot.
- Met a jour le backlog, le release log, la memoire et le journal quand necessaire.
- Execute les verifications pertinentes et rapporte les commandes.
- Genere un export ChatGPT quand une decision ou un handoff est utile.

### ChatGPT

- Lit `.codex/shareout/latest/` quand le proprietaire humain l'upload.
- Aide a choisir le prochain ticket.
- Aide a preparer prompts, QA et questions de decision.
- Ne remplace pas la verification locale ni le jugement produit.

### Human Owner

- Decide la direction produit et les compromis.
- Realise la QA manuelle si necessaire.
- Gere les secrets, comptes, domaines, hebergement et credentials.
- Upload l'export ChatGPT seulement apres verification qu'il ne contient pas de secrets.

## Core Rule

Un ticket = une branche = une verification = une mise a jour documentaire.

## Recommended Work Order

1. Lire `docs/project/VISUAL_BACKLOG.md`.
2. Choisir le prochain ticket P0/P1.
3. Implementer uniquement ce ticket.
4. Verifier.
5. Mettre a jour les documents.
6. Exporter le contexte si ChatGPT doit aider au pilotage.

## Export Commands

```bash
bash .codex/tools/export-chatgpt-context.sh
bash .codex/tools/export-chatgpt-context.sh --ticket VISUAL-001
bash .codex/tools/export-chatgpt-context.sh --full
```

## Guardrails

- Ne pas uploader de secrets.
- Ne pas utiliser `.codex/shareout/` comme documentation permanente.
- Garder les exports legers par defaut.
- Ne pas melanger plusieurs tickets sans decision explicite.

