# Medidas de tiempo de ogrid (WCET empírico)

Resultados en bruto de `build/host-bench/bench_wcet_ogrid` (ver `tests/integration/bench_wcet_ogrid.c` y `casos/casos-wcet-ogrid.md`). Generado el 2026-11-03.

- Equipo: Apple M5, macOS (Darwin arm64 25.5.0), 10 núcleos. Es el equipo de desarrollo, **no la ECU**.
- Compilación: preset `host-bench` (`gcc-16`, Release, sin sanitizers ni cobertura).
- Se ejecutó 3 veces seguidas (ejecución 1, 2 y 3); las tres tablas están más abajo. Un valor «max» es el máximo observado en cada ejecución, no una cota demostrada.

## Ejecución 1

- Arquitectura: Darwin arm64 25.5.0
- Compilador: 16.2.0
- Reloj: clock_gettime_nsec_np(CLOCK_UPTIME_RAW)
- Resolucion medida del reloj (menor diferencia no nula entre lecturas consecutivas): 41 ns
- Repeticiones: 2000 (rapidas), 500 (con recorrido del grid, rayos largos), precalentamiento 50
- Lote K: llamadas entre dos lecturas del reloj, calibrado por escenario para que un lote dure al menos 200 us (salvo tope de K indicado en el informe); ns por llamada = tiempo del lote / K
- Cuant. (%): resolucion del reloj / duracion mediana del lote

| Escenario | Funcion | K | Repeticiones | Cuant. (%) | min (ns) | mediana (ns) | p99 (ns) | max (ns) |
|---|---|---:|---:|---:|---:|---:|---:|---:|
| reloj: lectura vacia | clock_gettime | 12516 | 2000 | 0.04 | 8.86 | 8.88 | 20.5 | 24.7 |
| reset: grid 3520 sucio | ogrid_reset_grid | 265 | 500 | 0.02 | 796 | 796 | 852 | 872 |
| prob: barrido [-0.8, 3.4] | ogrid_log_odds_to_prob | 122970 | 2000 | 0.02 | 1.75 | 1.77 | 1.94 | 2.08 |
| prob: l = -0.8 (minimo) | ogrid_log_odds_to_prob | 122970 | 2000 | 0.02 | 1.72 | 1.75 | 1.91 | 2.08 |
| prob: l = 0 (exacto 0.5) | ogrid_log_odds_to_prob | 122970 | 2000 | 0.02 | 1.72 | 1.76 | 1.88 | 1.96 |
| prob: l = 3.4 (maximo) | ogrid_log_odds_to_prob | 122970 | 2000 | 0.02 | 1.72 | 1.76 | 1.92 | 2.09 |
| prob: l = 5 (fuera de rango) | ogrid_log_odds_to_prob | 245939 | 2000 | 0.02 | 0.742 | 0.896 | 0.958 | 1.01 |
| clamp: por debajo (-5) | ogrid_clamp_log_odds | 163841 | 2000 | 0.03 | 0.784 | 0.802 | 1.35 | 1.52 |
| clamp: dentro (1.0) | ogrid_clamp_log_odds | 292382 | 2000 | 0.02 | 0.746 | 0.762 | 0.832 | 1.08 |
| clamp: por encima (10) | ogrid_clamp_log_odds | 284625 | 2000 | 0.02 | 0.746 | 0.762 | 0.829 | 0.893 |
| cell: eco desde 0.0 | ogrid_update_cell | 104008 | 2000 | 0.02 | 2.02 | 2.07 | 2.23 | 2.32 |
| cell: paso desde 0.0 | ogrid_update_cell | 108100 | 2000 | 0.02 | 2.02 | 2.05 | 2.23 | 2.35 |
| cell: oculta desde 0.0 | ogrid_update_cell | 193208 | 2000 | 0.02 | 1.12 | 1.15 | 1.41 | 1.51 |
| cell: eco desde 3.4 (saturada) | ogrid_update_cell | 108204 | 2000 | 0.02 | 2.02 | 2.06 | 2.23 | 2.37 |
| cell: eco desde 2.8 (satura) | ogrid_update_cell | 108100 | 2000 | 0.02 | 2.02 | 2.07 | 2.23 | 2.35 |
| cell: paso desde -0.8 (saturada) | ogrid_update_cell | 108204 | 2000 | 0.02 | 2.02 | 2.06 | 2.23 | 2.33 |
| cell: paso desde -0.5 (satura) | ogrid_update_cell | 104008 | 2000 | 0.02 | 2.02 | 2.07 | 2.25 | 2.37 |
| cell: eco en celda no observada | ogrid_update_cell | 104008 | 2000 | 0.02 | 2.02 | 2.08 | 2.24 | 2.49 |
| state: observada l=2.55 (ocupada) | ogrid_get_cell_state | 84502 | 2000 | 0.02 | 2.54 | 2.62 | 2.83 | 2.99 |
| state: observada l=1.70 (prob. baja) | ogrid_get_cell_state | 87183 | 2000 | 0.02 | 2.54 | 2.62 | 2.83 | 2.98 |
| state: no observada | ogrid_get_cell_state | 235157 | 2000 | 0.02 | 0.903 | 0.903 | 0.970 | 1.08 |
| state: corrupta (observed=2) | ogrid_get_cell_state | 318193 | 2000 | 0.02 | 0.686 | 0.706 | 0.763 | 0.806 |
| state: corrupta (l=9 observada) | ogrid_get_cell_state | 117579 | 2000 | 0.02 | 1.57 | 1.80 | 1.93 | 2.06 |
| ray: 10 celdas sin eco | ogrid_update_ray | 3520 | 2000 | 0.05 | 21.2 | 21.9 | 24.1 | 27.3 |
| ray: 10 celdas, eco en la ultima | ogrid_update_ray | 1408 | 2000 | 0.13 | 21.3 | 22.4 | 26.5 | 35.0 |
| ray: 880 celdas sin eco | ogrid_update_ray | 40 | 500 | 0.06 | 1720 | 1770 | 1910 | 2230 |
| ray: 880 celdas, eco en la ultima | ogrid_update_ray | 16 | 500 | 0.14 | 1720 | 1770 | 2230 | 2560 |
| ray: 3520 celdas sin eco | ogrid_update_ray | 10 | 500 | 0.06 | 6820 | 7030 | 7720 | 8830 |
| ray: 3520 celdas, eco en la ultima | ogrid_update_ray | 4 | 500 | 0.15 | 6810 | 7030 | 7880 | 11900 |
| extract: 3520 observadas, mitad ocupadas | ogrid_extract_occupied | 24 | 500 | 0.02 | 8870 | 9150 | 9920 | 10100 |
| extract: 3520 observadas, todas ocupadas | ogrid_extract_occupied | 25 | 500 | 0.02 | 8880 | 9150 | 9920 | 10200 |
| extract: 3520 observadas, ninguna ocupada | ogrid_extract_occupied | 25 | 500 | 0.02 | 8870 | 9150 | 9830 | 10100 |
| extract: grid sin observar | ogrid_extract_occupied | 89 | 500 | 0.02 | 2400 | 2440 | 3210 | 3310 |

### Estimaciones por haz y por celda (a partir del maximo)

| Escenario | Celdas | K | max (ns/llamada) | max / celdas (ns/celda) |
|---|---:|---:|---:|---:|
| ray: 10 celdas sin eco | 10 | 3520 | 27.3 | 2.73 |
| ray: 10 celdas, eco en la ultima | 10 | 1408 | 35.0 | 3.50 |
| ray: 880 celdas sin eco | 880 | 40 | 2230 | 2.53 |
| ray: 880 celdas, eco en la ultima | 880 | 16 | 2560 | 2.91 |
| ray: 3520 celdas sin eco | 3520 | 10 | 8830 | 2.51 |
| ray: 3520 celdas, eco en la ultima | 3520 | 4 | 11900 | 3.37 |
| extract: 3520 observadas, mitad ocupadas | 3520 | 24 | 10100 | 2.87 |
| extract: 3520 observadas, todas ocupadas | 3520 | 25 | 10200 | 2.91 |
| extract: 3520 observadas, ninguna ocupada | 3520 | 25 | 10100 | 2.88 |
| extract: grid sin observar | 3520 | 89 | 3310 | 0.942 |

Acumuladores (evitan la eliminacion de llamadas): 4386226573.079236 4220048352

## Ejecución 2

- Arquitectura: Darwin arm64 25.5.0
- Compilador: 16.2.0
- Reloj: clock_gettime_nsec_np(CLOCK_UPTIME_RAW)
- Resolucion medida del reloj (menor diferencia no nula entre lecturas consecutivas): 41 ns
- Repeticiones: 2000 (rapidas), 500 (con recorrido del grid, rayos largos), precalentamiento 50
- Lote K: llamadas entre dos lecturas del reloj, calibrado por escenario para que un lote dure al menos 200 us (salvo tope de K indicado en el informe); ns por llamada = tiempo del lote / K
- Cuant. (%): resolucion del reloj / duracion mediana del lote

| Escenario | Funcion | K | Repeticiones | Cuant. (%) | min (ns) | mediana (ns) | p99 (ns) | max (ns) |
|---|---|---:|---:|---:|---:|---:|---:|---:|
| reloj: lectura vacia | clock_gettime | 25032 | 2000 | 0.02 | 8.79 | 8.88 | 9.47 | 10.2 |
| reset: grid 3520 sucio | ogrid_reset_grid | 264 | 500 | 0.02 | 796 | 796 | 851 | 908 |
| prob: barrido [-0.8, 3.4] | ogrid_log_odds_to_prob | 122970 | 2000 | 0.02 | 1.75 | 1.80 | 1.95 | 2.01 |
| prob: l = -0.8 (minimo) | ogrid_log_odds_to_prob | 122970 | 2000 | 0.02 | 1.72 | 1.77 | 1.92 | 2.04 |
| prob: l = 0 (exacto 0.5) | ogrid_log_odds_to_prob | 122970 | 2000 | 0.02 | 1.72 | 1.77 | 1.91 | 2.07 |
| prob: l = 3.4 (maximo) | ogrid_log_odds_to_prob | 128732 | 2000 | 0.02 | 1.72 | 1.77 | 1.90 | 1.98 |
| prob: l = 5 (fuera de rango) | ogrid_log_odds_to_prob | 292382 | 2000 | 0.02 | 0.707 | 0.896 | 0.957 | 1.01 |
| clamp: por debajo (-5) | ogrid_clamp_log_odds | 159097 | 2000 | 0.03 | 0.784 | 0.808 | 1.15 | 1.29 |
| clamp: dentro (1.0) | ogrid_clamp_log_odds | 284625 | 2000 | 0.02 | 0.746 | 0.770 | 0.937 | 0.990 |
| clamp: por encima (10) | ogrid_clamp_log_odds | 292382 | 2000 | 0.02 | 0.747 | 0.770 | 1.83 | 2.39 |
| cell: eco desde 0.0 | ogrid_update_cell | 107023 | 2000 | 0.02 | 2.02 | 2.08 | 2.24 | 2.31 |
| cell: paso desde 0.0 | ogrid_update_cell | 104008 | 2000 | 0.02 | 2.02 | 2.08 | 2.25 | 2.35 |
| cell: oculta desde 0.0 | ogrid_update_cell | 193042 | 2000 | 0.02 | 1.12 | 1.15 | 1.42 | 1.54 |
| cell: eco desde 3.4 (saturada) | ogrid_update_cell | 108204 | 2000 | 0.02 | 2.02 | 2.08 | 2.31 | 2.57 |
| cell: eco desde 2.8 (satura) | ogrid_update_cell | 104008 | 2000 | 0.02 | 2.02 | 2.08 | 2.25 | 2.37 |
| cell: paso desde -0.8 (saturada) | ogrid_update_cell | 104008 | 2000 | 0.02 | 2.02 | 2.08 | 2.24 | 2.35 |
| cell: paso desde -0.5 (satura) | ogrid_update_cell | 105920 | 2000 | 0.02 | 2.02 | 2.08 | 2.25 | 2.43 |
| cell: eco en celda no observada | ogrid_update_cell | 108204 | 2000 | 0.02 | 2.02 | 2.08 | 2.25 | 2.50 |
| state: observada l=2.55 (ocupada) | ogrid_get_cell_state | 87183 | 2000 | 0.02 | 2.54 | 2.62 | 2.84 | 2.94 |
| state: observada l=1.70 (prob. baja) | ogrid_get_cell_state | 84502 | 2000 | 0.02 | 2.54 | 2.62 | 2.84 | 3.04 |
| state: no observada | ogrid_get_cell_state | 235157 | 2000 | 0.02 | 0.680 | 0.903 | 0.978 | 1.05 |
| state: corrupta (observed=2) | ogrid_get_cell_state | 317968 | 2000 | 0.02 | 0.686 | 0.707 | 0.763 | 0.815 |
| state: corrupta (l=9 observada) | ogrid_get_cell_state | 117579 | 2000 | 0.02 | 1.37 | 1.79 | 1.96 | 2.34 |
| ray: 10 celdas sin eco | ogrid_update_ray | 3520 | 2000 | 0.05 | 21.3 | 21.9 | 24.9 | 27.8 |
| ray: 10 celdas, eco en la ultima | ogrid_update_ray | 1408 | 2000 | 0.13 | 21.2 | 22.3 | 28.1 | 33.5 |
| ray: 880 celdas sin eco | ogrid_update_ray | 40 | 500 | 0.06 | 1720 | 1770 | 2080 | 2190 |
| ray: 880 celdas, eco en la ultima | ogrid_update_ray | 16 | 500 | 0.14 | 1720 | 1780 | 2110 | 2310 |
| ray: 3520 celdas sin eco | ogrid_update_ray | 10 | 500 | 0.06 | 6810 | 7030 | 8160 | 10100 |
| ray: 3520 celdas, eco en la ultima | ogrid_update_ray | 4 | 500 | 0.15 | 6800 | 7020 | 7540 | 11900 |
| extract: 3520 observadas, mitad ocupadas | ogrid_extract_occupied | 25 | 500 | 0.02 | 8880 | 9150 | 9850 | 10200 |
| extract: 3520 observadas, todas ocupadas | ogrid_extract_occupied | 25 | 500 | 0.02 | 8870 | 9150 | 9850 | 10600 |
| extract: 3520 observadas, ninguna ocupada | ogrid_extract_occupied | 25 | 500 | 0.02 | 8880 | 9150 | 9880 | 10100 |
| extract: grid sin observar | ogrid_extract_occupied | 67 | 500 | 0.02 | 3180 | 3200 | 3530 | 3610 |

### Estimaciones por haz y por celda (a partir del maximo)

| Escenario | Celdas | K | max (ns/llamada) | max / celdas (ns/celda) |
|---|---:|---:|---:|---:|
| ray: 10 celdas sin eco | 10 | 3520 | 27.8 | 2.78 |
| ray: 10 celdas, eco en la ultima | 10 | 1408 | 33.5 | 3.35 |
| ray: 880 celdas sin eco | 880 | 40 | 2190 | 2.49 |
| ray: 880 celdas, eco en la ultima | 880 | 16 | 2310 | 2.62 |
| ray: 3520 celdas sin eco | 3520 | 10 | 10100 | 2.88 |
| ray: 3520 celdas, eco en la ultima | 3520 | 4 | 11900 | 3.37 |
| extract: 3520 observadas, mitad ocupadas | 3520 | 25 | 10200 | 2.89 |
| extract: 3520 observadas, todas ocupadas | 3520 | 25 | 10600 | 3.01 |
| extract: 3520 observadas, ninguna ocupada | 3520 | 25 | 10100 | 2.86 |
| extract: grid sin observar | 3520 | 67 | 3610 | 1.02 |

Acumuladores (evitan la eliminacion de llamadas): 4434855031.705857 119911728

## Ejecución 3

- Arquitectura: Darwin arm64 25.5.0
- Compilador: 16.2.0
- Reloj: clock_gettime_nsec_np(CLOCK_UPTIME_RAW)
- Resolucion medida del reloj (menor diferencia no nula entre lecturas consecutivas): 41 ns
- Repeticiones: 2000 (rapidas), 500 (con recorrido del grid, rayos largos), precalentamiento 50
- Lote K: llamadas entre dos lecturas del reloj, calibrado por escenario para que un lote dure al menos 200 us (salvo tope de K indicado en el informe); ns por llamada = tiempo del lote / K
- Cuant. (%): resolucion del reloj / duracion mediana del lote

| Escenario | Funcion | K | Repeticiones | Cuant. (%) | min (ns) | mediana (ns) | p99 (ns) | max (ns) |
|---|---|---:|---:|---:|---:|---:|---:|---:|
| reloj: lectura vacia | clock_gettime | 25032 | 2000 | 0.02 | 8.79 | 8.87 | 9.47 | 10.2 |
| reset: grid 3520 sucio | ogrid_reset_grid | 265 | 500 | 0.02 | 796 | 796 | 850 | 904 |
| prob: barrido [-0.8, 3.4] | ogrid_log_odds_to_prob | 128732 | 2000 | 0.02 | 1.75 | 1.80 | 1.94 | 2.03 |
| prob: l = -0.8 (minimo) | ogrid_log_odds_to_prob | 122970 | 2000 | 0.02 | 1.72 | 1.77 | 1.91 | 2.10 |
| prob: l = 0 (exacto 0.5) | ogrid_log_odds_to_prob | 122970 | 2000 | 0.02 | 1.72 | 1.77 | 1.92 | 2.06 |
| prob: l = 3.4 (maximo) | ogrid_log_odds_to_prob | 122970 | 2000 | 0.02 | 1.72 | 1.77 | 1.92 | 1.99 |
| prob: l = 5 (fuera de rango) | ogrid_log_odds_to_prob | 308815 | 2000 | 0.02 | 0.693 | 0.731 | 0.805 | 0.871 |
| clamp: por debajo (-5) | ogrid_clamp_log_odds | 193042 | 2000 | 0.03 | 0.784 | 0.824 | 1.34 | 14.6 |
| clamp: dentro (1.0) | ogrid_clamp_log_odds | 277268 | 2000 | 0.02 | 0.751 | 0.807 | 1.71 | 2.58 |
| clamp: por encima (10) | ogrid_clamp_log_odds | 284625 | 2000 | 0.02 | 0.755 | 0.781 | 1.05 | 1.26 |
| cell: eco desde 0.0 | ogrid_update_cell | 104008 | 2000 | 0.02 | 2.02 | 2.08 | 2.22 | 2.35 |
| cell: paso desde 0.0 | ogrid_update_cell | 104008 | 2000 | 0.02 | 2.02 | 2.08 | 2.34 | 2.45 |
| cell: oculta desde 0.0 | ogrid_update_cell | 158984 | 2000 | 0.02 | 1.12 | 1.38 | 1.64 | 2.38 |
| cell: eco desde 3.4 (saturada) | ogrid_update_cell | 93246 | 2000 | 0.02 | 2.02 | 2.08 | 2.26 | 3.22 |
| cell: eco desde 2.8 (satura) | ogrid_update_cell | 104008 | 2000 | 0.02 | 2.02 | 2.08 | 2.26 | 2.59 |
| cell: paso desde -0.8 (saturada) | ogrid_update_cell | 104008 | 2000 | 0.02 | 2.02 | 2.08 | 2.25 | 2.33 |
| cell: paso desde -0.5 (satura) | ogrid_update_cell | 104008 | 2000 | 0.02 | 2.02 | 2.08 | 2.25 | 2.40 |
| cell: eco en celda no observada | ogrid_update_cell | 104008 | 2000 | 0.02 | 2.02 | 2.08 | 2.25 | 2.40 |
| state: observada l=2.55 (ocupada) | ogrid_get_cell_state | 84502 | 2000 | 0.02 | 2.56 | 2.62 | 2.84 | 2.99 |
| state: observada l=1.70 (prob. baja) | ogrid_get_cell_state | 81921 | 2000 | 0.02 | 2.55 | 2.62 | 2.85 | 3.29 |
| state: no observada | ogrid_get_cell_state | 235157 | 2000 | 0.02 | 0.903 | 0.923 | 1.00 | 1.07 |
| state: corrupta (observed=2) | ogrid_get_cell_state | 300374 | 2000 | 0.02 | 0.687 | 0.709 | 0.769 | 0.851 |
| state: corrupta (l=9 observada) | ogrid_get_cell_state | 135223 | 2000 | 0.02 | 1.57 | 1.62 | 1.75 | 1.84 |
| ray: 10 celdas sin eco | ogrid_update_ray | 3520 | 2000 | 0.05 | 21.4 | 22.2 | 26.2 | 32.8 |
| ray: 10 celdas, eco en la ultima | ogrid_update_ray | 1408 | 2000 | 0.13 | 21.4 | 22.5 | 27.0 | 36.7 |
| ray: 880 celdas sin eco | ogrid_update_ray | 40 | 500 | 0.06 | 1720 | 1860 | 2010 | 2140 |
| ray: 880 celdas, eco en la ultima | ogrid_update_ray | 16 | 500 | 0.14 | 1720 | 1780 | 1790 | 2710 |
| ray: 3520 celdas sin eco | ogrid_update_ray | 10 | 500 | 0.06 | 6820 | 7030 | 7920 | 9050 |
| ray: 3520 celdas, eco en la ultima | ogrid_update_ray | 4 | 500 | 0.15 | 7010 | 7030 | 7900 | 8520 |
| extract: 3520 observadas, mitad ocupadas | ogrid_extract_occupied | 25 | 500 | 0.02 | 8880 | 9150 | 9870 | 10300 |
| extract: 3520 observadas, todas ocupadas | ogrid_extract_occupied | 25 | 500 | 0.02 | 8980 | 9150 | 9910 | 10100 |
| extract: 3520 observadas, ninguna ocupada | ogrid_extract_occupied | 25 | 500 | 0.02 | 8930 | 9150 | 9850 | 9870 |
| extract: grid sin observar | ogrid_extract_occupied | 67 | 500 | 0.02 | 3180 | 3240 | 3500 | 3580 |

### Estimaciones por haz y por celda (a partir del maximo)

| Escenario | Celdas | K | max (ns/llamada) | max / celdas (ns/celda) |
|---|---:|---:|---:|---:|
| ray: 10 celdas sin eco | 10 | 3520 | 32.8 | 3.28 |
| ray: 10 celdas, eco en la ultima | 10 | 1408 | 36.7 | 3.67 |
| ray: 880 celdas sin eco | 880 | 40 | 2140 | 2.43 |
| ray: 880 celdas, eco en la ultima | 880 | 16 | 2710 | 3.08 |
| ray: 3520 celdas sin eco | 3520 | 10 | 9050 | 2.57 |
| ray: 3520 celdas, eco en la ultima | 3520 | 4 | 8520 | 2.42 |
| extract: 3520 observadas, mitad ocupadas | 3520 | 25 | 10300 | 2.94 |
| extract: 3520 observadas, todas ocupadas | 3520 | 25 | 10100 | 2.87 |
| extract: 3520 observadas, ninguna ocupada | 3520 | 25 | 9870 | 2.80 |
| extract: grid sin observar | 3520 | 67 | 3580 | 1.02 |

Acumuladores (evitan la eliminacion de llamadas): 4193164439.132170 171386699

