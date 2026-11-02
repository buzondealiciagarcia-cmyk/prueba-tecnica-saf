# Casos de test F-004 · ogrid_update_cell

| Caso | Requisito | Cláusula | Técnica | Entrada | Resultado esperado |
|---|---|---|---|---|---|
| TC-F004-01 | SYS-PR-08 | err OGRID_OK | Partición válida | eco, pase, oculta sobre celda 42 | OGRID_OK en las tres |
| TC-F004-02 | SYS-PR-08 | POST-1 | Valor analítico | eco desde 0 | 0,85 (tol 1e-6), observed = true |
| TC-F004-03 | SYS-PR-08 | POST-1 | Secuencia analítica | 3 ecos desde 0 | 0,85 / 1,70 / 2,55 (tol 1e-6) |
| TC-F004-04 | SYS-SAF-07 | POST-1 | Límite máx. | eco desde 3,0 y desde 3,4 | 3,4 exacto |
| TC-F004-05 | SYS-PR-08 | POST-1 | Límite mín. | eco desde -0,8 | 0,05 (tol 1e-6) |
| TC-F004-06 | SYS-PR-08 | POST-2 | Valor analítico | pase desde 0 | -0,40, observed = true |
| TC-F004-07 | SYS-PR-08 | POST-2 | Límite mín. | pase desde -0,8 (observed false) | -0,8 exacto, observed = true |
| TC-F004-08 | SYS-SAF-07 | POST-2 | Justo fuera (saturación) | pase desde -0,5 | -0,8 exacto |
| TC-F004-09 | SYS-PR-08 | POST-2 | Secuencia | eco y pase desde 0 | 0,45 (tol 1e-6) |
| TC-F004-10 | SYS-PR-08 | POST-3 | Partición | oculta, celda no observada a 0 | valor 0 y observed false |
| TC-F004-11 | SYS-PR-08 | POST-3 | Partición | oculta, celda observada a 1,25 | valor 1,25 y observed true |
| TC-F004-12 | SYS-PR-08 | POST-4 | Invariante | grid con patrón; 3 votos sobre celda 42 | resto de celdas idénticas bit a bit |
| TC-F004-13 | SYS-PR-08 | POST-4 | Límites de índice | idx 0, 3, 4, 3519 con eco | solo esa celda cambia a 0,85 |
| TC-F004-14 | SYS-SAF-07 | POST-4 | Propiedad (acotación) | 400 votos mixtos (7 eco, 3 pase, 1 oculta) | siempre OK y valor en [-0,8; 3,4] |
| TC-F004-15 | SYS-PR-08 | POST-1 | Propiedad (monotonía) | 10 ecos desde 0 | no decrece; final 3,4 exacto |
| TC-F004-16 | SYS-PR-08 | POST-5 | Analítico | 3 ecos desde -0,8 | 1,75 (tol 1e-6) > 1,7346 |
| TC-F004-17 | SYS-PR-08 | POST-5 | Analítico | 1 eco desde 0 | 0,85 < 1,7346 (no confirma) |
| TC-F004-18 | SYS-SAF-07 | PRE-1 / err PARAM | Puntero nulo | grid NULL, 3 votos | OGRID_ERR_PARAM |
| TC-F004-19 | SYS-SAF-07 | PRE-2 / err PARAM | Justo fuera / valor inválido | vote = COUNT, 99, -1 | OGRID_ERR_PARAM |
| TC-F004-20 | SYS-SAF-07 | POST-6 | Sin efectos | vote inválido | grid idéntico |
| TC-F004-21 | SYS-SAF-07 | PRE-3 / err RANGE | Límite justo fuera | idx 3520, 3521, UINT32_MAX | OGRID_ERR_RANGE |
| TC-F004-22 | SYS-SAF-07 | POST-6 | Sin efectos | idx fuera de rango | grid idéntico |
| TC-F004-23 | SYS-SAF-07 | PRE-4 / err STATE | Valores corruptos | celda NaN, +inf, -inf; eco y pase | OGRID_ERR_STATE |
| TC-F004-24 | SYS-SAF-07 | PRE-4 / err STATE | Justo fuera de rango | 1 ulp fuera de MIN/MAX, ±10, ±FLT_MAX; eco y pase | OGRID_ERR_STATE |
| TC-F004-25 | SYS-SAF-07 | POST-6 | Sin efectos | celda NaN o 5,0 | grid idéntico bit a bit (valor y observed) |
| TC-F004-26 | SYS-SAF-07 | PRE-4 | Límites válidos | almacenado -0,8 (eco) y 3,4 (pase) | OGRID_OK; 3,4 - 0,4 = 3,0 |
| TC-F004-27 | SYS-SAF-07 | POST-6 | Recuperación | errores y luego eco válido | OGRID_OK, 0,85 |
| TC-F004-28 | SYS-PR-08 | perf (determinismo) | Determinismo | eco,pase,eco,oculta,eco en celdas 7 y 3000 | resultados idénticos bit a bit; 2,15 (tol 1e-6) |

El WCET (clausula perf) se mide en integración, no en unitarios.
