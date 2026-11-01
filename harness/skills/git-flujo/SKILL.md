---
name: git-flujo
description: Flujo git del proyecto (ramas, commits, merges --no-ff, tags de fase, push) automatizado con h git. Úsala siempre que vayas a crear ramas, commitear o integrar.
---
# Flujo git (STD-GIT)

CLI: `H="@@HARNESS@@/bin/h"` · Estándar completo: `@@HARNESS@@/standards/git.md`

| Situación | Comando |
|---|---|
| Trabajar en una fase documental | `$H git start-phase req|design|impl|offer` |
| Empezar una función del plan | `$H git start-feature F-NNN` → `feature/<REQ>-<func>` |
| Otra rama de trabajo | `$H git start <tipo>/<desc>` (valida el nombre) |
| Integrar la rama actual en develop | `$H git finish [--refs "REQ-…, F-…"]` (feature: exige `h test` PASS en HEAD y marca la función integrada) |
| develop → main con versión | `$H git release --tag vX.Y.Z --message "…"` |
| Cerrar fase (merges + tag de fase) | lo hace `$H gate close <fase>` |
| Comprobar un nombre de rama | `$H git check-branch <nombre>` |

Commits: `git commit -m "<tipo>(<ámbito>): <resumen imperativo ≤72, sin punto>" -m "Refs: <IDs>"`. Tipos: feat, fix, test, docs, refactor, perf, build, ci, chore, style, revert. Ámbitos: módulo C, `req`, `design`, `impl`, `offer`, `dossier`, `harness`, `ui`, `deploy`.

Prohibido: commits directos en `main`/`develop` (hook), `--no-verify`, `push --force`, `rebase` de ramas publicadas, reescribir historia. Si el push falla por credenciales, informa y continúa (queda pendiente).
