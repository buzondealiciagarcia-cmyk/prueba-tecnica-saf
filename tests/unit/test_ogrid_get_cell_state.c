/**
 * @file test_ogrid_get_cell_state.c
 * @brief Unit tests of ogrid_get_cell_state (F-005).
 * @details Probabilities are p = 1 / (1 + exp(-l)) (F-002). The decision threshold is
 *          l* = ln(0.85 / 0.15) = 1.734601. Cases never sit exactly on l*, because float32 rounding
 *          makes p == 0.85 undecidable; instead they use l = 1.70 and 2.55 (given by the contract)
 *          and l = 1.73 (p = 0.8494) and l = 1.74 (p = 0.8504), 4e-4 away from the threshold, far
 *          above the float32 error of exp (about 1e-7). Log-odds limits are compared with the
 *          macros themselves, so no numeric tolerance is needed for them.
 */
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ogrid/ogrid.h"
#include "ogrid/ogrid_get_cell_state.h"
#include "ogrid/ogrid_limits.h"
#include "unity.h"

/** Cell used by most tests. */
#define CELL (42U)
/** Sentinel that ogrid_get_cell_state never writes on success paths of PARAM and RANGE. */
#define SENTINEL (OGRID_CELL_STATE_COUNT)

static ogrid_t s_grid;
static ogrid_t s_ref;

/** Number of cells whose log-odds bit pattern or observed flag differ between s_grid and s_ref. */
static uint32_t grid_changes(void)
{
    uint32_t n = 0U;
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        const ogrid_real_t a = s_grid.cells[i].log_odds_lo;
        const ogrid_real_t b = s_ref.cells[i].log_odds_lo;
        const bool same_value = (isnan(a) && isnan(b)) || (a == b);
        if (!same_value || (s_grid.cells[i].observed != s_ref.cells[i].observed))
        {
            n++;
        }
    }
    return n;
}

/** Set one cell and refresh the reference copy. */
static void set_cell(uint32_t idx, ogrid_real_t lo, uint8_t observed)
{
    s_grid.cells[idx].log_odds_lo = lo;
    s_grid.cells[idx].observed = observed;
    s_ref = s_grid;
}

/** Query CELL and return the state written; status goes to *st. */
static ogrid_cell_state_t query(uint32_t idx, ogrid_status_t *st)
{
    ogrid_cell_state_t out = SENTINEL;
    *st = ogrid_get_cell_state(&s_grid, idx, &out);
    return out;
}

void setUp(void)
{
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        s_grid.cells[i].log_odds_lo = OGRID_LOG_ODDS_INIT;
        s_grid.cells[i].observed = OGRID_OBSERVED_NO;
    }
    s_ref = s_grid;
}

void tearDown(void)
{
}

/** @req SYS-PR-08 @contract err OGRID_OK con parametros validos y celda coherente */
void test_TC_F005_01_valid_call_returns_ok(void)
{
    ogrid_status_t st = OGRID_ERR_PARAM;
    set_cell(CELL, 1.0F, OGRID_OBSERVED_YES);
    (void)query(CELL, &st);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st,
                                  "[TC-F005-01][SYS-PR-08] valid call must return OGRID_OK");
}

/** @req SYS-PR-08 @contract post POST-1 celda no observada con l = 0 es zona no estudiada */
void test_TC_F005_02_unobserved_is_unsurveyed(void)
{
    ogrid_status_t st = OGRID_ERR_PARAM;
    const ogrid_cell_state_t s = query(CELL, &st);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st,
                                  "[TC-F005-02][SYS-PR-08] unobserved cell must return OGRID_OK");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_CELL_UNSURVEYED, s,
                                  "[TC-F005-02][SYS-PR-08] unobserved cell must be UNSURVEYED");
}

/** @req SYS-PR-08 @contract post POST-1 no observada es UNSURVEYED con cualquier log-odds
 * (particion) */
void test_TC_F005_03_unobserved_any_log_odds(void)
{
    const ogrid_real_t values[] = {
        OGRID_LOG_ODDS_MAX, OGRID_LOG_ODDS_MIN, 2.55F, -5.0F, 100.0F, NAN, INFINITY, -INFINITY};
    for (size_t i = 0U; i < (sizeof(values) / sizeof(values[0])); i++)
    {
        ogrid_status_t st = OGRID_ERR_PARAM;
        set_cell(CELL, values[i], OGRID_OBSERVED_NO);
        const ogrid_cell_state_t s = query(CELL, &st);
        TEST_ASSERT_EQUAL_INT_MESSAGE(
            OGRID_OK, st, "[TC-F005-03][SYS-PR-08] unobserved cell must return OGRID_OK for any l");
        TEST_ASSERT_EQUAL_INT_MESSAGE(
            OGRID_CELL_UNSURVEYED, s,
            "[TC-F005-03][SYS-PR-08] unobserved cell must be UNSURVEYED for any l");
    }
}

/** @req SYS-PR-08 @contract post POST-3 l = 1,70 (p = 0,8455 <= 0,85) es LOW_PROB (dato del
 * contrato) */
void test_TC_F005_04_l_1_70_is_low_prob(void)
{
    ogrid_status_t st = OGRID_ERR_PARAM;
    set_cell(CELL, 1.70F, OGRID_OBSERVED_YES);
    const ogrid_cell_state_t s = query(CELL, &st);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st,
                                  "[TC-F005-04][SYS-PR-08] l=1.70 must return OGRID_OK");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_CELL_LOW_PROB, s,
                                  "[TC-F005-04][SYS-PR-08] l=1.70 (p=0.8455) must be LOW_PROB");
}

/** @req SYS-PR-08 @contract post POST-2 l = 2,55 (p = 0,9276 > 0,85) es OCCUPIED (dato del
 * contrato) */
void test_TC_F005_05_l_2_55_is_occupied(void)
{
    ogrid_status_t st = OGRID_ERR_PARAM;
    set_cell(CELL, 2.55F, OGRID_OBSERVED_YES);
    const ogrid_cell_state_t s = query(CELL, &st);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st,
                                  "[TC-F005-05][SYS-PR-08] l=2.55 must return OGRID_OK");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_CELL_OCCUPIED, s,
                                  "[TC-F005-05][SYS-PR-08] l=2.55 (p=0.9276) must be OCCUPIED");
}

/** @req SYS-PR-08 @contract post POST-3 l = 1,73 (p = 0,8494), justo bajo el umbral, es LOW_PROB */
void test_TC_F005_06_just_below_threshold_is_low_prob(void)
{
    ogrid_status_t st = OGRID_ERR_PARAM;
    set_cell(CELL, 1.73F, OGRID_OBSERVED_YES);
    const ogrid_cell_state_t s = query(CELL, &st);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st,
                                  "[TC-F005-06][SYS-PR-08] l=1.73 must return OGRID_OK");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_CELL_LOW_PROB, s,
                                  "[TC-F005-06][SYS-PR-08] l=1.73 (p=0.8494) must be LOW_PROB");
}

/** @req SYS-PR-08 @contract post POST-2 l = 1,74 (p = 0,8504), justo sobre el umbral, es OCCUPIED
 */
void test_TC_F005_07_just_above_threshold_is_occupied(void)
{
    ogrid_status_t st = OGRID_ERR_PARAM;
    set_cell(CELL, 1.74F, OGRID_OBSERVED_YES);
    const ogrid_cell_state_t s = query(CELL, &st);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st,
                                  "[TC-F005-07][SYS-PR-08] l=1.74 must return OGRID_OK");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_CELL_OCCUPIED, s,
                                  "[TC-F005-07][SYS-PR-08] l=1.74 (p=0.8504) must be OCCUPIED");
}

/** @req SYS-PR-08 @contract post POST-3 limite inferior l = -0,8 (p = 0,31) y l = 0 (p = 0,5) son
 * LOW_PROB */
void test_TC_F005_08_lower_limit_is_low_prob(void)
{
    ogrid_status_t st = OGRID_ERR_PARAM;
    set_cell(CELL, OGRID_LOG_ODDS_MIN, OGRID_OBSERVED_YES);
    ogrid_cell_state_t s = query(CELL, &st);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st,
                                  "[TC-F005-08][SYS-PR-08] l=-0.8 must return OGRID_OK");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_CELL_LOW_PROB, s,
                                  "[TC-F005-08][SYS-PR-08] l=-0.8 must be LOW_PROB");
    set_cell(CELL, 0.0F, OGRID_OBSERVED_YES);
    s = query(CELL, &st);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st,
                                  "[TC-F005-08][SYS-PR-08] l=0 observed must return OGRID_OK");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_CELL_LOW_PROB, s,
                                  "[TC-F005-08][SYS-PR-08] l=0 observed must be LOW_PROB");
}

/** @req SYS-PR-08 @contract post POST-2 limite superior l = 3,4 (p = 0,968) es OCCUPIED y devuelve
 * OK */
void test_TC_F005_09_upper_limit_is_occupied(void)
{
    ogrid_status_t st = OGRID_ERR_PARAM;
    set_cell(CELL, OGRID_LOG_ODDS_MAX, OGRID_OBSERVED_YES);
    const ogrid_cell_state_t s = query(CELL, &st);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st,
                                  "[TC-F005-09][SYS-PR-08] l=3.4 must return OGRID_OK");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_CELL_OCCUPIED, s,
                                  "[TC-F005-09][SYS-PR-08] l=3.4 must be OCCUPIED");
}

/** @req SYS-PR-08 @contract post POST-2 monotonia: al subir l el estado solo pasa de LOW_PROB a
 * OCCUPIED */
void test_TC_F005_10_monotonic_in_log_odds(void)
{
    ogrid_cell_state_t prev = OGRID_CELL_LOW_PROB;
    for (uint32_t k = 0U; k < 84U; k++)
    {
        /* l in [-0.8, 3.35], inside the valid range; step 0.05 */
        const ogrid_real_t l = -0.8F + ((ogrid_real_t)k * 0.05F);
        ogrid_status_t st = OGRID_ERR_PARAM;
        set_cell(CELL, l, OGRID_OBSERVED_YES);
        const ogrid_cell_state_t s = query(CELL, &st);
        TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st,
                                      "[TC-F005-10][SYS-PR-08] valid l must return OGRID_OK");
        TEST_ASSERT_TRUE_MESSAGE(
            (s == OGRID_CELL_LOW_PROB) || (s == OGRID_CELL_OCCUPIED),
            "[TC-F005-10][SYS-PR-08] observed cell must be LOW_PROB or OCCUPIED");
        TEST_ASSERT_TRUE_MESSAGE(s >= prev,
                                 "[TC-F005-10][SYS-PR-08] state must not decrease when l grows");
        if (l < 1.72F)
        {
            TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_CELL_LOW_PROB, s,
                                          "[TC-F005-10][SYS-PR-08] l < 1.72 must be LOW_PROB");
        }
        if (l > 1.75F)
        {
            TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_CELL_OCCUPIED, s,
                                          "[TC-F005-10][SYS-PR-08] l > 1.75 must be OCCUPIED");
        }
        prev = s;
    }
}

/** @req SYS-SAF-07 @contract post POST-4 la salida es siempre uno de los tres estados (barrido de
 * casos) */
void test_TC_F005_11_output_always_valid_state(void)
{
    const ogrid_real_t values[] = {-0.8F, 0.0F,  1.0F, 1.7F, 1.75F,   2.55F,
                                   3.4F,  -1.0F, 4.0F, NAN,  INFINITY};
    const uint8_t flags[] = {0U, 1U, 2U, 255U};
    for (size_t i = 0U; i < (sizeof(values) / sizeof(values[0])); i++)
    {
        for (size_t j = 0U; j < (sizeof(flags) / sizeof(flags[0])); j++)
        {
            ogrid_status_t st = OGRID_ERR_PARAM;
            set_cell(CELL, values[i], flags[j]);
            const ogrid_cell_state_t s = query(CELL, &st);
            TEST_ASSERT_TRUE_MESSAGE(
                (s == OGRID_CELL_UNSURVEYED) || (s == OGRID_CELL_LOW_PROB) ||
                    (s == OGRID_CELL_OCCUPIED),
                "[TC-F005-11][SYS-SAF-07] output must be one of the three cell states");
            TEST_ASSERT_TRUE_MESSAGE((st == OGRID_OK) || (st == OGRID_ERR_STATE),
                                     "[TC-F005-11][SYS-SAF-07] status must be OK or ERR_STATE");
        }
    }
}

/** @req SYS-SAF-07 @contract post POST-5 observed = 2 (corrupta) devuelve ERR_STATE y UNSURVEYED */
void test_TC_F005_12_observed_2_is_corrupt(void)
{
    ogrid_status_t st = OGRID_OK;
    ogrid_cell_state_t out = OGRID_CELL_OCCUPIED;
    set_cell(CELL, 2.55F, 2U);
    st = ogrid_get_cell_state(&s_grid, CELL, &out);
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_STATE, st, "[TC-F005-12][SYS-SAF-07] observed=2 must return OGRID_ERR_STATE");
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_CELL_UNSURVEYED, out,
        "[TC-F005-12][SYS-SAF-07] observed=2 must write UNSURVEYED, not OCCUPIED");
}

/** @req SYS-SAF-07 @contract post POST-5 observed = 255 (corrupta) con l bajo: ERR_STATE y
 * UNSURVEYED, no LOW_PROB */
void test_TC_F005_13_observed_255_is_corrupt(void)
{
    ogrid_status_t st = OGRID_OK;
    ogrid_cell_state_t out = OGRID_CELL_LOW_PROB;
    set_cell(CELL, 0.0F, 255U);
    st = ogrid_get_cell_state(&s_grid, CELL, &out);
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_STATE, st, "[TC-F005-13][SYS-SAF-07] observed=255 must return OGRID_ERR_STATE");
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_CELL_UNSURVEYED, out,
        "[TC-F005-13][SYS-SAF-07] observed=255 must write UNSURVEYED, not LOW_PROB");
}

/** @req SYS-SAF-07 @contract err ERR_STATE con observed corrupto aunque l sea NaN o fuera de rango
 */
void test_TC_F005_14_corrupt_observed_with_bad_log_odds(void)
{
    ogrid_status_t st = OGRID_OK;
    ogrid_cell_state_t out = OGRID_CELL_OCCUPIED;
    set_cell(CELL, NAN, 3U);
    st = ogrid_get_cell_state(&s_grid, CELL, &out);
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_STATE, st,
        "[TC-F005-14][SYS-SAF-07] observed=3 with NaN must return OGRID_ERR_STATE");
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_CELL_UNSURVEYED, out,
        "[TC-F005-14][SYS-SAF-07] observed=3 with NaN must write UNSURVEYED");
}

/** @req SYS-SAF-07 @contract post POST-5 celda observada con l = NaN: ERR_STATE y UNSURVEYED */
void test_TC_F005_15_nan_log_odds_is_corrupt(void)
{
    ogrid_status_t st = OGRID_OK;
    ogrid_cell_state_t out = OGRID_CELL_OCCUPIED;
    set_cell(CELL, NAN, OGRID_OBSERVED_YES);
    st = ogrid_get_cell_state(&s_grid, CELL, &out);
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_STATE, st, "[TC-F005-15][SYS-SAF-07] NaN log-odds must return OGRID_ERR_STATE");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_CELL_UNSURVEYED, out,
                                  "[TC-F005-15][SYS-SAF-07] NaN log-odds must write UNSURVEYED");
}

/** @req SYS-SAF-07 @contract post POST-5 celda observada con l = +inf o -inf: ERR_STATE y
 * UNSURVEYED */
void test_TC_F005_16_infinite_log_odds_is_corrupt(void)
{
    const ogrid_real_t values[] = {INFINITY, -INFINITY};
    for (size_t i = 0U; i < 2U; i++)
    {
        ogrid_status_t st = OGRID_OK;
        ogrid_cell_state_t out = OGRID_CELL_LOW_PROB;
        set_cell(CELL, values[i], OGRID_OBSERVED_YES);
        st = ogrid_get_cell_state(&s_grid, CELL, &out);
        TEST_ASSERT_EQUAL_INT_MESSAGE(
            OGRID_ERR_STATE, st,
            "[TC-F005-16][SYS-SAF-07] infinite log-odds must return OGRID_ERR_STATE");
        TEST_ASSERT_EQUAL_INT_MESSAGE(
            OGRID_CELL_UNSURVEYED, out,
            "[TC-F005-16][SYS-SAF-07] infinite log-odds must write UNSURVEYED");
    }
}

/** @req SYS-SAF-07 @contract post POST-5 l justo fuera de [-0,8; 3,4] en celda observada: ERR_STATE
 */
void test_TC_F005_17_out_of_range_log_odds_is_corrupt(void)
{
    const ogrid_real_t values[] = {nextafterf(OGRID_LOG_ODDS_MIN, -10.0F), -0.81F, -100.0F,
                                   nextafterf(OGRID_LOG_ODDS_MAX, 10.0F),  3.41F,  1.0e30F};
    for (size_t i = 0U; i < (sizeof(values) / sizeof(values[0])); i++)
    {
        ogrid_status_t st = OGRID_OK;
        ogrid_cell_state_t out = OGRID_CELL_OCCUPIED;
        set_cell(CELL, values[i], OGRID_OBSERVED_YES);
        st = ogrid_get_cell_state(&s_grid, CELL, &out);
        TEST_ASSERT_EQUAL_INT_MESSAGE(
            OGRID_ERR_STATE, st,
            "[TC-F005-17][SYS-SAF-07] l outside [-0.8, 3.4] must return OGRID_ERR_STATE");
        TEST_ASSERT_EQUAL_INT_MESSAGE(
            OGRID_CELL_UNSURVEYED, out,
            "[TC-F005-17][SYS-SAF-07] l outside [-0.8, 3.4] must write UNSURVEYED");
    }
}

/** @req SYS-SAF-07 @contract err ERR_STATE escribe UNSURVEYED aunque la salida previa fuera un
 * valor centinela */
void test_TC_F005_18_corrupt_overwrites_sentinel(void)
{
    ogrid_status_t st = OGRID_OK;
    ogrid_cell_state_t out = SENTINEL;
    set_cell(CELL, 1.0F, 7U);
    st = ogrid_get_cell_state(&s_grid, CELL, &out);
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_STATE, st, "[TC-F005-18][SYS-SAF-07] corrupt cell must return OGRID_ERR_STATE");
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_CELL_UNSURVEYED, out,
        "[TC-F005-18][SYS-SAF-07] corrupt cell must overwrite the output with UNSURVEYED");
}

/** @req SYS-PR-08 @contract err ERR_PARAM con grid NULL, salida sin cambios */
void test_TC_F005_19_null_grid(void)
{
    ogrid_cell_state_t out = SENTINEL;
    const ogrid_status_t st = ogrid_get_cell_state(NULL, CELL, &out);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_PARAM, st,
                                  "[TC-F005-19][SYS-PR-08] NULL grid must return OGRID_ERR_PARAM");
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        SENTINEL, out, "[TC-F005-19][SYS-PR-08] NULL grid must leave *state_out unchanged");
}

/** @req SYS-PR-08 @contract err ERR_PARAM con state_out NULL, sin modificar el grid */
void test_TC_F005_20_null_state_out(void)
{
    set_cell(CELL, 2.55F, OGRID_OBSERVED_YES);
    const ogrid_status_t st = ogrid_get_cell_state(&s_grid, CELL, NULL);
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_PARAM, st, "[TC-F005-20][SYS-PR-08] NULL state_out must return OGRID_ERR_PARAM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, grid_changes(), "[TC-F005-20][SYS-PR-08] NULL state_out must not modify the grid");
}

/** @req SYS-PR-08 @contract err ERR_PARAM con grid y state_out NULL */
void test_TC_F005_21_both_null(void)
{
    const ogrid_status_t st = ogrid_get_cell_state(NULL, CELL, NULL);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_PARAM, st,
                                  "[TC-F005-21][SYS-PR-08] both NULL must return OGRID_ERR_PARAM");
}

/** @req SYS-PR-08 @contract pre PRE-2 cell_idx = OGRID_CELL_COUNT (justo fuera): ERR_RANGE, salida
 * sin cambios */
void test_TC_F005_22_index_just_out_of_range(void)
{
    ogrid_cell_state_t out = SENTINEL;
    const ogrid_status_t st = ogrid_get_cell_state(&s_grid, OGRID_CELL_COUNT, &out);
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_RANGE, st, "[TC-F005-22][SYS-PR-08] cell_idx=COUNT must return OGRID_ERR_RANGE");
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        SENTINEL, out, "[TC-F005-22][SYS-PR-08] cell_idx=COUNT must leave *state_out unchanged");
}

/** @req SYS-PR-08 @contract pre PRE-2 cell_idx = COUNT+1 y UINT32_MAX: ERR_RANGE, salida sin
 * cambios */
void test_TC_F005_23_index_far_out_of_range(void)
{
    const uint32_t idx[] = {OGRID_CELL_COUNT + 1U, UINT32_MAX};
    for (size_t i = 0U; i < 2U; i++)
    {
        ogrid_cell_state_t out = SENTINEL;
        const ogrid_status_t st = ogrid_get_cell_state(&s_grid, idx[i], &out);
        TEST_ASSERT_EQUAL_INT_MESSAGE(
            OGRID_ERR_RANGE, st,
            "[TC-F005-23][SYS-PR-08] large cell_idx must return OGRID_ERR_RANGE");
        TEST_ASSERT_EQUAL_INT_MESSAGE(
            SENTINEL, out,
            "[TC-F005-23][SYS-PR-08] large cell_idx must leave *state_out unchanged");
    }
}

/** @req SYS-PR-08 @contract pre PRE-2 limites validos: cell_idx = 0 y COUNT-1 se leen de su celda
 */
void test_TC_F005_24_index_bounds_valid(void)
{
    ogrid_status_t st = OGRID_ERR_PARAM;
    set_cell(0U, 2.55F, OGRID_OBSERVED_YES);
    s_grid.cells[OGRID_CELL_COUNT - 1U].log_odds_lo = 0.5F;
    s_grid.cells[OGRID_CELL_COUNT - 1U].observed = OGRID_OBSERVED_YES;
    s_ref = s_grid;
    ogrid_cell_state_t s = query(0U, &st);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st,
                                  "[TC-F005-24][SYS-PR-08] cell_idx=0 must return OGRID_OK");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_CELL_OCCUPIED, s,
                                  "[TC-F005-24][SYS-PR-08] cell 0 must be OCCUPIED");
    s = query(OGRID_CELL_COUNT - 1U, &st);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st,
                                  "[TC-F005-24][SYS-PR-08] cell_idx=COUNT-1 must return OGRID_OK");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_CELL_LOW_PROB, s,
                                  "[TC-F005-24][SYS-PR-08] last cell must be LOW_PROB");
}

/** @req SYS-PR-08 @contract post POST-1 el estado depende solo de la celda indicada, no de las
 * vecinas */
void test_TC_F005_25_cells_are_independent(void)
{
    ogrid_status_t st = OGRID_ERR_PARAM;
    set_cell(CELL, 3.0F, OGRID_OBSERVED_YES);
    ogrid_cell_state_t s = query(CELL - 1U, &st);
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_CELL_UNSURVEYED, s,
        "[TC-F005-25][SYS-PR-08] previous neighbour must stay UNSURVEYED");
    s = query(CELL + 1U, &st);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_CELL_UNSURVEYED, s,
                                  "[TC-F005-25][SYS-PR-08] next neighbour must stay UNSURVEYED");
    s = query(CELL, &st);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_CELL_OCCUPIED, s,
                                  "[TC-F005-25][SYS-PR-08] the set cell must be OCCUPIED");
}

/** @req SYS-SAF-07 @contract post POST-6 no modifica el grid (OK, STATE, PARAM y RANGE) */
void test_TC_F005_26_grid_not_modified(void)
{
    ogrid_cell_state_t out = SENTINEL;
    set_cell(CELL, 2.55F, OGRID_OBSERVED_YES);
    s_grid.cells[CELL + 1U].log_odds_lo = NAN;
    s_grid.cells[CELL + 1U].observed = OGRID_OBSERVED_YES;
    s_grid.cells[CELL + 2U].log_odds_lo = 0.0F;
    s_grid.cells[CELL + 2U].observed = 9U;
    s_ref = s_grid;
    (void)ogrid_get_cell_state(&s_grid, CELL, &out);
    (void)ogrid_get_cell_state(&s_grid, CELL + 1U, &out);
    (void)ogrid_get_cell_state(&s_grid, CELL + 2U, &out);
    (void)ogrid_get_cell_state(&s_grid, OGRID_CELL_COUNT, &out);
    (void)ogrid_get_cell_state(&s_grid, CELL, NULL);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, grid_changes(), "[TC-F005-26][SYS-SAF-07] the grid must not be modified by any call");
}

/** @req SYS-SAF-07 @contract pre orden PRE-1 antes que PRE-2: grid NULL con indice fuera de rango
 * da ERR_PARAM */
void test_TC_F005_27_order_null_grid_before_range(void)
{
    ogrid_cell_state_t out = SENTINEL;
    const ogrid_status_t st = ogrid_get_cell_state(NULL, OGRID_CELL_COUNT, &out);
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_PARAM, st,
        "[TC-F005-27][SYS-SAF-07] NULL grid and bad index must return ERR_PARAM, not ERR_RANGE");
    TEST_ASSERT_EQUAL_INT_MESSAGE(SENTINEL, out,
                                  "[TC-F005-27][SYS-SAF-07] *state_out must stay unchanged");
}

/** @req SYS-SAF-07 @contract pre orden PRE-1 antes que PRE-2: state_out NULL con indice fuera de
 * rango da ERR_PARAM */
void test_TC_F005_28_order_null_out_before_range(void)
{
    const ogrid_status_t st = ogrid_get_cell_state(&s_grid, UINT32_MAX, NULL);
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_PARAM, st,
        "[TC-F005-28][SYS-SAF-07] NULL state_out and bad index must return ERR_PARAM");
}

/** @req SYS-SAF-07 @contract pre orden PRE-2 antes que POST-5: indice fuera de rango con todo el
 * grid corrupto da ERR_RANGE */
void test_TC_F005_29_order_range_before_state(void)
{
    ogrid_cell_state_t out = SENTINEL;
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        s_grid.cells[i].log_odds_lo = NAN;
        s_grid.cells[i].observed = 200U;
    }
    s_ref = s_grid;
    const ogrid_status_t st = ogrid_get_cell_state(&s_grid, OGRID_CELL_COUNT, &out);
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_RANGE, st,
        "[TC-F005-29][SYS-SAF-07] bad index on a corrupt grid must return ERR_RANGE");
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        SENTINEL, out, "[TC-F005-29][SYS-SAF-07] *state_out must stay unchanged (POST-6)");
}

/** @req SYS-SAF-07 @contract pre orden PRE-1 antes que POST-5: state_out NULL con celda corrupta da
 * ERR_PARAM */
void test_TC_F005_30_order_null_out_before_state(void)
{
    set_cell(CELL, NAN, 2U);
    const ogrid_status_t st = ogrid_get_cell_state(&s_grid, CELL, NULL);
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_PARAM, st,
        "[TC-F005-30][SYS-SAF-07] NULL state_out on a corrupt cell must return ERR_PARAM");
}

/** @req SYS-SAF-07 @contract post POST-5 una celda corrupta no contamina a las sanas del mismo grid
 */
void test_TC_F005_31_corrupt_cell_does_not_affect_others(void)
{
    ogrid_status_t st = OGRID_ERR_PARAM;
    set_cell(CELL, 2.55F, 2U);
    s_grid.cells[CELL + 1U].log_odds_lo = 2.55F;
    s_grid.cells[CELL + 1U].observed = OGRID_OBSERVED_YES;
    s_ref = s_grid;
    const ogrid_cell_state_t s = query(CELL + 1U, &st);
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_OK, st, "[TC-F005-31][SYS-SAF-07] healthy neighbour must return OGRID_OK");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_CELL_OCCUPIED, s,
                                  "[TC-F005-31][SYS-SAF-07] healthy neighbour must be OCCUPIED");
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_TC_F005_01_valid_call_returns_ok);
    RUN_TEST(test_TC_F005_02_unobserved_is_unsurveyed);
    RUN_TEST(test_TC_F005_03_unobserved_any_log_odds);
    RUN_TEST(test_TC_F005_04_l_1_70_is_low_prob);
    RUN_TEST(test_TC_F005_05_l_2_55_is_occupied);
    RUN_TEST(test_TC_F005_06_just_below_threshold_is_low_prob);
    RUN_TEST(test_TC_F005_07_just_above_threshold_is_occupied);
    RUN_TEST(test_TC_F005_08_lower_limit_is_low_prob);
    RUN_TEST(test_TC_F005_09_upper_limit_is_occupied);
    RUN_TEST(test_TC_F005_10_monotonic_in_log_odds);
    RUN_TEST(test_TC_F005_11_output_always_valid_state);
    RUN_TEST(test_TC_F005_12_observed_2_is_corrupt);
    RUN_TEST(test_TC_F005_13_observed_255_is_corrupt);
    RUN_TEST(test_TC_F005_14_corrupt_observed_with_bad_log_odds);
    RUN_TEST(test_TC_F005_15_nan_log_odds_is_corrupt);
    RUN_TEST(test_TC_F005_16_infinite_log_odds_is_corrupt);
    RUN_TEST(test_TC_F005_17_out_of_range_log_odds_is_corrupt);
    RUN_TEST(test_TC_F005_18_corrupt_overwrites_sentinel);
    RUN_TEST(test_TC_F005_19_null_grid);
    RUN_TEST(test_TC_F005_20_null_state_out);
    RUN_TEST(test_TC_F005_21_both_null);
    RUN_TEST(test_TC_F005_22_index_just_out_of_range);
    RUN_TEST(test_TC_F005_23_index_far_out_of_range);
    RUN_TEST(test_TC_F005_24_index_bounds_valid);
    RUN_TEST(test_TC_F005_25_cells_are_independent);
    RUN_TEST(test_TC_F005_26_grid_not_modified);
    RUN_TEST(test_TC_F005_27_order_null_grid_before_range);
    RUN_TEST(test_TC_F005_28_order_null_out_before_range);
    RUN_TEST(test_TC_F005_29_order_range_before_state);
    RUN_TEST(test_TC_F005_30_order_null_out_before_state);
    RUN_TEST(test_TC_F005_31_corrupt_cell_does_not_affect_others);
    return UNITY_END();
}
