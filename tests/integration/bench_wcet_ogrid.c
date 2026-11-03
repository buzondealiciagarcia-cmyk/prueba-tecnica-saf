/**
 * @file bench_wcet_ogrid.c
 * @brief Empirical timing (WCET estimate) of every public ogrid function, black box.
 * @details Release build, no Unity. Measures the time per call with worst-case inputs deduced from
 * the contracts (section 7) and prints a Markdown table on stdout. No random input, no arguments.
 */
#define _POSIX_C_SOURCE 200809L

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <time.h>

#include "ogrid/ogrid.h"
#include "ogrid/ogrid_clamp_log_odds.h"
#include "ogrid/ogrid_extract_occupied.h"
#include "ogrid/ogrid_get_cell_state.h"
#include "ogrid/ogrid_log_odds_to_prob.h"
#include "ogrid/ogrid_reset_grid.h"
#include "ogrid/ogrid_update_cell.h"
#include "ogrid/ogrid_update_ray.h"

/** Samples (timed repetitions) for the fast scenarios. */
#define REPS_FAST (2000U)
/** Samples for the scenarios that walk the grid. */
#define REPS_GRID (500U)
/** Calls between two clock reads for the very cheap functions. */
#define BATCH_FAST (256U)
/** Warm-up repetitions (not recorded). */
#define WARMUP_REPS (50U)
/** Maximum number of scenarios. */
#define MAX_SCENARIOS (48U)
/** Maximum samples per scenario. */
#define MAX_REPS (2000U)
/** Number of points of the log-odds sweep. */
#define SWEEP_POINTS (256U)

/** How the grid is filled before each timed repetition (untimed). */
typedef enum
{
    FILL_NONE = 0,
    FILL_UNIFORM,   /**< every cell log-odds = fill_lo, observed = fill_obs */
    FILL_ALTERNATE, /**< observed; even cells l = 2.55 (occupied), odd cells l = 1.70 */
    FILL_DIRTY      /**< observed with assorted valid log-odds, to be reset */
} fill_mode_t;

/** Scenario description. */
typedef struct scenario
{
    const char *name;
    const char *function;
    uint32_t reps;
    uint32_t batch;
    fill_mode_t fill;
    ogrid_real_t fill_lo;
    uint8_t fill_obs;
    ogrid_real_t arg_lo;    /**< log-odds argument or initial cell value */
    ogrid_vote_kind_t vote; /**< vote for update_cell */
    uint32_t cells;         /**< ray length */
    bool echo;              /**< ray with echo on the last cell */
    void (*run)(const struct scenario *sc, uint32_t k);
} scenario_t;

/** Result of one scenario. */
typedef struct
{
    double min_ns;
    double median_ns;
    double p99_ns;
    double max_ns;
    uint32_t reps;
    uint32_t batch;
} result_t;

static ogrid_t g_grid;
static bool g_mask[OGRID_CELL_COUNT];
static uint32_t g_ray_cells[OGRID_CELL_COUNT];
static ogrid_real_t g_sweep[SWEEP_POINTS];
static double g_samples[MAX_REPS];

/** Accumulators printed at the end so the calls cannot be optimised away. */
static volatile double g_sink_d;
static volatile uint32_t g_sink_u;
/** Volatile input source, so the compiler cannot hoist constant arguments. */
static volatile ogrid_real_t g_in_lo;

static uint64_t now_ns(void)
{
    struct timespec ts;
    (void)clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((uint64_t)ts.tv_sec * 1000000000ULL) + (uint64_t)ts.tv_nsec;
}

/* ---- fills (untimed) ---------------------------------------------------------------------- */

static void fill_grid(const scenario_t *sc)
{
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        switch (sc->fill)
        {
            case FILL_UNIFORM:
                g_grid.cells[i].log_odds_lo = sc->fill_lo;
                g_grid.cells[i].observed = sc->fill_obs;
                break;
            case FILL_ALTERNATE:
                g_grid.cells[i].log_odds_lo = ((i % 2U) == 0U) ? 2.55F : 1.70F;
                g_grid.cells[i].observed = (uint8_t)OGRID_OBSERVED_YES;
                break;
            case FILL_DIRTY:
                g_grid.cells[i].log_odds_lo = (0.5F * (ogrid_real_t)(i % 5U)) - 0.5F;
                g_grid.cells[i].observed = (uint8_t)OGRID_OBSERVED_YES;
                break;
            case FILL_NONE:
            default:
                break;
        }
    }
}

/* ---- run functions: execute k calls ---------------------------------------------------------- */

static void run_empty_clock(const scenario_t *sc, uint32_t k)
{
    (void)sc;
    uint64_t acc = 0U;
    for (uint32_t j = 0U; j < k; j++)
    {
        acc += now_ns();
    }
    g_sink_u = g_sink_u + (uint32_t)(acc & 0xFFU);
}

static void run_reset(const scenario_t *sc, uint32_t k)
{
    (void)sc;
    uint32_t acc = 0U;
    for (uint32_t j = 0U; j < k; j++)
    {
        acc += (uint32_t)ogrid_reset_grid(&g_grid);
    }
    g_sink_u = g_sink_u + acc;
}

static void run_prob_sweep(const scenario_t *sc, uint32_t k)
{
    (void)sc;
    double acc = 0.0;
    uint32_t st = 0U;
    for (uint32_t j = 0U; j < k; j++)
    {
        ogrid_real_t p = 0.0F;
        g_in_lo = g_sweep[j % SWEEP_POINTS];
        st += (uint32_t)ogrid_log_odds_to_prob(g_in_lo, &p);
        acc += (double)p;
    }
    g_sink_d = g_sink_d + acc;
    g_sink_u = g_sink_u + st;
}

static void run_prob_fixed(const scenario_t *sc, uint32_t k)
{
    double acc = 0.0;
    uint32_t st = 0U;
    for (uint32_t j = 0U; j < k; j++)
    {
        ogrid_real_t p = 0.0F;
        g_in_lo = sc->arg_lo;
        st += (uint32_t)ogrid_log_odds_to_prob(g_in_lo, &p);
        acc += (double)p;
    }
    g_sink_d = g_sink_d + acc;
    g_sink_u = g_sink_u + st;
}

static void run_clamp(const scenario_t *sc, uint32_t k)
{
    double acc = 0.0;
    uint32_t st = 0U;
    for (uint32_t j = 0U; j < k; j++)
    {
        ogrid_real_t out = 0.0F;
        g_in_lo = sc->arg_lo;
        st += (uint32_t)ogrid_clamp_log_odds(g_in_lo, &out);
        acc += (double)out;
    }
    g_sink_d = g_sink_d + acc;
    g_sink_u = g_sink_u + st;
}

static void run_update_cell(const scenario_t *sc, uint32_t k)
{
    double acc = 0.0;
    uint32_t st = 0U;
    for (uint32_t j = 0U; j < k; j++)
    {
        const uint32_t idx = (j * 7U) % OGRID_CELL_COUNT;
        /* The cell is restored on each call so that every call sees the scenario input. */
        g_grid.cells[idx].log_odds_lo = sc->arg_lo;
        g_grid.cells[idx].observed = sc->fill_obs;
        st += (uint32_t)ogrid_update_cell(&g_grid, idx, sc->vote);
        acc += (double)g_grid.cells[idx].log_odds_lo;
    }
    g_sink_d = g_sink_d + acc;
    g_sink_u = g_sink_u + st;
}

static void run_get_state(const scenario_t *sc, uint32_t k)
{
    (void)sc;
    uint32_t st = 0U;
    for (uint32_t j = 0U; j < k; j++)
    {
        ogrid_cell_state_t out = OGRID_CELL_UNSURVEYED;
        const uint32_t idx = (j * 7U) % OGRID_CELL_COUNT;
        st += (uint32_t)ogrid_get_cell_state(&g_grid, idx, &out);
        st += (uint32_t)out;
    }
    g_sink_u = g_sink_u + st;
}

static void run_ray(const scenario_t *sc, uint32_t k)
{
    uint32_t st = 0U;
    for (uint32_t j = 0U; j < k; j++)
    {
        st += (uint32_t)ogrid_update_ray(&g_grid, g_ray_cells, sc->cells, sc->echo, sc->cells - 1U);
    }
    g_sink_d = g_sink_d + (double)g_grid.cells[0].log_odds_lo;
    g_sink_u = g_sink_u + st;
}

static void run_extract(const scenario_t *sc, uint32_t k)
{
    (void)sc;
    uint32_t st = 0U;
    for (uint32_t j = 0U; j < k; j++)
    {
        st += (uint32_t)ogrid_extract_occupied(&g_grid, g_mask);
        st += (g_mask[j % OGRID_CELL_COUNT]) ? 1U : 0U;
    }
    g_sink_u = g_sink_u + st;
}

/* ---- measurement ------------------------------------------------------------------------------
 */

static int cmp_double(const void *a, const void *b)
{
    const double x = *(const double *)a;
    const double y = *(const double *)b;
    return (x > y) - (x < y);
}

static result_t measure(const scenario_t *sc)
{
    result_t r;
    const uint32_t k = sc->batch;

    for (uint32_t w = 0U; w < WARMUP_REPS; w++)
    {
        fill_grid(sc);
        sc->run(sc, k);
    }
    for (uint32_t i = 0U; i < sc->reps; i++)
    {
        fill_grid(sc);
        const uint64_t t0 = now_ns();
        sc->run(sc, k);
        const uint64_t t1 = now_ns();
        g_samples[i] = (double)(t1 - t0) / (double)k;
    }
    qsort(g_samples, sc->reps, sizeof g_samples[0], cmp_double);

    r.reps = sc->reps;
    r.batch = k;
    r.min_ns = g_samples[0];
    r.median_ns = g_samples[sc->reps / 2U];
    r.p99_ns = g_samples[(((sc->reps * 99U) + 99U) / 100U) - 1U];
    r.max_ns = g_samples[sc->reps - 1U];
    return r;
}

/* ---- scenario table ----------------------------------------------------------------------------
 */

static scenario_t g_sc[MAX_SCENARIOS];
static result_t g_res[MAX_SCENARIOS];
static uint32_t g_sc_count;

static scenario_t *add(const char *name, const char *function, uint32_t reps, uint32_t batch,
                       void (*run)(const scenario_t *, uint32_t))
{
    scenario_t *s = &g_sc[g_sc_count];
    g_sc_count++;
    (void)memset(s, 0, sizeof *s);
    s->name = name;
    s->function = function;
    s->reps = reps;
    s->batch = batch;
    s->run = run;
    s->fill = FILL_NONE;
    s->fill_obs = (uint8_t)OGRID_OBSERVED_YES;
    s->vote = OGRID_VOTE_KIND_ECHO;
    s->cells = 1U;
    return s;
}

static void build_scenarios(void)
{
    scenario_t *s;

    for (uint32_t i = 0U; i < SWEEP_POINTS; i++)
    {
        /* Linear sweep over [-0.8, 3.4], both ends included. */
        g_sweep[i] = OGRID_LOG_ODDS_MIN + ((OGRID_LOG_ODDS_MAX - OGRID_LOG_ODDS_MIN) *
                                           (ogrid_real_t)i / (ogrid_real_t)(SWEEP_POINTS - 1U));
    }
    g_sweep[0] = OGRID_LOG_ODDS_MIN;
    g_sweep[SWEEP_POINTS - 1U] = OGRID_LOG_ODDS_MAX;
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        g_ray_cells[i] = i;
    }

    (void)add("reloj: lectura vacia", "clock_gettime", REPS_FAST, BATCH_FAST, run_empty_clock);

    s = add("reset: grid 3520 sucio", "ogrid_reset_grid", REPS_GRID, 1U, run_reset);
    s->fill = FILL_DIRTY;

    (void)add("prob: barrido [-0.8, 3.4]", "ogrid_log_odds_to_prob", REPS_FAST, BATCH_FAST,
              run_prob_sweep);
    s = add("prob: l = -0.8 (minimo)", "ogrid_log_odds_to_prob", REPS_FAST, BATCH_FAST,
            run_prob_fixed);
    s->arg_lo = OGRID_LOG_ODDS_MIN;
    s = add("prob: l = 0 (exacto 0.5)", "ogrid_log_odds_to_prob", REPS_FAST, BATCH_FAST,
            run_prob_fixed);
    s->arg_lo = 0.0F;
    s = add("prob: l = 3.4 (maximo)", "ogrid_log_odds_to_prob", REPS_FAST, BATCH_FAST,
            run_prob_fixed);
    s->arg_lo = OGRID_LOG_ODDS_MAX;
    s = add("prob: l = 5 (fuera de rango)", "ogrid_log_odds_to_prob", REPS_FAST, BATCH_FAST,
            run_prob_fixed);
    s->arg_lo = 5.0F;

    s = add("clamp: por debajo (-5)", "ogrid_clamp_log_odds", REPS_FAST, BATCH_FAST, run_clamp);
    s->arg_lo = -5.0F;
    s = add("clamp: dentro (1.0)", "ogrid_clamp_log_odds", REPS_FAST, BATCH_FAST, run_clamp);
    s->arg_lo = 1.0F;
    s = add("clamp: por encima (10)", "ogrid_clamp_log_odds", REPS_FAST, BATCH_FAST, run_clamp);
    s->arg_lo = 10.0F;

    s = add("cell: eco desde 0.0", "ogrid_update_cell", REPS_FAST, BATCH_FAST, run_update_cell);
    s->arg_lo = 0.0F;
    s->vote = OGRID_VOTE_KIND_ECHO;
    s = add("cell: paso desde 0.0", "ogrid_update_cell", REPS_FAST, BATCH_FAST, run_update_cell);
    s->arg_lo = 0.0F;
    s->vote = OGRID_VOTE_KIND_PASS;
    s = add("cell: oculta desde 0.0", "ogrid_update_cell", REPS_FAST, BATCH_FAST, run_update_cell);
    s->arg_lo = 0.0F;
    s->vote = OGRID_VOTE_KIND_OCCLUDED;
    s = add("cell: eco desde 3.4 (saturada)", "ogrid_update_cell", REPS_FAST, BATCH_FAST,
            run_update_cell);
    s->arg_lo = OGRID_LOG_ODDS_MAX;
    s->vote = OGRID_VOTE_KIND_ECHO;
    s = add("cell: eco desde 2.8 (satura)", "ogrid_update_cell", REPS_FAST, BATCH_FAST,
            run_update_cell);
    s->arg_lo = 2.8F;
    s->vote = OGRID_VOTE_KIND_ECHO;
    s = add("cell: paso desde -0.8 (saturada)", "ogrid_update_cell", REPS_FAST, BATCH_FAST,
            run_update_cell);
    s->arg_lo = OGRID_LOG_ODDS_MIN;
    s->vote = OGRID_VOTE_KIND_PASS;
    s = add("cell: paso desde -0.5 (satura)", "ogrid_update_cell", REPS_FAST, BATCH_FAST,
            run_update_cell);
    s->arg_lo = -0.5F;
    s->vote = OGRID_VOTE_KIND_PASS;
    s = add("cell: eco en celda no observada", "ogrid_update_cell", REPS_FAST, BATCH_FAST,
            run_update_cell);
    s->arg_lo = 0.0F;
    s->fill_obs = (uint8_t)OGRID_OBSERVED_NO;
    s->vote = OGRID_VOTE_KIND_ECHO;

    s = add("state: observada l=2.55 (ocupada)", "ogrid_get_cell_state", REPS_FAST, BATCH_FAST,
            run_get_state);
    s->fill = FILL_UNIFORM;
    s->fill_lo = 2.55F;
    s = add("state: observada l=1.70 (prob. baja)", "ogrid_get_cell_state", REPS_FAST, BATCH_FAST,
            run_get_state);
    s->fill = FILL_UNIFORM;
    s->fill_lo = 1.70F;
    s = add("state: no observada", "ogrid_get_cell_state", REPS_FAST, BATCH_FAST, run_get_state);
    s->fill = FILL_UNIFORM;
    s->fill_lo = 0.0F;
    s->fill_obs = (uint8_t)OGRID_OBSERVED_NO;
    s = add("state: corrupta (observed=2)", "ogrid_get_cell_state", REPS_FAST, BATCH_FAST,
            run_get_state);
    s->fill = FILL_UNIFORM;
    s->fill_lo = 0.0F;
    s->fill_obs = 2U;
    s = add("state: corrupta (l=9 observada)", "ogrid_get_cell_state", REPS_FAST, BATCH_FAST,
            run_get_state);
    s->fill = FILL_UNIFORM;
    s->fill_lo = 9.0F;

    static const uint32_t ray_len[3] = {10U, 880U, OGRID_CELL_COUNT};
    static const char *const ray_name[6] = {
        "ray: 10 celdas sin eco",   "ray: 10 celdas, eco en la ultima",
        "ray: 880 celdas sin eco",  "ray: 880 celdas, eco en la ultima",
        "ray: 3520 celdas sin eco", "ray: 3520 celdas, eco en la ultima"};
    for (uint32_t n = 0U; n < 3U; n++)
    {
        for (uint32_t e = 0U; e < 2U; e++)
        {
            s = add(ray_name[(n * 2U) + e], "ogrid_update_ray", (n == 0U) ? REPS_FAST : REPS_GRID,
                    1U, run_ray);
            s->fill = FILL_UNIFORM;
            s->fill_lo = 0.0F;
            s->cells = ray_len[n];
            s->echo = (e == 1U);
        }
    }

    s = add("extract: 3520 observadas, mitad ocupadas", "ogrid_extract_occupied", REPS_GRID, 1U,
            run_extract);
    s->fill = FILL_ALTERNATE;
    s = add("extract: 3520 observadas, todas ocupadas", "ogrid_extract_occupied", REPS_GRID, 1U,
            run_extract);
    s->fill = FILL_UNIFORM;
    s->fill_lo = 2.55F;
    s = add("extract: 3520 observadas, ninguna ocupada", "ogrid_extract_occupied", REPS_GRID, 1U,
            run_extract);
    s->fill = FILL_UNIFORM;
    s->fill_lo = 1.70F;
    s = add("extract: grid sin observar", "ogrid_extract_occupied", REPS_GRID, 1U, run_extract);
    s->fill = FILL_UNIFORM;
    s->fill_lo = 0.0F;
    s->fill_obs = (uint8_t)OGRID_OBSERVED_NO;
}

/* ---- output -----------------------------------------------------------------------------------
 */

static void print_header(void)
{
    struct utsname u;
    (void)printf("# Medida de tiempos ogrid (WCET empirico)\n\n");
    if (uname(&u) == 0)
    {
        (void)printf("- Arquitectura: %s %s %s\n", u.sysname, u.machine, u.release);
    }
    else
    {
        (void)printf("- Arquitectura: desconocida\n");
    }
#ifdef __VERSION__
    (void)printf("- Compilador: %s\n", __VERSION__);
#endif
    (void)printf("- Reloj: CLOCK_MONOTONIC\n");
    (void)printf("- Repeticiones: %u (rapidas), %u (con recorrido del grid, rayos largos), "
                 "precalentamiento %u\n",
                 (unsigned)REPS_FAST, (unsigned)REPS_GRID, (unsigned)WARMUP_REPS);
    (void)printf(
        "- Lote K: llamadas entre dos lecturas del reloj; ns por llamada = tiempo / K\n\n");
}

int main(void)
{
    build_scenarios();
    print_header();

    for (uint32_t i = 0U; i < g_sc_count; i++)
    {
        g_res[i] = measure(&g_sc[i]);
    }

    (void)printf("| Escenario | Funcion | K | Repeticiones | min (ns) | mediana (ns) | p99 (ns) | "
                 "max (ns) |\n");
    (void)printf("|---|---|---:|---:|---:|---:|---:|---:|\n");
    for (uint32_t i = 0U; i < g_sc_count; i++)
    {
        (void)printf("| %s | %s | %u | %u | %.1f | %.1f | %.1f | %.1f |\n", g_sc[i].name,
                     g_sc[i].function, (unsigned)g_res[i].batch, (unsigned)g_res[i].reps,
                     g_res[i].min_ns, g_res[i].median_ns, g_res[i].p99_ns, g_res[i].max_ns);
    }

    (void)printf("\n## Estimaciones por haz y por celda (a partir del maximo)\n\n");
    (void)printf("| Escenario | Celdas | max (ns/llamada) | max / celdas (ns/celda) |\n");
    (void)printf("|---|---:|---:|---:|\n");
    for (uint32_t i = 0U; i < g_sc_count; i++)
    {
        uint32_t cells = 0U;
        if (strcmp(g_sc[i].function, "ogrid_update_ray") == 0)
        {
            cells = g_sc[i].cells;
        }
        else if (strcmp(g_sc[i].function, "ogrid_extract_occupied") == 0)
        {
            cells = OGRID_CELL_COUNT;
        }
        else
        {
            continue;
        }
        (void)printf("| %s | %u | %.1f | %.2f |\n", g_sc[i].name, (unsigned)cells, g_res[i].max_ns,
                     g_res[i].max_ns / (double)cells);
    }

    (void)printf("\nAcumuladores (evitan la eliminacion de llamadas): %.6f %u\n", g_sink_d,
                 (unsigned)g_sink_u);
    return 0;
}
