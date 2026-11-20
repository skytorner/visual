# AV Sampler — Architecture minimale

Cadrage : 2026-11-20. État : POC AudioReactiveImagePOC existant, pas refonte.

## Stack conservé

- JUCE 8.0.8 (référence épinglée dans CMake), C++17, CMake.
- Builds AU macOS, VST3 et Standalone; aucun changement de stack imposé.
- UI JUCE native : pas de WebView, React, TypeScript, TouchDesigner ou rendu GPU dans le MVP.
- Ne pas changer les identifiants de plugin, manufacturer, bundle, ni IDs de paramètres existants sans plan de migration des projets DAW. Ne pas renommer le binaire existant dans les corrections de stabilité.

## Contrats actuels

```text
DAW (audio effect) -> AudioProcessor::processBlock()
                   -> RMS / Peak / Envelope -> atomics
                   -> AudioProcessorEditor (timer UI) -> image / transforms

State APVTS + imagePath -> sauvegarde/restauration du projet DAW
```

- Le MIDI est explicitement désactivé (`NEEDS_MIDI_INPUT FALSE`, `acceptsMidi() == false`), et les messages MIDI sont ignorés. Un instrument MIDI déclenche un son *avant* l'effet; le POC réagit à l'audio produit, pas aux notes MIDI.
- Le buffer audio reste inchangé, sauf nécessité technique avec canaux supplémentaires; le traitement audio n'appelle ni interface, ni chargement de fichiers, ni mutex.
- `PluginProcessor` traite les données et expose les métriques; `PluginEditor` charge/dessine l'image et affiche les contrôles.
- Les paramètres JUCE/APVTS et l'extension média sont restaurés depuis l'état; la portabilité des chemins externes n'est pas garantie.

## Invariants à préserver

1. Audio transparency : un effet de visualisation n'ajoute pas volontairement de traitement audio ni de latence.
2. Temps réel : pas de verrou, allocation dynamique, accès disque, appel UI ou opération bloquante dans `processBlock()`.
3. UI asynchrone : fermer l'éditeur pendant une sélection de fichier ne doit pas créer d'accès à un objet détruit.
4. Instance isolation : toutes les instances et leurs médias/paramètres restent indépendants.
5. Persistence : paramètres existants lisibles dans les projets créés avec le POC; ne pas casser les IDs sans migration.
6. Dépendances reproductibles : version JUCE épinglée, builds CMake et checks automatisés.
7. Erreur média : chemin inaccessible traité sans crash et avec possibilité de remplacement.

## Points techniques issus de l'audit statique, non régressions démontrées

- `isBusesLayoutSupported()` accepte uniquement stereo->stereo : insertion mono impossible (échec manuel confirmé).
- `processBlock()` calcule un lissage de l'enveloppe par bloc plutôt qu'en temps réel : la vitesse peut dépendre de la taille du buffer; écrire un test et corriger si nécessaire en préservant le rendu de référence.
- Le `FileChooser::launchAsync` capture `this` : examiner le cycle de vie pendant la fermeture de l'éditeur.
- Les chemins d'images ne sont pas portables entre ordinateurs; comportement de remplacement testé.
- Les formats / versions AU et VST3 affichés par pluginval diffèrent (AU 0.1.0; VST3 1.0.0) : vérifier les binaires réellement installés avant de modifier les métadonnées.
- Vérifier les nombres de canaux réels à partir du layout négocié avant toute modification du DSP.

## Refactoring autorisé, uniquement motivé

Ne pas changer l'organisation des 4 fichiers C++ pour l'esthétique. Extraire par exemple un `AudioEnvelope` indépendant uniquement pour rendre les calculs testables, sans introduire manager, event bus, framework ou couches abstraites sans usage vérifié.

Règles :
- KISS d'abord; DRY pour la logique métier répétée, pas pour chaque ligne commune.
- Responsabilité unique là où elle permet de tester ou faire évoluer le composant.
- Préférer des fonctions pures pour le calcul DSP et le mapping visuel; adapter progressivement.
- Chaque ticket apporte tests, justification de la modification, compatibilité d'état et compte rendu QA.
- Documenter la dette acceptée et le moment de la revoir.

## Extension future MIDI (pas dans le MVP Audio Reactive)

Nouveau contrat interne de contrôle visuel : sources AudioEnvelope ou MidiTrigger -> paramètres visuels par scène/pad -> Renderer JUCE. Vérifier la disponibilité des événements MIDI selon type de plugin et DAW avant d'implémenter; ne pas présumer qu'un AU FX reçoit automatiquement les notes MIDI de l'instrument. Isoler les événements temps réel du rendu UI avec échange lock-free adapté.

Le premier prototype MIDI devra démontrer la réception de notes distinctes et velocity dans l'hôte cible, puis seulement généraliser à quatre pads. Le renderer existant doit rester réutilisable.
