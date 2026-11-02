/**
 * @file test_ogrid_extract_occupied.c
 * @brief Unit tests of ogrid_extract_occupied (F-007).
 * @details Probabilities are p = 1 / (1 + exp(-l)). The decision threshold is
 *          l* = ln(0.85 / 0.15) = 1.734601. Cases never sit exactly on l*, because float32 rounding
 *          makes p == 0.85 undecidable; they use l = 1.70 (p = 0.8455) and 2.55 (p = 0.9276) given
 *          by the contract, and l = 1.73 (p = 0.8494) and 1.74 (p = 0.8504), 4e-4 away from the
 *          threshold, far above the float32 error of exp (about 1e-7). Log-odds limits are
 *          compared with the macros themselves, so no numeric tolerance is needed. Expected masks
 *          are computed by hand from the cell patterns, not by running the implementation, except
 *          in TC-F007-14 whose purpose is the consistency with ogrid_get_cell_state (POST-1).
 */
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ogrid/ogrid.h"
#include "ogrid/ogrid_extract_occupied.h"
#include "ogrid/ogrid_get_cell_state.h"
#include "ogrid/ogrid_limits.h"
#include "unity.h"

/** Cell used by single-cell tests. */
#define CELL (42U)
/** Last valid cell index. */
#define LAST_CELL (OGRID_CELL_COUNT - 1U)
/** Mask has one extra guard element after the contract array to detect overruns. */
#define MASK_LEN (OGRID_CELL_COUNT + 1U)

static ogrid_t s_grid;
static ogrid_t s_ref;
static bool s_mask[MASK_LEN];

/** Fill the whole mask (guard included) with a value. */
static void fill_mask(bool v)
{
    for (uint32_t i = 0U; i < MASK_LEN; i++)
    {
        s_mask[i] = v;
    }
}

/** Number of true values among the OGRID_CELL_COUNT first mask elements. */
static uint32_t count_true(void)
{
    uint32_t n = 0U;
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        if (s_mask[i])
        {
            n++;
        }
    }
    return n;
}

/** Number of cells whose log-odds or observed flag differ between s_grid and s_ref. */
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

void setUp(void)
{
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        s_grid.cells[i].log_odds_lo = OGRID_LOG_ODDS_INIT;
        s_grid.cells[i].observed = OGRID_OBSERVED_NO;
    }
    s_ref = s_grid;
    fill_mask(false);
}

void tearDown(void)
{
}

/** @req SYS-PR-04 @contract pre PRE-1 (grid NULL) / err OGRID_ERR_PARAM / post POST-5 */
void test_TC_F007_01_null_grid_returns_param_and_keeps_mask(void)
{
    fill_mask(true);
    s_mask[7] = false;
    const ogrid_status_t st = ogrid_extract_occupied(NULL, s_mask);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_PARAM, st,
                                  "[TC-F007-01][SYS-PR-04] grid NULL must return OGRID_ERR_PARAM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        OGRID_CELL_COUNT - 1U, count_true(),
        "[TC-F007-01][SYS-PR-04] mask must stay unmodified (one false)");
    TEST_ASSERT_FALSE_MESSAGE(s_mask[7], "[TC-F007-01][SYS-PR-04] mask[7] must stay false");
    TEST_ASSERT_TRUE_MESSAGE(s_mask[8], "[TC-F007-01][SYS-PR-04] mask[8] must stay true");
}

/** @req SYS-PR-04 @contract pre PRE-1 (mask_out NULL) / err OGRID_ERR_PARAM / post POST-5 */
void test_TC_F007_02_null_mask_returns_param_and_keeps_grid(void)
{
    set_cell(CELL, 2.55F, OGRID_OBSERVED_YES);
    const ogrid_status_t st = ogrid_extract_occupied(&s_grid, NULL);
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_PARAM, st, "[TC-F007-02][SYS-PR-04] mask_out NULL must return OGRID_ERR_PARAM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, grid_changes(),
                                     "[TC-F007-02][SYS-PR-04] grid must not be modified");
}

/** @req SYS-PR-04 @contract pre PRE-1 (both NULL) / err OGRID_ERR_PARAM */
void test_TC_F007_03_both_null_returns_param(void)
{
    const ogrid_status_t st = ogrid_extract_occupied(NULL, NULL);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_PARAM, st,
                                  "[TC-F007-03][SYS-PR-04] both NULL must return OGRID_ERR_PARAM");
}

/** @req SYS-PR-04 @contract post POST-1 (all unobserved, prefilled true gets overwritten) */
void test_TC_F007_04_all_unobserved_gives_all_false(void)
{
    fill_mask(true);
    const ogrid_status_t st = ogrid_extract_occupied(&s_grid, s_mask);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st,
                                  "[TC-F007-04][SYS-PR-04] fresh grid must return OGRID_OK");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, count_true(),
                                     "[TC-F007-04][SYS-PR-04] every cell must be written false");
    TEST_ASSERT_TRUE_MESSAGE(s_mask[OGRID_CELL_COUNT],
                             "[TC-F007-04][SYS-PR-04] element after the array must not be written");
}

/** @req SYS-PR-08 @contract post POST-2 (unobserved never true, whatever its log-odds) */
void test_TC_F007_05_unobserved_with_high_log_odds_is_false(void)
{
    set_cell(1U, OGRID_LOG_ODDS_MAX, OGRID_OBSERVED_NO);
    set_cell(2U, 100.0F, OGRID_OBSERVED_NO);
    set_cell(3U, NAN, OGRID_OBSERVED_NO);
    set_cell(4U, INFINITY, OGRID_OBSERVED_NO);
    set_cell(5U, -INFINITY, OGRID_OBSERVED_NO);
    fill_mask(true);
    const ogrid_status_t st = ogrid_extract_occupied(&s_grid, s_mask);
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_OK, st, "[TC-F007-05][SYS-PR-08] unobserved cells are never corrupt: OGRID_OK");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, count_true(),
                                     "[TC-F007-05][SYS-PR-08] unobserved cells must be false");
}

/** @req SYS-PR-08 @contract post POST-3 (l = 1.70 -> p = 0.8455 false) */
void test_TC_F007_06_probability_0_8455_is_not_occupied(void)
{
    set_cell(CELL, 1.70F, OGRID_OBSERVED_YES);
    fill_mask(true);
    const ogrid_status_t st = ogrid_extract_occupied(&s_grid, s_mask);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st, "[TC-F007-06][SYS-PR-08] must return OGRID_OK");
    TEST_ASSERT_FALSE_MESSAGE(s_mask[CELL], "[TC-F007-06][SYS-PR-08] p = 0.8455 must be false");
}

/** @req SYS-PR-08 @contract post POST-3 (l = 2.55 -> p = 0.9276 true) */
void test_TC_F007_07_probability_0_9276_is_occupied(void)
{
    set_cell(CELL, 2.55F, OGRID_OBSERVED_YES);
    const ogrid_status_t st = ogrid_extract_occupied(&s_grid, s_mask);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st, "[TC-F007-07][SYS-PR-08] must return OGRID_OK");
    TEST_ASSERT_TRUE_MESSAGE(s_mask[CELL], "[TC-F007-07][SYS-PR-08] p = 0.9276 must be true");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, count_true(),
                                     "[TC-F007-07][SYS-PR-08] only that cell must be true");
}

/** @req SYS-PR-08 @contract post POST-1 (just either side of the threshold, 4e-4 in p) */
void test_TC_F007_08_threshold_neighbourhood(void)
{
    set_cell(10U, 1.73F, OGRID_OBSERVED_YES);
    set_cell(11U, 1.74F, OGRID_OBSERVED_YES);
    const ogrid_status_t st = ogrid_extract_occupied(&s_grid, s_mask);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st, "[TC-F007-08][SYS-PR-08] must return OGRID_OK");
    TEST_ASSERT_FALSE_MESSAGE(s_mask[10], "[TC-F007-08][SYS-PR-08] l = 1.73 (p=0.8494) is false");
    TEST_ASSERT_TRUE_MESSAGE(s_mask[11], "[TC-F007-08][SYS-PR-08] l = 1.74 (p=0.8504) is true");
}

/** @req SYS-PR-08 @contract post POST-1 (limits of the log-odds range, observed) */
void test_TC_F007_09_log_odds_range_limits_are_valid(void)
{
    set_cell(20U, OGRID_LOG_ODDS_MIN, OGRID_OBSERVED_YES);
    set_cell(21U, OGRID_LOG_ODDS_MAX, OGRID_OBSERVED_YES);
    set_cell(22U, 0.0F, OGRID_OBSERVED_YES);
    fill_mask(true);
    const ogrid_status_t st = ogrid_extract_occupied(&s_grid, s_mask);
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_OK, st, "[TC-F007-09][SYS-PR-08] log-odds exactly at the limits are valid: OGRID_OK");
    TEST_ASSERT_FALSE_MESSAGE(s_mask[20], "[TC-F007-09][SYS-PR-08] l = -0.8 must be false");
    TEST_ASSERT_TRUE_MESSAGE(s_mask[21], "[TC-F007-09][SYS-PR-08] l = 3.4 (p~0.97) must be true");
    TEST_ASSERT_FALSE_MESSAGE(s_mask[22], "[TC-F007-09][SYS-PR-08] l = 0 (p=0.5) must be false");
}

/** @req SYS-PR-08 @contract err OGRID_ERR_STATE / post POST-4 (just above the maximum) */
void test_TC_F007_10_log_odds_just_above_max_is_corrupt(void)
{
    set_cell(CELL, nextafterf(OGRID_LOG_ODDS_MAX, INFINITY), OGRID_OBSERVED_YES);
    fill_mask(true);
    const ogrid_status_t st = ogrid_extract_occupied(&s_grid, s_mask);
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_STATE, st, "[TC-F007-10][SYS-PR-08] l above 3.4 must return OGRID_ERR_STATE");
    TEST_ASSERT_FALSE_MESSAGE(s_mask[CELL],
                              "[TC-F007-10][SYS-PR-08] the corrupt cell must be false");
}

/** @req SYS-PR-08 @contract err OGRID_ERR_STATE / post POST-4 (just below the minimum) */
void test_TC_F007_11_log_odds_just_below_min_is_corrupt(void)
{
    set_cell(CELL, nextafterf(OGRID_LOG_ODDS_MIN, -INFINITY), OGRID_OBSERVED_YES);
    fill_mask(true);
    const ogrid_status_t st = ogrid_extract_occupied(&s_grid, s_mask);
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_STATE, st, "[TC-F007-11][SYS-PR-08] l below -0.8 must return OGRID_ERR_STATE");
    TEST_ASSERT_FALSE_MESSAGE(s_mask[CELL],
                              "[TC-F007-11][SYS-PR-08] the corrupt cell must be false");
}

/** @req SYS-PR-08 @contract err OGRID_ERR_STATE / post POST-4 (NaN and infinities, observed) */
void test_TC_F007_12_non_finite_log_odds_is_corrupt(void)
{
    const ogrid_real_t bad[3] = {NAN, INFINITY, -INFINITY};
    for (uint32_t k = 0U; k < 3U; k++)
    {
        setUp();
        set_cell(CELL, bad[k], OGRID_OBSERVED_YES);
        fill_mask(true);
        const ogrid_status_t st = ogrid_extract_occupied(&s_grid, s_mask);
        TEST_ASSERT_EQUAL_INT_MESSAGE(
            OGRID_ERR_STATE, st,
            "[TC-F007-12][SYS-PR-08] NaN/inf on an observed cell must return OGRID_ERR_STATE");
        TEST_ASSERT_FALSE_MESSAGE(s_mask[CELL],
                                  "[TC-F007-12][SYS-PR-08] non-finite cell must be false");
    }
}

/** @req SYS-PR-08 @contract err OGRID_ERR_STATE / post POST-4 (observed neither 0 nor 1) */
void test_TC_F007_13_observed_flag_out_of_domain_is_corrupt(void)
{
    const uint8_t bad[3] = {2U, 128U, 255U};
    for (uint32_t k = 0U; k < 3U; k++)
    {
        setUp();
        set_cell(CELL, 3.0F, bad[k]); /* high log-odds: must still be false */
        fill_mask(true);
        const ogrid_status_t st = ogrid_extract_occupied(&s_grid, s_mask);
        TEST_ASSERT_EQUAL_INT_MESSAGE(
            OGRID_ERR_STATE, st,
            "[TC-F007-13][SYS-PR-08] observed not in {0,1} must return OGRID_ERR_STATE");
        TEST_ASSERT_FALSE_MESSAGE(s_mask[CELL],
                                  "[TC-F007-13][SYS-PR-08] corrupt flag cell must be false");
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, count_true(),
                                         "[TC-F007-13][SYS-PR-08] no other cell is true");
    }
}

/**
 * @req SYS-PR-04 @req SYS-PR-08 @contract post POST-1
 * Cell pattern by residue of idx modulo 7 (hand computed expectation):
 * 0 unobserved l=3.0 -> false; 1 l=1.70 -> false; 2 l=2.55 -> true; 3 l=-0.8 -> false;
 * 4 l=3.4 -> true; 5 l=0 -> false; 6 l=1.74 -> true.
 * 3520 = 7 * 502 + 6, so residues 0..5 occur 503 times and residue 6 occurs 502 times for the
 * extra indices 3514..3519 (residues 0..5): true count = 502*3 + 2 (residues 2 and 4) = 1508.
 */
static void load_pattern(void)
{
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        switch (i % 7U)
        {
            case 0U:
                s_grid.cells[i].log_odds_lo = 3.0F;
                s_grid.cells[i].observed = OGRID_OBSERVED_NO;
                break;
            case 1U:
                s_grid.cells[i].log_odds_lo = 1.70F;
                s_grid.cells[i].observed = OGRID_OBSERVED_YES;
                break;
            case 2U:
                s_grid.cells[i].log_odds_lo = 2.55F;
                s_grid.cells[i].observed = OGRID_OBSERVED_YES;
                break;
            case 3U:
                s_grid.cells[i].log_odds_lo = OGRID_LOG_ODDS_MIN;
                s_grid.cells[i].observed = OGRID_OBSERVED_YES;
                break;
            case 4U:
                s_grid.cells[i].log_odds_lo = OGRID_LOG_ODDS_MAX;
                s_grid.cells[i].observed = OGRID_OBSERVED_YES;
                break;
            case 5U:
                s_grid.cells[i].log_odds_lo = 0.0F;
                s_grid.cells[i].observed = OGRID_OBSERVED_YES;
                break;
            default:
                s_grid.cells[i].log_odds_lo = 1.74F;
                s_grid.cells[i].observed = OGRID_OBSERVED_YES;
                break;
        }
    }
    s_ref = s_grid;
}

/** Hand expectation for load_pattern. */
static bool pattern_expected(uint32_t i)
{
    const uint32_t r = i % 7U;
    return (r == 2U) || (r == 4U) || (r == 6U);
}

/** @req SYS-PR-04 @contract post POST-1 / err OGRID_OK (full mixed grid, every cell checked) */
void test_TC_F007_14_full_pattern_matches_hand_expectation(void)
{
    load_pattern();
    fill_mask(false);
    const ogrid_status_t st = ogrid_extract_occupied(&s_grid, s_mask);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st, "[TC-F007-14][SYS-PR-04] must return OGRID_OK");
    uint32_t wrong = 0U;
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        if (s_mask[i] != pattern_expected(i))
        {
            wrong++;
        }
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, wrong,
                                     "[TC-F007-14][SYS-PR-04] all 3520 cells must match");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1508U, count_true(),
                                     "[TC-F007-14][SYS-PR-04] exactly 1508 occupied cells");
    TEST_ASSERT_FALSE_MESSAGE(s_mask[OGRID_CELL_COUNT],
                              "[TC-F007-14][SYS-PR-04] guard element must not be written");
}

/** @req SYS-PR-04 @req SYS-PR-08 @contract post POST-1 (consistency with ogrid_get_cell_state) */
void test_TC_F007_15_mask_agrees_with_cell_state_everywhere(void)
{
    load_pattern();
    const ogrid_status_t st = ogrid_extract_occupied(&s_grid, s_mask);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st, "[TC-F007-15][SYS-PR-08] must return OGRID_OK");
    uint32_t disagree = 0U;
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        ogrid_cell_state_t cs = OGRID_CELL_UNSURVEYED;
        const ogrid_status_t gs = ogrid_get_cell_state(&s_grid, i, &cs);
        if ((gs != OGRID_OK) || (s_mask[i] != (cs == OGRID_CELL_OCCUPIED)))
        {
            disagree++;
        }
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, disagree, "[TC-F007-15][SYS-PR-08] mask must equal (state == OCCUPIED) in every cell");
}

/** @req SYS-PR-04 @contract post POST-1 (first and last index of the array) */
void test_TC_F007_16_first_and_last_cell_are_handled(void)
{
    set_cell(0U, 2.55F, OGRID_OBSERVED_YES);
    set_cell(LAST_CELL, 2.55F, OGRID_OBSERVED_YES);
    const ogrid_status_t st = ogrid_extract_occupied(&s_grid, s_mask);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st, "[TC-F007-16][SYS-PR-04] must return OGRID_OK");
    TEST_ASSERT_TRUE_MESSAGE(s_mask[0], "[TC-F007-16][SYS-PR-04] cell 0 must be true");
    TEST_ASSERT_TRUE_MESSAGE(s_mask[LAST_CELL], "[TC-F007-16][SYS-PR-04] last cell must be true");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2U, count_true(),
                                     "[TC-F007-16][SYS-PR-04] only two cells are true");
    TEST_ASSERT_FALSE_MESSAGE(s_mask[OGRID_CELL_COUNT],
                              "[TC-F007-16][SYS-PR-04] guard element must not be written");
}

/**
 * @req SYS-PR-08 @contract post POST-4 / err OGRID_ERR_STATE
 * Several corrupt cells give one ERR_STATE, the whole mask is still computed, first and last
 * cell included.
 */
void test_TC_F007_17_several_corrupt_cells_rest_of_mask_is_computed(void)
{
    set_cell(0U, 3.0F, 2U);                             /* corrupt: flag */
    set_cell(10U, 2.55F, OGRID_OBSERVED_YES);           /* true */
    set_cell(500U, 2.55F, 255U);                        /* corrupt: flag, high l */
    set_cell(501U, 2.55F, OGRID_OBSERVED_YES);          /* true */
    set_cell(1000U, NAN, OGRID_OBSERVED_YES);           /* corrupt: NaN */
    set_cell(2000U, INFINITY, OGRID_OBSERVED_YES);      /* corrupt: +inf */
    set_cell(3000U, 3.5F, OGRID_OBSERVED_YES);          /* corrupt: > max */
    set_cell(3001U, -0.9F, OGRID_OBSERVED_YES);         /* corrupt: < min */
    set_cell(3002U, 1.70F, OGRID_OBSERVED_YES);         /* false */
    set_cell(3003U, 2.55F, OGRID_OBSERVED_YES);         /* true, after corrupt ones */
    set_cell(LAST_CELL, -INFINITY, OGRID_OBSERVED_YES); /* corrupt: -inf, last */
    fill_mask(true);
    const ogrid_status_t st = ogrid_extract_occupied(&s_grid, s_mask);
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_STATE, st, "[TC-F007-17][SYS-PR-08] corrupt cells must give OGRID_ERR_STATE");
    TEST_ASSERT_FALSE_MESSAGE(s_mask[0], "[TC-F007-17][SYS-PR-08] corrupt cell 0 is false");
    TEST_ASSERT_FALSE_MESSAGE(s_mask[500], "[TC-F007-17][SYS-PR-08] corrupt cell 500 is false");
    TEST_ASSERT_FALSE_MESSAGE(s_mask[1000], "[TC-F007-17][SYS-PR-08] NaN cell 1000 is false");
    TEST_ASSERT_FALSE_MESSAGE(s_mask[2000], "[TC-F007-17][SYS-PR-08] +inf cell 2000 is false");
    TEST_ASSERT_FALSE_MESSAGE(s_mask[3000], "[TC-F007-17][SYS-PR-08] cell 3000 above max is false");
    TEST_ASSERT_FALSE_MESSAGE(s_mask[3001], "[TC-F007-17][SYS-PR-08] cell 3001 below min is false");
    TEST_ASSERT_FALSE_MESSAGE(s_mask[LAST_CELL],
                              "[TC-F007-17][SYS-PR-08] last corrupt cell is false");
    TEST_ASSERT_TRUE_MESSAGE(s_mask[10], "[TC-F007-17][SYS-PR-08] valid occupied cell 10 is true");
    TEST_ASSERT_TRUE_MESSAGE(s_mask[501],
                             "[TC-F007-17][SYS-PR-08] valid occupied cell 501 is true");
    TEST_ASSERT_FALSE_MESSAGE(s_mask[3002], "[TC-F007-17][SYS-PR-08] valid p=0.8455 cell is false");
    TEST_ASSERT_TRUE_MESSAGE(s_mask[3003],
                             "[TC-F007-17][SYS-PR-08] valid cell after corrupt ones is true");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        3U, count_true(), "[TC-F007-17][SYS-PR-08] exactly cells 10, 501, 3003 are true");
}

/** @req SYS-PR-08 @contract post POST-5 (grid untouched on OK, on STATE and on PARAM) */
void test_TC_F007_18_grid_is_never_modified(void)
{
    load_pattern();
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_extract_occupied(&s_grid, s_mask),
                                  "[TC-F007-18][SYS-PR-08] pattern grid must return OGRID_OK");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, grid_changes(),
                                     "[TC-F007-18][SYS-PR-08] grid unchanged after OGRID_OK");
    set_cell(5U, NAN, OGRID_OBSERVED_YES);
    set_cell(6U, 0.0F, 9U);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_STATE, ogrid_extract_occupied(&s_grid, s_mask),
                                  "[TC-F007-18][SYS-PR-08] corrupt grid must return ERR_STATE");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, grid_changes(), "[TC-F007-18][SYS-PR-08] grid unchanged after OGRID_ERR_STATE");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_PARAM, ogrid_extract_occupied(&s_grid, NULL),
                                  "[TC-F007-18][SYS-PR-08] NULL mask must return ERR_PARAM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, grid_changes(), "[TC-F007-18][SYS-PR-08] grid unchanged after OGRID_ERR_PARAM");
}

/**
 * @req SYS-PR-08 @contract post POST-1
 * Property: monotonicity in l. Cell i holds l = -0.8 + 0.05 * i (i = 0..83, so l <= 3.35). The
 * mask must be false up to l = 1.70 (i = 50), true from l = 1.75 (i = 51): a single step. The step
 * of 0.05 is far larger than the float32 error, so l(i) never reaches the threshold 1.7346.
 */
void test_TC_F007_19_mask_is_monotone_in_log_odds(void)
{
    for (uint32_t i = 0U; i <= 83U; i++)
    {
        ogrid_real_t l = OGRID_LOG_ODDS_MIN + (0.05F * (ogrid_real_t)i);
        l = (l < OGRID_LOG_ODDS_MIN) ? OGRID_LOG_ODDS_MIN : l;
        l = (l > OGRID_LOG_ODDS_MAX) ? OGRID_LOG_ODDS_MAX : l;
        s_grid.cells[i].log_odds_lo = l;
        s_grid.cells[i].observed = OGRID_OBSERVED_YES;
    }
    s_ref = s_grid;
    fill_mask(true);
    const ogrid_status_t st = ogrid_extract_occupied(&s_grid, s_mask);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st, "[TC-F007-19][SYS-PR-08] must return OGRID_OK");
    uint32_t transitions = 0U;
    for (uint32_t i = 1U; i <= 83U; i++)
    {
        TEST_ASSERT_FALSE_MESSAGE(s_mask[i - 1U] && !s_mask[i],
                                  "[TC-F007-19][SYS-PR-08] mask must not go from true to false");
        if (s_mask[i] != s_mask[i - 1U])
        {
            transitions++;
        }
    }
    TEST_ASSERT_FALSE_MESSAGE(s_mask[50], "[TC-F007-19][SYS-PR-08] l = 1.70 must be false");
    TEST_ASSERT_TRUE_MESSAGE(s_mask[51], "[TC-F007-19][SYS-PR-08] l = 1.75 must be true");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, transitions,
                                     "[TC-F007-19][SYS-PR-08] exactly one false->true transition");
}

/** @req SYS-PR-04 @contract post POST-1 (determinism: same input, same output) */
void test_TC_F007_20_repeated_calls_give_same_mask(void)
{
    static bool second[MASK_LEN];
    load_pattern();
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_extract_occupied(&s_grid, s_mask),
                                  "[TC-F007-20][SYS-PR-04] first call must return OGRID_OK");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_extract_occupied(&s_grid, second),
                                  "[TC-F007-20][SYS-PR-04] second call must return OGRID_OK");
    uint32_t diff = 0U;
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        if (s_mask[i] != second[i])
        {
            diff++;
        }
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, diff, "[TC-F007-20][SYS-PR-04] both masks must be equal");
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_TC_F007_01_null_grid_returns_param_and_keeps_mask);
    RUN_TEST(test_TC_F007_02_null_mask_returns_param_and_keeps_grid);
    RUN_TEST(test_TC_F007_03_both_null_returns_param);
    RUN_TEST(test_TC_F007_04_all_unobserved_gives_all_false);
    RUN_TEST(test_TC_F007_05_unobserved_with_high_log_odds_is_false);
    RUN_TEST(test_TC_F007_06_probability_0_8455_is_not_occupied);
    RUN_TEST(test_TC_F007_07_probability_0_9276_is_occupied);
    RUN_TEST(test_TC_F007_08_threshold_neighbourhood);
    RUN_TEST(test_TC_F007_09_log_odds_range_limits_are_valid);
    RUN_TEST(test_TC_F007_10_log_odds_just_above_max_is_corrupt);
    RUN_TEST(test_TC_F007_11_log_odds_just_below_min_is_corrupt);
    RUN_TEST(test_TC_F007_12_non_finite_log_odds_is_corrupt);
    RUN_TEST(test_TC_F007_13_observed_flag_out_of_domain_is_corrupt);
    RUN_TEST(test_TC_F007_14_full_pattern_matches_hand_expectation);
    RUN_TEST(test_TC_F007_15_mask_agrees_with_cell_state_everywhere);
    RUN_TEST(test_TC_F007_16_first_and_last_cell_are_handled);
    RUN_TEST(test_TC_F007_17_several_corrupt_cells_rest_of_mask_is_computed);
    RUN_TEST(test_TC_F007_18_grid_is_never_modified);
    RUN_TEST(test_TC_F007_19_mask_is_monotone_in_log_odds);
    RUN_TEST(test_TC_F007_20_repeated_calls_give_same_mask);
    return UNITY_END();
}
