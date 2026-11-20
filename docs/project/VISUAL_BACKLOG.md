# Visual Backlog

Dernière mise à jour : 2026-11-20. Produit cible : AV Sampler / Audio Reactive MVP.

Statuts : `Done` (confirmé), `Open` (à faire), `Needs verification` (à contrôler), `Deferred` (hors phase). P0 nécessaire au MVP; P1 souhaitable avant bêta; P2 itération ultérieure.

Une tâche = une branche = tests = PR = mise à jour docs. Les tickets ne doivent pas devenir de gros chantiers multidomaines.

## Roadmap et tickets

### VISUAL-001 — Vision produit, architecture et organisation

- Status: Done (documentation et décisions du 2026-11-20).
- Priority: P0.
- Scope: `VISUAL_PRODUCT.md`, `VISUAL_ARCHITECTURE.md`, `VISUAL_DECISIONS.md`, backlog, QA et instructions agent.
- Acceptance: audio reactive first, cible JUCE native, non-objectifs, règles qualité, contrat de non-régression, résultats POC documentés.

### VISUAL-002 — Skills et outils Codex

- Status: Deferred.
- Priority: P2.
- Goal: uniquement les skills utiles au workflow réel; ne pas bloquer le développement.
- Acceptance: AGENTS.md utilisable sans `.codex/`; aucun outil fictif ou dépendance à un répertoire local absent.

### VISUAL-003 — Support mono et stéréo sans régression

- Status: Open. **Premier ticket technique.**
- Priority: P0.
- Scope: adapter négociation des bus pour mono->mono et stereo->stereo, sans changement d'identité plugin ni UI.
- Acceptance:
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

### VISUAL-008 — Vertical slice MIDI

- Status: Deferred until Audio Reactive MVP.
- Priority: P2.
- Scope: preuve réception MIDI DAW -> note/velocity -> contrôle d'une image -> 4 pads. Pas de 16 pads directement.
- Acceptance: événement MIDI natif distinct de l'audio réactif prouvé sur un hôte cible.

## Non-objectifs explicites

Pas de vidéo export, FFT, beat detection, TouchDesigner runtime, Web/desktop, marché de modules, DSP audio destructif, multi-DAW/Windows revendiqués sans tests.
