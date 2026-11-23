# AV Sampler / visual — QA Baseline

Date du bilan : 2026-11-20.
Sources : tests manuels déclarés dans Logic Pro 13 et REAPER sur macOS; log utilisateur `log audio react.txt` de pluginval. **Ce document transcrit les résultats, il n'exécute pas de tests.**

## Matrice POC existant

| Test | Statut | Portée / preuve |
| --- | --- | --- |
| Logic Pro 13 : réactivité audio piste drums et note isolée | PASS manuel | Notes jouées dans l'instrument -> audio -> image animée; **pas** de preuve de MIDI natif. |
| Plusieurs instances dans Logic | PASS manuel | Instances indépendantes sur plusieurs pistes. |
| Fermeture / réouverture du projet | PASS manuel | Paramètres et image restaurés tant que chemin d'image accessible. |
| Buffers Logic 32/64/128/256/512/1024 | PASS manuel | Fonctionnement perçu avec plusieurs instances; identité temporelle de l'enveloppe non quantifiée. |
| Sample rates 44.1/48/96 kHz | PASS rapporté / pluginval | Tests pluginval également exécutés à ces taux; mesure perceptive équivalente non prouvée. |
| Import image, image absente et remplacement | PASS manuel | Message d'erreur + possibilité de choisir un autre média; pas de transport automatique entre machines. |
| Fermeture/réouverture de l'éditeur en lecture | PASS manuel | Pas de crash rapporté. |
| Bypass et audio sans coupure audible | PASS manuel | Test d'écoute; null-test bit-perfect encore non effectué. |
| Stress léger | PASS manuel | 4 pistes / beat de 2 minutes; pas une endurance longue. |
| REAPER | PASS manuel | Mêmes scénarios audio fonctionnels rapportés par l'utilisateur; format précis REAPER non consigné. |
| Piste mono | FAIL confirmé | Plugin impossible à insérer; `isBusesLayoutSupported` stereo/stereo seulement. |
| Boucle offline bounce | DEFERRED | Plugin génère visuel UI, pas fichier vidéo; audio bounce à qualifier ultérieurement. |
| Windows | NOT TESTED | Build CI prévu historiquement, pas de validation DAW utilisateur. |
| MIDI Note On/Off | OUT OF SCOPE | MIDI explicitement désactivé dans le POC. |

## Pluginval : résultat du log utilisateur

| Artefact | Strictness | Résultat | Observations |
| --- | --- | --- | --- |
| AudioUnit `aufx,Arip,Vpoc` | 5 | PASS | `ALL TESTS PASSED`; `auval` exit code 0; version affichée `0.1.0`; avertissement `Current program is -1`. |
| VST3 `AudioReactiveImagePOC.vst3` | 5 | PASS | `ALL TESTS PASSED`; version affichée `1.0.0`; `vst3 validator` SKIPPED car chemin non défini. |

Les deux tests pluginval ont parcouru audio processing, automation et state à 44.1/48/96 kHz et buffers 64/128/256/512/1024, avec sub-block 32 pour automation. `pluginval` ne prouve pas la compatibilité avec tous les hôtes. Les deux artefacts peuvent ne pas provenir du même build.

## QA nécessaire pour VISUAL-003 (support mono)

- [x] Build de la branche modifiée (AU et VST3 au minimum) : compilation macOS rapportée réussie, mais SHA local non affiché dans le log.
- [ ] Contrôle automatisé des layouts mono->mono, stereo->stereo; autres rejets documentés.
- [ ] Traitement audio inchangé (transparence) sur mono et stéréo.
- [x] Pluginval sur les **nouveaux** artefacts AU et VST3 : strictness 5, tous les tests pluginval PASS (v1.0.0 des deux côtés). L'exécutable Steinberg VST3 validator externe reste non configuré.
- [x] Insertion dans Logic Pro 13 : utilisateur confirme les trois choix proposés par l'hôte, stéréo, Dual Mono et mono. La lecture/réactivité exacte de chaque mode n'a pas été explicitement détaillée; le test REAPER de ce nouveau build reste à confirmer.
- [ ] Sauvegarder / rouvrir un projet existant sans modifier son état ni perdre les images.
- [ ] Vérifier explicitement le SHA du checkout source local compilé. Les artefacts contrôlés sont ceux de `build/`, versions AU/VST3 1.0.0; le log ne prouve pas le commit SHA.

## QA technique continue

- [ ] Tests unitaires DSP reproductibles (RMS/peak/silence/paramètres).
- [ ] Mesure comparée de l'enveloppe en temps réel selon sample rate / taille du buffer.
- [ ] Vérifier sécurité du FileChooser asynchrone en fermant l'éditeur.
- [ ] Vérifier signature/notarisation et installation sans Xcode pour une bêta distribuée.

## Politique de preuve

`PASS` = observation manuelle utilisateur ou sortie d'outil explicitement mentionnée. Une vérification non lancée reste `NOT TESTED`. Les résultats POC de novembre sont la référence; chaque nouveau build modifié doit être revérifié. Ne jamais transformer un avertissement pluginval ou une étape ignorée en test PASS.

## VISUAL-009 — Native MIDI Video POC

La nouvelle direction vidéo/MIDI est autorisée le 2026-11-23. Les validations ci-dessous concernent la cible AV Sampler, sans réutiliser les PASS du plugin audio historique.

| Vérification | Statut | Portée |
| --- | --- | --- |
| Tests CTest note→Cue, NoteOff/non-assignée, priorité, sortie silencieuse et sauvegarde/restauration | PASS | `CueRoutingTests` et `AVSamplerCoreTests`, CTest 2/2 sur le code de cette branche. |
| Build macOS AU `AVSampler_AU` | PASS | Cible instrument AU compilée; bundle `AVSampler.component` généré. |
| Plugin audio historique AU/VST3/Standalone | PASS build | `AudioReactiveImagePOC_All` compile depuis cette branche; pas de pluginval/regression DAW sur le nouveau build. |
| Métadonnées AU instrument/MIDI | PASS statique | `Info.plist` indique le type `aumu`; la config JUCE inclut `JucePlugin_WantsMidiInput=1`. Ce n'est pas un test Logic. |
| Chargement/lecture vidéo et cycle musical de Logic | PASS manuel rapporté | L'utilisateur confirme que sa vidéo légère se lit, que les cues MIDI fonctionnent et que le cycle Logic rejoue les notes correctement sans latence musicale perceptible. |
| Loop vidéo intégré | PASS manuel rapporté | L'utilisateur confirme que le Loop vidéo fonctionne dans le build utilisé actuellement. Les anciens échecs concernaient des essais antérieurs et sont supersédés. |
| Logic Pro: instrument, piano roll et déclenchement des cues | PASS manuel rapporté | L'utilisateur confirme les quatre cues en lecture et que la boucle de région Logic rejoue correctement leurs notes. Ce retour n'est pas une mesure de latence MIDI image. |
| Sauvegarde/réouverture du projet et média | PASS manuel rapporté + PASS unitaire | L'utilisateur confirme que tout fonctionne après sauvegarde et reload dans Logic; le test C++ de sérialisation/restauration passe également. |
| pluginval strictness 5 | BLOCKED BEFORE PLUGIN LOAD | pluginval 1.0.4 s'aborte à l'initialisation `NSApplication`/AppKit (`_RegisterApplication`, SIGABRT); aucun test plugin ne démarre. Rapport reçu le 2026-11-23. |
| Latence MIDI → nouvelle image | NOT MEASURED | Aucune précision temporelle revendiquée. |
| Lecture avec éditeur fermé | NOT TESTED | Dépendance éventuelle du rendu au composant UI à qualifier. |

## VISUAL-010 — Timeline interactive

| Vérification | Statut | Portée |
| --- | --- | --- |
| Conversion temps ↔ position et bornes vidéo | PASS automatisé | `TimelineMathTests`: round-trip temps/pixel et clamp aux bornes. |
| Nom des notes MIDI affiché sur les cartes | PASS automatisé | `TimelineMathTests`: MIDI 36/38/40/41 -> C1/D1/E1/F1 selon la notation Logic observée. |
| Build et copie AU macOS après timeline | PASS build/install | `AVSampler_AU` compilé puis copié dans `~/Library/Audio/Plug-Ins/Components/AVSampler.component`; SHA du binaire installé identique à l'artefact. |
| Hôte AU post-install | PASS Logic / `auval` NOT FOUND | Logic charge l'AU et l'utilisateur confirme le parcours visuel; la commande `auval -v aumu AVsm Vpoc` n'a pas trouvé le composant lors de son invocation antérieure. |
| Régressions de routage MIDI/état | PASS automatisé | `CueRoutingTests` et `AVSamplerCoreTests` exécutés avec la nouvelle build. |
| Loop vidéo avec VISUAL-010 | PASS manuel rapporté | L'utilisateur corrige son signalement: la vidéo boucle; l'apparence du bouton Loop faisait croire à tort qu'il était activé. |
| État visuel du bouton Loop OFF/ON | PASS manuel rapporté | L'utilisateur confirme que le bouton distingue maintenant l'état et que le Loop vidéo fonctionne. |
| Drag des quatre Cue Points et synchronisation d'état | PASS manuel rapporté | L'utilisateur confirme le déplacement des quatre cues, la mise à jour du temps, le déclenchement MIDI aux nouvelles positions et save/reload. |
| Lecture pendant déplacement et playhead indépendant | PASS manuel rapporté | Inclus dans la confirmation utilisateur « tout fonctionne » après les vérifications du parcours Logic. |
| Ajustement clavier une frame / dix frames | NOT TESTED | Pas nominal 1/30 s; la source JUCE ne fournit pas la cadence vidéo. |
| Tailles d'éditeur étroite et large | NOT TESTED | Vérifier limites 560–1400 px et lisibilité des cartes. |
| Aucun média / média remplacé / cues aux bornes et proches | NOT TESTED | Smoke test visuel et absence d'erreur/crash. |

## VISUAL-011 — Session et gestion du média

| Vérification | Statut | Portée |
| --- | --- | --- |
| Persistance Play/Pause/Loop et Cue sélectionné | PASS automatisé | `AVSamplerCoreTests`: round-trip de Loop, Pause, sélection et Cue timings. |
| Ancien état sans préférence de lecture | PASS automatisé / UI NOT TESTED | Restauration par défaut `playbackRequested=true`, Cue 1 sélectionné; autoplay réel au premier ouverture à confirmer dans Logic. |
| Restauration d'une session en Pause | PASS automatisé / UI NOT TESTED | L'état Pause est restauré par sérialisation; vérifier ensuite l'éditeur dans Logic. |
| Changement de chemin sans perte de timings | PASS automatisé | `AVSamplerCoreTests` remplace le chemin média et vérifie que les quatre Cue timings restent identiques. |
| Build et copie AU macOS VISUAL-011 | PASS build/install | Bundle installé dans `~/Library/Audio/Plug-Ins/Components/AVSampler.component`; SHA `8b39418aa3877e8750ca0ea34a0269911b36633d131624ef6e72b8b939ddc4ef` identique à l'artefact. |
| Média déplacé/absent et action Relink | NOT TESTED | Vérifier l'état visuel Missing media et le sélecteur de relink dans Logic. |
| Relink/remplacement en état Pause/Loop | NOT TESTED | Vérifier le chargement, Play/Pause conservé, Loop/sélection et cues au temps existant. |
| Composition UI hôte | NOT TESTED | Header session, barre média/transport, aperçu, timeline et cartes; aucune reprise graphique propriétaire. |

Note VISUAL-011 : l'utilisateur a validé globalement la version avant fusion, sans rapport de chaque scénario séparé; garder les limites de preuve détaillées ci-dessus.

## VISUAL-012 — Composition UI et pads assignés

| Vérification | Statut | Portée |
| --- | --- | --- |
| Build, installation AU et suites CTest | PASS build/install; Logic PASS rapporté | AU installé dans `~/Library/Audio/Plug-Ins/Components/AVSampler.component`; SHA identique au binaire construit (`6588d64c…e351`). CTest 3/3. L'utilisateur confirme que Logic charge la mise à jour et que tout fonctionne. |
| Ordre des cinq zones et composants FX inactifs | PASS statique + confirmation visuelle utilisateur | L'utilisateur confirme voir la mise à jour visuelle dans Logic; aucun détail zone par zone ni capture d'écran archivée. |
| Sélection croisée timeline/pad | PASS code + confirmation globale utilisateur | Les deux composants mettent à jour le même index; le retour « tout fonctionne » n'énumère pas séparément ce contrôle. |
| Notes, Cue IDs et positions des pads | PASS statique | Quatre pads issus des quatre index; libellés A–D, notes du mapping fixe et positions du `ProjectState`. |
| Eject | PASS automatisé de préservation d'état / UI Logic NOT TESTED | Test processeur vérifie qu'effacer le chemin ne modifie pas Cue timings, Loop, sélection ni préférence de lecture; action visuelle à confirmer. |
| Ratio du preview | PASS source JUCE / média réel NOT TESTED | `juce::VideoComponent` centre la taille native dans le rectangle fourni, sans étirement. |
| Nom long, durée et états média dans l'hôte | NOT TESTED | À vérifier avec une vidéo, un nom long, un fichier absent et une erreur de lecture. |
| Resize, captures avant/après même taille | CAPTURES NOT ARCHIVED | L'utilisateur confirme l'UI mise à jour; aucune paire avant/après de taille comparable n'est archivée. |
| Play/Pause, Loop, MIDI, relink, sauvegarde et reload | PASS global rapporté, pas itemisé | L'utilisateur dit que tout fonctionne sur la mise à jour; les essais individuels ne sont pas détaillés. |

## VISUAL-003 — Branche corrective : résultats intermédiaires

La condition de layouts a été modifiée pour annoncer mono->mono et stereo->stereo, sans modification de `processBlock()` ni identifiants plugin/paramètres. La compilation locale AU/VST3/Standalone a été rapportée réussie, avec warnings JUCE/SDK non bloquants. L'utilisateur confirme désormais que Logic Pro 13 **propose/accepte Stéréo, Dual Mono et Mono**. Le « Dual Mono » est un mode de traitement/instanciation de l'hôte Logic, **pas** un troisième layout JUCE ajouté au plugin : le code n'accepte explicitement que mono/mono et stéréo/stéréo.

**Nouveau rapport utilisateur :** `log audio react2.txt`, sur les fichiers sous `build/AudioReactiveImagePOC_artefacts/Release`. AU `aufx,Arip,Vpoc` et VST3 `AudioReactiveImagePOC.vst3` sont tous deux détectés en **v1.0.0** et terminent `ALL TESTS PASSED` à strictness 5. Le traitement audio, l'automation, l'état, l'éditeur et les bus passent sur 44.1/48/96 kHz, avec buffers 64/128/256/512/1024. La liste des layouts contient **Mono et Stereo** pour les deux formats. `auval` retourne le code 0. Avertissement AU `Current program is -1` toujours présent. Validateur Steinberg VST3 externe **skipped**, chemin non défini. Dual Mono reste un choix de Logic, non un bus JUCE distinct.

**Conclusion ciblée :** bug d'insertion mono corrigé dans Logic et compatibilité pluginval du nouvel artefact validée. `VISUAL-003` est clôturable. Points reportés à la QA technique / tickets suivants : SHA source du build local non consigné, contrôle automatisé des layouts, transparence bit-perfect mono/stéréo, vérification lecture/réactivité détaillée selon mode, restauration d'un ancien projet et REAPER sur ce nouveau build. Les lignes de la matrice historique ci-dessus décrivent encore le POC initial et non une régression actuelle.

## VISUAL-004 — Suite de tests audio (branche en validation)

- [x] Extraction minimale de la mesure RMS/peak et de l'enveloppe dans `Source/AudioSignalAnalysis.h`; `processBlock()` utilise la même logique et conserve le passthrough.
- [x] Tests C++17 autonomes sur mono, stéréo, silence, buffers vides, tailles 32/64/128/256/512/1024, enveloppe et paramètres extrêmes.
- [x] Les tests vérifient que `analyseAudio()` ne modifie pas les tableaux sources.
- [x] Tests locaux Linux : configuration CMake, compilation, CTest 1/1 PASS; compilation avec `-Wall -Wextra -Werror -pedantic` sans erreur.
- [x] Vérification GitHub Actions sur le head actualisé de la PR avec `.github/workflows/audio-tests.yml` : run `45275406778`, conclusion success.
- [x] Recompiler AU/VST3/Standalone macOS après ce refactoring; AVSampler_AU compile également.
- [ ] Relancer pluginval strictness 5 sur le build modifié. L'exécutable local s'arrête avec le code 134 sur `--version`, avant de charger un plugin; aucun résultat pluginval.
- [x] Test du `processBlock()` complet en mono et stéréo : chaque échantillon de sortie est identique bit à bit à l'entrée, RMS et peak attendus; CTest JUCE 4/4.
- [x] Tests d'intégration des paramètres APVTS et du `imagePath` après sauvegarde-restauration; CTest JUCE 4/4.

La formule de lissage **par bloc** n'est pas changée intentionnellement ici : sa dépendance à la taille du buffer reste traitée par VISUAL-005.
