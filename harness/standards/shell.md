# STD-SH · Shell scripting (y herramientas Python)

| Campo | Valor |
|---|---|
| Versión | 1.0.0 |
| Intérprete | Bash ≥ 3.2 (el de macOS); también debe funcionar en Bash 5 de Linux |
| Verificación | `shellcheck -S style` sin hallazgos (hook `pre-commit` y autotests del harness) |

## 1. Bash

| ID | Nivel | Regla |
|---|---|---|
| SH-01 | OBL | Shebang `#!/usr/bin/env bash` y `set -euo pipefail` en la primera línea ejecutable; `IFS=$'\n\t'` si se iteran rutas. |
| SH-02 | OBL | Compatible con Bash 3.2: prohibidos `declare -A`, `mapfile/readarray`, `${var,,}`, `${var^^}`, `&>>`, `coproc`, `wait -n`. |
| SH-03 | OBL | Todo script tiene `usage()` y responde a `-h/--help`; argumentos con `while/case`, nunca posicionales ambiguos. |
| SH-04 | OBL | Variables entre comillas `"${var}"`; locales con `local`; constantes en `readonly UPPER_CASE`. |
| SH-05 | OBL | Funciones `snake_case`, un `main "$@"` al final; nada se ejecuta al hacer `source`. |
| SH-06 | OBL | Rutas resueltas respecto al script (`SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"`), nunca respecto al directorio actual. |
| SH-07 | OBL | Ficheros temporales con `mktemp` y limpieza con `trap '…' EXIT`. |
| SH-08 | OBL | Errores a `stderr` con el formato de STD-LOG y códigos de salida: `0` OK, `1` fallo de comprobación, `2` uso incorrecto, `3` dependencia ausente. |
| SH-09 | OBL | Herramientas portables macOS/Linux: nada de `sed -i` sin sufijo, `readlink -f`, `date -d`, `grep -P`; si hace falta, se usa Python. |
| SH-10 | OBL | Operaciones destructivas (`rm`, `git reset`, sobrescrituras) con `--dry-run` disponible y confirmación o `--force` explícito. |

## 2. Python (herramientas y harness)

| ID | Nivel | Regla |
|---|---|---|
| PY-01 | OBL | Python ≥ 3.9 (el de macOS): sin `match`, sin `X | Y` en anotaciones evaluadas. |
| PY-02 | OBL | Solo biblioteca estándar salvo `python-docx`, `python-pptx` y `paho-mqtt`, comprobadas por `h doctor`. |
| PY-03 | OBL | `argparse` con ayuda; códigos de salida como SH-08; sin efectos al importar. |
| PY-04 | OBL | PEP 8, nombres `snake_case`, docstring por módulo y función pública. |
