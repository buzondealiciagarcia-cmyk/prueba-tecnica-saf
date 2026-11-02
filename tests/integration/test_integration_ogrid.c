/**
 * @file test_integration_ogrid.c
 * @brief Black-box integration tests of the ogrid module through its public API.
 * @details Combines ogrid_reset_grid, ogrid_update_ray, ogrid_get_cell_state and
 *          ogrid_extract_occupied. Every expected value is computed by hand from the requirements
 *          and the design: echo vote +0.85, pass vote -0.40, clamp [-0.8, 3.4], initial 0,
 *          occupied when l > ln(0.85 / 0.15) = 1.7346. No case sits on that threshold: the
 *          occupied/not occupied boundaries used are 1.70 / 1.75 (about 0.015 away, far above the
 *          float32 accumulation error of a few 1e-7).
 *
 *          Hand-computed echo series from 0:       0.85, 1.70 (low), 2.55 (occupied).
 *          Hand-computed pass series from 0:       -0.40, -0.80, -0.80 (clamped).
 *          Echo series from the minimum -0.8:      0.05, 0.90, 1.75 (occupied).
 *          Log-odds tolerance 1e-4: at most four float32 additions of magnitude < 4, error < 1e-6.
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ogrid/ogrid.h"
#include "ogrid/ogrid_extract_occupied.h"
#include "ogrid/ogrid_get_cell_state.h"
#include "ogrid/ogrid_limits.h"
#include "ogrid/ogrid_reset_grid.h"
#include "ogrid/ogrid_update_ray.h"
#include "unity.h"

/** Message with the mandatory format [TC-INT-NN][REQ] text (TST-CAS-05). */
#define MSG(tc, req, text) "[TC-INT-" tc "][" req "] " text

/** Tolerance for hand-computed log-odds. */
#define LO_TOL (1e-4F)

/** Cell index from along-track index s and across index d (row-major). */
#define CELL_AT(s, d) (((uint32_t)(s) * OGRID_WIDTH_CELLS) + (uint32_t)(d))

/** Counters of derived states. */
typedef struct
{
    uint32_t occupied;
    uint32_t low;
    uint32_t unsurveyed;
} state_count_t;

static ogrid_t s_grid;
static bool s_mask[OGRID_CELL_COUNT];

void setUp(void)
{
    (void)ogrid_reset_grid(&s_grid);
    /* Dirty mask: proves that the extraction writes every one of the 3 520 entries. */
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        s_mask[i] = true;
    }
}

void tearDown(void)
{
}

/** Apply one beam. */
static ogrid_status_t beam(const uint32_t *cells, uint32_t n, bool echo, uint32_t pos)
{
    return ogrid_update_ray(&s_grid, cells, n, echo, pos);
}

/** Apply a one-cell beam with echo. */
static ogrid_status_t echo1(uint32_t idx)
{
    const uint32_t cells[1] = {idx};
    return beam(cells, 1U, true, 0U);
}

/** Apply a one-cell beam without echo. */
static ogrid_status_t pass1(uint32_t idx)
{
    const uint32_t cells[1] = {idx};
    return beam(cells, 1U, false, 0U);
}

/**
 * Extract the mask, derive the state of all cells and check that both agree on the 3 520 cells and
 * that only the three allowed states appear (no free state). Counts go to *cnt.
 */
static void check_all(state_count_t *cnt, const char *msg)
{
    cnt->occupied = 0U;
    cnt->low = 0U;
    cnt->unsurveyed = 0U;
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_extract_occupied(&s_grid, s_mask), msg);
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        ogrid_cell_state_t st = OGRID_CELL_STATE_COUNT;
        TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_get_cell_state(&s_grid, i, &st), msg);
        TEST_ASSERT_TRUE_MESSAGE((st == OGRID_CELL_UNSURVEYED) || (st == OGRID_CELL_LOW_PROB) ||
                                     (st == OGRID_CELL_OCCUPIED),
                                 msg);
        TEST_ASSERT_TRUE_MESSAGE(s_mask[i] == (st == OGRID_CELL_OCCUPIED), msg);
        if (st == OGRID_CELL_OCCUPIED)
        {
            cnt->occupied++;
        }
        else if (st == OGRID_CELL_LOW_PROB)
        {
            cnt->low++;
        }
        else
        {
            cnt->unsurveyed++;
        }
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(OGRID_CELL_COUNT, cnt->occupied + cnt->low + cnt->unsurveyed,
                                     msg);
}

/** Check state, observed flag and log-odds of one cell. */
static void expect_cell(uint32_t idx, ogrid_cell_state_t expected, float lo, const char *msg)
{
    ogrid_cell_state_t st = OGRID_CELL_STATE_COUNT;
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_get_cell_state(&s_grid, idx, &st), msg);
    TEST_ASSERT_EQUAL_INT_MESSAGE(expected, st, msg);
    TEST_ASSERT_EQUAL_UINT8_MESSAGE((expected == OGRID_CELL_UNSURVEYED) ? OGRID_OBSERVED_NO
                                                                        : OGRID_OBSERVED_YES,
                                    s_grid.cells[idx].observed, msg);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(LO_TOL, lo, s_grid.cells[idx].log_odds_lo, msg);
    TEST_ASSERT_TRUE_MESSAGE(s_mask[idx] == (expected == OGRID_CELL_OCCUPIED), msg);
}

/** @req SYS-IMP-01, SYS-SAF-07, SYS-PR-04 @contract post Fresh grid: all unsurveyed, empty mask. */
void test_TC_INT_01_reset_grid_is_all_unsurveyed(void)
{
    state_count_t c;
    check_all(&c, MSG("01", "SYS-SAF-07", "mask and state agree on all 3520 cells"));
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(OGRID_CELL_COUNT, c.unsurveyed,
                                     MSG("01", "SYS-IMP-01", "all 3520 cells unsurveyed"));
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, c.occupied,
                                     MSG("01", "SYS-PR-04", "no occupied cell after reset"));
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, c.low, MSG("01", "SYS-SAF-07", "no low-probability cell after reset"));
}

/** @req SYS-PR-08, SYS-PR-04 @contract post One isolated echo does not confirm. */
void test_TC_INT_02_isolated_echo_does_not_confirm(void)
{
    state_count_t c;
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, echo1(100U), MSG("02", "SYS-PR-08", "echo applied"));
    check_all(&c, MSG("02", "SYS-PR-04", "mask and state agree on all cells"));
    expect_cell(100U, OGRID_CELL_LOW_PROB, 0.85F,
                MSG("02", "SYS-PR-08", "one echo gives l=0.85, low"));
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, c.occupied, MSG("02", "SYS-PR-04", "mask empty"));
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, c.low,
                                     MSG("02", "SYS-PR-08", "exactly one observed cell"));
}

/** @req SYS-PR-08, SYS-PR-04 @contract post Two echoes (l=1.70) still do not confirm. */
void test_TC_INT_03_two_echoes_do_not_confirm(void)
{
    state_count_t c;
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, echo1(100U), MSG("03", "SYS-PR-08", "first echo"));
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, echo1(100U), MSG("03", "SYS-PR-08", "second echo"));
    check_all(&c, MSG("03", "SYS-PR-04", "mask and state agree on all cells"));
    expect_cell(100U, OGRID_CELL_LOW_PROB, 1.70F,
                MSG("03", "SYS-PR-08", "two echoes give l=1.70, low"));
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, c.occupied, MSG("03", "SYS-PR-04", "mask empty"));
}

/** @req SYS-PR-08, SYS-PR-04 @contract post Three consecutive echoes from the initial state
 * confirm. */
void test_TC_INT_04_three_echoes_confirm_occupied(void)
{
    state_count_t c;
    for (uint32_t k = 0U; k < 3U; k++)
    {
        TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, echo1(100U),
                                      MSG("04", "SYS-PR-08", "echo applied"));
    }
    check_all(&c, MSG("04", "SYS-PR-04", "mask and state agree on all cells"));
    expect_cell(100U, OGRID_CELL_OCCUPIED, 2.55F,
                MSG("04", "SYS-PR-08", "three echoes give l=2.55, occupied"));
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, c.occupied,
                                     MSG("04", "SYS-PR-04", "exactly one cell in the mask"));
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(OGRID_CELL_COUNT - 1U, c.unsurveyed,
                                     MSG("04", "SYS-SAF-07", "every other cell stays unsurveyed"));
}

/** @req SYS-PR-08, SYS-SAF-07 @contract post Passes lower the value down to the minimum; never
 * free. */
void test_TC_INT_05_passes_lower_value_to_minimum(void)
{
    state_count_t c;
    const float expected[4] = {-0.40F, -0.80F, -0.80F, -0.80F};
    for (uint32_t k = 0U; k < 4U; k++)
    {
        TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, pass1(7U), MSG("05", "SYS-PR-08", "pass applied"));
        check_all(&c, MSG("05", "SYS-SAF-07", "mask and state agree on all cells"));
        expect_cell(
            7U, OGRID_CELL_LOW_PROB, expected[k],
            MSG("05", "SYS-SAF-07", "pass series -0.40, -0.80, clamp -0.80; state low, not free"));
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, c.occupied,
                                         MSG("05", "SYS-PR-04", "mask stays empty"));
    }
}

/** @req SYS-PR-08, SYS-PR-04 @contract post Three echoes after a pass streak at the minimum
 * confirm. */
void test_TC_INT_06_three_echoes_after_pass_streak_confirm(void)
{
    state_count_t c;
    const float expected[3] = {0.05F, 0.90F, 1.75F};
    for (uint32_t k = 0U; k < 5U; k++)
    {
        TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, pass1(7U), MSG("06", "SYS-PR-08", "pass applied"));
    }
    check_all(&c, MSG("06", "SYS-PR-04", "mask and state agree after the pass streak"));
    expect_cell(7U, OGRID_CELL_LOW_PROB, -0.80F,
                MSG("06", "SYS-PR-08", "streak ends at minimum -0.80"));
    for (uint32_t k = 0U; k < 3U; k++)
    {
        TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, echo1(7U), MSG("06", "SYS-PR-08", "echo applied"));
        check_all(&c, MSG("06", "SYS-PR-04", "mask and state agree on all cells"));
        expect_cell(
            7U, (k == 2U) ? OGRID_CELL_OCCUPIED : OGRID_CELL_LOW_PROB, expected[k],
            MSG("06", "SYS-PR-08", "echo series 0.05, 0.90, 1.75; occupied only at the third"));
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, c.occupied,
                                     MSG("06", "SYS-PR-04", "confirmed cell is in the mask"));
}

/** @req SYS-PR-08, SYS-SAF-07 @contract post A never-observed cell behind an echo does not change.
 */
void test_TC_INT_07_hidden_cell_behind_echo_unchanged(void)
{
    state_count_t c;
    const uint32_t cells[3] = {20U, 21U, 22U};
    for (uint32_t k = 0U; k < 3U; k++)
    {
        TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, beam(cells, 3U, true, 0U),
                                      MSG("07", "SYS-PR-08", "beam applied"));
    }
    check_all(&c, MSG("07", "SYS-PR-04", "mask and state agree on all cells"));
    expect_cell(20U, OGRID_CELL_OCCUPIED, 2.55F, MSG("07", "SYS-PR-08", "echo cell occupied"));
    expect_cell(21U, OGRID_CELL_UNSURVEYED, 0.0F,
                MSG("07", "SYS-SAF-07", "hidden cell stays unsurveyed"));
    expect_cell(22U, OGRID_CELL_UNSURVEYED, 0.0F,
                MSG("07", "SYS-SAF-07", "hidden cell stays unsurveyed"));
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, c.occupied,
                                     MSG("07", "SYS-PR-04", "only the echo cell in the mask"));
}

/** @req SYS-PR-08, SYS-SAF-07 @contract post An observed cell behind an echo keeps its value. */
void test_TC_INT_08_hidden_observed_cell_keeps_value(void)
{
    state_count_t c;
    const uint32_t cells[2] = {40U, 41U};
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, pass1(41U), MSG("08", "SYS-PR-08", "pass on cell 41"));
    check_all(&c, MSG("08", "SYS-PR-04", "mask and state agree before the echoes"));
    expect_cell(41U, OGRID_CELL_LOW_PROB, -0.40F, MSG("08", "SYS-PR-08", "cell 41 at -0.40"));
    for (uint32_t k = 0U; k < 3U; k++)
    {
        TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, beam(cells, 2U, true, 0U),
                                      MSG("08", "SYS-PR-08", "beam with echo at cell 40"));
    }
    check_all(&c, MSG("08", "SYS-PR-04", "mask and state agree on all cells"));
    expect_cell(40U, OGRID_CELL_OCCUPIED, 2.55F, MSG("08", "SYS-PR-08", "echo cell occupied"));
    expect_cell(41U, OGRID_CELL_LOW_PROB, -0.40F,
                MSG("08", "SYS-SAF-07", "hidden cell still -0.40 (not lowered, not raised)"));
}

/** @req SYS-PR-08, SYS-PR-04 @contract post Pass before the echo, echo, hidden after. */
void test_TC_INT_09_beam_with_pass_echo_and_hidden_cells(void)
{
    state_count_t c;
    const uint32_t cells[4] = {30U, 31U, 32U, 33U};
    for (uint32_t k = 0U; k < 3U; k++)
    {
        TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, beam(cells, 4U, true, 2U),
                                      MSG("09", "SYS-PR-08", "beam applied"));
        check_all(&c, MSG("09", "SYS-PR-04", "mask and state agree on all cells"));
    }
    expect_cell(30U, OGRID_CELL_LOW_PROB, -0.80F,
                MSG("09", "SYS-PR-08", "pass cell at clamp -0.80 after 3 beams"));
    expect_cell(31U, OGRID_CELL_LOW_PROB, -0.80F,
                MSG("09", "SYS-PR-08", "pass cell at clamp -0.80 after 3 beams"));
    expect_cell(32U, OGRID_CELL_OCCUPIED, 2.55F, MSG("09", "SYS-PR-08", "echo cell 2.55 occupied"));
    expect_cell(33U, OGRID_CELL_UNSURVEYED, 0.0F,
                MSG("09", "SYS-SAF-07", "cell behind echo unsurveyed"));
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, c.occupied,
                                     MSG("09", "SYS-PR-04", "only cell 32 in the mask"));
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2U, c.low,
                                     MSG("09", "SYS-SAF-07", "two low-probability cells"));
}

/** @req SYS-PR-08 @contract post A beam of zero cells without echo changes nothing. */
void test_TC_INT_10_empty_beam_without_echo_does_nothing(void)
{
    state_count_t c;
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, beam(NULL, 0U, false, 0U),
                                  MSG("10", "SYS-PR-08", "empty beam accepted"));
    check_all(&c, MSG("10", "SYS-PR-04", "mask and state agree on all cells"));
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(OGRID_CELL_COUNT, c.unsurveyed,
                                     MSG("10", "SYS-SAF-07", "all cells still unsurveyed"));
}

/** @req SYS-PR-08, SYS-PR-04 @contract post Passes erode a confirmed cell: 2.55, 2.15, 1.75, 1.35.
 */
void test_TC_INT_11_passes_erode_occupied_cell(void)
{
    state_count_t c;
    for (uint32_t k = 0U; k < 3U; k++)
    {
        TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, echo1(500U),
                                      MSG("11", "SYS-PR-08", "echo applied"));
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, pass1(500U), MSG("11", "SYS-PR-08", "first pass"));
    check_all(&c, MSG("11", "SYS-PR-04", "mask and state agree"));
    expect_cell(500U, OGRID_CELL_OCCUPIED, 2.15F, MSG("11", "SYS-PR-08", "2.15 still occupied"));
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, pass1(500U), MSG("11", "SYS-PR-08", "second pass"));
    check_all(&c, MSG("11", "SYS-PR-04", "mask and state agree"));
    expect_cell(500U, OGRID_CELL_OCCUPIED, 1.75F,
                MSG("11", "SYS-PR-08", "1.75 still occupied (above 1.7346)"));
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, pass1(500U), MSG("11", "SYS-PR-08", "third pass"));
    check_all(&c, MSG("11", "SYS-PR-04", "mask and state agree"));
    expect_cell(500U, OGRID_CELL_LOW_PROB, 1.35F, MSG("11", "SYS-PR-08", "1.35 low probability"));
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, c.occupied, MSG("11", "SYS-PR-04", "mask empty again"));
}

/** @req SYS-IMP-01, SYS-SAF-07, SYS-PR-04 @contract post Reset after evidence returns every cell to
 * unsurveyed. */
void test_TC_INT_12_reset_after_evidence_restores_unsurveyed(void)
{
    state_count_t c;
    const uint32_t cells[4] = {0U, 1U, 2U, 3U};
    for (uint32_t k = 0U; k < 3U; k++)
    {
        TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, beam(cells, 4U, true, 3U),
                                      MSG("12", "SYS-PR-08", "beam applied"));
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, echo1(OGRID_CELL_COUNT - 1U),
                                  MSG("12", "SYS-PR-08", "echo in last cell"));
    check_all(&c, MSG("12", "SYS-PR-04", "mask and state agree before reset"));
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, c.occupied,
                                     MSG("12", "SYS-PR-04", "cell 3 occupied before reset"));
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(4U, c.low,
                                     MSG("12", "SYS-PR-08", "four low cells before reset"));

    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_reset_grid(&s_grid),
                                  MSG("12", "SYS-IMP-01", "reset ok"));
    check_all(&c, MSG("12", "SYS-SAF-07", "mask and state agree after reset"));
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        OGRID_CELL_COUNT, c.unsurveyed,
        MSG("12", "SYS-SAF-07", "all 3520 cells unsurveyed after reset"));
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, c.occupied,
                                     MSG("12", "SYS-PR-04", "mask empty after reset"));
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(OGRID_OBSERVED_NO, s_grid.cells[i].observed,
                                        MSG("12", "SYS-IMP-01", "observed cleared on every cell"));
        TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
            LO_TOL, 0.0F, s_grid.cells[i].log_odds_lo,
            MSG("12", "SYS-IMP-01", "log-odds back to 0 on every cell"));
    }
}

/** @req SYS-SAF-07, SYS-PR-04 @contract post Mixed grid: only three states appear, counts add to
 * 3520. */
void test_TC_INT_13_mixed_grid_has_no_free_state(void)
{
    state_count_t c;
    for (uint32_t k = 0U; k < 3U; k++)
    {
        TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, echo1(0U),
                                      MSG("13", "SYS-PR-08", "echo on cell 0"));
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, echo1(1U), MSG("13", "SYS-PR-08", "echo on cell 1"));
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, pass1(2U), MSG("13", "SYS-PR-08", "pass on cell 2"));
    check_all(&c, MSG("13", "SYS-SAF-07", "only unsurveyed, low or occupied; mask agrees"));
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, c.occupied, MSG("13", "SYS-PR-04", "one occupied cell"));
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        2U, c.low, MSG("13", "SYS-SAF-07", "cells 1 and 2 low probability, none free"));
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(OGRID_CELL_COUNT - 3U, c.unsurveyed,
                                     MSG("13", "SYS-SAF-07", "3517 cells unsurveyed"));
}

/**
 * @req SYS-PR-08, SYS-PR-04, SYS-SAF-07, SYS-IMP-01 @contract post
 * Full scenario: 4 cycles of 3 beams each. Track columns: A (d=0), B (d=1), C (d=2).
 * A: cells s=0..9, echo at s=5 every cycle (s=6..9 hidden).
 * B: cells s=0..9, no echo every cycle.
 * C: cells s=0..4, echo at s=4 in cycles 1, 3 and 4; pass (no echo) in cycle 2.
 * Hand-computed: A s0-4 -0.40, -0.80, -0.80, -0.80; A s5 0.85, 1.70, 2.55, 3.40 (clamp);
 * B -0.40, -0.80, -0.80, -0.80; C s0-3 same as B; C s4 0.85, 0.45, 1.30, 2.15.
 * Observed cells every cycle: 6 (A) + 10 (B) + 5 (C) = 21. Occupied after cycles: 0, 0, 1, 2.
 */
void test_TC_INT_14_full_cycles_with_several_beams(void)
{
    state_count_t c;
    uint32_t a[10];
    uint32_t b[10];
    uint32_t cc[5];
    const uint32_t expected_occupied[4] = {0U, 0U, 1U, 2U};
    const float a_low[4] = {-0.40F, -0.80F, -0.80F, -0.80F};
    const float a_echo[4] = {0.85F, 1.70F, 2.55F, 3.40F};
    const float c_echo[4] = {0.85F, 0.45F, 1.30F, 2.15F};

    for (uint32_t s = 0U; s < 10U; s++)
    {
        a[s] = CELL_AT(s, 0U);
        b[s] = CELL_AT(s, 1U);
    }
    for (uint32_t s = 0U; s < 5U; s++)
    {
        cc[s] = CELL_AT(s, 2U);
    }

    for (uint32_t cycle = 0U; cycle < 4U; cycle++)
    {
        TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, beam(a, 10U, true, 5U),
                                      MSG("14", "SYS-PR-08", "beam A applied"));
        TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, beam(b, 10U, false, 0U),
                                      MSG("14", "SYS-PR-08", "beam B applied"));
        TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, beam(cc, 5U, cycle != 1U, 4U),
                                      MSG("14", "SYS-PR-08", "beam C applied"));

        check_all(&c, MSG("14", "SYS-PR-04", "mask and state agree on all 3520 cells every cycle"));
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(
            expected_occupied[cycle], c.occupied,
            MSG("14", "SYS-PR-04", "occupied cells per cycle 0, 0, 1, 2"));
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(
            21U - expected_occupied[cycle], c.low,
            MSG("14", "SYS-SAF-07", "observed non-occupied cells are low probability"));
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(
            OGRID_CELL_COUNT - 21U, c.unsurveyed,
            MSG("14", "SYS-SAF-07", "3499 cells never observed stay unsurveyed"));

        for (uint32_t s = 0U; s < 5U; s++)
        {
            expect_cell(CELL_AT(s, 0U), OGRID_CELL_LOW_PROB, a_low[cycle],
                        MSG("14", "SYS-PR-08", "A cells before the echo follow the pass series"));
            expect_cell(CELL_AT(s, 1U), OGRID_CELL_LOW_PROB, a_low[cycle],
                        MSG("14", "SYS-PR-08", "B cells follow the pass series"));
        }
        for (uint32_t s = 5U; s < 10U; s++)
        {
            expect_cell(CELL_AT(s, 1U), OGRID_CELL_LOW_PROB, a_low[cycle],
                        MSG("14", "SYS-PR-08", "B cells follow the pass series"));
        }
        for (uint32_t s = 0U; s < 4U; s++)
        {
            expect_cell(CELL_AT(s, 2U), OGRID_CELL_LOW_PROB, a_low[cycle],
                        MSG("14", "SYS-PR-08", "C cells before the echo follow the pass series"));
        }
        expect_cell(CELL_AT(5U, 0U), (cycle >= 2U) ? OGRID_CELL_OCCUPIED : OGRID_CELL_LOW_PROB,
                    a_echo[cycle],
                    MSG("14", "SYS-PR-08", "A echo cell series 0.85, 1.70, 2.55, 3.40"));
        expect_cell(CELL_AT(4U, 2U), (cycle == 3U) ? OGRID_CELL_OCCUPIED : OGRID_CELL_LOW_PROB,
                    c_echo[cycle],
                    MSG("14", "SYS-PR-08", "C echo cell series 0.85, 0.45, 1.30, 2.15"));
        for (uint32_t s = 6U; s < 10U; s++)
        {
            expect_cell(CELL_AT(s, 0U), OGRID_CELL_UNSURVEYED, 0.0F,
                        MSG("14", "SYS-SAF-07", "A cells behind the echo never change"));
        }
    }

    /* A reset after the whole sequence discards everything. */
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_reset_grid(&s_grid),
                                  MSG("14", "SYS-IMP-01", "reset ok"));
    check_all(&c, MSG("14", "SYS-SAF-07", "mask and state agree after reset"));
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        OGRID_CELL_COUNT, c.unsurveyed,
        MSG("14", "SYS-SAF-07", "all cells unsurveyed after the cycles and a reset"));
}

/** @req SYS-PR-08, SYS-IMP-01 @contract post Evidence after a reset starts from scratch. */
void test_TC_INT_15_evidence_after_reset_starts_from_scratch(void)
{
    state_count_t c;
    for (uint32_t k = 0U; k < 3U; k++)
    {
        TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, echo1(900U),
                                      MSG("15", "SYS-PR-08", "echo applied"));
    }
    check_all(&c, MSG("15", "SYS-PR-04", "mask and state agree before reset"));
    expect_cell(900U, OGRID_CELL_OCCUPIED, 2.55F, MSG("15", "SYS-PR-08", "occupied before reset"));
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_reset_grid(&s_grid),
                                  MSG("15", "SYS-IMP-01", "reset ok"));
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, echo1(900U),
                                  MSG("15", "SYS-PR-08", "echo after reset"));
    check_all(&c, MSG("15", "SYS-PR-04", "mask and state agree on all cells"));
    expect_cell(900U, OGRID_CELL_LOW_PROB, 0.85F,
                MSG("15", "SYS-PR-08", "one echo after reset gives 0.85, low"));
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, c.occupied, MSG("15", "SYS-PR-04", "isolated echo after reset does not confirm"));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_TC_INT_01_reset_grid_is_all_unsurveyed);
    RUN_TEST(test_TC_INT_02_isolated_echo_does_not_confirm);
    RUN_TEST(test_TC_INT_03_two_echoes_do_not_confirm);
    RUN_TEST(test_TC_INT_04_three_echoes_confirm_occupied);
    RUN_TEST(test_TC_INT_05_passes_lower_value_to_minimum);
    RUN_TEST(test_TC_INT_06_three_echoes_after_pass_streak_confirm);
    RUN_TEST(test_TC_INT_07_hidden_cell_behind_echo_unchanged);
    RUN_TEST(test_TC_INT_08_hidden_observed_cell_keeps_value);
    RUN_TEST(test_TC_INT_09_beam_with_pass_echo_and_hidden_cells);
    RUN_TEST(test_TC_INT_10_empty_beam_without_echo_does_nothing);
    RUN_TEST(test_TC_INT_11_passes_erode_occupied_cell);
    RUN_TEST(test_TC_INT_12_reset_after_evidence_restores_unsurveyed);
    RUN_TEST(test_TC_INT_13_mixed_grid_has_no_free_state);
    RUN_TEST(test_TC_INT_14_full_cycles_with_several_beams);
    RUN_TEST(test_TC_INT_15_evidence_after_reset_starts_from_scratch);
    return UNITY_END();
}
