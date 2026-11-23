# Visual Release Log

## VISUAL-013 — Premier effet vidéo piloté par MIDI CC

Date : 2026-11-23. Branche : `codex/VISUAL-013-midi-speed-cc`.

### Portée

- Speed est le premier slot FX actif. MIDI CC1 est mappé sur 0,5×–1,5×, valeur 64 = 1×.
- Le processeur publie la valeur CC via un atomique; l'éditeur applique `VideoComponent::setPlaySpeed()` sur le thread message, y compris après chargement asynchrone.
- La valeur est incluse dans l'état du projet; les sessions anciennes sans propriété démarrent à 1×. Les autres slots restent désactivés.

### Vérifications

| Date | Commande / QA | Résultat | Notes |
| --- | --- | --- | --- |
| 2026-11-23 | `cmake --build /private/tmp/visual-009-build --config Release --target AVSampler_AU CueRoutingTests AVSamplerCoreTests TimelineMathTests AudioReactiveImageProcessorTests -j 4` | PASS | Cibles AU et tests compilées; avertissements non bloquants du SDK macOS dans les modules JUCE Metal/AppKit. |
| 2026-11-23 | `ctest --test-dir /private/tmp/visual-009-build --output-on-failure -C Release` | PASS, 4/4 | CueRoutingTests, TimelineMathTests, AudioReactiveImageProcessorTests et AVSamplerCoreTests. |
| 2026-11-23 | `git diff --check` | PASS | Aucun problème whitespace détecté. |
| — | Logic Pro : CC1, états extrêmes et sauvegarde/réouverture | NOT TESTED | Validation humaine nécessaire après build; pas d'installation automatique. |

pluginval sur le nouvel artefact, installation et test hôte Logic ne sont pas exécutés dans cette tâche. L'artefact AU est généré dans `/private/tmp/visual-009-build/AVSampler_artefacts/Release/AU/AVSampler.component`; il n'est pas installé.

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

## VISUAL-009 — Native MIDI Video POC (2026-11-23)

- Direction produit confirmée explicitement par l'utilisateur : instrument MIDI vidéo JUCE natif sans génération audio. AudioReactiveImagePOC reste conservé dans une cible distincte.
- Audit : le dépôt ne contenait pas de lecteur vidéo ni de routage MIDI. JUCE 8.0.8 inclut `juce_video`/`VideoComponent` pour chargement, lecture et seek macOS; la documentation ne garantit pas la latence ni la vue AU, qui restent à mesurer.
- Branche dédiée : `codex/VISUAL-009-native-midi-video-poc`, créée depuis `main` GitHub vérifié `741f4b6252cc4514c5e5ffcc38a92d7d41875527`.
- Phase 1 seulement : une vidéo, 4 cues, notes fixes, boucle, état JUCE et tests MIDI pur. Résultats build/tests et QA Logic à compléter; les anciens résultats audio ne s'appliquent pas.

### Vérifications locales VISUAL-009

- CMake configure en Release avec tests activés et `FETCHCONTENT_SOURCE_DIR_JUCE` pointant vers le cache local JUCE 8.0.8 : PASS.
- `cmake --build /private/tmp/visual-009-build --config Release --target AVSampler_AU` : PASS; bundle AU `AVSampler.component` généré. Quatre avertissements compilateur viennent de JUCE/SDK Apple (API Metal dépréciées et switch d'événement); aucun avertissement restant dans le code AV Sampler après renommage du paramètre masquant.
- `cmake --build /private/tmp/visual-009-build --config Release --target AudioReactiveImagePOC_All` : PASS; AU, VST3 et Standalone audio construits depuis cette branche.
- `ctest --test-dir /private/tmp/visual-009-build --output-on-failure` : PASS, 2/2. Couvre mapping MIDI, NoteOff, notes non assignées, priorité du dernier cue, sortie silencieuse et restauration du chemin/Cues/loop.
- Inspection de `AVSampler.component/Contents/Info.plist` avec `plutil` : type AU `aumu`, subtype `AVsm`, manufacturer `Vpoc`, version AU encodée 256 (0.1.0); génération CMake indique `JucePlugin_WantsMidiInput=1`.
- Tentative pluginval 1.0.4 : **non exécutée sur le plugin**; le processus s'aborte dans `AppKit` lors de `NSApplication`/`_RegisterApplication`, confirmé par le rapport utilisateur (SIGABRT). Aucun résultat fonctionnel pluginval.
- `git diff --check` final : PASS. QA Logic reste NOT RUN; aucune installation/écriture dans les dossiers Audio Units n'a été faite.

### Suivi Loop vidéo — 2026-11-23

- Retour manuel Logic de l'utilisateur : la vidéo locale se lit; boucler la région dans Logic ne produit pas de latence musicale perceptible. Le bouton Loop d'AVSampler, lui, ramène l'image au début puis laisse la vidéo arrêtée.
- Cause identifiée dans JUCE 8.0.8 macOS : la notification de fin arrête AVPlayer et déclenche déjà un seek vers 0; le timer AVSampler attendait encore une position de fin et appelait play trop tôt pour le seek asynchrone.
- Correctif : détecter le passage lecture→arrêt à proximité de la fin, attendre que la position soit revenue à 0 puis démarrer la lecture; la relance est répétée jusqu'à confirmation de reprise. Un arrêt manuel dans l'éditeur n'est pas traité comme une fin de média.
- `cmake --build /private/tmp/visual-009-build --config Release --target AVSampler_AU` : PASS. Tests automatisés non relancés; retest fonctionnel Logic du nouveau build NOT RUN.
- AU 0.1.0 corrigé installé dans `~/Library/Audio/Plug-Ins/Components/AVSampler.component`; ancienne copie conservée dans `/private/tmp/AVSampler.component.pre-loopfix`. Logic doit rescanner/redémarrer pour charger le binaire mis à jour.

### Suivi QA supplémentaire — 2026-11-23

- L'utilisateur rapporte que le Loop s'arrête toujours à la fin avec l'AU mis à jour. La tentative de correction n'est donc **pas validée**; statut FAIL rapporté. Conformément à sa demande, l'investigation Loop est différée.
- CTest sur le build courant : PASS, 2/2 (`CueRoutingTests`, `AVSamplerCoreTests`). Ces tests couvrent le routage pur et l'état, pas le backend vidéo macOS ni l'hôte Logic.
- Inspection statique en Piano Roll de la session Logic déjà ouverte : la région sélectionnée sur une piste AVSampler contient C1, D1, E1 et F1. La lecture live et l'arrivée visible des quatre cues n'ont pas été observées; aucune modification de région/projet n'a été faite.
- Suite de QA hors Loop : vérifier la réception NoteOn/Cues en lecture, puis sauvegarde/réouverture de projet et relink d'un média absent. La boucle musicale Logic reste rapportée sans latence audible.

### Confirmation QA Logic — 2026-11-23

- L'utilisateur confirme que les quatre cues MIDI se déclenchent en lecture, que le cycle Logic rejoue les notes correctement et que la sauvegarde/réouverture du projet fonctionne.
- La boucle musicale de Logic reste sans latence perceptible selon l'utilisateur. Ceci ne valide pas le bouton Loop vidéo d'AVSampler, toujours en échec et différé.
- Ces résultats sont des confirmations manuelles de l'utilisateur; aucune mesure instrumentée de latence ou capture de lecture n'a été faite.

### Correction QA Loop et reload — 2026-11-23

- Correction utilisateur : Loop vidéo et sauvegarde/réouverture sont tous deux bons dans le scénario actuellement utilisé. Cette confirmation supersède les anciens signalements d'échec Loop et le statut d'investigation différée; aucun problème de reload n'est à suivre.
- Le statut PASS reste une confirmation manuelle rapportée. Les limites séparées (pluginval bloqué avant chargement, latence non mesurée, lecture éditeur fermé non testée) restent inchangées.

## VISUAL-010 — Timeline interactive et Cue Points déplaçables

- Corrections QA VISUAL-009 enregistrées dans le commit `acc79a4`; la branche VISUAL-009 reste séparée de `main`.
- Branche de travail dédiée: `codex/VISUAL-010-interactive-cue-timeline`, créée au-dessus de VISUAL-009 pour conserver l'instrument vidéo qui n'est pas encore intégré à `main`.
- Implémentation locale: `CueTimelineComponent` avec règle en secondes, durée, playhead indépendant, quatre marqueurs déplaçables, cartes C1/D1/E1/F1, sélection et nudging clavier nominal. Aucun média ni moteur parallèle n'est introduit.
- `cmake --build /private/tmp/visual-009-build --config Release --target AVSampler_AU TimelineMathTests AVSamplerCoreTests CueRoutingTests -j 4`: PASS après correction d'une erreur de compile initiale; la build finale a terminé sans avertissement dans les fichiers AVSampler.
- `ctest --test-dir /private/tmp/visual-009-build --output-on-failure`: PASS, 3/3 (`CueRoutingTests`, `TimelineMathTests`, `AVSamplerCoreTests`).
- `git diff --check`: PASS sur l'état final inspecté après les dernières corrections de layout.
- Installation utilisateur: bundle copié dans `~/Library/Audio/Plug-Ins/Components/AVSampler.component`; l'ancienne version est sauvegardée dans `/private/tmp/AVSampler.component.pre-visual010`. Le SHA-256 de l'exécutable installé correspond à l'artefact: `1111e39b3ce57f2c1c5c3ab3952570a63f80b07ca27656783256a6e7c6cb4155`.
- `auval -v aumu AVsm Vpoc`: NOT FOUND après la copie; le registre Audio Unit ne voit pas encore le composant. Cette tentative a eu lieu après le premier rescan/redémarrage rapporté par l'utilisateur, mais avant un nouveau scan Logic post-install.
- QA visuelle/manuelle dans Logic après installation non exécutée. Restent drag/playhead, raccourcis clavier dans l'hôte, lecture/trigger après déplacement, petites/grandes tailles, remplacement/absence média et contrôle des logs.
- Limite: l'API JUCE `VideoComponent` utilisée expose la durée mais pas la fréquence d'images de la source; le nudging est à 30 fps nominales et les miniatures vidéo sont différées.

### Clarification état du bouton Loop — 2026-11-23

- L'utilisateur annule le signalement de régression: la lecture Loop vidéo fonctionne. La confusion vient du style de bouton, qui donne l'impression d'être activé quand le paramètre est désactivé.
- Le correctif candidat du moteur Loop/repaint a été annulé. La build timeline précédente (SHA `1111e39b3ce57f2c1c5c3ab3952570a63f80b07ca27656783256a6e7c6cb4155`) a été restaurée; le candidat Loop (SHA `644e63eca2ccd0d2539316aff5869521c7d6ecc99ec0246f7e3d17aa2c7199fd`) est conservé à `/private/tmp/AVSampler.component.post-loopstate-candidate`.
- Correction UI: labels `Loop ON`/`Loop OFF` et couleurs explicites; aucune modification moteur Loop conservée.
- Build/install: `cmake --build /private/tmp/visual-009-build --config Release --target AVSampler_AU TimelineMathTests AVSamplerCoreTests CueRoutingTests -j 4`: PASS; CTest PASS 3/3; `git diff --check`: PASS. L'AU à SHA `c15b3f0c95f4a6f1338992b404d91f52f4977694fab5b567412155717100cd66` est installé dans `~/Library/Audio/Plug-Ins/Components/AVSampler.component`; la version précédente est sauvegardée dans `/private/tmp/AVSampler.component.pre-loop-button-state`.
- Affichage de la nouvelle distinction ON/OFF dans Logic: QA manuelle à confirmer après redémarrage/scan post-install.

### Confirmation QA utilisateur VISUAL-010 — 2026-11-23

- Après l'installation et le redémarrage de Logic, l'utilisateur confirme que tout le parcours vérifié fonctionne: timeline et noms de notes, Loop ON/OFF, déplacement des quatre Cue Points, lecture continue, déclenchement MIDI aux nouvelles positions et sauvegarde/réouverture.
- Cela supersède le statut antérieur « QA visuelle après installation non exécutée » pour ce parcours. Les tests clavier, tailles d'éditeur, absence/remplacement de média et les logs restent à vérifier.
- CTest reste PASS 3/3 et build AU PASS (SHA `c15b3f0c95f4a6f1338992b404d91f52f4977694fab5b567412155717100cd66`). La vérification `auval` antérieure n'avait pas trouvé le composant; Logic confirme maintenant le chargement réel.

## VISUAL-011 — Session et gestion du média

- Branche dédiée `codex/VISUAL-011-session-media-management`, empilée sur VISUAL-010.
- Composition principale: titre AV SAMPLER et mention de session hôte, barre Load/Relink/Replace Media + Play/Pause/Loop + média courant/durée, aperçu vidéo, timeline/Cues existants; palette conservée.
- L'état JUCE inclut désormais `playbackRequested` et `selectedCueIndex`; les états antérieurs à ces propriétés se restaurent en autoplay/Cue 1. Loop, position des quatre cues et chemin média conservent le même schéma.
- Un chemin absent affiche le fichier manquant et le bouton Relink Media. Choisir le fichier met à jour seulement la référence; Cue timings, Loop et préférence lecture ne sont pas réinitialisés.
- `cmake --build /private/tmp/visual-009-build --config Release --target AVSampler_AU TimelineMathTests AVSamplerCoreTests CueRoutingTests -j 4`: PASS; `ctest --test-dir /private/tmp/visual-009-build --output-on-failure`: PASS, 3/3; `git diff --check`: PASS.
- Build finale installée dans `~/Library/Audio/Plug-Ins/Components/AVSampler.component`; SHA-256 `8b39418aa3877e8750ca0ea34a0269911b36633d131624ef6e72b8b939ddc4ef`. La build intermédiaire VISUAL-011 est sauvegardée dans `/private/tmp/AVSampler.component.pre-visual011-r2`; VISUAL-010 reste dans `/private/tmp/AVSampler.component.pre-visual011-session`.
- QA visuelle Logic de l'ouverture/autoplay, pause restaurée, bouton Relink, timings après remplacement et nouvelle composition: NOT TESTED après cette installation.

## VISUAL-012 — Composition UI et pads assignés

- Cadrage créé depuis les références fournies : cinq zones dans cet ordre, design AV Sampler conservé; aucun effet actif, 16 pads ni MIDI Learn.
- Branche dédiée `codex/VISUAL-012-serato-inspired-ui` depuis `main` à `f76d6f2`.
- JUCE natif : barre Play/Pause/Loop/média/durée/Eject, VideoComponent existant, timeline sans cartes, `FxControlBar` inertiel et `AssignedPadsComponent` à quatre pads liés au même index Cue.
- Eject ferme le média et retire sa référence de session sans modifier Cue timings, Loop, Cue sélectionné ou lecture demandée. Relink/remplacement reste le chemin VISUAL-011.
- Validation exécutée : `cmake --build /private/tmp/visual-009-build --config Release --target AVSampler_AU TimelineMathTests AVSamplerCoreTests CueRoutingTests -j 4` PASS; CTest 3/3; `git diff --check` PASS. SHA-256 build AU: `6588d64c5b99ef69b73be1c1a8621ccae5a1dca851f28a9773780342b3eee351`.
- Installation locale effectuée dans `~/Library/Audio/Plug-Ins/Components/AVSampler.component`; le SHA installé correspond exactement à celui du build. L'ancienne version est sauvegardée dans `/private/tmp/AVSampler.component.pre-visual012-ui`.
- Après installation, l'utilisateur confirme : « tout fonctionne bien, on a la maj visu ». Logic charge donc la mise à jour visuelle dans son parcours manuel global; les vérifications individuelles ne sont pas détaillées.
- Les captures avant/après de taille comparable ne sont pas archivées. L'accès CUA à Logic avait expiré (`-10005 timeoutReached`); aucune capture de maquette n'est substituée à une capture réelle.

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

## VISUAL-004 — Tests de l'analyse audio (2026-11-21)

Status: Done — PR #3 fusionnée dans `main` par le commit `d83663e`; branche distante supprimée. Les validations pluginval fournies sont strictness 4; strictness 5 et le SHA source du journal ne sont pas documentés.

- Extraction minimale de `analyseAudio` (RMS/peak multi-canal) et `nextEnvelope` vers un en-tête C++17 sans dépendance JUCE, utilisé par `processBlock`.
- Le calcul suit les mêmes équations; aucune modification intentionnelle des buffers, paramètres JUCE, versions et identifiants du plugin.
- Test autonome `cmake -S tests -B build-tests -DCMAKE_BUILD_TYPE=Release` : PASS sur environnement Linux local.
- `cmake --build build-tests --config Release` : PASS.
- `ctest --test-dir build-tests --output-on-failure` : PASS (1/1).
- `g++ -std=c++17 -Wall -Wextra -Werror -pedantic -fsyntax-only tests/AudioSignalAnalysisTests.cpp` : PASS.
- CMake du plugin / AU/VST3 macOS : NOT RUN sur ce changement; pluginval nouveau build : NOT RUN.
- CI courte GitHub Actions ajoutée : statut encore inconnu à la création du commit.
- Limitations : l'analyse pure read-only ne prouve pas le passthrough end-to-end de l'effet; état APVTS et imagePath restent à tester via intégration JUCE. Le lissage temporel reste l'objet de VISUAL-005.

### VISUAL-004 — Raccourcis développeur (2026-11-21)

- Ajout d'un `Makefile` versionné pour `make` (test + build), `make test`, `make plugin`, `make status`, `make install[-au|-vst3]`, `make package` et `make help`.
- Installation DAW explicitement séparée du build, uniquement sur macOS dans `~/Library/Audio/Plug-Ins/`; elle ne s'exécute jamais via `make` seul.
- Contrôle de la syntaxe et des commandes Make sur Linux local : `make help` PASS, `make -n` PASS, `make -n install` PASS; `make install-au CMAKE=true` rejette correctement Linux. Les actions réelles macOS d'installation / de build restent NOT RUN.

### VISUAL-004 — Validation d'intégration macOS (2026-11-23)

- La PR #3 est actualisée avec le `main` courant (VISUAL-009 à 012); le merge Git local est sans conflit. La PR reste ouverte.
- Tests autonomes macOS : `cmake -S tests -B /private/tmp/visual-004-tests -DCMAKE_BUILD_TYPE=Release`, build, puis CTest : PASS, 1/1.
- Build JUCE Release : `AudioReactiveImagePOC_AU`, `AudioReactiveImagePOC_VST3`, `AudioReactiveImagePOC_Standalone`, `AVSampler_AU`, `AudioReactiveImageProcessorTests`, `TimelineMathTests`, `AVSamplerCoreTests`, `CueRoutingTests` : PASS.
- CTest JUCE : PASS, 4/4. Le nouveau `AudioReactiveImageProcessorTests` vérifie l'égalité bit-à-bit entrée/sortie de `processBlock()` en mono et stéréo, les valeurs RMS/peak, ainsi que la restauration des six paramètres APVTS et de `imagePath`.
- Les seuls warnings de compilation observés viennent de JUCE / SDK Apple (Metal déprécié et énumération AppKit); aucun warning du test ajouté après correction de sa comparaison.
- L'utilisateur fournit `log audio react3.txt` : pluginval strictness 4 **ALL TESTS PASSED** pour AudioReactive VST3 (`AudioReactiveImagePOC.vst3`) et AU (`aufx,Arip,Vpoc`), ainsi que AVSampler AU (`aumu,AVsm,Vpoc`). Warning AU existant `Current program is -1`; strictness 5 non exécuté/documenté.
- Le log pointe le VST3 vers `/private/tmp/visual-009-build/AudioReactiveImagePOC_artefacts/Release/VST3/AudioReactiveImagePOC.vst3`, mais ne consigne pas le SHA source; la correspondance avec le commit testé n'est donc pas indépendante.
- Run GitHub Actions `Audio analysis tests` sur le head `c422d56f7470ab50c78caf26b411f4e9743d6f6d` : PASS, run `45278115656`.
- L'utilisateur confirme globalement : « on validé ». Les scénarios DAW précis ne sont pas détaillés. Un précédent lancement local du binaire pluginval avait échoué avant chargement (`--version`, code 134); le journal utilisateur fournit ensuite un run réussi.
