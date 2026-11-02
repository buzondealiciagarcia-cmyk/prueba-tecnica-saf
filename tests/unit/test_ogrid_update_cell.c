/**
 * @file test_ogrid_update_cell.c
 * @brief Unit tests of ogrid_update_cell (F-004).
 * @details Numeric tolerance of sums is 1e-6 (contract POST-1); it covers the float32 rounding
 *          of one or three additions of magnitude <= 3.4 (ulp(3.4) = 2.4e-7). Clamped results
 *          are compared bit by bit with the limits.
 */
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ogrid/ogrid.h"
#include "ogrid/ogrid_limits.h"
#include "ogrid/ogrid_update_cell.h"
#include "unity.h"

/** Tolerance of the contract for sums (POST-1). */
#define TOL (1.0e-6F)
/** Cell used by most tests (s_index 10, d_index 2). */
#define CELL (42U)
/** Echo log-odds threshold ln(0.85 / 0.15). */
#define LN_THRESHOLD (1.7346F)

static ogrid_t s_grid;
static ogrid_t s_ref;

/** True when a and b have identical bit patterns. */
static bool same_bits(ogrid_real_t a, ogrid_real_t b)
{
    return memcmp(&a, &b, sizeof(a)) == 0;
}

/** Neighbour of a nonzero float: away from zero (away = true) or towards zero. */
static ogrid_real_t step_bits(ogrid_real_t v, bool away)
{
    uint32_t bits = 0U;
    ogrid_real_t r = 0.0F;
    (void)memcpy(&bits, &v, sizeof(bits));
    bits = away ? (bits + 1U) : (bits - 1U);
    (void)memcpy(&r, &bits, sizeof(r));
    return r;
}

/** Number of cells whose log-odds (bit by bit) or observed flag differ between two grids. */
static uint32_t diff_cells(const ogrid_t *a, const ogrid_t *b)
{
    uint32_t n = 0U;
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        if (!same_bits(a->cells[i].log_odds_lo, b->cells[i].log_odds_lo) ||
            (a->cells[i].observed != b->cells[i].observed))
        {
            n++;
        }
    }
    return n;
}

/** Number of cells different between s_grid and s_ref, ignoring cell skip. */
static uint32_t diff_except(uint32_t skip)
{
    uint32_t n = diff_cells(&s_grid, &s_ref);
    if (!same_bits(s_grid.cells[skip].log_odds_lo, s_ref.cells[skip].log_odds_lo) ||
        (s_grid.cells[skip].observed != s_ref.cells[skip].observed))
    {
        n--;
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

/** @req SYS-PR-08 @contract err OGRID_OK con parametros validos (eco, pase, oculta) */
void test_TC_F004_01_valid_calls_return_ok(void)
{
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_update_cell(&s_grid, CELL, OGRID_VOTE_KIND_ECHO),
                                  "[TC-F004-01][SYS-PR-08] echo must return OGRID_OK");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_update_cell(&s_grid, CELL, OGRID_VOTE_KIND_PASS),
                                  "[TC-F004-01][SYS-PR-08] pass must return OGRID_OK");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK,
                                  ogrid_update_cell(&s_grid, CELL, OGRID_VOTE_KIND_OCCLUDED),
                                  "[TC-F004-01][SYS-PR-08] occluded must return OGRID_OK");
}

/** @req SYS-PR-08 @contract post POST-1 eco desde 0 da 0,85 y observed = OGRID_OBSERVED_YES */
void test_TC_F004_02_echo_from_zero(void)
{
    (void)ogrid_update_cell(&s_grid, CELL, OGRID_VOTE_KIND_ECHO);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, 0.85F, s_grid.cells[CELL].log_odds_lo,
                                     "[TC-F004-02][SYS-PR-08] echo from 0 must give 0.85");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(
        OGRID_OBSERVED_YES, s_grid.cells[CELL].observed,
        "[TC-F004-02][SYS-PR-08] echo must set observed to OGRID_OBSERVED_YES");
}

/** @req SYS-PR-08 @contract post POST-1 tres ecos seguidos desde 0: 0,85 / 1,70 / 2,55 */
void test_TC_F004_03_three_echoes_sequence(void)
{
    (void)ogrid_update_cell(&s_grid, CELL, OGRID_VOTE_KIND_ECHO);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, 0.85F, s_grid.cells[CELL].log_odds_lo,
                                     "[TC-F004-03][SYS-PR-08] 1st echo must give 0.85");
    (void)ogrid_update_cell(&s_grid, CELL, OGRID_VOTE_KIND_ECHO);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, 1.70F, s_grid.cells[CELL].log_odds_lo,
                                     "[TC-F004-03][SYS-PR-08] 2nd echo must give 1.70");
    (void)ogrid_update_cell(&s_grid, CELL, OGRID_VOTE_KIND_ECHO);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, 2.55F, s_grid.cells[CELL].log_odds_lo,
                                     "[TC-F004-03][SYS-PR-08] 3rd echo must give 2.55");
}

/** @req SYS-SAF-07 @contract post POST-1 eco por encima del maximo se limita a 3,4 exacto */
void test_TC_F004_04_echo_saturates_at_max(void)
{
    set_cell(CELL, 3.0F, OGRID_OBSERVED_YES);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_update_cell(&s_grid, CELL, OGRID_VOTE_KIND_ECHO),
                                  "[TC-F004-04][SYS-SAF-07] echo near max must return OGRID_OK");
    TEST_ASSERT_TRUE_MESSAGE(same_bits(s_grid.cells[CELL].log_odds_lo, OGRID_LOG_ODDS_MAX),
                             "[TC-F004-04][SYS-SAF-07] 3.0 + 0.85 must clamp to 3.4 exactly");
    (void)ogrid_update_cell(&s_grid, CELL, OGRID_VOTE_KIND_ECHO);
    TEST_ASSERT_TRUE_MESSAGE(same_bits(s_grid.cells[CELL].log_odds_lo, OGRID_LOG_ODDS_MAX),
                             "[TC-F004-04][SYS-SAF-07] echo at max must stay at 3.4");
}

/** @req SYS-PR-08 @contract post POST-1 eco desde el minimo -0,8 da 0,05 */
void test_TC_F004_05_echo_from_min(void)
{
    set_cell(CELL, OGRID_LOG_ODDS_MIN, OGRID_OBSERVED_YES);
    (void)ogrid_update_cell(&s_grid, CELL, OGRID_VOTE_KIND_ECHO);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, 0.05F, s_grid.cells[CELL].log_odds_lo,
                                     "[TC-F004-05][SYS-PR-08] -0.8 + 0.85 must give 0.05");
}

/** @req SYS-PR-08 @contract post POST-2 pase desde 0 da -0,40 y observed = OGRID_OBSERVED_YES */
void test_TC_F004_06_pass_from_zero(void)
{
    (void)ogrid_update_cell(&s_grid, CELL, OGRID_VOTE_KIND_PASS);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, -0.40F, s_grid.cells[CELL].log_odds_lo,
                                     "[TC-F004-06][SYS-PR-08] pass from 0 must give -0.40");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(
        OGRID_OBSERVED_YES, s_grid.cells[CELL].observed,
        "[TC-F004-06][SYS-PR-08] pass must set observed to OGRID_OBSERVED_YES");
}

/** @req SYS-PR-08 @contract post POST-2 pase desde el minimo se queda en -0,8 exacto, observed */
void test_TC_F004_07_pass_at_min_stays(void)
{
    set_cell(CELL, OGRID_LOG_ODDS_MIN, OGRID_OBSERVED_NO);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_update_cell(&s_grid, CELL, OGRID_VOTE_KIND_PASS),
                                  "[TC-F004-07][SYS-PR-08] pass at min must return OGRID_OK");
    TEST_ASSERT_TRUE_MESSAGE(same_bits(s_grid.cells[CELL].log_odds_lo, OGRID_LOG_ODDS_MIN),
                             "[TC-F004-07][SYS-PR-08] pass at -0.8 must stay at -0.8 exactly");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(
        OGRID_OBSERVED_YES, s_grid.cells[CELL].observed,
        "[TC-F004-07][SYS-PR-08] pass must set observed to OGRID_OBSERVED_YES even if clamped");
}

/** @req SYS-SAF-07 @contract post POST-2 pase que cruzaria el minimo (-0,5 - 0,4) se limita a -0,8
 */
void test_TC_F004_08_pass_crossing_min_is_clamped(void)
{
    set_cell(CELL, -0.5F, OGRID_OBSERVED_YES);
    (void)ogrid_update_cell(&s_grid, CELL, OGRID_VOTE_KIND_PASS);
    TEST_ASSERT_TRUE_MESSAGE(same_bits(s_grid.cells[CELL].log_odds_lo, OGRID_LOG_ODDS_MIN),
                             "[TC-F004-08][SYS-SAF-07] -0.5 - 0.4 must clamp to -0.8 exactly");
}

/** @req SYS-PR-08 @contract post POST-2 pase tras un eco: 0,85 - 0,40 = 0,45 */
void test_TC_F004_09_echo_then_pass(void)
{
    (void)ogrid_update_cell(&s_grid, CELL, OGRID_VOTE_KIND_ECHO);
    (void)ogrid_update_cell(&s_grid, CELL, OGRID_VOTE_KIND_PASS);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, 0.45F, s_grid.cells[CELL].log_odds_lo,
                                     "[TC-F004-09][SYS-PR-08] echo then pass must give 0.45");
}

/** @req SYS-PR-08 @contract post POST-3 oculta en celda no observada: nada cambia */
void test_TC_F004_10_occluded_unobserved_unchanged(void)
{
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK,
                                  ogrid_update_cell(&s_grid, CELL, OGRID_VOTE_KIND_OCCLUDED),
                                  "[TC-F004-10][SYS-PR-08] occluded must return OGRID_OK");
    TEST_ASSERT_TRUE_MESSAGE(same_bits(s_grid.cells[CELL].log_odds_lo, 0.0F),
                             "[TC-F004-10][SYS-PR-08] occluded must keep log-odds 0");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(
        OGRID_OBSERVED_NO, s_grid.cells[CELL].observed,
        "[TC-F004-10][SYS-PR-08] occluded must keep observed at OGRID_OBSERVED_NO");
}

/** @req SYS-PR-08 @contract post POST-3 oculta en celda observada con valor 1,25: nada cambia */
void test_TC_F004_11_occluded_observed_unchanged(void)
{
    set_cell(CELL, 1.25F, OGRID_OBSERVED_YES);
    (void)ogrid_update_cell(&s_grid, CELL, OGRID_VOTE_KIND_OCCLUDED);
    TEST_ASSERT_TRUE_MESSAGE(same_bits(s_grid.cells[CELL].log_odds_lo, 1.25F),
                             "[TC-F004-11][SYS-PR-08] occluded must keep log-odds 1.25");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(
        OGRID_OBSERVED_YES, s_grid.cells[CELL].observed,
        "[TC-F004-11][SYS-PR-08] occluded must keep observed at OGRID_OBSERVED_YES");
}

/** @req SYS-PR-08 @contract post POST-4 las demas celdas no cambian con ningun voto */
void test_TC_F004_12_other_cells_unchanged(void)
{
    const ogrid_vote_kind_t votes[] = {OGRID_VOTE_KIND_ECHO, OGRID_VOTE_KIND_PASS,
                                       OGRID_VOTE_KIND_OCCLUDED};
    uint32_t bad = 0U;
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        s_grid.cells[i].log_odds_lo = -0.8F + (0.001F * (ogrid_real_t)(i % 4200U));
        s_grid.cells[i].observed = ((i % 3U) == 0U) ? OGRID_OBSERVED_YES : OGRID_OBSERVED_NO;
    }
    s_ref = s_grid;
    for (size_t v = 0U; v < 3U; v++)
    {
        (void)ogrid_update_cell(&s_grid, CELL, votes[v]);
        bad += diff_except(CELL);
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, bad, "[TC-F004-12][SYS-PR-08] cells other than the voted one must not change");
}

/** @req SYS-PR-08 @contract post POST-4 indices 0, 3, 4 y ultimo se actualizan en su celda
 * (s_index * 4 + d_index) */
void test_TC_F004_13_index_boundaries_update_own_cell(void)
{
    const uint32_t idxs[] = {0U, 3U, 4U, OGRID_CELL_COUNT - 1U};
    uint32_t bad = 0U;
    for (size_t k = 0U; k < 4U; k++)
    {
        setUp();
        if ((ogrid_update_cell(&s_grid, idxs[k], OGRID_VOTE_KIND_ECHO) != OGRID_OK) ||
            (s_grid.cells[idxs[k]].observed != OGRID_OBSERVED_YES) ||
            (fabsf(s_grid.cells[idxs[k]].log_odds_lo - 0.85F) > TOL) ||
            (diff_except(idxs[k]) != 0U))
        {
            bad++;
        }
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, bad, "[TC-F004-13][SYS-PR-08] first and last valid indices must update only own cell");
}

/** @req SYS-SAF-07 @contract post POST-4 salida siempre en [-0,8; 3,4] con secuencia mixta de 400
 * votos */
void test_TC_F004_14_mixed_sequence_bounded(void)
{
    uint32_t bad = 0U;
    for (uint32_t i = 0U; i < 400U; i++)
    {
        /* Pattern: 7 echoes, 3 passes, 1 occluded, 11 votes long. */
        const uint32_t m = i % 11U;
        const ogrid_vote_kind_t v = (m < 7U)    ? OGRID_VOTE_KIND_ECHO
                                    : (m < 10U) ? OGRID_VOTE_KIND_PASS
                                                : OGRID_VOTE_KIND_OCCLUDED;
        const ogrid_status_t st = ogrid_update_cell(&s_grid, CELL, v);
        const ogrid_real_t lo = s_grid.cells[CELL].log_odds_lo;
        if ((st != OGRID_OK) || !(lo >= OGRID_LOG_ODDS_MIN) || !(lo <= OGRID_LOG_ODDS_MAX))
        {
            bad++;
        }
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, bad, "[TC-F004-14][SYS-SAF-07] log-odds must always stay within [-0.8, 3.4]");
}

/** @req SYS-PR-08 @contract post POST-1 monotonia: ecos repetidos no decrecen y 10 ecos dan 3,4 */
void test_TC_F004_15_repeated_echoes_monotonic_to_max(void)
{
    uint32_t bad = 0U;
    ogrid_real_t prev = s_grid.cells[CELL].log_odds_lo;
    for (uint32_t i = 0U; i < 10U; i++)
    {
        (void)ogrid_update_cell(&s_grid, CELL, OGRID_VOTE_KIND_ECHO);
        if (!(s_grid.cells[CELL].log_odds_lo >= prev))
        {
            bad++;
        }
        prev = s_grid.cells[CELL].log_odds_lo;
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, bad,
                                     "[TC-F004-15][SYS-PR-08] echoes must never lower log-odds");
    TEST_ASSERT_TRUE_MESSAGE(same_bits(prev, OGRID_LOG_ODDS_MAX),
                             "[TC-F004-15][SYS-PR-08] 10 echoes (8.5 > 3.4) must reach 3.4");
}

/** @req SYS-PR-08 @contract post POST-5 desde el minimo, tres ecos superan p = 0,85 (1,75 > 1,7346)
 */
void test_TC_F004_16_three_echoes_from_min_confirm(void)
{
    set_cell(CELL, OGRID_LOG_ODDS_MIN, OGRID_OBSERVED_YES);
    for (uint32_t i = 0U; i < 3U; i++)
    {
        (void)ogrid_update_cell(&s_grid, CELL, OGRID_VOTE_KIND_ECHO);
    }
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, 1.75F, s_grid.cells[CELL].log_odds_lo,
                                     "[TC-F004-16][SYS-PR-08] -0.8 + 3 x 0.85 must give 1.75");
    TEST_ASSERT_TRUE_MESSAGE(s_grid.cells[CELL].log_odds_lo > LN_THRESHOLD,
                             "[TC-F004-16][SYS-PR-08] 3 echoes from min must exceed ln(0.85/0.15)");
}

/** @req SYS-PR-08 @contract post POST-5 un eco aislado desde 0 no supera p = 0,85 */
void test_TC_F004_17_single_echo_does_not_confirm(void)
{
    (void)ogrid_update_cell(&s_grid, CELL, OGRID_VOTE_KIND_ECHO);
    TEST_ASSERT_TRUE_MESSAGE(s_grid.cells[CELL].log_odds_lo < LN_THRESHOLD,
                             "[TC-F004-17][SYS-PR-08] a single echo must not confirm an obstacle");
}

/** @req SYS-SAF-07 @contract pre PRE-1 grid_io NULL devuelve OGRID_ERR_PARAM con los tres votos */
void test_TC_F004_18_null_grid_is_param_error(void)
{
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_PARAM,
                                  ogrid_update_cell(NULL, CELL, OGRID_VOTE_KIND_ECHO),
                                  "[TC-F004-18][SYS-SAF-07] NULL grid with echo is ERR_PARAM");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_PARAM,
                                  ogrid_update_cell(NULL, CELL, OGRID_VOTE_KIND_PASS),
                                  "[TC-F004-18][SYS-SAF-07] NULL grid with pass is ERR_PARAM");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_PARAM,
                                  ogrid_update_cell(NULL, CELL, OGRID_VOTE_KIND_OCCLUDED),
                                  "[TC-F004-18][SYS-SAF-07] NULL grid with occluded is ERR_PARAM");
}

/** @req SYS-SAF-07 @contract pre PRE-2 vote invalido (COUNT, 99, -1) devuelve OGRID_ERR_PARAM */
void test_TC_F004_19_invalid_vote_is_param_error(void)
{
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_PARAM,
                                  ogrid_update_cell(&s_grid, CELL, OGRID_VOTE_KIND_COUNT),
                                  "[TC-F004-19][SYS-SAF-07] vote == KIND_COUNT is ERR_PARAM");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_PARAM,
                                  ogrid_update_cell(&s_grid, CELL, (ogrid_vote_kind_t)99),
                                  "[TC-F004-19][SYS-SAF-07] vote == 99 is ERR_PARAM");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_PARAM,
                                  ogrid_update_cell(&s_grid, CELL, (ogrid_vote_kind_t)-1),
                                  "[TC-F004-19][SYS-SAF-07] vote == -1 is ERR_PARAM");
}

/** @req SYS-SAF-07 @contract post POST-6 con vote invalido el grid no se modifica */
void test_TC_F004_20_invalid_vote_keeps_grid(void)
{
    (void)ogrid_update_cell(&s_grid, CELL, OGRID_VOTE_KIND_COUNT);
    (void)ogrid_update_cell(&s_grid, CELL, (ogrid_vote_kind_t)99);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, diff_cells(&s_grid, &s_ref),
                                     "[TC-F004-20][SYS-SAF-07] invalid vote must not modify grid");
}

/** @req SYS-SAF-07 @contract err OGRID_ERR_RANGE para cell_idx = COUNT (justo fuera), COUNT+1 y
 * UINT32_MAX */
void test_TC_F004_21_index_out_of_range_is_range_error(void)
{
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_RANGE, ogrid_update_cell(&s_grid, OGRID_CELL_COUNT, OGRID_VOTE_KIND_ECHO),
        "[TC-F004-21][SYS-SAF-07] idx == 3520 is ERR_RANGE");
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_RANGE, ogrid_update_cell(&s_grid, OGRID_CELL_COUNT + 1U, OGRID_VOTE_KIND_PASS),
        "[TC-F004-21][SYS-SAF-07] idx == 3521 is ERR_RANGE");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_RANGE,
                                  ogrid_update_cell(&s_grid, UINT32_MAX, OGRID_VOTE_KIND_OCCLUDED),
                                  "[TC-F004-21][SYS-SAF-07] idx == UINT32_MAX is ERR_RANGE");
}

/** @req SYS-SAF-07 @contract post POST-6 con cell_idx fuera de rango el grid no se modifica */
void test_TC_F004_22_range_error_keeps_grid(void)
{
    (void)ogrid_update_cell(&s_grid, OGRID_CELL_COUNT, OGRID_VOTE_KIND_ECHO);
    (void)ogrid_update_cell(&s_grid, UINT32_MAX, OGRID_VOTE_KIND_PASS);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, diff_cells(&s_grid, &s_ref),
                                     "[TC-F004-22][SYS-SAF-07] ERR_RANGE must not modify grid");
}

/** @req SYS-SAF-07 @contract err OGRID_ERR_STATE con log-odds NaN, +inf, -inf almacenados */
void test_TC_F004_23_non_finite_stored_is_state_error(void)
{
    const ogrid_real_t bad_vals[] = {NAN, INFINITY, -INFINITY};
    const ogrid_vote_kind_t votes[] = {OGRID_VOTE_KIND_ECHO, OGRID_VOTE_KIND_PASS};
    uint32_t bad = 0U;
    for (size_t k = 0U; k < 3U; k++)
    {
        for (size_t v = 0U; v < 2U; v++)
        {
            set_cell(CELL, bad_vals[k], OGRID_OBSERVED_YES);
            if (ogrid_update_cell(&s_grid, CELL, votes[v]) != OGRID_ERR_STATE)
            {
                bad++;
            }
        }
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, bad, "[TC-F004-23][SYS-SAF-07] NaN/inf stored must return ERR_STATE");
}

/** @req SYS-SAF-07 @contract err OGRID_ERR_STATE con log-odds un ulp fuera de limites y FLT_MAX */
void test_TC_F004_24_out_of_limits_stored_is_state_error(void)
{
    const ogrid_real_t bad_vals[] = {step_bits(OGRID_LOG_ODDS_MAX, true),
                                     step_bits(OGRID_LOG_ODDS_MIN, true),
                                     10.0F,
                                     -10.0F,
                                     FLT_MAX,
                                     -FLT_MAX};
    const ogrid_vote_kind_t votes[] = {OGRID_VOTE_KIND_ECHO, OGRID_VOTE_KIND_PASS};
    uint32_t bad = 0U;
    for (size_t k = 0U; k < 6U; k++)
    {
        for (size_t v = 0U; v < 2U; v++)
        {
            set_cell(CELL, bad_vals[k], OGRID_OBSERVED_NO);
            if (ogrid_update_cell(&s_grid, CELL, votes[v]) != OGRID_ERR_STATE)
            {
                bad++;
            }
        }
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, bad, "[TC-F004-24][SYS-SAF-07] stored value outside [-0.8, 3.4] is ERR_STATE");
}

/** @req SYS-SAF-07 @contract post POST-6 con celda corrupta el grid no se modifica (bit a bit) */
void test_TC_F004_25_state_error_keeps_grid(void)
{
    uint32_t bad = 0U;
    set_cell(CELL, NAN, OGRID_OBSERVED_NO);
    (void)ogrid_update_cell(&s_grid, CELL, OGRID_VOTE_KIND_ECHO);
    bad += diff_cells(&s_grid, &s_ref);
    set_cell(CELL, 5.0F, OGRID_OBSERVED_YES);
    (void)ogrid_update_cell(&s_grid, CELL, OGRID_VOTE_KIND_PASS);
    bad += diff_cells(&s_grid, &s_ref);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, bad, "[TC-F004-25][SYS-SAF-07] ERR_STATE must not modify observed nor log-odds");
}

/** @req SYS-SAF-07 @contract pre PRE-4 los valores almacenados exactamente -0,8 y 3,4 son validos
 */
void test_TC_F004_26_stored_limits_are_valid(void)
{
    set_cell(CELL, OGRID_LOG_ODDS_MIN, OGRID_OBSERVED_YES);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_update_cell(&s_grid, CELL, OGRID_VOTE_KIND_ECHO),
                                  "[TC-F004-26][SYS-SAF-07] stored -0.8 is a valid state");
    set_cell(CELL, OGRID_LOG_ODDS_MAX, OGRID_OBSERVED_YES);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_update_cell(&s_grid, CELL, OGRID_VOTE_KIND_PASS),
                                  "[TC-F004-26][SYS-SAF-07] stored 3.4 is a valid state");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, 3.0F, s_grid.cells[CELL].log_odds_lo,
                                     "[TC-F004-26][SYS-SAF-07] 3.4 - 0.4 must give 3.0");
}

/** @req SYS-SAF-07 @contract post POST-6 tras un error una llamada valida actualiza la celda */
void test_TC_F004_27_valid_call_after_error_works(void)
{
    (void)ogrid_update_cell(&s_grid, OGRID_CELL_COUNT, OGRID_VOTE_KIND_ECHO);
    (void)ogrid_update_cell(NULL, CELL, OGRID_VOTE_KIND_ECHO);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_update_cell(&s_grid, CELL, OGRID_VOTE_KIND_ECHO),
                                  "[TC-F004-27][SYS-SAF-07] valid call after errors must be OK");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, 0.85F, s_grid.cells[CELL].log_odds_lo,
                                     "[TC-F004-27][SYS-SAF-07] valid call after errors gives 0.85");
}

/** @req SYS-PR-08 @contract perf determinismo: misma secuencia sobre dos celdas, mismo resultado
 * exacto */
void test_TC_F004_28_deterministic(void)
{
    const ogrid_vote_kind_t seq[] = {OGRID_VOTE_KIND_ECHO, OGRID_VOTE_KIND_PASS,
                                     OGRID_VOTE_KIND_ECHO, OGRID_VOTE_KIND_OCCLUDED,
                                     OGRID_VOTE_KIND_ECHO};
    for (size_t i = 0U; i < 5U; i++)
    {
        (void)ogrid_update_cell(&s_grid, 7U, seq[i]);
        (void)ogrid_update_cell(&s_grid, 3000U, seq[i]);
    }
    TEST_ASSERT_TRUE_MESSAGE(same_bits(s_grid.cells[7].log_odds_lo, s_grid.cells[3000].log_odds_lo),
                             "[TC-F004-28][SYS-PR-08] same sequence must give identical result");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
        TOL, 2.15F, s_grid.cells[7].log_odds_lo,
        "[TC-F004-28][SYS-PR-08] echo,pass,echo,occluded,echo must give 2.15");
}

/** @req SYS-SAF-07 @contract pre PRE-4 con voto oculta una celda corrupta devuelve ERR_STATE sin
 * cambios */
void test_TC_F004_29_occluded_with_corrupt_cell_is_state_error(void)
{
    const ogrid_real_t bad_vals[] = {NAN,
                                     INFINITY,
                                     -INFINITY,
                                     step_bits(OGRID_LOG_ODDS_MAX, true),
                                     step_bits(OGRID_LOG_ODDS_MIN, true),
                                     10.0F,
                                     -10.0F};
    uint32_t bad = 0U;
    for (size_t k = 0U; k < 7U; k++)
    {
        set_cell(CELL, bad_vals[k], (((k % 2U) == 0U) ? OGRID_OBSERVED_YES : OGRID_OBSERVED_NO));
        if (ogrid_update_cell(&s_grid, CELL, OGRID_VOTE_KIND_OCCLUDED) != OGRID_ERR_STATE)
        {
            bad++;
        }
        bad += diff_cells(&s_grid, &s_ref);
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, bad, "[TC-F004-29][SYS-SAF-07] occluded on corrupt cell must be ERR_STATE, no change");
}

/** @req SYS-SAF-07 @contract pre PRE-1 gana a PRE-2 y PRE-3: grid nulo con indice o voto invalidos
 */
void test_TC_F004_30_null_grid_wins_over_other_failures(void)
{
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_PARAM, ogrid_update_cell(NULL, OGRID_CELL_COUNT, OGRID_VOTE_KIND_ECHO),
        "[TC-F004-30][SYS-SAF-07] NULL grid + idx out of range must be ERR_PARAM");
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_PARAM, ogrid_update_cell(NULL, CELL, OGRID_VOTE_KIND_COUNT),
        "[TC-F004-30][SYS-SAF-07] NULL grid + invalid vote must be ERR_PARAM");
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_PARAM, ogrid_update_cell(NULL, UINT32_MAX, (ogrid_vote_kind_t)99),
        "[TC-F004-30][SYS-SAF-07] NULL grid + invalid vote + idx out must be ERR_PARAM");
}

/** @req SYS-SAF-07 @contract pre PRE-2 gana a PRE-3 y PRE-4: voto invalido con indice fuera o celda
 * corrupta */
void test_TC_F004_31_invalid_vote_wins_over_range_and_state(void)
{
    uint32_t bad = 0U;
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_PARAM, ogrid_update_cell(&s_grid, OGRID_CELL_COUNT, OGRID_VOTE_KIND_COUNT),
        "[TC-F004-31][SYS-SAF-07] invalid vote + idx out of range must be ERR_PARAM");
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_PARAM, ogrid_update_cell(&s_grid, UINT32_MAX, (ogrid_vote_kind_t)-1),
        "[TC-F004-31][SYS-SAF-07] invalid vote -1 + idx UINT32_MAX must be ERR_PARAM");
    set_cell(CELL, NAN, OGRID_OBSERVED_YES);
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_PARAM, ogrid_update_cell(&s_grid, CELL, (ogrid_vote_kind_t)99),
        "[TC-F004-31][SYS-SAF-07] invalid vote + corrupt cell must be ERR_PARAM");
    bad += diff_cells(&s_grid, &s_ref);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, bad,
                                     "[TC-F004-31][SYS-SAF-07] ERR_PARAM must not modify grid");
}

/** @req SYS-SAF-07 @contract pre PRE-3 gana a PRE-4: indice fuera de rango con grid corrupto */
void test_TC_F004_32_range_wins_over_corrupt_cell(void)
{
    const ogrid_vote_kind_t votes[] = {OGRID_VOTE_KIND_ECHO, OGRID_VOTE_KIND_PASS,
                                       OGRID_VOTE_KIND_OCCLUDED};
    uint32_t bad = 0U;
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        s_grid.cells[i].log_odds_lo = NAN;
        s_grid.cells[i].observed = OGRID_OBSERVED_YES;
    }
    s_ref = s_grid;
    for (size_t v = 0U; v < 3U; v++)
    {
        if (ogrid_update_cell(&s_grid, OGRID_CELL_COUNT, votes[v]) != OGRID_ERR_RANGE)
        {
            bad++;
        }
        if (ogrid_update_cell(&s_grid, UINT32_MAX, votes[v]) != OGRID_ERR_RANGE)
        {
            bad++;
        }
        /* Same grid, valid index: now the corruption is what fails. */
        if (ogrid_update_cell(&s_grid, CELL, votes[v]) != OGRID_ERR_STATE)
        {
            bad++;
        }
    }
    bad += diff_cells(&s_grid, &s_ref);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, bad,
        "[TC-F004-32][SYS-SAF-07] idx out of range must be ERR_RANGE, valid idx ERR_STATE");
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_TC_F004_01_valid_calls_return_ok);
    RUN_TEST(test_TC_F004_02_echo_from_zero);
    RUN_TEST(test_TC_F004_03_three_echoes_sequence);
    RUN_TEST(test_TC_F004_04_echo_saturates_at_max);
    RUN_TEST(test_TC_F004_05_echo_from_min);
    RUN_TEST(test_TC_F004_06_pass_from_zero);
    RUN_TEST(test_TC_F004_07_pass_at_min_stays);
    RUN_TEST(test_TC_F004_08_pass_crossing_min_is_clamped);
    RUN_TEST(test_TC_F004_09_echo_then_pass);
    RUN_TEST(test_TC_F004_10_occluded_unobserved_unchanged);
    RUN_TEST(test_TC_F004_11_occluded_observed_unchanged);
    RUN_TEST(test_TC_F004_12_other_cells_unchanged);
    RUN_TEST(test_TC_F004_13_index_boundaries_update_own_cell);
    RUN_TEST(test_TC_F004_14_mixed_sequence_bounded);
    RUN_TEST(test_TC_F004_15_repeated_echoes_monotonic_to_max);
    RUN_TEST(test_TC_F004_16_three_echoes_from_min_confirm);
    RUN_TEST(test_TC_F004_17_single_echo_does_not_confirm);
    RUN_TEST(test_TC_F004_18_null_grid_is_param_error);
    RUN_TEST(test_TC_F004_19_invalid_vote_is_param_error);
    RUN_TEST(test_TC_F004_20_invalid_vote_keeps_grid);
    RUN_TEST(test_TC_F004_21_index_out_of_range_is_range_error);
    RUN_TEST(test_TC_F004_22_range_error_keeps_grid);
    RUN_TEST(test_TC_F004_23_non_finite_stored_is_state_error);
    RUN_TEST(test_TC_F004_24_out_of_limits_stored_is_state_error);
    RUN_TEST(test_TC_F004_25_state_error_keeps_grid);
    RUN_TEST(test_TC_F004_26_stored_limits_are_valid);
    RUN_TEST(test_TC_F004_27_valid_call_after_error_works);
    RUN_TEST(test_TC_F004_28_deterministic);
    RUN_TEST(test_TC_F004_29_occluded_with_corrupt_cell_is_state_error);
    RUN_TEST(test_TC_F004_30_null_grid_wins_over_other_failures);
    RUN_TEST(test_TC_F004_31_invalid_vote_wins_over_range_and_state);
    RUN_TEST(test_TC_F004_32_range_wins_over_corrupt_cell);
    return UNITY_END();
}
