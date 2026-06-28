# Visual Decisions

Ce fichier enregistre les decisions stables du projet.

## Initial Workflow Decisions

- Le projet utilise un workflow inspire de `real-mvp`.
- Le prefixe de tickets est `VISUAL-`.
- `.codex/` contient la memoire, les outils, les skills locaux et les exports.
- `.codex/` n'est pas versionne dans Git pour ce projet.
- `docs/project/` contient la couche de pilotage humain: backlog, release log, decisions, QA et workflow.
- `.codex/shareout/` est ignore par Git et sert uniquement aux exports temporaires pour ChatGPT.
- Aucun secret ne doit etre committe ou exporte.

## Git Versioning Policy

Versionner:

- Sources C++ du plugin dans `Source/`.
- Configuration build dans `CMakeLists.txt`.
- Documentation utilisateur et release: `README.md`, `RELEASE_NOTES.md`, `docs/project/`.
- Scripts reproductibles comme `scripts/package-macos-release.sh`.
- `.gitignore` et consignes agent.

Ne pas versionner:

- `.codex/` et tous ses sous-dossiers.
- `build/` et artefacts CMake/JUCE generes.
- `dist/` et ZIPs de release generes.
- Secrets, `.env`, fichiers locaux d'IDE, logs et caches.

Les fichiers partageables par telechargement sont produits localement dans `dist/`, puis attaches a une release Git ou envoyes separement. Ils ne sont pas committes.

## Product Decisions

- Produit V1: `AudioReactiveImagePOC`.
- Stack V1: JUCE/C++ avec CMake.
- Formats V1: VST3 prioritaire, AU macOS et Standalone macOS si le build local les produit.
- Distribution V1: ZIP macOS simple contenant VST3, AU, Standalone, README, notes de release et instructions d'installation.
