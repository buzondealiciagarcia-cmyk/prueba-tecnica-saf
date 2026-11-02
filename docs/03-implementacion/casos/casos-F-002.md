# Casos de test · F-002 `ogrid_log_odds_to_prob`

| Caso | Requisito | Cláusula | Técnica | Entrada | Resultado esperado |
|---|---|---|---|---|---|
| TC-F002-01 | SYS-PR-08 | err OGRID_OK | Partición válida | l=0, salida válida | OGRID_OK |
| TC-F002-02 | SYS-PR-08 | POST-4 | Analítico | l=0 | p=0,5 (±1e-6) |
| TC-F002-03 | SYS-PR-08 | POST-2 | Analítico | l=0,85 | 0,7005671 (±1e-6) |
| TC-F002-04 | SYS-PR-08 | POST-2 | Analítico | l=1,70 | 0,8455347 (±1e-6) |
| TC-F002-05 | SYS-PR-08 | POST-2 | Analítico | l=2,55 | 0,9275735 (±1e-6) |
| TC-F002-06 | SYS-PR-08 | POST-2, PRE-2 | Límite máx. | l=3,4 | OK, 0,9677045 (±1e-6) |
| TC-F002-07 | SYS-PR-08 | POST-2, PRE-2 | Límite mín. | l=-0,8 | OK, 0,3100255 (±1e-6) |
| TC-F002-08 | SYS-PR-08 | POST-2 | Valores literales del contrato | 0,85; 1,70; 2,55; 3,4; -0,8 | 0,7006; 0,8455; 0,9276; 0,9677; 0,3100 (±5e-5, redondeo) |
| TC-F002-09 | SYS-PR-08 | POST-1 | Analítico interior | l=1; 2; 3; -0,4 | 0,7310586; 0,8807971; 0,9525741; 0,4013123 (±1e-6) |
| TC-F002-10 | SYS-PR-08 | POST-3 | Propiedad (acotación) | barrido 85 puntos en [MIN,MAX] | OK y 0<p<1 en todos |
| TC-F002-11 | SYS-PR-08 | POST-3 | Propiedad (monotonía) | mismo barrido | p estrictamente creciente |
| TC-F002-12 | SYS-PR-08 | POST-3 | Propiedad (monotonía) | MIN,0,0,85,1,70,2,55,MAX | cadena estrictamente creciente |
| TC-F002-13 | SYS-PR-08 | POST-1 | Propiedad (simetría) | l y -l, l∈{0;0,1;0,4;0,8} | p(l)+p(-l)=1 (±2e-6) |
| TC-F002-14 | SYS-PR-08 | err RANGE, PRE-2 | Límite mín.-ε | MIN menos 1 ulp | OGRID_ERR_RANGE |
| TC-F002-15 | SYS-PR-08 | err RANGE, PRE-2 | Límite máx.+ε | MAX más 1 ulp | OGRID_ERR_RANGE |
| TC-F002-16 | SYS-PR-08 | err RANGE | Partición inválida | -1,0; 4,0 | OGRID_ERR_RANGE |
| TC-F002-17 | SYS-PR-08 | err RANGE, PRE-2 | Valor especial | NaN | OGRID_ERR_RANGE |
| TC-F002-18 | SYS-PR-08 | err RANGE, PRE-2 | Valor especial | +inf, -inf | OGRID_ERR_RANGE |
| TC-F002-19 | SYS-PR-08 | err RANGE | Extremos float | ±FLT_MAX | OGRID_ERR_RANGE |
| TC-F002-20 | SYS-PR-08 | POST-5 | Error sin efecto | MIN-1ulp | salida intacta (bit a bit) |
| TC-F002-21 | SYS-PR-08 | POST-5 | Error sin efecto | MAX+1ulp | salida intacta |
| TC-F002-22 | SYS-PR-08 | POST-5 | Error sin efecto | NaN | salida intacta |
| TC-F002-23 | SYS-PR-08 | POST-5 | Error sin efecto | ±inf | salida intacta |
| TC-F002-24 | SYS-PR-08 | err PARAM, PRE-1 | Puntero nulo | l=0,85, prob_out=NULL | OGRID_ERR_PARAM |
| TC-F002-25 | SYS-PR-08 | err PARAM, POST-5 | Puntero nulo | l=0, NULL | ni OK ni RANGE; datos ajenos intactos |
| TC-F002-26 | SYS-PR-08 | POST-5 | Sin estado residual | NaN y luego l=0 | OK, p=0,5 |
| TC-F002-27 | SYS-PR-08 | perf (pura, sin estado) | Determinismo | 1,70 dos veces con llamada intercalada | salidas idénticas bit a bit |

Cobertura: POST-1 (03-09,13), POST-2 (03-08), POST-3 (10-12), POST-4 (02), POST-5 (20-23,25,26), PRE-1 (24,25), PRE-2 (06,07,14-19), OK/PARAM/RANGE (01,24,14-19), perf (27; el WCET se mide en integración).
