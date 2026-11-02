# Casos de test · F-005 `ogrid_get_cell_state`

| Caso | Requisito | Cláusula | Técnica | Entrada | Resultado esperado |
|---|---|---|---|---|---|
| TC-F005-01 | SYS-PR-08 | err OK | Partición | no observada; observada 0,5; observada 2,55 | OGRID_OK las tres |
| TC-F005-02 | SYS-PR-08 | post POST-1 | Partición | grid inicial (l=0, no observada) | OK, UNSURVEYED |
| TC-F005-03 | SYS-SAF-07 | post POST-1 | Partición / robustez | no observada con l = 0, 2,55, ±límites, NaN, ±inf, ±10 | OK, UNSURVEYED siempre |
| TC-F005-04 | SYS-PR-08 | post POST-2 | Ejemplo analítico | observada, l=2,55 (p=0,9276) | OK, OCCUPIED |
| TC-F005-05 | SYS-PR-08 | post POST-3 | Ejemplo analítico | observada, l=1,70 (p=0,8455) | OK, LOW_PROB |
| TC-F005-06 | SYS-PR-08 | post POST-2 | Valor límite (justo encima) | observada, l=1,74 (p=0,8507) | OCCUPIED |
| TC-F005-07 | SYS-PR-08 | post POST-3 | Valor límite (justo debajo) | observada, l=1,73 (p=0,8494) | LOW_PROB |
| TC-F005-08 | SYS-PR-08 | post POST-2 | Analítico | observada, l=1,75 (-0,8+3·0,85) | OCCUPIED |
| TC-F005-09 | SYS-PR-08 | post POST-3 | Analítico | observada, l=0,85 (p=0,7006) | LOW_PROB |
| TC-F005-10 | SYS-PR-08 | post POST-3 | Partición | observada, l=0 y l=-0,4 | LOW_PROB (no UNSURVEYED) |
| TC-F005-11 | SYS-SAF-07 | post POST-3 | Límite mín. válido | observada, l=-0,8 | OK, LOW_PROB |
| TC-F005-12 | SYS-SAF-07 | post POST-2 | Límite máx. válido | observada, l=3,4 | OK, OCCUPIED |
| TC-F005-13 | SYS-SAF-07 | post POST-5 / err STATE | Robustez | observada con NaN, +inf, -inf; centinelas previos OCCUPIED y LOW_PROB | ERR_STATE, state_out=UNSURVEYED |
| TC-F005-14 | SYS-SAF-07 | post POST-5 / err STATE | Límite (mín−ε, máx+ε) | observada con 3,4 y -0,8 un ulp fuera, 3,5, -0,9, ±10, ±FLT_MAX | ERR_STATE, UNSURVEYED |
| TC-F005-15 | SYS-SAF-07 | post POST-5 | Propiedad de seguridad | datos corruptos observados | nunca LOW_PROB ni OCCUPIED |
| TC-F005-16 | SYS-SAF-07 | post POST-4 | Propiedad (acotación) | barrido l=-0,8..3,39 paso 0,01, observada y no | OK y salida ∈ {3 estados} |
| TC-F005-17 | SYS-PR-08 | post POST-2/3 | Propiedad (monotonía) | barrido observado | una sola transición LOW→OCC, en ≈1,7346 |
| TC-F005-18 | SYS-PR-08 | post POST-1..3 | Partición / mapeo de índice | 3520 celdas con patrón no obs./0/2,55 | cada celda su estado |
| TC-F005-19 | SYS-PR-08 | pre PRE-2 | Valor límite | índices 0, 3, 4, COUNT-1 | OK, LOW_PROB |
| TC-F005-20 | SYS-SAF-07 | err PARAM / pre PRE-1 | Puntero nulo | grid NULL | ERR_PARAM, state_out sin cambios |
| TC-F005-21 | SYS-SAF-07 | err PARAM / pre PRE-1 | Puntero nulo | state_out NULL | ERR_PARAM, grid sin cambios |
| TC-F005-22 | SYS-SAF-07 | pre PRE-1 | Puntero nulo | ambos NULL | ERR_PARAM |
| TC-F005-23 | SYS-SAF-07 | err RANGE / pre PRE-2 | Límite (máx+ε) | idx = COUNT, COUNT+1, UINT32_MAX | ERR_RANGE, state_out sin cambios |
| TC-F005-24 | SYS-SAF-07 | post POST-6 | Inmutabilidad | errores PARAM/RANGE sobre grid con patrón | grid idéntico bit a bit |
| TC-F005-25 | SYS-PR-08 | post POST-6 | Inmutabilidad | consulta de las 3520 celdas (una corrupta) | grid idéntico bit a bit |
| TC-F005-26 | SYS-SAF-07 | pre PRE-1 vs PRE-2 | Precedencia (orden de las precondiciones) | NULL + idx fuera de rango | ERR_PARAM, state_out sin cambios |
| TC-F005-27 | SYS-SAF-07 | err RANGE / err STATE | Precedencia | grid corrupto: idx fuera de rango y idx válido | ERR_RANGE (sin cambios) / ERR_STATE (UNSURVEYED) |
| TC-F005-28 | SYS-SAF-07 | post POST-5 | Aislamiento | celda corrupta y vecina válida 2,55 | vecina OK, OCCUPIED |
| TC-F005-29 | SYS-PR-08 | perf | Determinismo | 100 llamadas idénticas | mismo estado y status |
| TC-F005-30 | SYS-SAF-07 | post POST-4 | Recuperación tras error | NULL y RANGE, luego llamada válida | OK, OCCUPIED |
