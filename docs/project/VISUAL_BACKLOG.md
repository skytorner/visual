# Visual Backlog

Dernière mise à jour : 2026-11-25. Produit prioritaire : AV Sampler, instrument vidéo MIDI natif; POC audio conservé comme cible distincte.

Statuts : `Done` (confirmé), `In progress` (en cours), `Open` (à faire), `Needs verification` (à contrôler), `Deferred` (hors phase). P0 nécessaire au MVP; P1 souhaitable avant bêta; P2 itération ultérieure.

Une tâche = une branche = tests = PR = mise à jour docs. Les tickets ne doivent pas devenir de gros chantiers multidomaines.

## Roadmap et tickets

### VISUAL-001 — Vision produit, architecture et organisation

- Status: Done (documentation et décisions du 2026-11-20).
- Priority: P0.
- Scope: `VISUAL_PRODUCT.md`, `VISUAL_ARCHITECTURE.md`, `VISUAL_DECISIONS.md`, backlog, QA et instructions agent.
- Acceptance: JUCE natif; direction produit révisée vers l'instrument vidéo MIDI (VISUAL-009); POC audio conservé sans régression d'identité/état.

### VISUAL-002 — Skills et outils Codex

- Status: Deferred.
- Priority: P2.
- Goal: uniquement les skills utiles au workflow réel; ne pas bloquer le développement.
- Acceptance: AGENTS.md utilisable sans `.codex/`; aucun outil fictif ou dépendance à un répertoire local absent.

### VISUAL-003 — Support mono et stéréo sans régression

- Status: Done — **correctif mono validé** par compilation macOS rapportée PASS, insertion Logic Pro en Mono / Dual Mono / Stereo et pluginval AU+VST3 strictness 5 (`ALL TESTS PASSED`, v1.0.0). Les contrôles DSP automatisés, la confirmation du SHA local et la régression ancien projet / REAPER du nouveau build restent en suivi, sans bloquer ce correctif ciblé.
- Priority: P0.
- Scope: adapter négociation des bus pour mono->mono et stereo->stereo, sans changement d'identité plugin ni UI.
- Acceptance (correction fonctionnelle):
  - `isBusesLayoutSupported` accepte mono/mono et stereo/stereo; rejette les layouts asymétriques/non souhaités.
  - Passthrough et mesure RMS/peak cohérents pour 1 et 2 canaux; pas de duplication, perte de canal ni allocation dans `processBlock`.
  - Build macOS AU/VST3/Standalone et, si accessible, validation pluginval niveau 5 + tests manuels Logic/REAPER mono/stéréo.
  - Les résultats effectivement exécutés sont explicités; aucun PASS inventé.

### VISUAL-004 — Tests de calculs audio et transparence

- Status: Done — tests/build/CI PASS et validation globale utilisateur. Le journal fourni rapporte pluginval AU/VST3 à strictness 4 PASS; strictness 5 n'est pas documenté et le SHA source n'apparaît pas dans le rapport.
- Priority: P0.
- Scope: tests déterministes RMS/peak, silence, paramètres extrêmes, accès read-only aux buffers audio; intégration JUCE séparée pour la transparence complète de `processBlock()` et la restauration d'état.
- Acceptance: commandes reproductibles + CI courte; build macOS et tests JUCE passent. Pluginval et QA hôte du nouvel artefact restent documentés séparément et ne sont pas revendiqués sans résultat.

### VISUAL-005 — Enveloppe audio temporellement cohérente (POC historique)

- Status: Open.
- Priority: P2 pour la maintenance du POC audio (promouvoir si mesure de bug utilisateur).
- Scope: test mesurant enveloppe à buffers 32–1024 et sample rates 44.1/48/96 kHz; correction de l'attaque/release si nécessaire.
- Acceptance: courbes temporelles cohérentes, dégradation sonore/visuelle évitée.

### VISUAL-006 — Cycle de vie import image (POC historique)

- Status: Open.
- Priority: P2 pour la maintenance du POC audio.
- Scope: sécuriser l'import image asynchrone, état manquant/réassignation, restitution des paramètres et robustesse.
- Acceptance: fermeture éditeur pendant sélection; aucun crash; comportement documenté pour les médias externes. L'embarquement des médias est hors scope.

### VISUAL-007 — Packaging historique AudioReactiveImagePOC

- Status: Open.
- Priority: P2; cible distincte du lancement AV Sampler.
- Scope: vérifier versions AU/VST3 du POC audio, provenance de ses builds, packaging et compatibilité de cette cible historique.
- Acceptance: aucune compatibilité non testée annoncée; package audio testé et limites visibles. Ne pas confondre ce packaging avec celui du sampler.

### VISUAL-008 — Vertical slice MIDI audio-reactive

- Status: Superseded by VISUAL-009; le besoin MIDI porte désormais sur une cible vidéo instrument distincte.
- Priority: P2.
- Scope: preuve réception MIDI DAW -> note/velocity -> contrôle d'une image -> 4 pads. Pas de 16 pads directement.
- Acceptance: ancien objectif de routage MIDI/audio conservé comme historique, non prioritaire.

### VISUAL-009 — Native MIDI Video POC

- Status: Done — le scénario essentiel de lecture/loop, déclenchement des quatre cues dans Logic, cycle de région et sauvegarde/réouverture est confirmé manuellement puis prolongé par VISUAL-010 à VISUAL-014. Latence mesurée et lecture éditeur fermé restent des qualifications séparées.
- Priority: P0.
- Scope: ajouter une cible instrument AU silencieuse; charger une vidéo locale; lecture/pause continue et boucle; quatre Cue Points réglables à la position courante; MIDI 36, 38, 40, 41 déclenche les cues; sauvegarder chemin, positions et loop; tests du mapping pur.
- Exclusions: timeline éditable, mappings personnalisés, 16 pads, MIDI Learn, CC, effets vélocité, export vidéo, transport sync et moteur vidéo personnalisé.
- Acceptance:
  - Nouvelle cible et identité AV Sampler, sans modifier plugin audio, identifiants ni état.
  - Instrument AU accepte NoteOn et produit le silence; NoteOff/non-assignées ignorées; dernier cue valide prioritaire.
  - UI charge une vidéo locale, lit en continu, boucle, mémorise 4 points, seek puis reprend sur Cue.
  - À la fin du média, Loop reprend la lecture au début; Pause manuel reste arrêté.
  - État vidéo/Cues/loop restauré; chemin absent récupérable.
  - Tests mapping, build AU macOS et QA Logic piano roll/lecture/projet réellement exécutés ou marqués NOT RUN.
  - Limites de latence, codecs, éditeur fermé et synchronisation documentées.
  - Limite initiale de coalescence des notes sur le tick UI explicitée; mesurer si un tampon fixe devient nécessaire.

### Roadmap après le vertical slice vidéo/MIDI

- **Phase 2 — Video Sampler UX:** VISUAL-010 ajoute la timeline et le déplacement des quatre Cue Points existants. 16 pads, mapping éditable et MIDI Learn restent des tickets ultérieurs.
- **Phase 2b — Session et médias:** VISUAL-011 organise les contrôles de session/média, restaure les états de lecture/Loop et facilite le relink sans déplacer les Cue Points.
- **Phase 2c — Composition de l'instrument:** VISUAL-012 réorganise les commandes, l'aperçu, la timeline, les emplacements FX inactifs et les quatre pads déjà assignés.
- **Terminé sur main:** timeline/session/média/UI VISUAL-010 à 012; Speed CC1 VISUAL-013; renderer Metal et RGB Split VISUAL-014a/b.
- **Prochain jalon:** VISUAL-015 est terminé et VISUAL-016 met en place le build Windows/VST3 en CI. Continuer les fonctionnalités d'abord sur macOS/AU/Logic; la validation dans REAPER et la parité Windows seront reprises quand ce périmètre macOS sera stabilisé.
- La première cohorte Alpha attend le préflight des deux plateformes. 16 pads, MIDI Learn, mapping éditable, nouveaux effets et modes de pad restent ultérieurs.
- VISUAL-004 à VISUAL-007 restent des tickets de maintenance du plugin audio historique, priorité secondaire.

### VISUAL-010 — Timeline interactive et Cue Points déplaçables

- Status: Done — parcours principal confirmé par l'utilisateur dans Logic puis fusionné à `main`; la composition UI VISUAL-012 traite séparément la présentation des pads. Raccourcis clavier, petites tailles et quelques cas média restent hors de la QA rapportée.
- Priority: P1.
- Scope: afficher sous la vidéo la durée complète, une règle en secondes, le playhead indépendant et quatre Cue Points colorés; sélectionner et déplacer chaque Cue Point; montrer les quatre cartes avec nom de note et temps; déplacer au clavier.
- Mapping visible des cartes: MIDI 36/38/40/41 affichés C1/D1/E1/F1 selon le nom d'octave présenté par Logic de l'utilisateur. Les marqueurs de timeline restent A/B/C/D.
- Exclusions: 16 pads, MIDI Learn, remapping, miniatures, nouveaux effets et refonte du moteur.
- Acceptance:
  - L'UI garde la vidéo et les contrôles existants; la timeline reste lisible entre 560 et 1400 px.
  - Durée vidéo, graduation, playhead, marqueurs A–D et cartes C1/D1/E1/F1 sont synchronisés aux Cue Points existants.
  - Glisser un marqueur borne sa position entre 0 et la durée; la lecture continue et le playhead ne saute pas.
  - Cliquer sur marqueur/carte sélectionne le Cue; flèches le déplacent d'une frame nominale de 1/30 s, Maj+flèche de 10 frames.
  - Les nouveaux temps sont persistés par l'état existant et utilisés par le déclenchement MIDI après déplacement et reload.
  - Tests automatisés couvrent les conversions temps/pixel, les bornes, le nudging et les noms de notes; smoke tests DAW notés dans la checklist.
  - L'absence de miniatures et l'absence d'API frame-rate exploitable restent explicites.

### VISUAL-011 — Session et gestion du média

- Status: Done — session/média validés par l'utilisateur et fusionnés à `main`; relink après déplacement avec Cue timings/Loop préservés confirmé manuellement dans Logic le 2026-11-24. Vérification à répéter sur l'artefact candidat de distribution.
- Priority: P1.
- Scope: organiser l'éditeur avec un en-tête AV Sampler/session, une barre média/transport, l'aperçu vidéo et la timeline/Cues existants; afficher le média courant ou son état manquant; charger, relier ou remplacer le média.
- Persistance: chemin média, Cue Points, Loop ON/OFF, lecture demandée (Pause/Play) et Cue sélectionné restent dans l'état du projet hôte. Une ancienne session sans champ lecture démarre automatiquement la vidéo lors de l'ouverture de l'éditeur.
- Acceptance:
  - Le nouvel éditeur conserve le design AV Sampler; seules les positions/groupes d'éléments s'inspirent des références jointes.
  - Réouvrir une session restaure Loop, sélection, état Play/Pause; par défaut compatible aux anciens états, la vidéo démarre dès l'ouverture si aucun état lecture n'existe.
  - Un chemin manquant est signalé avec le nom du fichier et une action Relink; choisir le bon fichier ou un média de remplacement actualise le chemin et garde les temps absolus des Cue Points.
  - Remplacer un média respecte l'état de lecture actuel; il ne réinitialise pas Loop ni les Cue Points.
  - Tests couvrent save/reload des nouveaux champs, rétrocompatibilité des anciens états et préservation des Cue timings au relink.
  - QA Logic vérifie ouverture/réouverture, Play/Pause, Loop, déplacement/relink média et sauvegarde/réouverture.
- Exclusions: session autonome/fichier `.avsampler`, plusieurs vidéos simultanées, banque de médias, 16 pads, waveform/audio slicing, tempo/sync et reproduction du design propriétaire des captures.

### VISUAL-012 — Composition UI de l'instrument vidéo

- Status: Done — l'utilisateur confirme que Logic charge la mise à jour visuelle et que tout fonctionne. Les captures avant/après comparables ne sont pas archivées; les contrôles manuels détaillés ne sont pas itemisés dans son retour.
- Priority: P1.
- Scope: réorganiser l'éditeur JUCE en cinq zones ordonnées: Global Control Bar, Video Preview, Timeline + Cue Points, FX / Pad Controls Bar, Assigned Pads. Reprendre les groupes et la densité fonctionnelle des références, garder l'identité AV Sampler.
- Media: afficher le vrai nom de fichier et la durée disponible; tronquer les noms longs avec tooltip; ajouter Eject uniquement avec une sémantique non destructive (retirer la référence active sans effacer les Cue Points ni les autres états).
- Timeline/pads: conserver quatre Cue Points A–D, MIDI 36/38/40/41 affiché C1/D1/E1/F1, positions et état partagés. Sélectionner depuis timeline ou pad met en évidence les deux sans déclencher la vidéo.
- FX: afficher des emplacements « Coming soon » inactifs et purement présentatifs; ne pas ajouter paramètres, fausses valeurs, comportements ou propriétés de persistance.
- Acceptance:
  - Cinq zones visibles et dans le bon ordre, dans une fenêtre redimensionnable sans scroll interne superflu; l'aperçu conserve le ratio/rendu vidéo existant.
  - Play/Pause, Loop, chargement, relink/remplacement, déplacement Cue, déclenchement MIDI et persistance gardent les contrats VISUAL-009/010/011.
  - Eject ferme le média actif et vide seulement son chemin sauvegardé; les Cue timings, Loop, Cue sélectionné et préférence de lecture sont préservés.
  - Chaque pad expose Cue A–D, note MIDI, position temporelle et sélection; la sélection timeline/pad reste synchronisée et ne déclenche pas la lecture.
  - La zone FX ne modifie pas le moteur ni l'état du projet.
  - Build/tests JUCE exécutés; QA Logic réelle distingue ce qui est observé de ce qui reste à tester.
  - Rapport avec captures avant/après à dimensions comparables et mise à jour des documents de suivi.
- Exclusions: nouveaux effets, 16 pads, MIDI Learn, changement de notes, mapping éditable, nouveau moteur vidéo, création d'un nouveau thème ou copie des logos/icônes/style propriétaire.

### VISUAL-013 — Premier effet vidéo piloté par MIDI CC

- Status: Done — PR #8 fusionnée dans `main` au commit `7bfddf9`; l'utilisateur confirme dans Logic le contrôle CC1, les extrêmes 0/127, la neutralité 64 et save/reload (validation rapportée le 2026-11-24). Le SHA de l'AU testé n'est pas consigné; revalider sur l'artefact candidat de distribution. Un pluginval strictness 4 générique passe sur l'AU, sans remplacer ces vérifications MIDI manuelles.
- Priority: P1.
- Scope: activer uniquement Speed parmi les emplacements FX, piloté par CC1 (0–127 -> 0,5×–1,5×; 64 = 1×), avec affichage de la vitesse et sauvegarde/restauration de la valeur.
- Acceptance:
  - CC1 modifie la vitesse de lecture vidéo sans affecter le routage NoteOn des Cue Points ni la sortie silencieuse.
  - `processBlock()` reste sans verrou, allocation, I/O, chargement média et appel UI; `setPlaySpeed()` est appelé sur le thread message.
  - Une session sauvegardée restaure la valeur CC; un état ancien sans le nouveau champ démarre à 1×.
  - Le slot Speed affiche sa valeur et CC1; Level, Color et Glitch restent désactivés.
  - Tests C++ couvrent mapping, bornes, routage des événements et persistance; build AU/CTest et `git diff --check` sont consignés.
  - QA Logic sur le build exact confirme le contrôle depuis CC1, la neutralité à 64, les extrêmes et le save/reload; toute mesure de latence reste distincte.
- Exclusions: choix dynamique de CC, MIDI Learn, vélocité, autres effets, synchronisation tempo/frame et promesse de précision temporelle.

### VISUAL-014 — RGB Split déclenché par la vélocité MIDI

- Status: Done — VISUAL-014a et VISUAL-014b implémentés, build AU installé et validation utilisateur Logic rapportée.
- Priority: P1.
- Dépendance historique résolue: PR #9/VISUAL-014a a remplacé l'ancien rendu `juce::VideoComponent` par le chemin AVFoundation/Metal; PR #10/VISUAL-014b a ajouté RGB Split.

#### VISUAL-014a — Prototype d'accès aux frames et rendu GPU

- Status: Validated in Logic — l'utilisateur confirme après rescan et relance de Logic que l'affichage et le fonctionnement sont bons. Aucun relevé chiffré CPU/GPU; garder cette mesure comme suivi de performance.
- Scope: remplacer uniquement le composant d'affichage vidéo par un rendu macOS qui reçoit les frames décodées et les présente via Metal, en gardant l'architecture transport/session du sampler.
- Acceptance: vidéo affichée dans l'AU Logic; ouverture, lecture, seek Cue, Loop, vitesse CC1, fermeture/rechargement du média restent fonctionnels; lecture des frames sans copie CPU complète; fluidité et usage CPU/GPU consignés. Aucun effet visuel actif dans ce sous-ticket.
- Exclusions: RGB Split, vélocité, nouveaux effets et migration VST3/autres plateformes.

#### VISUAL-014b — RGB Split avec enveloppe de vélocité

- Status: Done — validation utilisateur : intensité répond à la vélocité; effet, transport et boucle fonctionnent correctement et restent fluides.
- Scope: vitesse indépendante sur CC1; vélocité NoteOn 1–127 pilote l'amplitude du décalage RGB, note velocity 0/NoteOff ignorée; rouge à droite, bleu à gauche, vert centré; enveloppe attack/hold/release et retrigger.
- Initial values: vélocité normalisée `(velocity - 1) / 126`, amplitude max proposée 1,5 % de la largeur, Attack 0–10 ms, Hold 40 ms, Release 180 ms; clamp aux bords pour éviter le noir.
- Acceptance: frappes faibles/fortes donnent un Split faible/fort, l'effet revient au neutre et se retrigger; cues/loop/CC1/save-reload inchangés; désactivation rétablit l'image originale; anciennes sessions sans état RGB sont compatibles.
- Exclusions: opacité/Level velocity, effets supplémentaires, MIDI Learn, 16 pads, Hold/Toggle/One-Shot et autres moteurs.

## Non-objectifs explicites

Pas de vidéo export, FFT, beat detection, TouchDesigner runtime, Web/desktop, marché de modules, DSP audio destructif, multi-DAW/Windows revendiqués sans tests.

### VISUAL-015 — Audit documentaire et cohérence du repository

- Status: Done — audit documentaire et mise à jour QA fusionnés dans la PR #12, merge commit `c2eb6a4`.
- Priority: P0 avant de distribuer une Closed Alpha; aucune fonctionnalité produit nouvelle.
- Scope: comparer README, AGENTS, CMake/Makefile, scripts/workflows, sources, tests et `docs/project/`; réconcilier vision, décisions supersédées, architecture actuelle, backlog, QA, release log et état de distribution.
- Acceptance: README présente AV Sampler en premier et distingue le POC audio; docs indiquent les formats/hôtes réellement qualifiés; matrice sépare preuves automatiques/manuelles et NOT TESTED; audit packaging conclut explicitement GO/NO-GO; prochaines actions alpha sont des tickets séparés; branches/worktrees sont signalés sans suppression automatique.
- Exclusions: changement de code runtime, fabrication de package, publication, merge/push et suppression de branches.

### VISUAL-016 — Processus de build Windows / VST3

- Status: Done pour le processus de build — branche `codex/VISUAL-016-dual-platform-preflight`; cible CMake VST3 Windows, wrapper JUCE VideoComponent et workflow GitHub Actions x64 ajoutés. Build Windows x64 et artefact QA PASS en CI (run `45435924725`). Le renderer Windows est un prototype compilable; son comportement en hôte est volontairement reporté.
- Priority: P0 avant toute qualification ou distribution Windows.
- Scope: préserver le comportement AU macOS, permettre la compilation AV Sampler en VST3 Windows, et automatiser un build x64 avec SHA source/binaire identifiables.
- Acceptance: workflow Windows x64 compile `AVSampler_VST3`; l'artefact QA fournit la provenance source et binaire; compilation macOS AU/VST3 conservée; écarts connus du prototype Windows documentés.
- Exclusions: test REAPER/utilisateur, parité RGB Split, package de cohorte, wizard, publication, Windows ARM et promesse de codec universelle. Ces points suivent dans VISUAL-017 après stabilisation des fonctionnalités macOS.

### VISUAL-017 — Qualification Windows et préparation Closed Alpha 1

- Status: In progress — démarré le 2026-11-24 après validation utilisateur de Flash dans Logic; la compilation Windows est automatisée, l'artefact VST3 exact et sa QA REAPER restent à obtenir.
- Priority: P0 pour une cohorte macOS/Windows.
- Scope: valider le VST3 dans REAPER sur Windows, qualifier lecture/transport/MIDI/session et codecs, décider puis combler les écarts de rendu (dont RGB Split), établir les exigences système, puis fabriquer deux archives identifiées par SHA avec instructions d'installation/mise à jour/désinstallation et guide de feedback pour 3–5 testeurs.
- Acceptance: Logic/AU sur Mac et REAPER/VST3 sur Windows vérifiés sur leurs artefacts exacts; vidéo/Cues/Loop, CC1, RGB Split, save/reopen et relink testés; pluginval effectué sur chaque artefact; versions, SHA, installation propre et limites documentés; aucune publication publique.
- QA restante côté Mac avant Closed Alpha: pluginval strictness 4 sur l'artefact AU exact; strictness 5 n'est pas rapporté. CC1=64/save-reload et relink après déplacement sont confirmés manuellement, sans SHA d'artefact.
- Première étape lancée: build VST3 x64 depuis le SHA de cette branche et test REAPER sur Windows du chargement vidéo, transport/Loop, MIDI Cues, CC1, save/reload et relink. RGB Split et Flash sont connus indisponibles sur le renderer Windows; noter le comportement visible et décider le portage avant alpha.

### VISUAL-018 — Clôture QA Logic CC1 et relink média

- Status: Done — le 2026-11-24, l'utilisateur confirme dans Logic CC1=64 à vitesse normale avec save/reload, puis le déplacement/relink de la vidéo en conservant les Cue timings et l'état Loop.
- Portée: consigner les retours manuels sur les fonctions macOS existantes; aucune modification de code.
- Limite: le SHA exact de l'AU testé n'est pas consigné. Rejouer ces scénarios sur l'artefact candidat de distribution; pluginval sur l'artefact alpha reste séparément requis.

### VISUAL-019 — Flash v1 piloté par la vélocité

- Status: Done — implémentation, build/tests et validation utilisateur dans Logic terminés le 2026-11-24.
- Priority: P1.
- Scope: sur le renderer Metal macOS, ajouter un mélange bref vers le blanc déclenché par NoteOn d'un cue; utiliser le même événement de vélocité et la même enveloppe fixe qu'RGB Split; ajouter un toggle Flash sauvegardé.
- Acceptance: intensité augmente avec vélocité 1–127 (maximum 65 %); durée reste fixe (Attack/Hold/Release 5/40/180 ms); l'image revient intacte au neutre; vélocité 0/NoteOff/non-assignée n'active pas l'effet; RGB Split, CC1, cue, Loop et lecture restent indépendants; l'état Flash est persisté, absent/false dans les sessions historiques; AU build/tests et installation passent; l'utilisateur confirme que les essais fonctionnent parfaitement dans Logic. Le contrôle Flash est indisponible et clairement signalé sous Windows.
- Exclusions: autres effets, réglages utilisateur d'enveloppe/intensité, MIDI Learn, parité Windows, pluginval de distribution et mesure de performance chiffrée.

### VISUAL-020 — Zoom Punch piloté par la vélocité (macOS)

- Status: Done — validation Logic rapportée par l'utilisateur sur l'AU installé; PR #18 fusionnée dans `main` au commit `595c533`.
- Priority: P1 pour le prototype macOS; l'inclusion dans Closed Alpha 1 reste à décider.
- Scope: ajouter au shader Metal un zoom centré déclenché par les NoteOn Cue existants. La vélocité contrôle l'amplitude jusqu'à 1,12× à 127; durée fixe Attack/Hold/Release de 5/40/180 ms. Toggle sauvegardé, désactivé par défaut pour les nouveaux et anciens projets.
- Acceptance: vélocité 0/NoteOff/non assignée n'active pas l'effet; frappes faibles/fortes donnent des amplitudes distinctes; chaque trigger repart avec la même enveloppe et l'image revient à 1×; RGB Split, Flash, CC1, Loop, Cue et audio-silence restent inchangés; état sauvé/rechargé et ancienne session désactivée par défaut; build/test/install AU et QA manuelle Logic; renderer Windows annonce explicitement indisponible.
- Exclusions: contrôles de zoom séparés, enveloppe configurable, parité Windows, changement du callback audio et décision d'élargir le package alpha.

### VISUAL-021 — Color Boost piloté par la vélocité (macOS)

- Status: Done — implémenté et validé par l'utilisateur dans Logic; PR #19 ouverte en attente de fusion.
- Priority: P1 pour le prototype macOS; inclusion dans Closed Alpha 1 à réévaluer après validation.
- Scope: augmenter temporairement la saturation dans le shader Metal macOS, sans modifier la teinte intentionnellement. La vélocité 1–127 contrôle un multiplicateur de saturation de 1,0× à 1,6×; durée fixe Attack/Hold/Release 5/40/180 ms. Toggle persisté, désactivé par défaut pour les nouveaux et anciens projets.
- Acceptance: velocity 0/NoteOff/non assignée n'active pas l'effet; faible/forte vélocité donne une saturation différente; l'image revient à la saturation originale après l'enveloppe; RGB Split, Flash, Zoom Punch, CC1, Cue, Loop et silence audio restent inchangés; save/reload et compatibilité des anciens projets testés; build/tests/install AU et QA Logic; renderer Windows indique explicitement indisponible.
- Exclusions: hue shift, luminosité/contraste indépendants, réglage continu du multiplicateur/enveloppe, parité Windows, changement de `processBlock()` et inclusion automatique dans l'alpha.
