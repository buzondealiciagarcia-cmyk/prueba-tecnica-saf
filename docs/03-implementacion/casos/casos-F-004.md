# Casos de prueba F-004 · ogrid_update_cell

`observed` es `uint8_t` con `OGRID_OBSERVED_NO` (0) / `OGRID_OBSERVED_YES` (1).

| Caso | Requisito | Cláusula | Técnica | Entrada | Resultado esperado |
|---|---|---|---|---|---|
| TC-F004-01 | SYS-PR-08 | err OK | partición | eco, pase, oculta válidos | OGRID_OK |
| TC-F004-02 | SYS-PR-08 | POST-1 | analítico | eco desde 0 | 0,85 (tol 1e-6); observed = YES |
| TC-F004-03 | SYS-PR-08 | POST-1 | analítico | 3 ecos desde 0 | 0,85 / 1,70 / 2,55 |
| TC-F004-04 | SYS-SAF-07 | POST-1 | límite | 3,0 + eco; eco en 3,4 | 3,4 exacto |
| TC-F004-05 | SYS-PR-08 | POST-1 | límite | eco desde -0,8 | 0,05 |
| TC-F004-06 | SYS-PR-08 | POST-2 | analítico | pase desde 0 | -0,40; observed = YES |
| TC-F004-07 | SYS-PR-08 | POST-2 | límite | pase en -0,8 (observed NO) | -0,8 exacto; observed = YES |
| TC-F004-08 | SYS-SAF-07 | POST-2 | límite | -0,5 + pase | -0,8 exacto |
| TC-F004-09 | SYS-PR-08 | POST-2 | analítico | eco y pase | 0,45 |
| TC-F004-10 | SYS-PR-08 | POST-3 | partición | oculta, celda no observada | sin cambios; observed = NO |
| TC-F004-11 | SYS-PR-08 | POST-3 | partición | oculta, celda observada 1,25 | sin cambios; observed = YES |
| TC-F004-12 | SYS-PR-08 | POST-4 | propiedad | 3 votos, grid con patrón | resto de celdas intactas |
| TC-F004-13 | SYS-PR-08 | POST-4 | límite | índices 0, 3, 4, 3519 | solo su celda cambia |
| TC-F004-14 | SYS-SAF-07 | POST-4 | propiedad | 400 votos mixtos | siempre en [-0,8; 3,4] |
| TC-F004-15 | SYS-PR-08 | POST-1 | monotonía | 10 ecos | no decrece; llega a 3,4 |
| TC-F004-16 | SYS-PR-08 | POST-5 | analítico | 3 ecos desde -0,8 | 1,75 > 1,7346 |
| TC-F004-17 | SYS-PR-08 | POST-5 | analítico | 1 eco desde 0 | < 1,7346 |
| TC-F004-18 | SYS-SAF-07 | PRE-1 | puntero nulo | grid NULL, 3 votos | OGRID_ERR_PARAM |
| TC-F004-19 | SYS-SAF-07 | PRE-2 | límite | COUNT, 99, -1 | OGRID_ERR_PARAM |
| TC-F004-20 | SYS-SAF-07 | POST-6 | partición | voto inválido | grid sin cambios |
| TC-F004-21 | SYS-SAF-07 | PRE-3 | límite | idx 3520, 3521, UINT32_MAX | OGRID_ERR_RANGE |
| TC-F004-22 | SYS-SAF-07 | POST-6 | partición | idx fuera | grid sin cambios |
| TC-F004-23 | SYS-SAF-07 | PRE-4 | partición | NaN, +inf, -inf | OGRID_ERR_STATE |
| TC-F004-24 | SYS-SAF-07 | PRE-4 | límite | 1 ulp fuera, ±10, ±FLT_MAX | OGRID_ERR_STATE |
| TC-F004-25 | SYS-SAF-07 | POST-6 | partición | celda corrupta | grid sin cambios (log-odds y observed) |
| TC-F004-26 | SYS-SAF-07 | PRE-4 | límite | -0,8 y 3,4 almacenados | OGRID_OK; 3,4 - 0,4 = 3,0 |
| TC-F004-27 | SYS-SAF-07 | POST-6 | secuencia | llamada válida tras errores | OK, 0,85 |
| TC-F004-28 | SYS-PR-08 | perf | determinismo | misma secuencia en 2 celdas | idéntico bit a bit; 2,15 |
| TC-F004-29 | SYS-SAF-07 | PRE-4 | partición | oculta con celda corrupta | OGRID_ERR_STATE sin cambios |
| TC-F004-30 | SYS-SAF-07 | PRE-1 | orden | grid NULL + otros fallos | OGRID_ERR_PARAM |
| TC-F004-31 | SYS-SAF-07 | PRE-2 | orden | voto inválido + idx fuera / celda corrupta | OGRID_ERR_PARAM |
| TC-F004-32 | SYS-SAF-07 | PRE-3 | orden | idx fuera + grid corrupto | OGRID_ERR_RANGE; idx válido da ERR_STATE |
