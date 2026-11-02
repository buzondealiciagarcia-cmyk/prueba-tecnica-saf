# Casos de test F-007 · ogrid_extract_occupied

| Caso | Requisito | Cláusula | Técnica | Entrada | Resultado esperado |
|---|---|---|---|---|---|
| TC-F007-01 | SYS-PR-04 | pre PRE-1 / err PARAM / POST-5 | Puntero nulo | grid=NULL, máscara rellena | ERR_PARAM, máscara intacta |
| TC-F007-02 | SYS-PR-04 | pre PRE-1 / err PARAM / POST-5 | Puntero nulo | mask_out=NULL | ERR_PARAM, grid intacto |
| TC-F007-03 | SYS-PR-04 | pre PRE-1 / err PARAM | Puntero nulo | ambos NULL | ERR_PARAM |
| TC-F007-04 | SYS-PR-04 | post POST-1 / err OK | Partición (grid nuevo) | todo no observado, máscara previa true | OK, 3 520 false, sin escribir fuera |
| TC-F007-05 | SYS-PR-08 | post POST-2 | Partición | no observadas con l=3,4; 100; NaN; ±inf | OK, todo false |
| TC-F007-06 | SYS-PR-08 | post POST-3 | Analítico | observada l=1,70 (p=0,8455) | OK, false |
| TC-F007-07 | SYS-PR-08 | post POST-3 | Analítico | observada l=2,55 (p=0,9276) | OK, true (única) |
| TC-F007-08 | SYS-PR-08 | post POST-1 | Valor límite | l=1,73 y 1,74 (umbral l*=1,7346) | false / true |
| TC-F007-09 | SYS-PR-08 | post POST-1 | Valor límite | l=-0,8; 3,4; 0 observadas | OK; false/true/false |
| TC-F007-10 | SYS-PR-08 | err STATE / POST-4 | Límite máx+ε | l=nextafter(3,4) observada | ERR_STATE, celda false |
| TC-F007-11 | SYS-PR-08 | err STATE / POST-4 | Límite mín−ε | l=nextafter(-0,8) observada | ERR_STATE, celda false |
| TC-F007-12 | SYS-PR-08 | err STATE / POST-4 | Partición inválida | observada con NaN, +inf, −inf | ERR_STATE, celda false |
| TC-F007-13 | SYS-PR-08 | err STATE / POST-4 | Partición inválida | observed=2, 128, 255 con l=3,0 | ERR_STATE, celda false, nada más true |
| TC-F007-14 | SYS-PR-04 | post POST-1 | Partición, patrón módulo 7 | 3 520 celdas mixtas | OK, máscara = patrón, 1 508 true |
| TC-F007-15 | SYS-PR-04, SYS-PR-08 | post POST-1 | Consistencia con F-005 | mismo patrón | máscara == (estado OCCUPIED) en todas |
| TC-F007-16 | SYS-PR-04 | post POST-1 | Límite de índices | celdas 0 y 3 519 ocupadas | OK, solo esas dos true |
| TC-F007-17 | SYS-PR-08 | post POST-4 / err STATE | Mezcla de corruptas | 7 celdas corruptas (primera y última incluidas) y 4 válidas, máscara previa true | un ERR_STATE; corruptas false; válidas correctas; 3 true |
| TC-F007-18 | SYS-PR-08 | post POST-5 | Inmutabilidad | llamadas con OK, STATE, PARAM | grid sin cambios |
| TC-F007-19 | SYS-PR-08 | post POST-1 | Propiedad (monotonía) | l=-0,8+0,05·i, i=0..83 | sin descensos, una sola transición entre 1,70 y 1,75 |
| TC-F007-20 | SYS-PR-04 | post POST-1 | Determinismo | dos llamadas, mismo grid | máscaras iguales |

Cláusula `perf` (WCET < 1 ms): se mide en integración, sin caso unitario.
