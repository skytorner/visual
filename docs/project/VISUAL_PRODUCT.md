# AV Sampler — Vision produit et périmètre

Date de cadrage : 2026-11-24. Dépôt : `skytorner/visual`.

## Vision

**AV Sampler — Play video like an instrument.** AV Sampler est un instrument visuel natif JUCE pour les musiciens et beatmakers travaillant dans un DAW. Les notes MIDI déclenchent des Cue Points d'une vidéo locale; le DAW enregistre et rejoue la performance. Le premier produit ne génère pas de son.

La direction évolue volontairement du POC audio vers un sampler vidéo MIDI natif. Le plugin audio existant est conservé comme cible distincte pendant la transition; ses identifiants, paramètres et projets ne changent pas.

## Produit livré sur `main` — prototype interne

Le prototype comprend une cible instrument AU macOS, un lecteur vidéo AVFoundation avec présentation Metal, une lecture/Loop, quatre Cue Points manipulables et quatre notes MIDI fixes (36/38/40/41). Les notes sont présentées comme C1/D1/E1/F1 dans la configuration Logic testée. CC1 pilote la vitesse de lecture; la vélocité NoteOn déclenche RGB Split et Flash. L'état de projet conserve le média référencé, les Cue Points, Loop, Play/Pause demandé, Cue sélectionné, valeur CC1 et activation des deux effets.

L'utilisateur confirme manuellement que le parcours fonctionne dans Logic, que le cycle de région rejoue les notes et que la lecture vidéo reste fluide en boucle. Cela constitue une validation fonctionnelle rapportée, pas une mesure instrumentée de latence, cadence ou usage CPU/GPU.

Le parcours Mac est validé manuellement dans Logic avec l'AU, y compris RGB Split et Flash. La qualification VST3 Windows/REAPER commence dans VISUAL-017; le prototype Windows ne prend pas encore en charge les effets Metal. Autres DAW, versions macOS minimales et compatibilité codec étendue restent non qualifiés. Une seule vidéo et une seule tête de lecture sont prises en charge. Aucune précision frame-perfect ni latence garantie n'est promise.

## POC audio conservé

Le plugin `AudioReactiveImagePOC` conserve l'analyse RMS/peak/envelope, le passthrough, le rendu d'image et ses contrôles. Son identité et son comportement restent séparés d'AV Sampler; il n'est pas la cible produit prioritaire.

## Fonctionnalités actuellement disponibles

- Timeline interactive et sélection synchronisée entre Cue Points et quatre pads.
- Référence vidéo sauvegardée dans le projet hôte; les pixels ne sont pas incorporés. L'utilisateur confirme dans Logic le relink après déplacement du média avec conservation des Cue timings et du Loop; refaire sur l'artefact candidat de distribution.
- CC1 mappé sur 0,5×–1,5×. L'utilisateur confirme dans Logic la valeur neutre 64 et sa restauration après save/reload, en plus des extrêmes 0/127; refaire sur l'artefact candidat de distribution.
- RGB Split déclenché par la vélocité MIDI avec activation sauvegardée. L'utilisateur confirme l'effet et une lecture fluide en boucle.
- Flash v1 est déclenché par NoteOn des cues sur le renderer Metal macOS; la vélocité règle l'intensité, avec une enveloppe fixe de 5/40/180 ms. Son activation est sauvegardée et désactivée par défaut pour préserver les anciennes sessions. Pas encore disponible sur Windows.

## Produit et phases de distribution

Le dépôt est en phase **Internal / Dev**. Le build Windows x64/VST3 est automatisé afin que le processus de création soit en place, mais le développement continue d'abord sur macOS/Logic. La qualification Windows/REAPER, la parité vidéo et les packages bi-plateformes seront repris quand le périmètre fonctionnel macOS sera stabilisé. La Closed Alpha 1 vise ensuite une cohorte contrôlée de 3 à 5 utilisateurs et reste bloquée par les deux parcours hôtes qualifiés, les packages installables sans environnement de compilation, leurs instructions de mise à jour/suppression, leur provenance et les matrices système/codecs.

### Closed Alpha 1 — périmètre proposé

- Une vidéo locale; lecture/pause/loop; timeline et quatre cues/pads MIDI.
- Déclenchement et relecture de notes dans Logic; vitesse par CC1; RGB Split et Flash par vélocité.
- Sauvegarde/réouverture du projet et récupération après média absent.
- Installation guidée et session réelle sans crash bloquant; recueil des difficultés d'installation et d'usage.

### Critères d'entrée

- Packages AU macOS et VST3 Windows générés depuis des SHA enregistrés, testés sur des machines propres et installables sans compiler.
- Procédures d'installation, scan hôte, mise à jour et désinstallation compréhensibles; exigences de signature et systèmes documentés pour chaque plateforme.
- Scénarios save/reload, relink de média déplacé, neutralité CC1 à 64 et pluginval tentés sur les artefacts candidats, avec résultats et blocages consignés.
- Parcours Logic/macOS et REAPER/Windows sans défaut bloquant; matrice de formats vidéo et limites visibles.

### Critères de sortie vers Closed Alpha 2

- 3 à 5 participants ont installé le package et terminé le parcours essentiel, ou les abandons sont expliqués et corrigés.
- Aucun crash bloquant ni perte de projet/mapping média non récupérable rapportés sur la cohorte.
- Installation, relink, cues, boucle, CC1, RGB Split et sauvegarde/réouverture sont qualifiés sur le package diffusé; versions/SHA et retours sont consignés.
- Les risques restants sont compris et acceptés avant d'élargir à 5–10 personnes.

Closed Alpha 2 vise 5 à 10 utilisateurs et les corrections d'Alpha 1. Closed Beta vise 10 à 20 utilisateurs, un workflow autonome confirmé et une qualification progressive des autres DAW/VST3.

## Suite produit après Closed Alpha

16 pads, MIDI Learn, mapping entièrement personnalisable, plusieurs lecteurs vidéo, polyphonie vidéo, export et effets supplémentaires restent en aval de la validation de cette première cohorte. D'autres DAW Windows/macOS restent à qualifier séparément.

## Hors périmètre initial

Polyphonie vidéo, plusieurs lecteurs, moteur 3D, export vidéo, séquenceur MIDI interne, synchronisation frame-perfect, marketplace et gestion de plugins tiers.

## Utilisateur et distribution

Utilisateur initial : beatmaker/producteur sur macOS, notamment Logic Pro. Premier succès : programmer quatre notes dans une région MIDI et rejouer la performance visuelle correspondante.

Ordre de travail et distribution : développement fonctionnel AU macOS/Logic (validé pour le périmètre présent) -> QA hôte et parité Windows VST3/REAPER (VISUAL-017 en cours) -> deux packages Closed Alpha 1 contrôlés -> Closed Alpha 2 -> Closed Beta -> qualification progressive d'autres DAW. Le workflow Windows x64 de compilation et d'artefact QA est déjà en place; il ne vaut pas qualification Windows.

## Média

Le plugin sauvegarde la référence (chemin) de la vidéo, pas ses pixels. Si le fichier est absent, afficher un état récupérable et permettre de relier le fichier déplacé ou sélectionner un remplacement sans modifier les Cue timings. Le projet hôte conserve aussi l'état Loop, l'état Play/Pause demandé et le Cue sélectionné; les anciens états sans préférence de lecture démarrent la vidéo à l'ouverture de l'éditeur. Ne jamais promettre la portabilité du média entre machines.

## Source de vérité

Décisions durables : `VISUAL_DECISIONS.md`; tickets : `VISUAL_BACKLOG.md`; preuves de tests : `VISUAL_QA_CHECKLIST.md`.
