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
| 2026-06-28 | GitHub Actions Windows run `28331997364` | Fail | Step `Build` a echoue sur Windows; logs bruts non accessibles localement sans session GitHub. |
| 2026-06-28 | Windows build log from user | Fail root cause found | MSVC refusait la lambda de layout dans `PluginEditor.cpp`: `gap` et `labelHeight` non captures, puis appels `layoutSlider` invalides. |
| 2026-06-28 | `PluginEditor.cpp` layout lambda portability fix | Prepared | Constantes de layout deplacees au scope fichier pour eviter les captures dependantes du compilateur. |
| 2026-06-28 | Workflow Windows target narrowing | Prepared | Le job Windows build maintenant explicitement `AudioReactiveImagePOC_VST3` et liste les artefacts generes avant packaging. |
| 2026-06-28 | Windows packaging path fallback | Prepared | `package-windows-release.ps1` recherche le dossier VST3 genere si le chemin standard JUCE/CMake varie. |
| 2026-06-28 | `cmake --build build --config Release --target AudioReactiveImagePOC_VST3` | Pass | Verification locale macOS de la cible VST3 apres correctif; seul warning restant issu de JUCE. |
| 2026-06-28 | GitHub Release automation | Prepared | Le workflow telecharge les ZIPs macOS/Windows et cree ou met a jour automatiquement la GitHub Release quand un tag `v*` est pousse. |

## QA baseline Audio Reactive — 2026-11-20

Status: POC valide fonctionnellement sur les scénarios audio testés, corrections requises avant MVP.

- Tests manuels utilisateur : Logic Pro 13/macOS, REAPER/macOS, réactivité audio sur drums et note isolée; plusieurs instances et restauration Logic.
- Logic : buffers 32/64/128/256/512/1024 PASS; sample rates 44.1/48/96 kHz rapportés PASS; ouverture/fermeture UI et écoute bypass sans coupure rapportés PASS.
- Stress manuel léger : 4 pistes jouant un beat de 2 minutes PASS (pas une endurance longue).
- Image absente : message et remplacement possible PASS; portabilité cross-machine non vérifiée.
- Pluginval strictness 5 : AU ALL TESTS PASSED, `auval` code 0; VST3 ALL TESTS PASSED.
- Observations pluginval : AU version 0.1.0 vs VST3 version 1.0.0; warning AU `Current program is -1`; validateur externe VST3 SKIPPED faute de chemin.
- Régression confirmée : insertion sur piste mono impossible (seul stereo/stereo déclaré).
- Éléments différés : bounce offline, tests Windows/DAW non essayés, MIDI natif.
- Aucune compilation ou revalidation du code n'a été exécutée lors de ce cadrage documentaire.

## VISUAL-003 — Mono/stereo layout fix (code change, 2026-11-20)

- Changement ciblé : `isBusesLayoutSupported()` accepte mono->mono ou stereo->stereo; rejette tout autre layout.
- Le traitement RMS/peak/envelope, la sortie audio, les identifiants plugin/paramètres, la sauvegarde et l'interface restent inchangés.
- Relecture statique de la modification : condition par layout explicite; pas de nouveau mutex/allocation ou I/O dans `processBlock()`.
- Build CMake JUCE : **NOT RUN** dans cette intervention GitHub distante.
- Pluginval sur le nouveau build : **NOT RUN**.
- Tests Logic Pro 13 et REAPER mono/stéréo sur le nouveau build : **NOT RUN** (QA humaine nécessaire avant merge).
- Les anciennes validations POC ne sont pas transférées automatiquement au nouveau binaire.

### Suivi QA VISUAL-003 — Retour Logic Pro 13 (2026-11-20)

- L'utilisateur rapporte que le plugin peut maintenant être sélectionné/inséré dans Logic en **stéréo, Dual Mono et mono**; échec d'insertion mono d'origine corrigé dans son scénario manuel.
- « Dual Mono » est une modalité de Logic : le code JUCE ne négocie que mono/mono ou stéréo/stéréo; pas de troisième type de bus implémenté.
- Build macOS signalé terminé sans erreur; le SHA du checkout local n'était pas inclus dans le log.
- QA encore ouverte : nouveau passage pluginval AU/VST3 niveau 5, confirmation du commit compilé et régressions sur un ancien projet / REAPER si possible.

### Final validation VISUAL-003 — pluginval (2026-11-21)

- Nouveau journal utilisateur `log audio react2.txt`, fichiers de `build/AudioReactiveImagePOC_artefacts/Release/` (et non ancien `dist/`).
- **VST3 v1.0.0 : PASS, strictness 5, `ALL TESTS PASSED`**, 44.1/48/96 kHz, tailles 64/128/256/512/1024, état/automation/éditeur, layouts Mono et Stereo. Le validateur Steinberg VST3 *externe* a été **skipped** faute de chemin.
- **AU v1.0.0 : PASS, strictness 5, `ALL TESTS PASSED`**, mêmes variations audio, `auval` code 0, layouts Mono et Stereo. Avertissement non bloquant `Current program is -1`.
- Versions AU et VST3 désormais cohérentes à 1.0.0 (ancienne différence de versions résolue pour les binaires testés).
- Logic Pro 13 : utilisateur confirme sélection/insertion sur Mono, Dual Mono, Stereo; pas de preuve séparée de MIDI natif.
- **Décision : clôture de VISUAL-003 pour le correctif fonctionnel mono**, sans confondre pluginval avec un test null audio ou le validateur Steinberg; SHA local de compilation non consigné, QA nouvelle version sur ancien projet / REAPER non exécutée.
- Suite : VISUAL-004 tests automatisés audio/layout/transparence; VISUAL-007 qualification des artefacts et packaging.
