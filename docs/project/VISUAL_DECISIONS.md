# Visual Decisions

Décisions durables et état de leur validation. Dernier cadrage : 2026-11-24.

## Organisation du dépôt (conservée)

- Répertoire : `skytorner/visual`, précurseur technique du produit **AV Sampler**.
- Préfixe de tickets : `VISUAL-`; une tâche = une branche = des vérifications = une revue PR = suivi documentaire.
- `docs/project/` est la **source de vérité versionnée** (vision, architecture, décisions, backlog, QA, workflow, historique).
- `.codex/` contient éventuellement les mémoires/skills/outils/exports locaux et **n'est pas versionné**; un clone vierge doit fonctionner sans ce répertoire.
- Secrets, `.env`, logs, captures de credentials, `build/`, `dist/` et archives binaires ne sont pas committés.
- Scripts reproductibles, sources C++, CMake, README, AGENTS.md, release notes et docs projet sont versionnés.

## Product & Architecture Decisions — 2026-11-20

- **D-001 — Historique (supersédé le 2026-11-23 par D-011) :** Audio Reactive MVP d'abord; MIDI et pads après stabilisation. Le POC audio reste conservé mais AV Sampler vidéo MIDI est désormais prioritaire.
- **D-002 — Réutiliser le POC (partiellement supersédée par D-020/D-021) :** conserver JUCE/C++17/CMake et l'interface native; pas de réécriture, WebView, TS, React ou TouchDesigner. L'exclusion GPU reste valable pour AudioReactiveImagePOC, pas pour AV Sampler qui utilise désormais Metal.
- **D-003 — Cibles :** macOS/AU/Logic Pro d'abord. AV Sampler est actuellement configuré en AU macOS; VST3, Windows et autres DAW ne sont pas qualifiés. Les validations REAPER historiques concernent le POC audio.
- **D-004 — Compatibilité DAW :** préserver IDs plugin/paramètres, comportement et état; refactorings minimaux avec non-régression.
- **D-005 — Audio safe du POC historique :** pas de blocage ni chargement média sur le thread audio; passthrough sans modification intentionnelle; gérer mono/stéréo.
- **D-006 — Validation proportionnée :** tests unitaires DSP + pluginval (niveau 5, AU et VST3), QA manuelle ciblée; bounce offline et endurance lourde différés.
- **D-007 — Médias :** AV Sampler enregistre une référence externe. Le chemin absent est prévu comme récupérable; le test complet de déplacement/relink sur le build candidat de distribution reste à documenter. Pas de promesse de portabilité multi-machine.
- **D-008 — MIDI (mis à jour par D-011/D-014) :** les Cue MIDI et le contrôle vidéo sont confirmés manuellement dans l'AU Logic actuel. Cette preuve ne qualifie ni un autre hôte ni VST3; les validations audio historiques ne prouvent pas la réception MIDI.
- **D-009 — Qualité :** SOLID/DRY/KISS appliqués pragmatiquement; découper uniquement sous pression d'un besoin/test; dette intentionnelle documentée.
- **D-010 — Déploiement :** le packaging historique construit AudioReactiveImagePOC seulement. Aucun package Closed Alpha AV Sampler n'existe encore. Toute distribution doit identifier le SHA, le format/hôte qualifié, le processus d'installation/mise à jour/suppression et les exigences de signature/notarisation.

## Historique du POC v1.0.0

Le POC `AudioReactiveImagePOC` contient actuellement RMS, peak, envelope, image PNG/JPG, transformations, paramètres et état basé sur chemin. Le JUCE tag actuellement épinglé est `8.0.8`. Formats CMake : VST3, Standalone, AU sur macOS. Il conserve le bundle `com.visual.audioreactiveimagepoc` et les identifiants existants.

## Risques / compromis du POC audio historique

- Support mono : le défaut d'insertion initial a été corrigé dans VISUAL-003 et l'insertion Logic mono/dual-mono/stéréo est rapportée. La nouvelle provenance par SHA des anciens builds reste inconnue.
- Lissage dépendant des blocs potentiellement non constant temporellement : test puis correction si nécessaire.
- Image externe manquante : défaut géré, mais déplacement de projet non portable.
- Pluginval AU annonce `v0.1.0`, VST3 annonce `v1.0.0` : ne pas supposer qu'ils proviennent du même build.
- Dans le log, le validateur VST3 Steinberg n'était pas configuré. `pluginval` a bien réussi mais n'équivaut pas à un passage du validateur externe.
- Ces constats décrivent le POC audio et ses anciens rapports; ils ne doivent pas être présentés comme un état actuel de l'instrument vidéo.

Voir `VISUAL_PRODUCT.md`, `VISUAL_ARCHITECTURE.md` et `VISUAL_QA_CHECKLIST.md`.

## Changement de direction produit — 2026-11-23

- **D-011 — Priorité vidéo MIDI (autorisation explicite utilisateur) :** AV Sampler devient d'abord un instrument visuel natif inspiré d'un sampler vidéo. Phase 1 : AU macOS, Logic Pro, une vidéo continue, quatre Cue Points et quatre notes du piano roll. UX avancée, effets MIDI et validation produit restent séparés.
- **D-012 — Identités séparées :** conserver `AudioReactiveImagePOC`, ses paramètres et sauvegardes. AV Sampler utilise un nouvel identifiant/plugin/bundle; aucune migration implicite.
- **D-013 — Backend initial AV Sampler (supersédée par D-020) :** commencer par `juce::VideoComponent`/`juce_video` et le backend natif macOS. Cette étape a été remplacée par le renderer AVFoundation/Metal de VISUAL-014a.
- **D-014 — Temps réel MIDI :** le callback audio ne fait que mapper NoteOn fixe et publier les cues vers l'UI sans verrou ni allocation. Le thread message exécute les seeks. NoteOff ignoré; Cue valide le plus récent prioritaire.
- **D-015 — État média :** persister chemin, quatre positions et boucle dans l'état JUCE. La vidéo reste un fichier externe, dont l'absence est récupérable.
- **D-016 — Timeline phase 2 :** conserver A–D comme identifiants de Cue sur la timeline, afficher les notes MIDI sur les cartes utilisateur (C1/D1/E1/F1 dans Logic pour 36/38/40/41), et réutiliser le state existant. La précision clavier est nominalement 1/30 s tant que la fréquence d'images source n'est pas disponible.
- **D-017 — Session média VISUAL-011 :** le projet DAW reste la session source; persister chemin, Cue timings, Loop, Play/Pause demandé et Cue sélectionné dans l'état JUCE. Les états hérités sans préférence de lecture démarrent en lecture à l'ouverture. Relinker/replacer un média ne réinitialise pas les Cue timings.
- **D-018 — Composition UI VISUAL-012 :** conserver JUCE natif et les contrats du sampler; présenter dans cet ordre la barre de contrôle globale, l'aperçu vidéo, la timeline, une barre FX purement présentative, puis quatre pads assignés. Les pads et la timeline partagent le même `selectedCueIndex`; sélectionner ne déclenche pas le transport. Eject vide uniquement le chemin du média actif; Cue, Loop et préférence de lecture restent intacts. Aucun paramètre FX fictif ou état de persistance nouveau.
- **D-019 — Premier effet MIDI CC (VISUAL-013) :** le slot Speed est piloté par CC1, valeur 0–127 mappée linéairement par segments sur 0,5×–1,5×, avec valeur nominale neutre 64 = 1×. L'état CC est persisté dans le projet. `processBlock()` lit les événements et publie une valeur atomique; le composant vidéo applique la vitesse sur le thread message. Les autres slots FX restent inactifs. La neutralité à 64 et la persistance de cette valeur restent à vérifier manuellement.
- **D-020 — RGB Split et pipeline vidéo (VISUAL-014) :** constat d'architecture pour l'ancien backend `juce::VideoComponent`: sa vue macOS n'expose pas les frames, donc un shader ne peut pas être ajouté à cette vue. Décision d'implémentation exécutée : VISUAL-014a introduit l'accès frame AVFoundation et Metal; VISUAL-014b ajoute le shader RGB. Cette migration remplace le chemin d'affichage précédent.
- **D-021 — RGB Split v1 (VISUAL-014b) :** NoteOn assigné à un Cue publie vélocité + compteur via un atomique; l'éditeur transfère l'événement au renderer hors callback audio. Vélocité 1–127 mappe linéairement `(v-1)/126`, vélocité 0/NoteOff ne déclenche pas. Le shader Metal décale les échantillons rouge/bleu en directions opposées (rouge visuellement à droite, bleu à gauche), vert centré, clamp UV aux bords. Amplitude maximale 1,5 % de la largeur de l'image. Enveloppe lissée Attack 5 ms, Hold 40 ms, Release 180 ms, redémarrée à chaque NoteOn. Interrupteur RGB Split persisté dans l'état projet, activé par défaut y compris pour anciennes sessions; l'intensité courante n'est jamais persistée. CC1 reste indépendant.

## État produit et décision de distribution — 2026-11-24

- **D-022 — Prototype validé, pas encore distribuable :** l'utilisateur confirme l'usage du nouvel AU dans Logic, le parcours MIDI/Loop, RGB Split et une lecture fluide en boucle. Ces observations sont manuelles; elles n'établissent pas de mesures chiffrées ni de compatibilité générale.
- **D-023 — Closed Alpha 1 avant élargissement :** commencer par une cohorte de 3 à 5 utilisateurs macOS/Logic, après préparation d'un package AV Sampler installable sans compiler et des instructions d'installation, mise à jour et suppression. Ne pas publier publiquement; le dépôt GitHub est public.
- Le package et workflow existants restent réservés à `AudioReactiveImagePOC`. La signature notarisation macOS, version minimale du système, matrice codec, provenance de package et qualification AV Sampler restent à déterminer dans des tickets dédiés.
- L'état `pluginval` de l'AU AV Sampler reste en attente d'un nouveau log utilisateur. Un redémarrage partiel de l'application ne constitue pas un résultat de validation.
