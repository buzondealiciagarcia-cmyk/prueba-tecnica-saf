# Escenarios de medida de tiempos del módulo ogrid

Ejecutable: `tests/integration/bench_wcet_ogrid.c` (sin Unity, C11, Release -O2, enlazado con la biblioteca ogrid). Caja negra: solo la API pública de `include/ogrid/`. Contratos: sección 7 (rendimiento) de F-001 a F-007.

## Metodología

- Precalentamiento: 50 repeticiones sin registrar por escenario.
- Reloj: en macOS `clock_gettime_nsec_np(CLOCK_UPTIME_RAW)` (resolución de unos 41 ns); en el resto `clock_gettime(CLOCK_MONOTONIC)`; compilación condicional por `__APPLE__`. Motivo del cambio: con `CLOCK_MONOTONIC` en macOS el reloj avanza en saltos de 1 µs, del mismo orden que lo medido. El escenario "reloj: lectura vacía" da el coste de una lectura.
- Resolución real: al arrancar se mide la menor diferencia no nula entre lecturas consecutivas (mínimo de 2 000 cambios de tic) y se imprime en la cabecera de la salida.
- Lotes adaptativos (todos los escenarios): K se calibra por escenario con una pasada previa (el lote se duplica, o se escala con la medida cuando es significativa, hasta que dura al menos 200 µs; mejor de 3 intentos). Con ello el error de cuantización (resolución / duración del lote, columna "Cuant. (%)") es inferior al 1 % mientras el lote dure 200 µs o más con resolución de 41 ns (0,02 %). Los escenarios lentos (reset, haces de 880 y 3 520 celdas, extracción) quedan con K pequeño pero mayor que 1. Se divide por K para obtener ns por llamada.
- Restauración del estado: el grid se rellena antes de cada lote, fuera del cronómetro. Dentro de un lote no hace falta restaurar en `ogrid_reset_grid` (sobrescribe todas las celdas sin leerlas: el coste es el mismo desde un grid limpio), `ogrid_get_cell_state`, `ogrid_extract_occupied` (solo leen el grid) ni `ogrid_update_cell` (la celda se reescribe en cada llamada; dos escrituras incluidas en el tiempo). `ogrid_update_ray` modifica las celdas, así que el lote se limita para que ninguna celda sature (el recorte cambiaría la ruta medida): cada llamada del lote usa una ventana distinta de `3520 / celdas` ventanas disjuntas; los haces sin eco parten de l = 3,4 con votos de paso (hasta 10 pasadas por ventana sin llegar a −0,8) y los haces con eco parten de l = −0,8 en la última celda (hasta 4 pasadas sin llegar a 3,4; el resto de celdas, de paso, como en el caso sin eco). K máximo: haz de 10 celdas 3 520 (sin eco) y 1 408 (eco); 880 celdas 40 y 16; 3 520 celdas 10 y 4. Si el tope impide llegar a 200 µs, la columna "Cuant. (%)" informa del error real. Los incrementos de voto (+0,85 eco, −0,40 paso) son los de diseño.
- Repeticiones: 2 000 en las rápidas y en el haz de 10 celdas; 500 en las que recorren el grid (reset, haces de 880 y 3 520 celdas, extracción).
- Salida: K, repeticiones, cuantización y mínimo/mediana/p99/máximo por llamada con tres cifras significativas.
- Anti-eliminación: el resultado y el estado de cada llamada se acumulan en variables `volatile` que se imprimen; las entradas se leen de una variable `volatile`.
- Entradas fijas, sin aleatoriedad. Estadísticos por escenario: mínimo, mediana, percentil 99 (rango más cercano) y máximo, en ns por llamada.
- Estimaciones: ns por haz (máximo de cada escenario de `ogrid_update_ray`) y ns por celda (máximo dividido entre las celdas del haz, y entre 3 520 en `ogrid_extract_occupied`).

## Escenarios

| Escenario | Función | Entrada | Justificación |
|---|---|---|---|
| reloj: lectura vacía | clock_gettime | lectura del reloj | Línea base del cronómetro. |
| reset: grid 3 520 sucio | ogrid_reset_grid (F-001) | todas las celdas observadas con log-odds variados | O(3 520); el contenido previo no se lee, el peor caso es el recorrido completo con escritura de todas las celdas. |
| prob: barrido [−0,8; 3,4] | ogrid_log_odds_to_prob (F-002) | 256 valores equiespaciados con los dos extremos | Ruta con exponencial; el coste puede depender del argumento. |
| prob: l = −0,8 / 0 / 3,4 | ogrid_log_odds_to_prob | extremos y valor central (0 da exactamente 0,5) | Límites del rango válido. |
| prob: l = 5 (fuera de rango) | ogrid_log_odds_to_prob | valor fuera de rango | Ruta de error ERR_RANGE; referencia de que no es más cara que la válida. |
| clamp: por debajo, dentro, por encima | ogrid_clamp_log_odds (F-003) | −5; 1,0; 10 | Tres ramas de las dos comparaciones. |
| cell: eco / paso / oculta desde 0,0 | ogrid_update_cell (F-004) | celda observada con l = 0 | Los tres votos sin saturación. |
| cell: eco desde 3,4; paso desde −0,8 | ogrid_update_cell | celdas saturadas | Voto que no cambia el valor tras el recorte. |
| cell: eco desde 2,8; paso desde −0,5 | ogrid_update_cell | suma que cruza el límite | Recorte efectivo (clamp activo). |
| cell: eco en celda no observada | ogrid_update_cell | observed = 0 | Primera observación: además se marca la celda. |
| state: observada l = 2,55 / 1,70 | ogrid_get_cell_state (F-005) | celda observada con log-odds válido | Ruta con exponencial; ocupada y de probabilidad baja (comparación con el umbral 0,85). |
| state: no observada | ogrid_get_cell_state | observed = 0 | Ruta corta, referencia. |
| state: corrupta (observed = 2; l = 9 observada) | ogrid_get_cell_state | dos tipos de corrupción | Rutas de ERR_STATE (una antes de la exponencial, otra tras validar el log-odds). |
| ray: 10, 880, 3 520 celdas sin eco | ogrid_update_ray (F-006) | celdas consecutivas (ventana distinta por llamada), observadas con l = 3,4, todas con voto de paso | O(celdas) con una llamada a F-004 por celda; 3 520 es el haz máximo. Sin saturación dentro del lote. |
| ray: 10, 880, 3 520 celdas, eco en la última | ogrid_update_ray | idem con eco en `cell_count − 1` (última celda de la ventana con l = −0,8) | Sin celdas ocultas, todas se leen: el máximo trabajo. Sin saturación dentro del lote. |
| extract: 3 520 observadas, mitad / todas / ninguna ocupadas | ogrid_extract_occupied (F-007) | grid observado y válido (l = 2,55 y/o 1,70) | Peor caso: 3 520 llamadas a F-005 por la ruta con exponencial; las tres mezclas cubren las dos salidas de la comparación. |
| extract: grid sin observar | ogrid_extract_occupied | observed = 0 en todas | Ruta corta, referencia inferior. |

## Notas

- Las rutas de error de PARAM/RANGE (puntero nulo, índice fuera de rango) son O(1) y no se miden salvo `prob l = 5`; el peor caso de cada contrato es la ruta de éxito.
- El WCET objetivo de F-007 (menor de 1 ms en la ECU) se compara con el máximo de los escenarios `extract`. Las cifras son de la máquina de medida, no de la ECU; sirven para estimar y no sustituyen la medida en destino.
