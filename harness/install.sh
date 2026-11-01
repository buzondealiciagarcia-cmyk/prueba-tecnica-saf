#!/usr/bin/env bash
# install.sh — despliega el harness en <destino>/.claude (STD-SH)
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly SCRIPT_DIR

usage() {
    cat <<USAGE
Uso: $(basename "$0") --target <ruta> [--update | --uninstall] [--force] [--dry-run]

  --target <ruta>  carpeta donde se creará/usará .claude (p.ej. '.', '~' o '/ruta/proyecto/.claude')
  --update         actualiza una instalación existente (respeta ficheros modificados)
  --uninstall      elimina lo instalado (respeta ficheros modificados salvo --force)
  --force          sobrescribe o borra aunque existan ficheros ajenos o modificados
  --dry-run        muestra lo que haría sin escribir nada

Ejemplos:
  $(basename "$0") --target .            # instala en ./.claude del proyecto
  $(basename "$0") --target ~            # instala para el usuario en ~/.claude
  $(basename "$0") --target . --update
USAGE
}

main() {
    if [[ $# -eq 0 ]]; then
        usage >&2
        exit 2
    fi
    case "${1:-}" in
        -h | --help)
            usage
            exit 0
            ;;
    esac
    if ! command -v python3 >/dev/null 2>&1; then
        echo "install.sh: python3 es necesario (xcode-select --install)" >&2
        exit 3
    fi
    python3 "${SCRIPT_DIR}/scripts/install.py" "$@"
}

main "$@"
