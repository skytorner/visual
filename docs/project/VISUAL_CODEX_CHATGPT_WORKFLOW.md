# Visual Codex / ChatGPT Workflow

Ce document définit la boucle légère entre l'humain, Codex et ChatGPT pour `skytorner/visual`.

## Source de vérité

1. `docs/project/VISUAL_PRODUCT.md` — quoi et pourquoi.
2. `docs/project/VISUAL_DECISIONS.md` — décisions durables, invariants et compromis.
3. `docs/project/VISUAL_ARCHITECTURE.md` — architecture actuelle et contraintes du temps réel.
4. `docs/project/VISUAL_BACKLOG.md` — ticket suivant et critères de done.
5. `docs/project/VISUAL_QA_CHECKLIST.md` — baseline et preuve des tests.
6. `docs/project/VISUAL_RELEASE_LOG.md` — résultats techniques / releases historiques.

`.codex/project-memory.md`, `.codex/progress.md`, `.codex/next-actions.md` et éventuels skills locaux sont **optionnels** et non versionnés; Codex doit pouvoir démarrer depuis un clone vierge avec uniquement `AGENTS.md` et `docs/project/`.

## Rôles

- Humain : priorité produit, tests avec vrai DAW, validation des changements et distribution.
- Codex : analyse ciblée, implémentation d'un ticket, tests, branche/PR, mise à jour des docs et rapport factuel.
- ChatGPT : aide au cadrage, audit des résultats/exports et préparation des tickets/prompts, sans se substituer à l'exécution locale.

## Workflow — une tâche, une branche

1. Lire `AGENTS.md` et les docs versionnées.
2. Choisir **un seul** ticket du backlog, définir les risques et tests avant modification.
3. Créer `codex/VISUAL-XXX-sujet` depuis main à jour.
4. Préserver les contrats existants; pour un produit distinct, créer une cible et identité plugin propres sans migrer silencieusement les anciens projets.
5. Exécuter les tests locaux accessibles et `git diff --check`; ne pas inventer de tests.
6. Documenter les fichiers, commandes, PASS/FAIL/NOT RUN, risques et QA humaine restante dans PR/release log.
7. Relecture puis merge; si la QA manuelle est nécessaire, laisser PR non fusionnée jusqu'à validation.
8. Mettre à jour l'état du ticket uniquement lorsque ses critères sont remplis.

## Raccourcis quotidiens sur macOS (Makefile versionné)

- `make` : tests DSP CTest puis compilation JUCE incrémentale; pour la QA complète, il reste à lancer pluginval sur les nouveaux artefacts.
- `make test` : tests DSP rapides; `make plugin` : build AU/VST3/Standalone.
- `make status` : branche et SHA à consigner dans les rapports QA.
- `make install` : copie volontaire des AU/VST3 compilés dans les dossiers utilisateur. **Fermer d'abord Logic Pro et REAPER**; ce n'est pas une étape implicite du build.
- `make package` : compile et utilise le packaging ZIP macOS déjà existant dans `dist/`.
- Le `Makefile` n'est qu'une couche de confort autour de CMake/CTest : aucun changement de stack ou de source de vérité. Sur Windows, utiliser les commandes CMake/PowerShell.

## Changements à haut risque

- DSP : tests reproductibles, aucun lock/allocation/I/O sur audio thread, passthrough préservé.
- Identifiants de paramètres, bundle/plugin et sauvegardes : migration documentée et test d'ouverture de vieux projets.
- Médias : aucune régression de l'import ou du message de fichier manquant.
- CI : compiler sur Mac/Windows selon l'artefact et contrôler le build précis au lieu de s'appuyer sur la seule compilation.

## Exports optionnels

Si l'outil local existe, `bash .codex/tools/export-chatgpt-context.sh --ticket VISUAL-XXX`. Ne jamais exiger cette commande pour exécuter un ticket : `.codex/` est ignoré.

## Hygiène

Jamais de credentials, `.env`, dumps, fichiers clients ou exports bruts sensibles dans Git. Ne pas committer les ZIP/artefacts. Les logs QA peuvent être résumés sans chemin personnel de la machine.

## Format de fin de tâche

Résumé, branche/PR, fichiers modifiés, commandes réellement exécutées et résultats, risques/dette restante, tests manuels nécessaires, ticket suivant.
