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
- [ ] Pluginval sur les **nouveaux** artefacts AU et VST3.
- [x] Insertion dans Logic Pro 13 : utilisateur confirme les trois choix proposés par l'hôte, stéréo, Dual Mono et mono. La lecture/réactivité exacte de chaque mode n'a pas été explicitement détaillée; le test REAPER de ce nouveau build reste à confirmer.
- [ ] Sauvegarder / rouvrir un projet existant sans modifier son état ni perdre les images.
- [ ] Vérifier explicitement que les versions/bundle des binaires testés correspondent au build.

## QA technique continue

- [ ] Tests unitaires DSP reproductibles (RMS/peak/silence/paramètres).
- [ ] Mesure comparée de l'enveloppe en temps réel selon sample rate / taille du buffer.
- [ ] Vérifier sécurité du FileChooser asynchrone en fermant l'éditeur.
- [ ] Vérifier signature/notarisation et installation sans Xcode pour une bêta distribuée.

## Politique de preuve

`PASS` = observation manuelle utilisateur ou sortie d'outil explicitement mentionnée. Une vérification non lancée reste `NOT TESTED`. Les résultats POC de novembre sont la référence; chaque nouveau build modifié doit être revérifié. Ne jamais transformer un avertissement pluginval ou une étape ignorée en test PASS.

## VISUAL-003 — Branche corrective : résultats intermédiaires

La condition de layouts a été modifiée pour annoncer mono->mono et stereo->stereo, sans modification de `processBlock()` ni identifiants plugin/paramètres. La compilation locale AU/VST3/Standalone a été rapportée réussie, avec warnings JUCE/SDK non bloquants. L'utilisateur confirme désormais que Logic Pro 13 **propose/accepte Stéréo, Dual Mono et Mono**. Le « Dual Mono » est un mode de traitement/instanciation de l'hôte Logic, **pas** un troisième layout JUCE ajouté au plugin : le code n'accepte explicitement que mono/mono et stéréo/stéréo.

Restent non vérifiés sur le nouvel artefact : SHA exact de la branche compilée, pluginval strictness 5 nouveau build, lecture/réactivité audio détaillée sur chaque mode, sauvegarde/restauration et régression REAPER. L'ancien log pluginval PASS se rapporte uniquement au POC antérieur. Statut du ticket : `Needs verification`.
