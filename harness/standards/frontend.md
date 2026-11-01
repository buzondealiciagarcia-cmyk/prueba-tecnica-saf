# STD-UI · Estándar visual y de front-end

| Campo | Valor |
|---|---|
| Versión | 1.0.0 |
| Ámbito | UI web, diagramas, gráficas, documentos Word y presentaciones PowerPoint |
| Fuente de verdad | `harness/tokens/tokens.json` (colores, tipografía, espaciado). `tokens.css` y `mermaid-config.json` derivan de él |
| Kit de componentes | `harness/templates/frontend/` |

## 1. Principios

1. **Minimalista**: blanco y grises; un único color de acento (azul señal). Sin degradados, sombras ni iconos decorativos.
2. **El color significa algo**: acento = interacción/selección/dato principal; verde/ámbar/rojo = estado (OK/aviso/error) y nunca decoración.
3. **Una sola paleta en todos los soportes**: si un valor no está en `tokens.json`, no se usa.
4. **Legible antes que bonito**: contraste WCAG AA (≥ 4.5:1 en texto), tamaño mínimo 12 px en UI y 9 pt en documentos.

## 2. Paleta

| Token | Claro | Oscuro | Uso |
|---|---|---|---|
| `background` | `#FFFFFF` | `#14191F` | fondo de página, diapositiva, documento |
| `surface` | `#F5F6F8` | `#1C232B` | paneles, filas alternas de tabla, nodos de diagrama |
| `border` | `#D9DCE1` | `#2E3842` | bordes de 1 px, rejillas, separadores |
| `text` | `#1F2933` | `#E6E9ED` | texto principal, títulos |
| `textMuted` | `#616E7C` | `#9AA5B1` | texto secundario, ejes, pies de figura |
| `accent` | `#1F5FAD` | `#5B8FD6` | enlaces, selección, cabecera de tabla, serie principal |
| `accentSoft` | `#8FB3E0` | `#2C4A73` | serie secundaria, bordes de notas |
| `accentTint` | `#E8F0FA` | `#1A2A3F` | resaltado suave, notas, fila seleccionada |
| `ok` / `warning` / `error` | `#2E7D4F` / `#B7791F` / `#B23A3A` | `#4FA374` / `#D69E3A` / `#D46363` | estados |

**Grid de ocupación**: escala continua `gridFree → gridOccupied` interpolada en espacio lineal sobre la probabilidad; celdas desconocidas (p = 0,5 sin observaciones) en `gridUnknown`; obstáculo confirmado: borde `error` de 2 px. Leyenda obligatoria con valores 0 · 0,5 · 1. Series de datos en gráficas: orden fijo `series[]` de `tokens.json`.

## 3. Tipografía

| Rol | UI | Word | PowerPoint |
|---|---|---|---|
| Texto | Inter 14 px | Inter → Calibri 10,5 pt | Inter → Calibri 16 pt |
| Título 1 / 2 / 3 | 24 / 18 / 14 px, 600 | 18 / 14 / 12 pt | 30 pt título de diapositiva |
| Código, IDs, valores numéricos | JetBrains Mono 13 px | JetBrains Mono → Consolas 9 pt | JetBrains Mono → Consolas 12 pt |

Números en tablas alineados a la derecha y con `font-variant-numeric: tabular-nums`. Decimales con coma en documentos en español, punto en UI técnica y código.

## 4. Espaciado y forma

Escala de 4 px (`xs 4 · sm 8 · md 16 · lg 24 · xl 40`). Radio 4 px (controles) y 6 px (paneles). Bordes de 1 px `border`. Sin sombras.

## 5. Componentes del kit (`templates/frontend/components.css`)

| Componente | Clase | Notas |
|---|---|---|
| Barra superior | `.ui-topbar` | título del sistema a la izquierda, estado de conexión a la derecha |
| Panel | `.ui-panel` + `.ui-panel__title` | contenedor de cada vista |
| Botón | `.ui-btn`, `.ui-btn--primary`, `.ui-btn--ghost` | solo un botón primario por vista |
| Indicador de estado | `.ui-status--ok/--warn/--err/--idle` | punto de 8 px + texto; nunca solo color |
| Tabla | `.ui-table` | cabecera `surface`, filas separadas por `border`, números en mono |
| Métrica | `.ui-kpi` | valor grande en mono + etiqueta `textMuted` + unidad |
| Control deslizante | `.ui-slider` | parámetros calibrables, muestra valor y unidad |
| Leyenda | `.ui-legend` | obligatoria en grid, nube de puntos y gráficas |
| Lienzo | `.ui-canvas` | `<canvas>` del grid/nube; fondo `background`, rejilla `border` |

## 6. Reglas técnicas del front-end

| ID | Nivel | Regla |
|---|---|---|
| UI-TEC-01 | OBL | HTML + JavaScript vanilla (ES2020 módulos) + Canvas 2D. Sin frameworks ni paso de build. |
| UI-TEC-02 | OBL | Los colores se leen de variables CSS (`getComputedStyle`) también en Canvas; prohibido hex en JS o CSS de componentes. |
| UI-TEC-03 | OBL | La UI solo consume datos; nunca envía órdenes al núcleo que afecten a la seguridad. Los cambios de parámetros pasan por el canal de configuración validado (STD-CFG). |
| UI-TEC-04 | OBL | Datos en vivo por Server-Sent Events (`EventSource`) desde el puente `tools/ui_bridge.py` (MQTT → SSE, solo biblioteca estándar + paho-mqtt). SSE es unidireccional, coherente con UI-TEC-03. Modo replay desde ficheros de STD-DAT. |
| UI-TEC-05 | OBL | Tema claro por defecto; oscuro con `data-theme="dark"` en `<html>`, conmutable y recordado en `localStorage` (con try/catch). |
| UI-TEC-06 | OBL | Refresco del canvas con `requestAnimationFrame`, desacoplado de la llegada de datos (se pinta el último frame). |
| UI-TEC-07 | REC | Accesible por teclado; `aria-label` en controles sin texto. |

## 7. Diagramas (Mermaid)

| ID | Nivel | Regla |
|---|---|---|
| UI-DIA-01 | OBL | Fuente en `docs/<fase>/diagramas/<nombre>.mmd`; se renderizan con `h diagram render` (aplica `mermaid-config.json`) a `.svg` (markdown) y `.png` a 3× (Word/PowerPoint). |
| UI-DIA-02 | OBL | Prohibido fijar colores dentro del `.mmd` salvo con las clases del harness: `classDef hl` (acento), `classDef warn`, `classDef err`, `classDef muted` (definidas por `h diagram` automáticamente). |
| UI-DIA-03 | OBL | Un diagrama = una idea; ≤ 15 nodos. Etiquetas en español, IDs de requisitos/componentes en mono. |
| UI-DIA-04 | OBL | Cada figura en markdown: `![Figura N. Descripción](diagramas/x.svg)`; el exportador numera y titula. |

## 8. Documentos y presentaciones

- Word: estilos de la plantilla registrada (`h template register`); tablas con el formato de la tabla de referencia de la plantilla; si no la hay, cabecera `accent` con texto blanco, filas alternas `surface`, bordes `border`.
- PowerPoint 16:9: fondo blanco, título arriba a la izquierda en `text`, barra de acento de 4 px bajo el título, máx. 6 viñetas, numeración de diapositiva y nombre del proyecto en el pie en `textMuted`.
