/**
 * @file test_ogrid_get_cell_state.c
 * @brief Unit tests of ogrid_get_cell_state (F-005).
 * @details Probability threshold 0.85 corresponds to log-odds ln(0.85/0.15) = 1.73460. Probe values
 *          are chosen at least 0.005 away from it (|dp| > 1e-4, far above the float32 error of
 *          expf, about 1e-7), except the contract examples 1.70 and 2.55. Clamped limits are
 *          compared bit by bit.
 */
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ogrid/ogrid.h"
#include "ogrid/ogrid_get_cell_state.h"
#include "unity.h"

/** Cell used by most tests (s_index 10, d_index 2). */
#define CELL (42U)
/** Sentinel never written by the function on success paths for LOW_PROB / UNSURVEYED probes. */
#define SENTINEL (OGRID_CELL_OCCUPIED)

static ogrid_t s_grid;
static ogrid_t s_ref;

/** True when a and b have identical bit patterns. */
static bool same_bits(ogrid_real_t a, ogrid_real_t b)
{
    return memcmp(&a, &b, sizeof(a)) == 0;
}

/** Neighbour of a nonzero float, away from zero. */
static ogrid_real_t step_away(ogrid_real_t v)
{
    uint32_t bits = 0U;
    ogrid_real_t r = 0.0F;
    (void)memcpy(&bits, &v, sizeof(bits));
    bits = bits + 1U;
    (void)memcpy(&r, &bits, sizeof(r));
    return r;
}

/** Number of cells whose log-odds (bit by bit) or observed flag differ between s_grid and s_ref. */
static uint32_t diff_cells(void)
{
    uint32_t n = 0U;
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        if (!same_bits(s_grid.cells[i].log_odds_lo, s_ref.cells[i].log_odds_lo) ||
            (s_grid.cells[i].observed != s_ref.cells[i].observed))
        {
            n++;
        }
    }
    return n;
}

/** Set one cell and refresh the reference copy. */
static void set_cell(uint32_t idx, ogrid_real_t lo, bool observed)
{
    s_grid.cells[idx].log_odds_lo = lo;
    s_grid.cells[idx].observed = observed;
    s_ref = s_grid;
}

/** Query CELL after setting it; returns the status and stores the state in *out (sentinel first).
 */
static ogrid_status_t query(ogrid_real_t lo, bool observed, ogrid_cell_state_t sentinel,
                            ogrid_cell_state_t *out)
{
    set_cell(CELL, lo, observed);
    *out = sentinel;
    return ogrid_get_cell_state(&s_grid, CELL, out);
}

void setUp(void)
{
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        s_grid.cells[i].log_odds_lo = OGRID_LOG_ODDS_INIT;
        s_grid.cells[i].observed = false;
    }
    s_ref = s_grid;
}

void tearDown(void)
{
}

/** @req SYS-PR-08 @contract err OGRID_OK con parametros validos en los tres estados */
void test_TC_F005_01_valid_calls_return_ok(void)
{
    ogrid_cell_state_t st = SENTINEL;
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, query(0.0F, false, SENTINEL, &st),
                                  "[TC-F005-01][SYS-PR-08] unobserved cell must return OGRID_OK");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, query(0.5F, true, SENTINEL, &st),
                                  "[TC-F005-01][SYS-PR-08] observed low cell must return OGRID_OK");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, query(2.55F, true, SENTINEL, &st),
                                  "[TC-F005-01][SYS-PR-08] occupied cell must return OGRID_OK");
}

/** @req SYS-PR-08 @contract post POST-1 celda inicial (log-odds 0, no observada) es UNSURVEYED */
void test_TC_F005_02_initial_cell_is_unsurveyed(void)
{
    ogrid_cell_state_t st = SENTINEL;
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_get_cell_state(&s_grid, CELL, &st),
                                  "[TC-F005-02][SYS-PR-08] initial cell must return OGRID_OK");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_CELL_UNSURVEYED, st,
                                  "[TC-F005-02][SYS-PR-08] initial cell must be UNSURVEYED");
}

/** @req SYS-SAF-07 @contract post POST-1 no observada: UNSURVEYED con cualquier log-odds
 * (alto, bajo, limites, corrupto) y status OK */
void test_TC_F005_03_unobserved_any_log_odds_is_unsurveyed(void)
{
    const ogrid_real_t vals[] = {0.0F,  2.55F, OGRID_LOG_ODDS_MAX, OGRID_LOG_ODDS_MIN,
                                 -0.4F, NAN,   INFINITY,           -INFINITY,
                                 10.0F, -10.0F};
    uint32_t bad = 0U;
    for (size_t k = 0U; k < (sizeof(vals) / sizeof(vals[0])); k++)
    {
        ogrid_cell_state_t st = SENTINEL;
        if ((query(vals[k], false, SENTINEL, &st) != OGRID_OK) || (st != OGRID_CELL_UNSURVEYED))
        {
            bad++;
        }
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, bad, "[TC-F005-03][SYS-SAF-07] unobserved cell must be UNSURVEYED/OK for any value");
}

/** @req SYS-PR-08 @contract post POST-2 ejemplo del contrato: l = 2,55 (p = 0,9276) es OCCUPIED */
void test_TC_F005_04_l_2_55_is_occupied(void)
{
    ogrid_cell_state_t st = OGRID_CELL_UNSURVEYED;
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, query(2.55F, true, OGRID_CELL_UNSURVEYED, &st),
                                  "[TC-F005-04][SYS-PR-08] l=2.55 must return OGRID_OK");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_CELL_OCCUPIED, st,
                                  "[TC-F005-04][SYS-PR-08] l=2.55 (p=0.9276) must be OCCUPIED");
}

/** @req SYS-PR-08 @contract post POST-3 ejemplo del contrato: l = 1,70 (p = 0,8455) es LOW_PROB */
void test_TC_F005_05_l_1_70_is_low_prob(void)
{
    ogrid_cell_state_t st = SENTINEL;
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, query(1.70F, true, SENTINEL, &st),
                                  "[TC-F005-05][SYS-PR-08] l=1.70 must return OGRID_OK");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_CELL_LOW_PROB, st,
                                  "[TC-F005-05][SYS-PR-08] l=1.70 (p=0.8455) must be LOW_PROB");
}

/** @req SYS-PR-08 @contract post POST-2 umbral estricto: justo por encima de ln(0,85/0,15)
 * (1,74, p = 0,8507) es OCCUPIED */
void test_TC_F005_06_just_above_threshold_is_occupied(void)
{
    ogrid_cell_state_t st = OGRID_CELL_UNSURVEYED;
    (void)query(1.74F, true, OGRID_CELL_UNSURVEYED, &st);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_CELL_OCCUPIED, st,
                                  "[TC-F005-06][SYS-PR-08] l=1.74 (p=0.8507) must be OCCUPIED");
}

/** @req SYS-PR-08 @contract post POST-3 justo por debajo del umbral (1,73, p = 0,8494) es LOW_PROB
 */
void test_TC_F005_07_just_below_threshold_is_low_prob(void)
{
    ogrid_cell_state_t st = SENTINEL;
    (void)query(1.73F, true, SENTINEL, &st);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_CELL_LOW_PROB, st,
                                  "[TC-F005-07][SYS-PR-08] l=1.73 (p=0.8494) must be LOW_PROB");
}

/** @req SYS-PR-08 @contract post POST-2 tres ecos desde el minimo (-0,8 + 3 x 0,85 = 1,75,
 * p = 0,8520) confirman OCCUPIED */
void test_TC_F005_08_three_echoes_from_min_is_occupied(void)
{
    ogrid_cell_state_t st = OGRID_CELL_UNSURVEYED;
    (void)query(1.75F, true, OGRID_CELL_UNSURVEYED, &st);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_CELL_OCCUPIED, st,
                                  "[TC-F005-08][SYS-PR-08] l=1.75 (p=0.8520) must be OCCUPIED");
}

/** @req SYS-PR-08 @contract post POST-3 un eco aislado (l = 0,85, p = 0,7006) es LOW_PROB */
void test_TC_F005_09_single_echo_is_low_prob(void)
{
    ogrid_cell_state_t st = SENTINEL;
    (void)query(0.85F, true, SENTINEL, &st);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_CELL_LOW_PROB, st,
                                  "[TC-F005-09][SYS-PR-08] l=0.85 (p=0.7006) must be LOW_PROB");
}

/** @req SYS-PR-08 @contract post POST-3 observada con l = 0 (p = 0,5) y l = -0,4 es LOW_PROB, no
 * UNSURVEYED */
void test_TC_F005_10_observed_zero_and_negative_are_low_prob(void)
{
    ogrid_cell_state_t st = SENTINEL;
    (void)query(0.0F, true, SENTINEL, &st);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_CELL_LOW_PROB, st,
                                  "[TC-F005-10][SYS-PR-08] observed l=0 must be LOW_PROB");
    st = SENTINEL;
    (void)query(-0.4F, true, SENTINEL, &st);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_CELL_LOW_PROB, st,
                                  "[TC-F005-10][SYS-PR-08] observed l=-0.4 must be LOW_PROB");
}

/** @req SYS-SAF-07 @contract post POST-3 limite inferior valido exacto -0,8 (observada):
 * LOW_PROB, status OK */
void test_TC_F005_11_observed_min_limit_is_low_prob(void)
{
    ogrid_cell_state_t st = OGRID_CELL_OCCUPIED;
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_OK, query(OGRID_LOG_ODDS_MIN, true, OGRID_CELL_OCCUPIED, &st),
        "[TC-F005-11][SYS-SAF-07] stored -0.8 is a valid value and must return OGRID_OK");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_CELL_LOW_PROB, st,
                                  "[TC-F005-11][SYS-SAF-07] observed l=-0.8 must be LOW_PROB");
}

/** @req SYS-SAF-07 @contract post POST-2 limite superior valido exacto 3,4 (observada):
 * OCCUPIED, status OK */
void test_TC_F005_12_observed_max_limit_is_occupied(void)
{
    ogrid_cell_state_t st = OGRID_CELL_UNSURVEYED;
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_OK, query(OGRID_LOG_ODDS_MAX, true, OGRID_CELL_UNSURVEYED, &st),
        "[TC-F005-12][SYS-SAF-07] stored 3.4 is a valid value and must return OGRID_OK");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_CELL_OCCUPIED, st,
                                  "[TC-F005-12][SYS-SAF-07] observed l=3.4 must be OCCUPIED");
}

/** @req SYS-SAF-07 @contract post POST-5 observada con NaN, +inf, -inf: ERR_STATE y
 * UNSURVEYED (centinela previo OCCUPIED y LOW_PROB) */
void test_TC_F005_13_non_finite_observed_is_state_error(void)
{
    const ogrid_real_t vals[] = {NAN, INFINITY, -INFINITY};
    const ogrid_cell_state_t sentinels[] = {OGRID_CELL_OCCUPIED, OGRID_CELL_LOW_PROB};
    uint32_t bad_status = 0U;
    uint32_t bad_state = 0U;
    for (size_t k = 0U; k < 3U; k++)
    {
        for (size_t s = 0U; s < 2U; s++)
        {
            ogrid_cell_state_t st = sentinels[s];
            if (query(vals[k], true, sentinels[s], &st) != OGRID_ERR_STATE)
            {
                bad_status++;
            }
            if (st != OGRID_CELL_UNSURVEYED)
            {
                bad_state++;
            }
        }
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, bad_status, "[TC-F005-13][SYS-SAF-07] NaN/inf observed must return OGRID_ERR_STATE");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, bad_state, "[TC-F005-13][SYS-SAF-07] NaN/inf observed must set state UNSURVEYED");
}

/** @req SYS-SAF-07 @contract post POST-5 observada con valor un ulp fuera de [-0,8; 3,4], +-10 y
 * +-FLT_MAX: ERR_STATE y UNSURVEYED */
void test_TC_F005_14_out_of_limits_observed_is_state_error(void)
{
    const ogrid_real_t vals[] = {step_away(OGRID_LOG_ODDS_MAX),
                                 step_away(OGRID_LOG_ODDS_MIN),
                                 3.5F,
                                 -0.9F,
                                 10.0F,
                                 -10.0F,
                                 FLT_MAX,
                                 -FLT_MAX};
    const ogrid_cell_state_t sentinels[] = {OGRID_CELL_OCCUPIED, OGRID_CELL_LOW_PROB};
    uint32_t bad_status = 0U;
    uint32_t bad_state = 0U;
    for (size_t k = 0U; k < (sizeof(vals) / sizeof(vals[0])); k++)
    {
        for (size_t s = 0U; s < 2U; s++)
        {
            ogrid_cell_state_t st = sentinels[s];
            if (query(vals[k], true, sentinels[s], &st) != OGRID_ERR_STATE)
            {
                bad_status++;
            }
            if (st != OGRID_CELL_UNSURVEYED)
            {
                bad_state++;
            }
        }
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, bad_status, "[TC-F005-14][SYS-SAF-07] value outside [-0.8, 3.4] must be ERR_STATE");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, bad_state, "[TC-F005-14][SYS-SAF-07] value outside [-0.8, 3.4] must give UNSURVEYED");
}

/** @req SYS-SAF-07 @contract post POST-5 el ERR_STATE nunca produce LOW_PROB ni OCCUPIED
 * (nunca libre ni probabilidad baja ante datos corruptos) */
void test_TC_F005_15_corrupt_never_low_or_occupied(void)
{
    const ogrid_real_t vals[] = {NAN, INFINITY, -INFINITY, 3.5F, -0.9F, 100.0F, -100.0F};
    uint32_t bad = 0U;
    for (size_t k = 0U; k < (sizeof(vals) / sizeof(vals[0])); k++)
    {
        ogrid_cell_state_t st = OGRID_CELL_LOW_PROB;
        (void)query(vals[k], true, OGRID_CELL_LOW_PROB, &st);
        if ((st == OGRID_CELL_LOW_PROB) || (st == OGRID_CELL_OCCUPIED))
        {
            bad++;
        }
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, bad, "[TC-F005-15][SYS-SAF-07] corrupt cell must never be LOW_PROB or OCCUPIED");
}

/** @req SYS-SAF-07 @contract post POST-4 barrido de log-odds -0,8..3,39 en pasos de 0,01
 * (observada) y no observada: la salida siempre es uno de los tres estados */
void test_TC_F005_16_output_always_one_of_three_states(void)
{
    uint32_t bad = 0U;
    for (uint32_t i = 0U; i < 420U; i++)
    {
        const ogrid_real_t lo = -0.8F + (0.01F * (ogrid_real_t)i);
        for (uint32_t o = 0U; o < 2U; o++)
        {
            ogrid_cell_state_t st = OGRID_CELL_STATE_COUNT;
            set_cell(CELL, lo, (o == 1U));
            if ((ogrid_get_cell_state(&s_grid, CELL, &st) != OGRID_OK) ||
                ((st != OGRID_CELL_UNSURVEYED) && (st != OGRID_CELL_LOW_PROB) &&
                 (st != OGRID_CELL_OCCUPIED)))
            {
                bad++;
            }
        }
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, bad, "[TC-F005-16][SYS-SAF-07] output must be one of the three states, status OK");
}

/** @req SYS-PR-08 @contract post POST-2 monotonia: al subir el log-odds observado, LOW_PROB pasa a
 * OCCUPIED una sola vez y nunca vuelve; umbral entre 1,70 y 1,75 */
void test_TC_F005_17_monotonic_in_log_odds(void)
{
    uint32_t bad = 0U;
    uint32_t transitions = 0U;
    ogrid_cell_state_t prev = OGRID_CELL_LOW_PROB;
    for (uint32_t i = 0U; i < 420U; i++)
    {
        const ogrid_real_t lo = -0.8F + (0.01F * (ogrid_real_t)i);
        ogrid_cell_state_t st = OGRID_CELL_UNSURVEYED;
        set_cell(CELL, lo, true);
        if ((ogrid_get_cell_state(&s_grid, CELL, &st) != OGRID_OK) ||
            ((st != OGRID_CELL_LOW_PROB) && (st != OGRID_CELL_OCCUPIED)))
        {
            bad++;
            continue;
        }
        if ((prev == OGRID_CELL_OCCUPIED) && (st == OGRID_CELL_LOW_PROB))
        {
            bad++;
        }
        if ((prev == OGRID_CELL_LOW_PROB) && (st == OGRID_CELL_OCCUPIED))
        {
            transitions++;
            /* Threshold 1.7346 lies in (1.72, 1.75]: first occupied sample must be 1.74 or 1.75. */
            if ((lo < 1.735F) || (lo > 1.755F))
            {
                bad++;
            }
        }
        prev = st;
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, bad, "[TC-F005-17][SYS-PR-08] state must be monotonic with threshold near 1.7346");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, transitions,
                                     "[TC-F005-17][SYS-PR-08] exactly one LOW_PROB->OCCUPIED step");
}

/** @req SYS-PR-08 @contract post POST-1 el estado depende solo de la celda indicada: cada celda de
 * un patron (no observada / l = 0 / l = 2,55) devuelve su propio estado */
void test_TC_F005_18_each_cell_reports_own_state(void)
{
    uint32_t bad = 0U;
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        const uint32_t m = i % 3U;
        s_grid.cells[i].observed = (m != 0U);
        s_grid.cells[i].log_odds_lo = (m == 2U) ? 2.55F : 0.0F;
    }
    s_ref = s_grid;
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        const uint32_t m = i % 3U;
        const ogrid_cell_state_t expected = (m == 0U)   ? OGRID_CELL_UNSURVEYED
                                            : (m == 1U) ? OGRID_CELL_LOW_PROB
                                                        : OGRID_CELL_OCCUPIED;
        ogrid_cell_state_t st = OGRID_CELL_STATE_COUNT;
        if ((ogrid_get_cell_state(&s_grid, i, &st) != OGRID_OK) || (st != expected))
        {
            bad++;
        }
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, bad, "[TC-F005-18][SYS-PR-08] every cell index must report its own state");
}

/** @req SYS-PR-08 @contract pre PRE-2 indices limite 0, 3, 4 y ultimo (COUNT-1) son validos y
 * leen su celda */
void test_TC_F005_19_index_boundaries_are_valid(void)
{
    const uint32_t idxs[] = {0U, 3U, 4U, OGRID_CELL_COUNT - 1U};
    uint32_t bad = 0U;
    for (size_t k = 0U; k < 4U; k++)
    {
        ogrid_cell_state_t st = SENTINEL;
        setUp();
        set_cell(idxs[k], 0.5F, true);
        if ((ogrid_get_cell_state(&s_grid, idxs[k], &st) != OGRID_OK) ||
            (st != OGRID_CELL_LOW_PROB))
        {
            bad++;
        }
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, bad, "[TC-F005-19][SYS-PR-08] first and last valid indices must be readable");
}

/** @req SYS-SAF-07 @contract err OGRID_ERR_PARAM con grid NULL; state_out no se modifica */
void test_TC_F005_20_null_grid_is_param_error(void)
{
    ogrid_cell_state_t st = SENTINEL;
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_PARAM, ogrid_get_cell_state(NULL, CELL, &st),
                                  "[TC-F005-20][SYS-SAF-07] NULL grid must return ERR_PARAM");
    TEST_ASSERT_EQUAL_INT_MESSAGE(SENTINEL, st,
                                  "[TC-F005-20][SYS-SAF-07] NULL grid must not modify state_out");
}

/** @req SYS-SAF-07 @contract err OGRID_ERR_PARAM con state_out NULL; el grid no se modifica */
void test_TC_F005_21_null_state_out_is_param_error(void)
{
    set_cell(CELL, 2.55F, true);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_PARAM, ogrid_get_cell_state(&s_grid, CELL, NULL),
                                  "[TC-F005-21][SYS-SAF-07] NULL state_out must be ERR_PARAM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, diff_cells(),
                                     "[TC-F005-21][SYS-SAF-07] NULL state_out must not alter grid");
}

/** @req SYS-SAF-07 @contract pre PRE-1 ambos punteros NULL devuelve ERR_PARAM */
void test_TC_F005_22_both_null_is_param_error(void)
{
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_PARAM, ogrid_get_cell_state(NULL, CELL, NULL),
                                  "[TC-F005-22][SYS-SAF-07] both NULL must return ERR_PARAM");
}

/** @req SYS-SAF-07 @contract err OGRID_ERR_RANGE con cell_idx = COUNT (justo fuera), COUNT+1 y
 * UINT32_MAX; state_out no se modifica */
void test_TC_F005_23_index_out_of_range_is_range_error(void)
{
    const uint32_t idxs[] = {OGRID_CELL_COUNT, OGRID_CELL_COUNT + 1U, UINT32_MAX};
    uint32_t bad_status = 0U;
    uint32_t bad_state = 0U;
    for (size_t k = 0U; k < 3U; k++)
    {
        ogrid_cell_state_t st = SENTINEL;
        if (ogrid_get_cell_state(&s_grid, idxs[k], &st) != OGRID_ERR_RANGE)
        {
            bad_status++;
        }
        if (st != SENTINEL)
        {
            bad_state++;
        }
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, bad_status,
                                     "[TC-F005-23][SYS-SAF-07] idx >= 3520 must be ERR_RANGE");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, bad_state, "[TC-F005-23][SYS-SAF-07] ERR_RANGE must not modify state_out");
}

/** @req SYS-SAF-07 @contract post POST-6 con ERR_PARAM / ERR_RANGE el grid no se modifica
 * (bit a bit) */
void test_TC_F005_24_errors_do_not_modify_grid(void)
{
    ogrid_cell_state_t st = SENTINEL;
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        s_grid.cells[i].log_odds_lo = -0.8F + (0.001F * (ogrid_real_t)(i % 4200U));
        s_grid.cells[i].observed = ((i % 3U) != 0U);
    }
    s_ref = s_grid;
    (void)ogrid_get_cell_state(&s_grid, OGRID_CELL_COUNT, &st);
    (void)ogrid_get_cell_state(&s_grid, UINT32_MAX, &st);
    (void)ogrid_get_cell_state(&s_grid, CELL, NULL);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, diff_cells(),
                                     "[TC-F005-24][SYS-SAF-07] error paths must not alter grid");
}

/** @req SYS-PR-08 @contract post POST-6 una consulta valida (cualquier estado) no modifica el grid
 * (bit a bit), incluida la de ERR_STATE */
void test_TC_F005_25_valid_queries_do_not_modify_grid(void)
{
    ogrid_cell_state_t st = SENTINEL;
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        s_grid.cells[i].log_odds_lo = -0.8F + (0.001F * (ogrid_real_t)(i % 4200U));
        s_grid.cells[i].observed = ((i % 3U) != 0U);
    }
    s_grid.cells[100].log_odds_lo = NAN;
    s_grid.cells[100].observed = true;
    s_ref = s_grid;
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        (void)ogrid_get_cell_state(&s_grid, i, &st);
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, diff_cells(),
                                     "[TC-F005-25][SYS-PR-08] queries must not modify the grid");
}

/** @req SYS-SAF-07 @contract pre PRE-1 gana a PRE-2: puntero NULL con indice fuera de rango
 * devuelve ERR_PARAM */
void test_TC_F005_26_null_wins_over_range(void)
{
    ogrid_cell_state_t st = SENTINEL;
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_PARAM, ogrid_get_cell_state(NULL, OGRID_CELL_COUNT, &st),
        "[TC-F005-26][SYS-SAF-07] NULL grid + idx out of range must be ERR_PARAM");
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_PARAM, ogrid_get_cell_state(&s_grid, UINT32_MAX, NULL),
        "[TC-F005-26][SYS-SAF-07] NULL state_out + idx out of range must be ERR_PARAM");
    TEST_ASSERT_EQUAL_INT_MESSAGE(SENTINEL, st,
                                  "[TC-F005-26][SYS-SAF-07] ERR_PARAM must not modify state_out");
}

/** @req SYS-SAF-07 @contract err ERR_RANGE con un grid totalmente corrupto e indice fuera de
 * rango; con indice valido la misma celda da ERR_STATE */
void test_TC_F005_27_range_error_on_corrupt_grid(void)
{
    ogrid_cell_state_t st = SENTINEL;
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        s_grid.cells[i].log_odds_lo = NAN;
        s_grid.cells[i].observed = true;
    }
    s_ref = s_grid;
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_RANGE, ogrid_get_cell_state(&s_grid, OGRID_CELL_COUNT, &st),
        "[TC-F005-27][SYS-SAF-07] idx out of range on corrupt grid must be ERR_RANGE");
    TEST_ASSERT_EQUAL_INT_MESSAGE(SENTINEL, st,
                                  "[TC-F005-27][SYS-SAF-07] ERR_RANGE must not modify state_out");
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_STATE, ogrid_get_cell_state(&s_grid, CELL, &st),
        "[TC-F005-27][SYS-SAF-07] valid idx on corrupt cell must be ERR_STATE");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_CELL_UNSURVEYED, st,
                                  "[TC-F005-27][SYS-SAF-07] ERR_STATE must give UNSURVEYED");
}

/** @req SYS-SAF-07 @contract post POST-5 una celda corrupta no afecta a la consulta de otras
 * celdas validas */
void test_TC_F005_28_corrupt_cell_does_not_affect_neighbours(void)
{
    ogrid_cell_state_t st = SENTINEL;
    set_cell(CELL, NAN, true);
    s_grid.cells[CELL + 1U].log_odds_lo = 2.55F;
    s_grid.cells[CELL + 1U].observed = true;
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_OK, ogrid_get_cell_state(&s_grid, CELL + 1U, &st),
        "[TC-F005-28][SYS-SAF-07] valid neighbour of corrupt cell must be OGRID_OK");
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_CELL_OCCUPIED, st,
        "[TC-F005-28][SYS-SAF-07] valid neighbour of corrupt cell must be OCCUPIED");
}

/** @req SYS-PR-08 @contract perf determinismo: 100 llamadas repetidas sobre la misma celda dan el
 * mismo estado y status */
void test_TC_F005_29_deterministic(void)
{
    uint32_t bad = 0U;
    set_cell(CELL, 2.55F, true);
    for (uint32_t i = 0U; i < 100U; i++)
    {
        ogrid_cell_state_t st = OGRID_CELL_UNSURVEYED;
        if ((ogrid_get_cell_state(&s_grid, CELL, &st) != OGRID_OK) || (st != OGRID_CELL_OCCUPIED))
        {
            bad++;
        }
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, bad, "[TC-F005-29][SYS-PR-08] repeated queries must give identical results");
}

/** @req SYS-SAF-07 @contract post POST-4 tras un error una llamada valida funciona con normalidad
 */
void test_TC_F005_30_valid_call_after_errors_works(void)
{
    ogrid_cell_state_t st = SENTINEL;
    set_cell(CELL, 2.55F, true);
    (void)ogrid_get_cell_state(NULL, CELL, &st);
    (void)ogrid_get_cell_state(&s_grid, OGRID_CELL_COUNT, &st);
    st = OGRID_CELL_UNSURVEYED;
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_get_cell_state(&s_grid, CELL, &st),
                                  "[TC-F005-30][SYS-SAF-07] valid call after errors must be OK");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_CELL_OCCUPIED, st,
                                  "[TC-F005-30][SYS-SAF-07] valid call after errors: OCCUPIED");
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_TC_F005_01_valid_calls_return_ok);
    RUN_TEST(test_TC_F005_02_initial_cell_is_unsurveyed);
    RUN_TEST(test_TC_F005_03_unobserved_any_log_odds_is_unsurveyed);
    RUN_TEST(test_TC_F005_04_l_2_55_is_occupied);
    RUN_TEST(test_TC_F005_05_l_1_70_is_low_prob);
    RUN_TEST(test_TC_F005_06_just_above_threshold_is_occupied);
    RUN_TEST(test_TC_F005_07_just_below_threshold_is_low_prob);
    RUN_TEST(test_TC_F005_08_three_echoes_from_min_is_occupied);
    RUN_TEST(test_TC_F005_09_single_echo_is_low_prob);
    RUN_TEST(test_TC_F005_10_observed_zero_and_negative_are_low_prob);
    RUN_TEST(test_TC_F005_11_observed_min_limit_is_low_prob);
    RUN_TEST(test_TC_F005_12_observed_max_limit_is_occupied);
    RUN_TEST(test_TC_F005_13_non_finite_observed_is_state_error);
    RUN_TEST(test_TC_F005_14_out_of_limits_observed_is_state_error);
    RUN_TEST(test_TC_F005_15_corrupt_never_low_or_occupied);
    RUN_TEST(test_TC_F005_16_output_always_one_of_three_states);
    RUN_TEST(test_TC_F005_17_monotonic_in_log_odds);
    RUN_TEST(test_TC_F005_18_each_cell_reports_own_state);
    RUN_TEST(test_TC_F005_19_index_boundaries_are_valid);
    RUN_TEST(test_TC_F005_20_null_grid_is_param_error);
    RUN_TEST(test_TC_F005_21_null_state_out_is_param_error);
    RUN_TEST(test_TC_F005_22_both_null_is_param_error);
    RUN_TEST(test_TC_F005_23_index_out_of_range_is_range_error);
    RUN_TEST(test_TC_F005_24_errors_do_not_modify_grid);
    RUN_TEST(test_TC_F005_25_valid_queries_do_not_modify_grid);
    RUN_TEST(test_TC_F005_26_null_wins_over_range);
    RUN_TEST(test_TC_F005_27_range_error_on_corrupt_grid);
    RUN_TEST(test_TC_F005_28_corrupt_cell_does_not_affect_neighbours);
    RUN_TEST(test_TC_F005_29_deterministic);
    RUN_TEST(test_TC_F005_30_valid_call_after_errors_works);
    return UNITY_END();
}
