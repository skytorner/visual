# AV Sampler / visual — QA Baseline

Date du bilan : 2026-11-24.
Sources : tests manuels déclarés dans Logic Pro 13 et REAPER sur macOS; logs pluginval transmis par l'utilisateur. **Ce document transcrit les résultats, il n'exécute pas de tests.** Les matrices audio-reactive et AV Sampler sont indépendantes: aucune preuve du POC audio ne vaut preuve de l'instrument vidéo.

## État AV Sampler sur main — résumé release readiness

- `main` contient VISUAL-009 à VISUAL-014, y compris la timeline/session, CC1, le renderer Metal et RGB Split.
- L'utilisateur confirme dans Logic le déclenchement MIDI, cycle des notes, Loop, save/reload et fonctionnement RGB Split; le dernier retour confirme aussi une lecture fluide en boucle. Résultats manuels rapportés, sans mesure chiffrée de latence, framerate ou CPU/GPU. Le journal pluginval récent ne relie pas le test à un SHA binaire.
- L'utilisateur confirme dans Logic CC1 à 0/64/127 et la restauration de la valeur après save/reload; SHA exact de l'AU utilisé non consigné.
- L'utilisateur confirme le déplacement puis le relink de la vidéo avec conservation des Cue timings et du Loop; refaire sur l'artefact candidat de distribution.
- Un ancien lancement pluginval s'était interrompu avant chargement (code 134). Le nouveau journal utilisateur `log audio react4.txt` rapporte deux validations complètes de l'AU AV Sampler v0.1.0 à strictness 4, `ALL TESTS PASSED`. Le journal identifie le composant par `aumu,AVsm,Vpoc`, mais ne donne ni chemin du bundle ni SHA de l'exécutable; strictness 5 n'est pas rapporté. L'avertissement `Current program is -1` apparaît lors du changement de programme et n'empêche pas le résultat `SUCCESS`.

## Porte Closed Alpha 1 — état actuel

| Élément | Statut | Preuve / limite |
| --- | --- | --- |
| AU macOS et Logic | PASS manuel rapporté | Le parcours actuel charge et fonctionne dans Logic. Autres hôtes non qualifiés. |
| Flash v1 (VISUAL-019) | EN COURS / NOT TESTED | Implémentation Mac sur la branche de travail; build/install et QA Logic restent à faire avant de revendiquer le nouvel effet fonctionnel. |
| Processus build VST3 Windows x64 | PASS CI / EN PLACE | GitHub Actions run `45435924725` compile `AVSampler_VST3` depuis le commit source `7dbc2e3` et publie l'artefact QA `AVSampler-Windows-VST3-7dbc2e3...`, SHA256 archive `4450ceddcef0f67ee92244b248a5df55755b42dab194896b069ae22f86451037`, avec provenance du binaire. Cela prouve le build, pas le fonctionnement dans un hôte. |
| VST3 Windows et REAPER | DEFERRED / NOT TESTED | La QA Windows est planifiée après stabilisation des fonctionnalités macOS. Le renderer prototype utilise JUCE `VideoComponent`; RGB Split y est indisponible. Ne pas annoncer Windows supporté avant qualification réelle dans REAPER. |
| Package installable sans environnement de développement | NOT AVAILABLE | Les scripts de release et le workflow versionnés package `AudioReactiveImagePOC`; `make install-avsampler-au` est une installation locale développeur qui compile sur la machine. |
| Installation/mise à jour/désinstallation par des testeurs | NOT TESTED | Aucun package AV Sampler pour cohorte n'existe encore. |
| Signature notarisation / provenance du package alpha | NOT TESTED | L'installation développeur applique une signature ad hoc; aucune notarisation ni signature de distribution établie. |
| pluginval AU | PASS strictness 4 | Deux essais GUI complets sur `aumu,AVsm,Vpoc`, version 0.1.0. Le rapport ne lie pas le test à un SHA; strictness 5 non exécuté/rapporté. |
| Versions macOS, codecs et matériel minimal | NOT DEFINED | Un média utilisateur a fonctionné; aucune matrice de compatibilité n'est formalisée. |
| Relink après déplacement média | PASS manuel rapporté, 2026-11-24 | L'utilisateur confirme que les Cue timings et Loop sont conservés après déplacement/relink dans Logic. SHA de l'AU testé non consigné; refaire sur le candidat de distribution. |
| CC1=64 et save/reload | PASS manuel rapporté, 2026-11-24 | L'utilisateur confirme la vitesse normale à 64 et la restauration après sauvegarde/réouverture dans Logic. SHA de l'AU testé non consigné; refaire sur le candidat de distribution. |
| pluginval sur le package alpha exact | NOT TESTED | Strictness 4 passe sur l'AU de développement enregistré, mais le log ne donne pas le chemin ou SHA. Refaire et consigner le test sur l'artefact de cohorte. |

**Décision actuelle : NO-GO pour envoyer une Closed Alpha multi-plateforme; GO pour le processus de build Windows de VISUAL-016. La QA REAPER/Windows est volontairement reportée pendant la poursuite du développement macOS.**

## Matrice AudioReactiveImagePOC (plugin audio historique seulement)

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

La ligne pluginval de cette matrice concerne la tentative strictness 5 du 2026-11-23. Le journal ultérieur rapporte deux passes strictness 4 sur l'AU enregistré après VISUAL-014b; voir la section VISUAL-014b. Ce nouveau résultat ne convertit pas l'essai strictness 5 en PASS.

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
| Média déplacé/absent et action Relink | PASS manuel rapporté | L'utilisateur confirme le déplacement/relink dans Logic et la préservation des Cue timings; l'état visuel Missing media n'est pas décrit séparément. Refaire sur l'artefact candidat de distribution. |
| Relink/remplacement et conservation du Loop | PASS manuel rapporté | L'utilisateur confirme que Loop et timings des cues restent en place après relink. La conservation de l'état Pause/Play n'est pas rapportée séparément. |
| Composition UI hôte | NOT TESTED | Header session, barre média/transport, aperçu, timeline et cartes; aucune reprise graphique propriétaire. |

Note VISUAL-011 : l'utilisateur a validé globalement la version avant fusion, sans rapport de chaque scénario séparé; garder les limites de preuve détaillées ci-dessus.

## VISUAL-013 — Speed piloté par MIDI CC

| Vérification | Statut | Portée |
| --- | --- | --- |
| Mapping CC1 0–127, CC non assigné ignoré, limites 0,5×/1×/1,5× | PASS automatisé | `CueRoutingTests`, CTest sur le build VISUAL-013. |
| MIDI CC1 capturé dans `processBlock()` avec cues NoteOn inchangés | PASS automatisé | `AVSamplerCoreTests` confirme l'état CC; les tests de Cue continuent à passer. |
| Sauvegarde/restauration CC1 et valeur neutre d'un état historique | PASS automatisé | `AVSamplerCoreTests` valide le round-trip et le défaut 64 (1×). |
| Build macOS AU et CTest | PASS build/test | `AVSampler_AU`, les quatre suites CTest et `git diff --check` passent; warnings du SDK JUCE Metal/AppKit seulement. |
| Modulation du clavier intégré de Logic contrôle la vitesse du Cue en cours | PASS manuel rapporté | L'utilisateur confirme que jouer sur la modulation change bien la vitesse du pad en cours. |
| Extrêmes CC1 (0 et 127) | PASS manuel rapporté | L'utilisateur confirme que les deux extrêmes donnent une vitesse très lente et très rapide. |
| Fermeture du clavier intégré de Logic | OBSERVÉ | La modulation revient à 0 à la fermeture; avec le mapping actuel, cela place Speed à son minimum de 0,5×. |
| Neutralité à 64 et save/reload dans Logic | PASS manuel rapporté, 2026-11-24 | Vitesse normale à 64 et restauration après réouverture confirmées par l'utilisateur; SHA exact non consigné. Aucune latence mesurée. |
| Autres emplacements FX | PASS statique | Level, Color, Glitch restent inactifs dans le code de l'éditeur. |

## VISUAL-014a — Preuve du rendu des frames Metal

| Vérification | Statut | Portée |
| --- | --- | --- |
| Rendu Metal JUCE personnalisé compilable dans l'AU | PASS build | L'AU `AVSampler_AU` et les exécutables C++ compilent sur la branche dédiée. |
| Tests CTest existants | PASS 4/4 | `ctest --test-dir /private/tmp/visual-009-build --output-on-failure -C Release`. |
| pluginval sur l'artefact du spike 014a | NOT RUN à l'époque | Le lancement local de cette phase s'était terminé avec le code 134 avant chargement. Un journal ultérieur valide l'AU actuel après 014b à strictness 4; il ne donne pas le SHA exact du bundle testé. |
| `auval` local | NOT DETECTED | `auval -v aumu AVsm Vpoc` échoue à trouver l'AU dans cet environnement; le test hôte Logic ci-dessous confirme toutefois la détection et l'ouverture. |
| AU détecté après rescan et relance de Logic | PASS manuel rapporté | L'utilisateur confirme que le nouveau build fonctionne dans Logic. `auval` local ne retrouve pas le composant, malgré cette validation hôte. |
| Image rendue, lecture et fonctionnement général | PASS manuel rapporté | L'utilisateur rapporte que tout fonctionne; aucune capture ni test instrumenté. |
| Cue Points, Loop vidéo et CC1 | PASS manuel rapporté | Inclus dans le retour « tout fonctionne » après la checklist proposée; aucune latence chiffrée. |
| Remplacement/rechargement de média | NOT REPORTED | Le retour global ne détaille pas ce scénario individuellement. |
| Cadence et coût CPU/GPU | NOT MEASURED | Fonctionnement jugé bon à l'usage par l'utilisateur; aucun relevé chiffré. |

## VISUAL-014b — RGB Split velocity

| Vérification | Statut | Portée |
| --- | --- | --- |
| Build Release AU AV Sampler | PASS | Cible `AVSampler_AU` compilée après implémentation. |
| Installation du build récent pour Logic | PASS installation | `make install-avsampler-au BUILD_DIR=/private/tmp/visual-009-build BUILD_TYPE=Release`; bundle signé ad hoc et présent dans le dossier utilisateur AU. |
| CTest | PASS 5/5 | CueRoutingTests, TimelineMathTests, VideoEffectMathTests, AudioReactiveImageProcessorTests et AVSamplerCoreTests. |
| Mapping vélocité 0/1/20/64/100/127 et limites | PASS automatisé | `VideoEffectMathTests` vérifie courbe, clamps, intensité max et bornes. |
| Attack/Hold/Release et retour neutre | PASS automatisé | 5/40/180 ms avec courbe smoothstep; durées et points d'enveloppe testés par `VideoEffectMathTests`. |
| NoteOn assigné publie la vélocité; NoteOff, vélocité 0 et notes non assignées ignorés | PASS automatisé | `AVSamplerCoreTests`; publication atomique sans traitement graphique dans `processBlock()`. |
| Activation RGB Split sauvegardée; intensité momentanée non sauvegardée; ancienne session | PASS automatisé + statique | Tests pour l'état actif/inactif et défaut des sessions sans clé; le compteur d'événement et l'enveloppe sont hors sérialisation. |
| Shader Metal RGB, déplacement/absence de bord noir, intensité liée à la vélocité | PASS manuel rapporté | L'utilisateur confirme que tout fonctionne dans Logic. |
| Frappe douce vs forte, retrigger, changement de Cue, retour progressif | PASS manuel rapporté | La checklist 014b a été confirmée globalement; niveaux et retours fonctionnent. |
| CC1 simultané, Loop, sauvegarde/réouverture | PASS manuel rapporté | L'utilisateur confirme que tout fonctionne correctement en boucle; fluidité confirmée. |
| Fluidité perçue | PASS manuel rapporté | L'utilisateur rapporte une lecture fluide en boucle; aucune mesure CPU/GPU instrumentée. |
| pluginval AU AV Sampler | PASS strictness 4, 2 essais | `log audio react4.txt`: `AudioUnit:Synths/aumu,AVsm,Vpoc`, AVSampler v0.1.0, `ALL TESTS PASSED` deux fois. Processing/state/editor/automation testés à 44.1/48/96 kHz, buffers 64–1024 et sub-block automation 32; zéro entrée, sortie stéréo. Warning non bloquant `Current program is -1`. Aucun chemin de bundle ou SHA d'exécutable dans le rapport; strictness 5 non rapporté. |

## VISUAL-019 — Flash velocity

| Vérification | Statut | Portée |
| --- | --- | --- |
| Mapping vélocité et enveloppe fixe 5/40/180 ms | PASS automatisé | CTest `VideoEffectMathTests`: intensité faible/forte, maximum 65 %, décroissance et retour au neutre. |
| Publication atomique NoteOn et exclusion NoteOff / vélocité 0 / note non assignée | PASS tests + inspection | `AVSamplerCoreTests` confirme la vélocité du cue; le traitement reste dans les atomiques existants, sans appel renderer dans `processBlock()`. |
| Persistance du toggle et compatibilité session historique | PASS automatisé | `AVSamplerCoreTests`: activation restaurée et propriété absente par défaut désactivée. |
| Build et installation AU macOS | PASS build/install | Build Release local; CTest 5/5; AU installé dans `~/Library/Audio/Plug-Ins/Components/AVSampler.component`; signature vérifiée. Exécutable installé après signature ad hoc SHA-256 `7f95d72f9054668f0d035d0fd90f6c107683831dd806d59d679c2e962afaba13`. |
| Validation utilisateur dans Logic | PASS manuel rapporté | Après installation du build identifié ci-dessus, l'utilisateur confirme que Flash fonctionne parfaitement et que tous les essais sont concluants. |
| Disponibilité VST3 Windows | Non disponible | L'UI doit signaler que Flash n'est pas porté au renderer Windows. |

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
- [x] pluginval strictness 4 rapporté PASS par l'utilisateur pour AudioReactive VST3 et AU et AVSampler AU (`log audio react3.txt`); strictness 5 non exécuté/documenté. L'AU mentionne le warning existant `Current program is -1`; le journal ne donne pas le SHA source du build.
- [x] Test du `processBlock()` complet en mono et stéréo : chaque échantillon de sortie est identique bit à bit à l'entrée, RMS et peak attendus; CTest JUCE 4/4.
- [x] Tests d'intégration des paramètres APVTS et du `imagePath` après sauvegarde-restauration; CTest JUCE 4/4.

Validation propriétaire : l'utilisateur confirme globalement « on validé » après communication des emplacements de build et fournit ensuite un journal pluginval. Scénarios DAW précis non énumérés; strictness 5 et correspondance SHA du rapport restent non vérifiées.

La formule de lissage **par bloc** n'est pas changée intentionnellement ici : sa dépendance à la taille du buffer reste traitée par VISUAL-005.
