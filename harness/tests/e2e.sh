#!/usr/bin/env bash
# e2e.sh — recorrido completo de las 4 fases en un proyecto temporal (STD-SH).
# Simula las entregas de los agentes con ficheros de tests/fixtures/e2e.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly SCRIPT_DIR
readonly H="${SCRIPT_DIR}/../bin/h"
readonly FX="${SCRIPT_DIR}/fixtures/e2e"

step() {
    printf '\n=== %s ===\n' "$*"
}

expect_fail() {
    if "$@"; then
        echo "se esperaba fallo: $*" >&2
        exit 1
    fi
}

TMP_DIR=""

cleanup() {
    if [[ -n "${TMP_DIR}" && -z "${H_KEEP:-}" ]]; then
        rm -rf "${TMP_DIR}"
    fi
}

main() {
    local tmp
    tmp="$(mktemp -d)"
    TMP_DIR="${tmp}"
    trap cleanup EXIT
    cd "${tmp}"
    git init -q -b main
    git config user.email e2e@example.com
    git config user.name E2E
    echo "# e2e" >README.md
    git add README.md
    git commit -qm "chore: init" --no-verify

    step "init"
    "${H}" init --name "SAF e2e" --code SAF --author "E2E" --module ogrid

    step "fase 1 · requisitos"
    "${H}" git start-phase req
    cp "${FX}/requisitos.md" docs/01-requisitos/requisitos.md
    git add -A
    git commit -qm "docs(req): add requirements"
    "${H}" gate close req
    "${H}" export requisitos

    step "fase 2 · diseño"
    "${H}" git start-phase design
    "${H}" scaffold design
    mkdir -p docs/02-diseno/diagramas docs/02-diseno/alternativas
    cp "${FX}/diseno.md" "${FX}/revision.md" "${FX}/comparativa.json" docs/02-diseno/
    cp "${FX}/arquitectura-sw.mmd" docs/02-diseno/diagramas/
    cp "${FX}/ALT-01-fusion.md" docs/02-diseno/alternativas/
    "${H}" diagram render
    "${H}" matrix render
    git add -A
    git commit -qm "docs(design): add design and comparison"
    "${H}" gate close design
    "${H}" export diseno
    "${H}" export comparativa

    step "fase 3 · implementación"
    "${H}" git start-phase impl
    "${H}" scaffold impl
    "${H}" scaffold c-project
    cp "${FX}/plan.json" docs/03-implementacion/plan.json
    "${H}" impl validate
    git add -A
    git commit -qm "docs(impl): add implementation plan"
    "${H}" git finish
    "${H}" git start-feature F-001
    mkdir -p docs/03-implementacion/contratos
    cp "${FX}/contrato.md" docs/03-implementacion/contratos/F-001-ogrid_clamp_prob.md
    cp "${FX}/ogrid.h" include/ogrid/ogrid.h
    git add -A
    git commit -qm "docs(impl): add contract for F-001"
    "${H}" sandbox prepare F-001
    local sb=.harness/sandbox/current
    mkdir -p "${sb}/test/out/tests/unit" "${sb}/impl/out/src/ogrid"
    cp "${FX}/test_ogrid_clamp_prob.c" "${sb}/test/out/tests/unit/"
    cp "${FX}/impl_bad.c" "${sb}/impl/out/src/ogrid/ogrid_clamp_prob.c"
    clang-format -i "${sb}/test/out/tests/unit/test_ogrid_clamp_prob.c" \
        "${sb}/impl/out/src/ogrid/ogrid_clamp_prob.c"
    "${H}" sandbox check-impl
    "${H}" sandbox collect test
    "${H}" sandbox collect impl
    expect_fail "${H}" test F-001
    grep -q "TC-F001-02" "${sb}/impl/feedback/feedback-1.md"
    if grep -q "TEST_ASSERT" "${sb}/impl/feedback/feedback-1.md"; then
        echo "el feedback del implementador contiene código de test" >&2
        exit 1
    fi
    expect_fail "${H}" git finish
    cp "${FX}/impl_good.c" "${sb}/impl/out/src/ogrid/ogrid_clamp_prob.c"
    clang-format -i "${sb}/impl/out/src/ogrid/ogrid_clamp_prob.c"
    "${H}" sandbox collect impl -m "clamp probabilities above one"
    "${H}" log change F-001 --agent implementador --cause "p>1 sin saturar" --desc "rama p > 1"
    "${H}" test F-001
    "${H}" git finish
    "${H}" sandbox clean
    "${H}" git start test/integration
    "${H}" test --all
    "${H}" git finish
    "${H}" git release --tag v1.0.0 --message "e2e"
    "${H}" git start-phase impl
    "${H}" trace
    git add -A
    git commit -qm "docs(impl): add traceability"
    "${H}" gate close impl
    "${H}" export implementacion

    step "fase 4 · oferta"
    "${H}" git start-phase offer
    "${H}" scaffold offer
    cp "${FX}/presupuesto.json" "${FX}/presentacion.json" docs/04-oferta/
    "${H}" offer budget
    local s
    for s in objeto introduccion datos objetivos alcance requisitos descripcion presupuesto plazo; do
        "${H}" offer section "${s}" aprobado
    done
    sed -i.bak 's/\[PENDIENTE\]/E2E/g' docs/04-oferta/oferta.md && rm docs/04-oferta/oferta.md.bak
    "${H}" pptx build
    "${H}" export oferta
    git add -A
    git commit -qm "docs(offer): write offer"
    "${H}" gate close offer
    "${H}" scaffold dossier
    echo "# Contexto del proyecto

Prueba e2e." >docs/dosier/contexto.md
    "${H}" dossier build
    "${H}" export dosier

    step "resultado"
    "${H}" gate status
    git tag
    git log --oneline --graph --all -n 40
    ls docs/export
}

main "$@"
