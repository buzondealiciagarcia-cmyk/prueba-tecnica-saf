# Casos de test · F-001 `ogrid_reset_grid`

| Caso | Requisito | Cláusula | Técnica | Entrada | Resultado esperado |
|---|---|---|---|---|---|
| TC-F001-01 | SYS-PR-08 | err OGRID_OK | Partición válida | grid sucio no nulo | devuelve OGRID_OK |
| TC-F001-02 | SYS-PR-08 | post POST-1 | Partición válida, exactitud bit a bit | grid con patrón sucio (MAX/MIN/ECHO) | las 3520 celdas con log_odds_lo idéntico bit a bit a 0,0 |
| TC-F001-03 | SYS-PR-08 | post POST-2 | Partición válida | grid con observed alternado | las 3520 celdas con observed == false |
| TC-F001-04 | SYS-PR-08 | post POST-1/2 | Valores límite de índice | grid sucio | celdas 0 y 3519 reiniciadas |
| TC-F001-05 | SYS-PR-08 | post POST-1 | Valor límite de tamaño | constantes | OGRID_CELL_COUNT == 3520 y array de 3520 celdas |
| TC-F001-06 | SYS-SAF-07 | post POST-3 | Partición (todo ocupado) | todas log-odds MAX, observed true | log-odds 0 y observed false (ninguna ocupada) |
| TC-F001-07 | SYS-SAF-07 | post POST-3 | Partición (todo prob. baja) | todas log-odds MIN, observed true | log-odds 0 y observed false |
| TC-F001-08 | SYS-SAF-07 | post POST-3 | Propiedad | tras reset | toda celda es derivable como UNSURVEYED (observed false, sin evidencia) |
| TC-F001-09 | SYS-PR-08 | post POST-1/2 | Valores extremos / contenido sin inicializar | memoria 0xFF (NaN) | log-odds 0 exacto y observed false |
| TC-F001-10 | SYS-PR-08 | post POST-4 | Propiedad idempotencia | dos llamadas seguidas | segunda llamada OK y estado idéntico bit a bit al de la primera |
| TC-F001-11 | SYS-PR-08 | post POST-1/2/4 | Partición | reset, modificar celda 100, reset | celda 100 vuelve a 0 / false |
| TC-F001-12 | SYS-PR-08 | err OGRID_ERR_PARAM, pre PRE-1 | Puntero nulo | grid_out = NULL | OGRID_ERR_PARAM |
| TC-F001-13 | SYS-PR-08 | post POST-5 | Puntero nulo | NULL con otro grid sucio en memoria | el otro grid sin cambios |
| TC-F001-14 | SYS-PR-08 | err | Partición de códigos | NULL | distinto de OK, ERR_RANGE y ERR_STATE |
| TC-F001-15 | SYS-IMP-01 | perf / efectos laterales | Aislamiento de efectos | dos grids contiguos, reset del primero | primero reiniciado, segundo intacto |

Notas: las comparaciones entre grids (TC-F001-10, 13, 15) se hacen campo a campo (log_odds_lo por patrón de bits y observed), nunca por bytes de estructura, porque el contrato no garantiza el relleno. La comparación de 0,0 es por patrón de bits (el contrato exige exactitud, sin tolerancia), por lo que no se usa tolerancia de coma flotante. POST-3 se verifica por derivación (observed == false) porque `ogrid_get_cell_state` (F-005) no está en la API disponible. El WCET (perf) se mide en integración según el contrato.
