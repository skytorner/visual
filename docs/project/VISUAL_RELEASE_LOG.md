# Visual Release Log

## Initial Setup

Status: Planning.

Date opened: 2026-06-19.

### Goal

Preparer le dossier `visual` pour un pilotage Codex / ChatGPT reproductible.

### Scope

- Memoire projet locale.
- Backlog actif.
- Archive des tickets termines.
- Journal d'avancement.
- Documents de pilotage.
- Skills locaux.
- Export ChatGPT.

### Verification Results

| Date | Command / QA | Result | Notes |
| --- | --- | --- | --- |
| 2026-06-19 | `bash -n .codex/tools/export-chatgpt-context.sh` | Pass | Syntaxe Bash valide. |
| 2026-06-19 | `bash .codex/tools/export-chatgpt-context.sh` | Pass | Export leger cree dans `.codex/shareout/latest/`. |
| 2026-06-19 | `bash .codex/tools/export-chatgpt-context.sh --ticket VISUAL-000` | Pass | Export ticket avec `CODEX_TICKET_REPORT.md`. |
| 2026-06-19 | `bash .codex/tools/export-chatgpt-context.sh --full` | Pass | Export complet avec memoire, docs projet et skills locaux. |
| 2026-06-19 | `git check-ignore -v .codex/shareout/latest/HANDOFF_SUMMARY.md` | Pass | `.codex/shareout/` est ignore par Git. |
| 2026-06-19 | `git diff --check` | Pass | Aucun probleme de whitespace detecte. |
| 2026-06-19 | `git branch --show-current` | Pass | Branche par defaut alignee sur `main`. |

## AudioReactiveImagePOC v1.0.0

Status: Preparing.

Date opened: 2026-06-28.

### Goal

Preparer une premiere version partageable par telechargement simple.

### Scope

- Version produit `1.0.0`.
- Sources JUCE/CMake versionnees.
- Artefacts generes exclus de Git.
- ZIP macOS local dans `dist/`.
- ZIP Windows produit sur runner Windows ou machine Windows.
- Notes de release et instructions d'installation incluses.

### Versioning Decision

Versionne dans Git:

- `CMakeLists.txt`
- `Source/`
- `README.md`
- `RELEASE_NOTES.md`
- `scripts/package-macos-release.sh`
- `scripts/package-windows-release.ps1`
- `.github/workflows/build-release.yml`
- `docs/project/`
- `.gitignore`
- `AGENTS.md`

Non versionne:

- `.codex/`
- `build/`
- `dist/`
- artefacts CMake/JUCE generes
- ZIPs de release generes
- secrets, env locaux, logs et caches IDE

### Verification Results

| Date | Command / QA | Result | Notes |
| --- | --- | --- | --- |
| 2026-06-28 | `bash -n scripts/package-macos-release.sh` | Pass | Script de packaging syntaxiquement valide. |
| 2026-06-28 | `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release` | Pass | Configuration CMake/JUCE generee. |
| 2026-06-28 | `cmake --build build --config Release` | Pass | VST3, AU et Standalone generes. Warnings restants issus de JUCE. |
| 2026-06-28 | `bash scripts/package-macos-release.sh` | Pass | ZIP cree: `dist/AudioReactiveImagePOC-v1.0.0-macos.zip`. |
| 2026-06-28 | `unzip -l dist/AudioReactiveImagePOC-v1.0.0-macos.zip` | Pass | Archive contient VST3, AU, Standalone, README, release notes et install guide. |
| 2026-06-28 | `unzip -l ... | grep '__MACOSX\\|/\\._\\|^.* \\._'` | Pass | Aucun fichier parasite macOS detecte dans le ZIP. |
| 2026-06-28 | `git check-ignore -v build/... dist/... .codex/...` | Pass | `build/`, `dist/` et `.codex/` sont ignores. |
| 2026-06-28 | `git diff --check` | Pass | Aucun probleme de whitespace detecte. |
| 2026-06-28 | `which x86_64-w64-mingw32-g++` | Info | Aucune toolchain Windows locale sur ce Mac; le binaire Windows doit etre produit sur Windows. |
| 2026-06-28 | `bash -n scripts/package-macos-release.sh` | Pass | Packaging macOS valide localement. |
| 2026-06-28 | GitHub Actions `build-release.yml` | Prepared | Workflow ajoute pour produire `AudioReactiveImagePOC-v1.0.0-windows.zip` sur `windows-latest`. |
