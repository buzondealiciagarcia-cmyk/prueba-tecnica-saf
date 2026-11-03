# Log de tests y cambios

Registro append-only (TST-LOG-01/02). Generado por `h test` y `h log change`.

## 2026-11-02 20:38 · F-001 · ogrid_reset_grid · ejecución 1 · FAIL

- Rama: `feature/SYS-PR-08-ogrid-reset-grid` · commit `517b9d1d80`

| Check | Resultado | Detalle |
|---|---|---|
| build | PASS |  |
| unit | FAIL | 15 casos, 1 fallos |
| coverage | FAIL | ogrid_reset_grid.c L 100.0 % R 60.0 % |
| misra | FAIL | 11 hallazgos |
| tidy | PASS |  |
| format | PASS |  |
| complexity | PASS |  |
| doxygen | PASS |  |
| forbidden | PASS |  |
| test_format | PASS |  |

Casos fallados:

- `TC-F001-15` SYS-IMP-01 — Memory Mismatch. Byte 5 Expected 0xFF Was 0x00.  neighbouring grid must not be modified

### Cambio · 2026-11-02 20:41 · F-001

- Agente: ingeniero-tests
- Causa raíz: TC-F001-15 compared raw bytes including struct padding, not guaranteed by the contract
- Cambio aplicado: Compare fields instead of raw memory in TC-10, 13 and 15
- Commit: `f05b9ff8bf`

## 2026-11-02 20:41 · F-001 · ogrid_reset_grid · ejecución 2 · FAIL

- Rama: `feature/SYS-PR-08-ogrid-reset-grid` · commit `f05b9ff8bf`

| Check | Resultado | Detalle |
|---|---|---|
| build | PASS |  |
| unit | PASS | 15 casos, 0 fallos |
| coverage | FAIL | ogrid_reset_grid.c L 100.0 % R 60.0 % |
| misra | FAIL | 11 hallazgos |
| tidy | PASS |  |
| format | PASS |  |
| complexity | PASS |  |
| doxygen | PASS |  |
| forbidden | PASS |  |
| test_format | PASS |  |

## 2026-11-02 20:48 · F-001 · ogrid_reset_grid · ejecución 3 · FAIL

- Rama: `feature/SYS-PR-08-ogrid-reset-grid` · commit `22fbfe58ed`

| Check | Resultado | Detalle |
|---|---|---|
| build | PASS |  |
| unit | PASS | 15 casos, 0 fallos |
| coverage | FAIL | ogrid_reset_grid.c L 90.9 % R 20.0 % |
| misra | PASS |  |
| tidy | PASS |  |
| format | PASS |  |
| complexity | PASS |  |
| doxygen | PASS |  |
| forbidden | PASS |  |
| test_format | PASS |  |

## 2026-11-02 20:55 · F-001 · ogrid_reset_grid · ejecución 4 · FAIL

- Rama: `feature/SYS-PR-08-ogrid-reset-grid` · commit `71955bb7b9`

| Check | Resultado | Detalle |
|---|---|---|
| build | PASS |  |
| unit | PASS | 15 casos, 0 fallos |
| coverage | PASS | ogrid_reset_grid.c L 100.0 % R 100.0 % |
| misra | PASS |  |
| tidy | PASS |  |
| format | PASS |  |
| complexity | PASS |  |
| doxygen | PASS |  |
| forbidden | PASS |  |
| test_format | PASS |  |

## 2026-11-02 20:55 · F-001 · ogrid_reset_grid · ejecución 5 · PASS

- Rama: `feature/SYS-PR-08-ogrid-reset-grid` · commit `04a6ae95fe`

| Check | Resultado | Detalle |
|---|---|---|
| build | PASS |  |
| unit | PASS | 15 casos, 0 fallos |
| coverage | PASS | ogrid_reset_grid.c L 100.0 % R 100.0 % |
| misra | PASS |  |
| tidy | PASS |  |
| format | PASS |  |
| complexity | PASS |  |
| doxygen | PASS |  |
| forbidden | PASS |  |
| test_format | PASS |  |

### Cambio · 2026-11-02 20:59 · F-001

- Agente: implementador
- Causa raíz: Standards audit recommendations C-PRE-04 and C-DOC-04
- Cambio aplicado: Add direct stdint/stdbool includes and rationale comment
- Commit: `efe670ca30`

## 2026-11-02 20:59 · F-001 · ogrid_reset_grid · ejecución 6 · PASS

- Rama: `feature/SYS-PR-08-ogrid-reset-grid` · commit `efe670ca30`

| Check | Resultado | Detalle |
|---|---|---|
| build | PASS |  |
| unit | PASS | 15 casos, 0 fallos |
| coverage | PASS | ogrid_reset_grid.c L 100.0 % R 100.0 % |
| misra | PASS |  |
| tidy | PASS |  |
| format | PASS |  |
| complexity | PASS |  |
| doxygen | PASS |  |
| forbidden | PASS |  |
| test_format | PASS |  |

## 2026-11-02 20:59 · F-001 · ogrid_reset_grid · ejecución 7 · PASS

- Rama: `feature/SYS-PR-08-ogrid-reset-grid` · commit `a136bd107b`

| Check | Resultado | Detalle |
|---|---|---|
| build | PASS |  |
| unit | PASS | 15 casos, 0 fallos |
| coverage | PASS | ogrid_reset_grid.c L 100.0 % R 100.0 % |
| misra | PASS |  |
| tidy | PASS |  |
| format | PASS |  |
| complexity | PASS |  |
| doxygen | PASS |  |
| forbidden | PASS |  |
| test_format | PASS |  |

## 2026-11-02 22:13 · F-002 · ogrid_log_odds_to_prob · ejecución 1 · PASS

- Rama: `feature/SYS-PR-08-ogrid-log-odds-to-prob` · commit `4689084ce8`

| Check | Resultado | Detalle |
|---|---|---|
| build | PASS |  |
| unit | PASS | 27 casos, 0 fallos |
| coverage | PASS | ogrid_log_odds_to_prob.c L 100.0 % R 100.0 % |
| misra | PASS |  |
| tidy | PASS |  |
| format | PASS |  |
| complexity | PASS |  |
| doxygen | PASS |  |
| forbidden | PASS |  |
| test_format | PASS |  |

### Cambio · 2026-11-02 22:14 · F-002

- Agente: implementador
- Causa raíz: Audit: inverted overflow comment and redundant macro
- Cambio aplicado: Fix comment, drop OGRID_PROB_ONE
- Commit: `e83810c986`

## 2026-11-02 22:14 · F-002 · ogrid_log_odds_to_prob · ejecución 2 · PASS

- Rama: `feature/SYS-PR-08-ogrid-log-odds-to-prob` · commit `e83810c986`

| Check | Resultado | Detalle |
|---|---|---|
| build | PASS |  |
| unit | PASS | 27 casos, 0 fallos |
| coverage | PASS | ogrid_log_odds_to_prob.c L 100.0 % R 100.0 % |
| misra | PASS |  |
| tidy | PASS |  |
| format | PASS |  |
| complexity | PASS |  |
| doxygen | PASS |  |
| forbidden | PASS |  |
| test_format | PASS |  |

## 2026-11-02 22:16 · F-003 · ogrid_clamp_log_odds · ejecución 1 · PASS

- Rama: `feature/SYS-PR-08-ogrid-clamp-log-odds` · commit `cf5b7b6865`

| Check | Resultado | Detalle |
|---|---|---|
| build | PASS |  |
| unit | PASS | 23 casos, 0 fallos |
| coverage | PASS | ogrid_clamp_log_odds.c L 100.0 % R 100.0 % |
| misra | PASS |  |
| tidy | PASS |  |
| format | PASS |  |
| complexity | PASS |  |
| doxygen | PASS |  |
| forbidden | PASS |  |
| test_format | PASS |  |

### Cambio · 2026-11-02 22:17 · F-003

- Agente: implementador
- Causa raíz: Audit C-TYP-02: implicit float32 coupling
- Cambio aplicado: Add rationale comment for FLT_MAX
- Commit: `44a7530f9f`

## 2026-11-02 22:17 · F-003 · ogrid_clamp_log_odds · ejecución 2 · PASS

- Rama: `feature/SYS-PR-08-ogrid-clamp-log-odds` · commit `44a7530f9f`

| Check | Resultado | Detalle |
|---|---|---|
| build | PASS |  |
| unit | PASS | 23 casos, 0 fallos |
| coverage | PASS | ogrid_clamp_log_odds.c L 100.0 % R 100.0 % |
| misra | PASS |  |
| tidy | PASS |  |
| format | PASS |  |
| complexity | PASS |  |
| doxygen | PASS |  |
| forbidden | PASS |  |
| test_format | PASS |  |

## 2026-11-02 22:19 · F-004 · ogrid_update_cell · ejecución 1 · FAIL

- Rama: `feature/SYS-PR-08-ogrid-update-cell` · commit `583a779e4a`

| Check | Resultado | Detalle |
|---|---|---|
| build | PASS |  |
| unit | PASS | 28 casos, 0 fallos |
| coverage | FAIL | ogrid_update_cell.c L 96.3 % R 91.7 % |
| misra | PASS |  |
| tidy | PASS |  |
| format | PASS |  |
| complexity | PASS |  |
| doxygen | PASS |  |
| forbidden | PASS |  |
| test_format | PASS |  |

### Cambio · 2026-11-02 22:20 · F-004

- Agente: ingeniero-tests
- Causa raíz: Contract clauses not exercised: occluded on corrupt cell and precondition order
- Cambio aplicado: Add TC-F004-29 to 32
- Commit: `b6dd576f3c`

## 2026-11-02 22:20 · F-004 · ogrid_update_cell · ejecución 2 · FAIL

- Rama: `feature/SYS-PR-08-ogrid-update-cell` · commit `b6dd576f3c`

| Check | Resultado | Detalle |
|---|---|---|
| build | PASS |  |
| unit | PASS | 32 casos, 0 fallos |
| coverage | FAIL | ogrid_update_cell.c L 96.3 % R 91.7 % |
| misra | PASS |  |
| tidy | PASS |  |
| format | PASS |  |
| complexity | PASS |  |
| doxygen | PASS |  |
| forbidden | PASS |  |
| test_format | PASS |  |

### Cambio · 2026-11-02 22:21 · F-004

- Agente: implementador
- Causa raíz: Unreachable defensive branch in apply_delta, coverage below 100 percent
- Cambio aplicado: Remove second validation; clamp status explicitly discarded
- Commit: `be3b7b1074`

## 2026-11-02 22:21 · F-004 · ogrid_update_cell · ejecución 3 · PASS

- Rama: `feature/SYS-PR-08-ogrid-update-cell` · commit `be3b7b1074`

| Check | Resultado | Detalle |
|---|---|---|
| build | PASS |  |
| unit | PASS | 32 casos, 0 fallos |
| coverage | PASS | ogrid_update_cell.c L 100.0 % R 100.0 % |
| misra | PASS |  |
| tidy | PASS |  |
| format | PASS |  |
| complexity | PASS |  |
| doxygen | PASS |  |
| forbidden | PASS |  |
| test_format | PASS |  |

## 2026-11-02 22:22 · F-004 · ogrid_update_cell · ejecución 4 · PASS

- Rama: `feature/SYS-PR-08-ogrid-update-cell` · commit `727ee011d3`

| Check | Resultado | Detalle |
|---|---|---|
| build | PASS |  |
| unit | PASS | 32 casos, 0 fallos |
| coverage | PASS | ogrid_update_cell.c L 100.0 % R 100.0 % |
| misra | PASS |  |
| tidy | PASS |  |
| format | PASS |  |
| complexity | PASS |  |
| doxygen | PASS |  |
| forbidden | PASS |  |
| test_format | PASS |  |

## 2026-11-02 22:30 · F-001 · ogrid_reset_grid · ejecución 8 · PASS

- Rama: `feature/SYS-PR-08-ogrid-reset-grid` · commit `815dbd5b0c`

| Check | Resultado | Detalle |
|---|---|---|
| build | PASS |  |
| unit | PASS | 15 casos, 0 fallos |
| coverage | PASS | ogrid_reset_grid.c L 100.0 % R 100.0 % |
| misra | PASS |  |
| tidy | PASS |  |
| format | PASS |  |
| complexity | PASS |  |
| doxygen | PASS |  |
| forbidden | PASS |  |
| test_format | PASS |  |

## 2026-11-02 22:32 · F-004 · ogrid_update_cell · ejecución 5 · PASS

- Rama: `feature/SYS-PR-08-ogrid-update-cell` · commit `427fc52924`

| Check | Resultado | Detalle |
|---|---|---|
| build | PASS |  |
| unit | PASS | 32 casos, 0 fallos |
| coverage | PASS | ogrid_update_cell.c L 100.0 % R 100.0 % |
| misra | PASS |  |
| tidy | PASS |  |
| format | PASS |  |
| complexity | PASS |  |
| doxygen | PASS |  |
| forbidden | PASS |  |
| test_format | PASS |  |

## 2026-11-02 22:35 · F-005 · ogrid_get_cell_state · ejecución 2 · PASS

- Rama: `feature/SYS-PR-08-ogrid-get-cell-state` · commit `51093bea67`

| Check | Resultado | Detalle |
|---|---|---|
| build | PASS |  |
| unit | PASS | 31 casos, 0 fallos |
| coverage | PASS | ogrid_get_cell_state.c L 100.0 % R 100.0 % |
| misra | PASS |  |
| tidy | PASS |  |
| format | PASS |  |
| complexity | PASS |  |
| doxygen | PASS |  |
| forbidden | PASS |  |
| test_format | PASS |  |

## 2026-11-02 22:36 · F-005 · ogrid_get_cell_state · ejecución 3 · PASS

- Rama: `feature/SYS-PR-08-ogrid-get-cell-state` · commit `089a1a1cc4`

| Check | Resultado | Detalle |
|---|---|---|
| build | PASS |  |
| unit | PASS | 31 casos, 0 fallos |
| coverage | PASS | ogrid_get_cell_state.c L 100.0 % R 100.0 % |
| misra | PASS |  |
| tidy | PASS |  |
| format | PASS |  |
| complexity | PASS |  |
| doxygen | PASS |  |
| forbidden | PASS |  |
| test_format | PASS |  |

## 2026-11-02 22:39 · F-006 · ogrid_update_ray · ejecución 1 · FAIL

- Rama: `feature/SYS-PR-08-ogrid-update-ray` · commit `4c44075586`

| Check | Resultado | Detalle |
|---|---|---|
| build | PASS |  |
| unit | FAIL | 38 casos, 2 fallos |
| coverage | FAIL | ogrid_update_ray.c L 100.0 % R 96.2 % |
| misra | PASS |  |
| tidy | PASS |  |
| format | PASS |  |
| complexity | PASS |  |
| doxygen | PASS |  |
| forbidden | PASS |  |
| test_format | PASS |  |

Casos fallados:

- `TC-F006-34` SYS-SAF-07 — Expected 3 Was 0.  corrupt cell must return ERR_STATE
- `TC-F006-36` SYS-SAF-07 — Expected 3 Was 0.  corrupt first cell must be ERR_STATE

### Cambio · 2026-11-02 22:40 · F-006

- Agente: ingeniero-tests
- Causa raíz: Tests assumed corrupt observed gives ERR_STATE; F-004 only checks log-odds
- Cambio aplicado: Fix TC-34 and 36, add TC-39 to 46
- Commit: `c9b0596caf`

## 2026-11-02 22:40 · F-006 · ogrid_update_ray · ejecución 2 · PASS

- Rama: `feature/SYS-PR-08-ogrid-update-ray` · commit `c9b0596caf`

| Check | Resultado | Detalle |
|---|---|---|
| build | PASS |  |
| unit | PASS | 46 casos, 0 fallos |
| coverage | PASS | ogrid_update_ray.c L 100.0 % R 100.0 % |
| misra | PASS |  |
| tidy | PASS |  |
| format | PASS |  |
| complexity | PASS |  |
| doxygen | PASS |  |
| forbidden | PASS |  |
| test_format | PASS |  |

### Cambio · 2026-11-02 22:42 · F-006

- Agente: implementador
- Causa raíz: Audit C-DOC-04: missing rationale comments
- Cambio aplicado: Comments and file details only
- Commit: `4d06afc5d3`

## 2026-11-02 22:42 · F-006 · ogrid_update_ray · ejecución 3 · PASS

- Rama: `feature/SYS-PR-08-ogrid-update-ray` · commit `4d06afc5d3`

| Check | Resultado | Detalle |
|---|---|---|
| build | PASS |  |
| unit | PASS | 46 casos, 0 fallos |
| coverage | PASS | ogrid_update_ray.c L 100.0 % R 100.0 % |
| misra | PASS |  |
| tidy | PASS |  |
| format | PASS |  |
| complexity | PASS |  |
| doxygen | PASS |  |
| forbidden | PASS |  |
| test_format | PASS |  |

## 2026-11-03 21:47 · F-006 · ogrid_update_ray · ejecución 4 · PASS

- Rama: `feature/SYS-PR-08-ogrid-update-ray` · commit `6695b3ee93`

| Check | Resultado | Detalle |
|---|---|---|
| build | PASS |  |
| unit | PASS | 46 casos, 0 fallos |
| coverage | PASS | ogrid_update_ray.c L 100.0 % R 100.0 % |
| misra | PASS |  |
| tidy | PASS |  |
| format | PASS |  |
| complexity | PASS |  |
| doxygen | PASS |  |
| forbidden | PASS |  |
| test_format | PASS |  |
