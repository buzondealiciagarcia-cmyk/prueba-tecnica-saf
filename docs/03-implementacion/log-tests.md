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
