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
