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
