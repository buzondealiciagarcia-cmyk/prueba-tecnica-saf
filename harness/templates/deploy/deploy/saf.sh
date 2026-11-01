#!/usr/bin/env bash
# saf.sh — arranque/parada de la simulación (STD-DEP DEP-07, STD-SH)
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly SCRIPT_DIR
readonly COMPOSE_FILE="${SCRIPT_DIR}/docker-compose.yml"

usage() {
    cat <<USAGE
Uso: $(basename "$0") {up|down|status|logs|replay <fichero.jsonl>}
  up       construye y arranca broker, núcleo y visor
  down     detiene la simulación
  status   estado de los servicios
  logs     sigue los logs
  replay   arranca solo el visor reproduciendo una grabación (STD-DAT)
USAGE
}

compose() {
    SAF_VERSION="$(git -C "${SCRIPT_DIR}/.." describe --tags --always 2>/dev/null || echo dev)" \
        docker compose -f "${COMPOSE_FILE}" "$@"
}

main() {
    local cmd="${1:-}"
    case "${cmd}" in
        up) compose up -d --build ;;
        down) compose down ;;
        status) compose ps ;;
        logs) compose logs -f ;;
        replay)
            [[ $# -ge 2 ]] || { usage >&2; exit 2; }
            python3 "${SCRIPT_DIR}/../tools/ui_bridge.py" --replay "$2"
            ;;
        -h | --help) usage ;;
        *) usage >&2; exit 2 ;;
    esac
}

main "$@"
