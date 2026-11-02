# Casos de prueba F-003 · ogrid_clamp_log_odds

Comparaciones exactas (bit a bit), sin tolerancia, segun POST-1..3.

| Caso | Requisito | Clausula | Tecnica | Entrada | Resultado esperado |
|---|---|---|---|---|---|
| TC-F003-01 | SYS-PR-08 | err OGRID_OK | Particion valida | 0,85 | OGRID_OK |
| TC-F003-02 | SYS-PR-08 | post POST-1 | Valor tipico | 0 | 0 |
| TC-F003-03 | SYS-PR-08 | post POST-1 | Particion interior | 0,85; 1,70; 2,55; -0,4; 3,0 | igual a la entrada |
| TC-F003-04 | SYS-SAF-07 | post POST-1 | Limite (min) | MIN | OK, MIN |
| TC-F003-05 | SYS-SAF-07 | post POST-1 | Limite (max) | MAX | OK, MAX |
| TC-F003-06 | SYS-SAF-07 | post POST-1 | Limite (justo dentro) | MIN+1ulp, MAX-1ulp | sin cambio |
| TC-F003-07 | SYS-SAF-07 | post POST-2 | Limite (justo fuera) | MIN-1ulp | OK, MIN |
| TC-F003-08 | SYS-SAF-07 | post POST-3 | Limite (justo fuera) | MAX+1ulp | OK, MAX |
| TC-F003-09 | SYS-PR-08 | post POST-2 | Particion inferior | -1,0; -100 | MIN |
| TC-F003-10 | SYS-PR-08 | post POST-3 | Particion superior | 3,5; 4,0; 100 | MAX |
| TC-F003-11 | SYS-SAF-07 | post POST-2/3 | Extremos float | +-FLT_MAX | OK, MAX / MIN |
| TC-F003-12 | SYS-SAF-07 | post POST-2/3 | Valores de diseno 6.2 | -10; 10 | -0,8; 3,4 |
| TC-F003-13 | SYS-PR-08 | post POST-4 | Propiedad (idempotencia) | -50, -0,8, -0,5, 0, 1,7, 3,4, 3,41, 50 | clamp(clamp(x)) = clamp(x) |
| TC-F003-14 | SYS-SAF-07 | post POST-2/3 | Propiedad (acotacion), barrido | -5..5 paso 0,05 | MIN <= salida <= MAX |
| TC-F003-15 | SYS-PR-08 | post POST-1 | Propiedad (monotonia), barrido | -5..5 paso 0,05 | no decreciente |
| TC-F003-16 | SYS-SAF-07 | err OGRID_ERR_RANGE | Particion invalida | NaN | OGRID_ERR_RANGE |
| TC-F003-17 | SYS-SAF-07 | err OGRID_ERR_RANGE | Particion invalida | +inf, -inf | OGRID_ERR_RANGE |
| TC-F003-18 | SYS-SAF-07 | post POST-5 | Salida intacta | NaN | salida sin modificar |
| TC-F003-19 | SYS-SAF-07 | post POST-5 | Salida intacta | +inf, -inf | salida sin modificar |
| TC-F003-20 | SYS-SAF-07 | err OGRID_ERR_PARAM | Puntero nulo | out = NULL, valor 0,85 y 10 | OGRID_ERR_PARAM |
| TC-F003-21 | SYS-SAF-07 | post POST-5 | Sin efectos laterales | out = NULL | dato ajeno intacto |
| TC-F003-22 | SYS-PR-08 | post POST-5 | Sin estado residual | NaN y luego 1,0 | OK, 1,0 |
| TC-F003-23 | SYS-SAF-07 | perf | Determinismo / sin estado | 7,0 dos veces con otra llamada intercalada | resultados identicos |

Notas: el rendimiento (WCET) se mide en integracion (contrato §7).
