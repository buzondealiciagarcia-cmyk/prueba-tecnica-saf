# Harness del proyecto — reglas para Claude

Este proyecto se desarrolla por fases con puertas (requisitos → diseño → implementación → oferta). El harness está instalado en `@@HARNESS@@` y su CLI es:

```
H="@@HARNESS@@/bin/h"     # h --help para ver todos los subcomandos
```

## Reglas que siempre se cumplen

1. **Lee los estándares antes de actuar**: índice en `@@HARNESS@@/standards/00-indice.md`. Cita los IDs de regla (p.ej. `C-FUN-01`, `GIT-BR-04`) en hallazgos y explicaciones.
2. **No inventes** requisitos, diseño, datos, precios ni plazos (PRO-A-02, DOC-GEN-01). Lo que propongas va marcado `[PROPUESTA]` hasta que el usuario lo acepte; lo que falte, `[PENDIENTE: pregunta]`.
3. **Pregunta ante cualquier duda** con `AskUserQuestion` (máx. 4 preguntas por ronda, opción recomendada primero y marcada "(Recomendado)"). No avances con supuestos silenciosos.
4. **Puertas de fase**: comprueba `$H gate require <fase>` antes de trabajar en una fase. Nunca cierres ni reabras una fase por iniciativa propia: solo el usuario con `/req-cerrar`, `/diseno-cerrar`, `/impl-cerrar`, `/oferta-cerrar`, `/fase-reabrir`.
5. **Git según STD-GIT**: trabaja siempre en la rama de la fase o de la función (`$H git start-phase <fase>`, `$H git start-feature F-NNN`); Conventional Commits en inglés con trailer `Refs:`; integra solo con `$H git finish` / `$H git release` (merge `--no-ff`). Nunca `--no-verify`, nunca commits en `main`/`develop`.
6. **Usa los scripts** del harness en lugar de rehacer a mano lo que ya hacen (exportar a Word, matrices, diagramas, tests, trazabilidad, presupuesto, PowerPoint, dosier).
7. **Idioma**: conversación y documentos en español; código, comentarios, commits y ramas en inglés.
8. **Estilo visual único** (STD-UI): paleta y tipografía de `@@HARNESS@@/tokens/tokens.json`; diagramas Mermaid en `docs/<fase>/diagramas/*.mmd` renderizados con `$H diagram render`.
9. **Implementación aislada** (STD-TST §1): tú orquestas; el código lo escribe solo el agente `implementador` y los tests solo `ingeniero-tests`, cada uno en su sandbox. Tú no escribes ni corriges código ni tests, y nunca pasas contenido de uno al otro.

## Dónde está cada cosa

| Qué | Dónde |
|---|---|
| Estado de fases | `.harness/state.json` (`$H gate status`) |
| Configuración del proyecto y de la plantilla Word | `harness.config.json`, `config/template-map.json` |
| Documentos de fase | `docs/01-requisitos`, `docs/02-diseno`, `docs/03-implementacion`, `docs/04-oferta`, `docs/dosier` |
| Exportaciones Word/PowerPoint | `docs/export/` (versionado; se actualiza al cerrar cada fase) |
| Informes de test | `.harness/reports/` · log versionado en `docs/03-implementacion/log-tests.md` |
