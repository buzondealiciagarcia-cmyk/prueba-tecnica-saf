# STD-CFG · Configuración y parámetros calibrables

| Campo | Valor |
|---|---|
| Versión | 1.0.0 |
| Ámbito | Todo parámetro que pueda cambiar sin recompilar: tamaño y resolución del grid, probabilidades del modelo inverso del sensor, umbrales, timeouts, geometría de la vía |

| ID | Nivel | Regla |
|---|---|---|
| CFG-01 | OBL | Un único fichero versionado `config/ogrid.json` (o el que fije el diseño) + su esquema `config/ogrid.schema.json` con tipo, unidad, rango `[min, max]`, valor por defecto, requisito y descripción de cada parámetro. |
| CFG-02 | OBL | La configuración se valida completa al arrancar (rangos, coherencias cruzadas, p.ej. `p_occ > p_free`). Si es inválida, el sistema no arranca en modo operativo y lo registra en `CRIT`. Nunca se usa un valor por defecto en silencio. |
| CFG-03 | OBL | En C, la configuración validada se copia a una estructura `const` (`ogrid_config_t`) que se pasa explícitamente al núcleo; el núcleo no lee ficheros. |
| CFG-04 | OBL | Límites de compilación (tamaño máximo del grid, máximo de ecos por nube) son macros en `include/<mod>/<mod>_limits.h`; la configuración solo puede elegir valores ≤ a esos límites. |
| CFG-05 | OBL | Cambios en caliente solo para parámetros marcados `hot: true` en el esquema, por el topic de configuración (QoS 1), validados igual que al arranque y registrados en `NOTICE`. |
| CFG-06 | OBL | El hash del fichero de configuración activo se incluye en el topic de estado y en los logs de arranque (trazabilidad de qué parámetros produjeron un resultado). |
