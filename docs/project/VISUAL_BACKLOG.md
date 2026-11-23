# Visual Backlog

Dernière mise à jour : 2026-11-23. Produit prioritaire : AV Sampler, instrument vidéo MIDI natif; POC audio conservé comme cible distincte.

Statuts : `Done` (confirmé), `Open` (à faire), `Needs verification` (à contrôler), `Deferred` (hors phase). P0 nécessaire au MVP; P1 souhaitable avant bêta; P2 itération ultérieure.

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

- Status: Open.
- Priority: P0.
- Scope: tests déterministes RMS/peak, silence, paramètres extrêmes, passthrough et restauration d'état, idéalement hors DAW.
- Acceptance: commandes reproductibles + CI courte.

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

- Status: Needs verification — implementation complete; Logic/codec/runtime checks pending; branche `codex/VISUAL-009-native-midi-video-poc`, issue de `main` `741f4b6`.
- Priority: P0.
- Scope: ajouter une cible instrument AU silencieuse; charger une vidéo locale; lecture/pause continue et boucle; quatre Cue Points réglables à la position courante; MIDI 36, 38, 40, 41 déclenche les cues; sauvegarder chemin, positions et loop; tests du mapping pur.
- Exclusions: timeline éditable, mappings personnalisés, 16 pads, MIDI Learn, CC, effets vélocité, export vidéo, transport sync et moteur vidéo personnalisé.
- Acceptance:
  - Nouvelle cible et identité AV Sampler, sans modifier plugin audio, identifiants ni état.
  - Instrument AU accepte NoteOn et produit le silence; NoteOff/non-assignées ignorées; dernier cue valide prioritaire.
  - UI charge une vidéo locale, lit en continu, boucle, mémorise 4 points, seek puis reprend sur Cue.
  - État vidéo/Cues/loop restauré; chemin absent récupérable.
  - Tests mapping, build AU macOS et QA Logic piano roll/lecture/projet réellement exécutés ou marqués NOT RUN.
  - Limites de latence, codecs, éditeur fermé et synchronisation documentées.
  - Limite initiale de coalescence des notes sur le tick UI explicitée; mesurer si un tampon fixe devient nécessaire.

### Roadmap après VISUAL-009

- **Phase 2 — Video Sampler UX:** timeline, Cue déplacés, jusqu'à 16 pads, mapping et MIDI Learn (ticket ultérieur).
- **Phase 3 — MIDI Effects:** effet(s) simples, CC, vélocité, Hold/Toggle/One Shot et lissage (tickets séparés).
- **Phase 4 — Validation produit:** beat réel, mesures de réactivité, VST3 et DAW additionnels.
- VISUAL-004 à VISUAL-007 restent des tickets de maintenance du plugin audio, priorité secondaire.

## Non-objectifs explicites

Pas de vidéo export, FFT, beat detection, TouchDesigner runtime, Web/desktop, marché de modules, DSP audio destructif, multi-DAW/Windows revendiqués sans tests.
