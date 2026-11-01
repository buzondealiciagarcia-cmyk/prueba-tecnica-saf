---
name: diagramas
description: Crea diagramas Mermaid limpios y minimalistas con la paleta del proyecto (arquitectura, secuencia, estados, flujo, Gantt, barras). Úsala siempre que un documento necesite un esquema.
---
# Diagramas (STD-UI §7)

CLI: `H="@@HARNESS@@/bin/h"`

1. Fuente en `docs/<carpeta de fase>/diagramas/<nombre-kebab>.mmd`. Una idea por diagrama, ≤ 15 nodos, etiquetas en español, identificadores (topics, funciones, IDs de requisito) tal cual.
2. **Sin colores ni estilos inline.** El tema (`@@HARNESS@@/tokens/mermaid-config.json`) aplica la paleta. Para resaltar en flowcharts usa las clases que el harness inyecta: `:::hl` (elemento principal), `:::warn`, `:::err` (estado seguro, fallo), `:::muted` (externo / fuera de alcance).
3. Renderiza: `$H diagram render [ruta]` → `.svg` con hash de la fuente (el cierre de fase comprueba que estén al día). `$H diagram check` lista los desactualizados.
4. En markdown: `![Figura. Descripción](diagramas/<nombre>.svg)`. El exportador numera y convierte a PNG.
5. Revisa el SVG/PNG (Read sobre el PNG de `build/export-cache/` o el SVG) y corrige solapes.

| Para | Tipo | Notas |
|---|---|---|
| Bloques HW/SW | `flowchart LR` + `subgraph` | subgraph por nodo físico o capa |
| Mensajes entre nodos | `sequenceDiagram` | `autonumber`, un participante por nodo |
| Modos / estado seguro | `stateDiagram-v2` | estado seguro con nota |
| Algoritmo por ciclo | `flowchart TB` | decisiones como rombos `{}` |
| Estructuras de datos | `classDiagram` | campos con unidad |
| Planificación | `gantt` | lo genera `$H offer budget` |
| Comparativas | `xychart-beta` | lo genera `$H matrix render` |

Ejemplo:
```
flowchart LR
  subgraph Tranvía
    R[Radar frontal] -->|nube de puntos · 50 ms| P[Unidad de proceso]:::hl
  end
  P -->|grid de ocupación| B[(Broker MQTT)]
  P -.->|fallo| S[Estado seguro]:::err
```
