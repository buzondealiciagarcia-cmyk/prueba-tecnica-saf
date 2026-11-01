# STD-TST · Estándar de verificación y tests

| Campo | Valor |
|---|---|
| Versión | 1.0.0 |
| Ámbito | Tests unitarios, de integración y de estándares de la fase de implementación |
| Herramientas | Unity (+ CMock), CTest, gcovr, cppcheck-misra, clang-tidy, clang-format, lizard, ASan/UBSan |
| Ejecución | `h test <F-NNN>` (una función) · `h test --all` (integración completa) |

## 1. Independencia entre implementación y test

| ID | Nivel | Regla |
|---|---|---|
| TST-IND-01 | OBL | El código funcional lo escribe **solo** el agente `implementador`; los tests **solo** el agente `ingeniero-tests`. El orquestador no escribe ni corrige ninguno de los dos. |
| TST-IND-02 | OBL | Ambos agentes parten exclusivamente del **contrato** (`docs/03-implementacion/contratos/F-NNN-*.md`), la cabecera pública, los requisitos y el diseño cerrados, y los estándares. |
| TST-IND-03 | OBL | El `ingeniero-tests` nunca ve `src/` ni la salida de cobertura; el `implementador` nunca ve `tests/`. Lo garantizan los sandboxes (`.harness/sandbox/current/{test,impl}`), el hook `guard_paths.py` y un fichero canario que cada agente debe comprobar bloqueado antes de trabajar. |
| TST-IND-04 | OBL | Ante un fallo, el implementador recibe solo: ID del caso, requisito y regla, esperado vs. obtenido (`feedback-N.md`). Nunca código del test. |
| TST-IND-05 | OBL | Si el implementador cree que un test contradice el contrato, abre una **disputa** (`DISPUTA: TC-… motivo`). El orquestador la resuelve contra el contrato; si el contrato es ambiguo, pregunta al usuario. Nunca se cambia un test para "hacerlo pasar". |

## 2. Diseño de los casos de test

| ID | Nivel | Regla |
|---|---|---|
| TST-CAS-01 | OBL | Cada caso tiene ID `TC-<F-NNN>-<NN>` y se nombra `test_TC_F003_04_<descripcion>()`. |
| TST-CAS-02 | OBL | Cada caso declara en comentario `@req` (requisitos) y `@contract` (cláusula del contrato: `pre`, `post`, `err`, `perf`). |
| TST-CAS-03 | OBL | Cobertura del contrato: al menos un caso por postcondición, por código de error, por límite de cada rango (mín, máx, mín−ε, máx+ε) y por cada parámetro puntero nulo. |
| TST-CAS-04 | OBL | Técnicas mínimas: particiones de equivalencia, valores límite, y para lógica probabilística: casos con resultado analítico conocido (p.ej. N actualizaciones de log-odds con valor exacto) y propiedades (monotonía, acotación en [0,1], simetría). |
| TST-CAS-05 | OBL | Mensajes de aserción con formato `"[TC-F003-04][SYS-PR-02] <qué se esperaba>"` usando `TEST_ASSERT_*_MESSAGE`. Es lo único que el runner extrae para el feedback. |
| TST-CAS-06 | OBL | Tolerancias de coma flotante explícitas y justificadas en el caso (`TEST_ASSERT_FLOAT_WITHIN(1e-5F, …)`). |
| TST-CAS-07 | OBL | Tests deterministas: sin reloj, sin aleatoriedad no sembrada, sin red, sin orden implícito entre casos (`setUp`/`tearDown` reinician todo). |
| TST-CAS-08 | REC | Datos de entrada grandes (nubes de puntos) desde `tests/data/` en el formato de STD-DAT. |

## 3. Criterios de aceptación de una función (gate de integración en `develop`)

Una función solo se integra si `h test F-NNN` devuelve **PASS** en todos los checks:

| Check | Herramienta | Criterio |
|---|---|---|
| build | CMake preset `host-test` | compila con C-LNG-02 sin avisos |
| unit | Unity + CTest | 100 % de casos pasan, con ASan/UBSan |
| coverage | gcovr | 100 % líneas y 100 % ramas del fichero de la función |
| misra | cppcheck `--addon=misra` | 0 hallazgos no registrados como desviación |
| tidy | clang-tidy | 0 hallazgos |
| format | clang-format | 0 diferencias |
| complexity | lizard | CC ≤ 10, ≤ 60 líneas, ≤ 5 parámetros |
| doxygen | `h test` (parser) | C-DOC-01/02 cumplidos en cabecera y fuente |
| forbidden | `h test` (parser) | 0 símbolos de C-LIB-01/02 y 0 marcadores C-DOC-05 |

## 4. Integración completa (gate de `main`)

`h test --all` en `develop`: todos los checks anteriores sobre todo el árbol, más los tests de `tests/integration/` (caja negra, escritos por `ingeniero-tests` desde requisitos), cobertura global 100 % líneas/ramas del núcleo y build cruzado `cross-aarch64` si el toolchain está disponible.

## 5. Registro (log de tests)

| ID | Nivel | Regla |
|---|---|---|
| TST-LOG-01 | OBL | Cada ejecución de `h test` añade una entrada a `docs/03-implementacion/log-tests.md`: fecha, función, iteración, rama, commit, resultado por check y casos fallados (ID, requisito, esperado/obtenido). |
| TST-LOG-02 | OBL | Cada corrección posterior a un fallo se registra con `h log change F-NNN "<descripción>"` indicando agente, causa raíz y commit. El log es append-only. |
| TST-LOG-03 | OBL | Los informes JSON completos quedan en `.harness/reports/F-NNN/run-N.json` (no versionados). |
| TST-LOG-04 | OBL | Máximo 5 iteraciones fallidas por función; en la 6.ª el orquestador se detiene y consulta al usuario con el historial. |
