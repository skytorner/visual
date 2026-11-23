# Visual Backlog

Dernière mise à jour : 2026-11-23. Produit prioritaire : AV Sampler, instrument vidéo MIDI natif; POC audio conservé comme cible distincte.

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

- Status: Done — l'utilisateur confirme globalement « on validé » après les builds/tests macOS et CI PASS. La portée des scénarios n'est pas détaillée; pluginval est resté NOT RUN, sans résultat inventé.
- Priority: P0.
- Scope: tests déterministes RMS/peak, silence, paramètres extrêmes, accès read-only aux buffers audio; intégration JUCE séparée pour la transparence complète de `processBlock()` et la restauration d'état.
- Acceptance: commandes reproductibles + CI courte; build macOS et tests JUCE passent. Pluginval et QA hôte du nouvel artefact restent documentés séparément et ne sont pas revendiqués sans résultat.

### VISUAL-005 — Enveloppe temporellement cohérente

- Status: Open.
- Priority: P1 (promouvoir P0 si mesure de bug utilisateur).
- Scope: test mesurant enveloppe à buffers 32–1024 et sample rates 44.1/48/96 kHz; correction de l'attaque/release si nécessaire.
- Acceptance: courbes temporelles cohérentes, dégradation sonore/visuelle évitée.

### VISUAL-006 — Cycle de vie UI et état média

- Status: Open.
- Priority: P1.
- Scope: sécuriser l'import image asynchrone, état manquant/réassignation, restitution des paramètres et robustesse.
- Acceptance: fermeture éditeur pendant sélection; aucun crash; comportement documenté pour les médias externes. L'embarquement des médias est hors scope.

### VISUAL-007 — Bêta et distribution

- Status: Open.
- Priority: P1.
- Scope: vérifier versions AU/VST3 et build exact, packaging, messages d'installation, qualification signature/notarisation, matrice de compatibilité, pluginval en workflow utile.
- Acceptance: aucune compatibilité non testée annoncée; limitations clairement visibles.

### VISUAL-008 — Vertical slice MIDI audio-reactive

- Status: Superseded by VISUAL-009; le besoin MIDI porte désormais sur une cible vidéo instrument distincte.
- Priority: P2.
- Scope: preuve réception MIDI DAW -> note/velocity -> contrôle d'une image -> 4 pads. Pas de 16 pads directement.
- Acceptance: ancien objectif de routage MIDI/audio conservé comme historique, non prioritaire.

### VISUAL-009 — Native MIDI Video POC

- Status: Needs verification — l'utilisateur confirme que le Loop vidéo et le save/reload fonctionnent, ainsi que les quatre cues MIDI et le cycle Logic qui rejoue les notes. Restent les validations hors du scénario manuel rapporté (pluginval, latence mesurée, éditeur fermé); branche `codex/VISUAL-009-native-midi-video-poc`, issue de `main` `741f4b6`.
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

### Roadmap après VISUAL-009

- **Phase 2 — Video Sampler UX:** VISUAL-010 ajoute la timeline et le déplacement des quatre Cue Points existants. 16 pads, mapping éditable et MIDI Learn restent des tickets ultérieurs.
- **Phase 2b — Session et médias:** VISUAL-011 organise les contrôles de session/média, restaure les états de lecture/Loop et facilite le relink sans déplacer les Cue Points.
- **Phase 2c — Composition de l'instrument:** VISUAL-012 réorganise les commandes, l'aperçu, la timeline, les emplacements FX inactifs et les quatre pads déjà assignés.
- **Phase 3 — MIDI Effects:** effet(s) simples, CC, vélocité, Hold/Toggle/One Shot et lissage (tickets séparés).
- **Phase 4 — Validation produit:** beat réel, mesures de réactivité, VST3 et DAW additionnels.
- VISUAL-004 à VISUAL-007 restent des tickets de maintenance du plugin audio, priorité secondaire.

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

- Status: Done — session/média validés par l'utilisateur et fusionnés à `main`.
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

## Non-objectifs explicites

Pas de vidéo export, FFT, beat detection, TouchDesigner runtime, Web/desktop, marché de modules, DSP audio destructif, multi-DAW/Windows revendiqués sans tests.
