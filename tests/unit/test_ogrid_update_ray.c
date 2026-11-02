/**
 * @file test_ogrid_update_ray.c
 * @brief Unit tests of ogrid_update_ray (F-006).
 * @details Expected values are computed by hand from the vote constants: pass = -0.40,
 *          echo = +0.85, clamp [-0.8, 3.4]. Float32 sums of a few terms of magnitude <= 3.4 carry
 * an error below 1e-6, so numeric checks use a tolerance of 1e-5, far below the smallest
 *          distinguishable difference between expected values (0.05). Cells outside the beam are
 *          compared exactly against a reference copy of the grid.
 */
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ogrid/ogrid.h"
#include "ogrid/ogrid_limits.h"
#include "ogrid/ogrid_reset_grid.h"
#include "ogrid/ogrid_update_ray.h"
#include "unity.h"

/** Tolerance for log-odds sums (see file header). */
#define TOL (1e-5F)

static ogrid_t s_grid;
static ogrid_t s_ref;
static uint32_t s_all[OGRID_CELL_COUNT + 1U];

/** True when value is one of list[0..n-1]. */
static bool in_list(const uint32_t *list, uint32_t n, uint32_t value)
{
    bool found = false;
    for (uint32_t i = 0U; i < n; i++)
    {
        if (list[i] == value)
        {
            found = true;
        }
    }
    return found;
}

/** True when cell idx is identical in s_grid and s_ref. */
static bool cell_same(uint32_t idx)
{
    const ogrid_real_t a = s_grid.cells[idx].log_odds_lo;
    const ogrid_real_t b = s_ref.cells[idx].log_odds_lo;
    const bool same_value = (isnan(a) && isnan(b)) || (a == b);
    return same_value && (s_grid.cells[idx].observed == s_ref.cells[idx].observed);
}

/** Number of cells not in list that differ from the reference. */
static uint32_t changes_outside(const uint32_t *list, uint32_t n)
{
    uint32_t count = 0U;
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        if (!in_list(list, n, i) && !cell_same(i))
        {
            count++;
        }
    }
    return count;
}

/** Number of cells that differ from the reference. */
static uint32_t grid_changes(void)
{
    return changes_outside(NULL, 0U);
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
    (void)ogrid_reset_grid(&s_grid);
    s_ref = s_grid;
    for (uint32_t i = 0U; i <= OGRID_CELL_COUNT; i++)
    {
        s_all[i] = i;
    }
}

void tearDown(void)
{
}

/** @req SYS-PR-08 @contract post POST-5 ejemplo [10,11,12,13] eco en 2 sobre grid reiniciado */
void test_TC_F006_01_example_from_contract(void)
{
    const uint32_t cells[] = {10U, 11U, 12U, 13U};
    const ogrid_status_t st = ogrid_update_ray(&s_grid, cells, 4U, true, 2U);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st, "[TC-F006-01][SYS-PR-08] example must return OK");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, -0.40F, s_grid.cells[10].log_odds_lo,
                                     "[TC-F006-01][SYS-PR-08] cell 10 must be -0.40");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, -0.40F, s_grid.cells[11].log_odds_lo,
                                     "[TC-F006-01][SYS-PR-08] cell 11 must be -0.40");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, 0.85F, s_grid.cells[12].log_odds_lo,
                                     "[TC-F006-01][SYS-PR-08] cell 12 must be +0.85");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(OGRID_OBSERVED_YES, s_grid.cells[12].observed,
                                    "[TC-F006-01][SYS-PR-08] echo cell must be observed");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, 0.0F, s_grid.cells[13].log_odds_lo,
                                     "[TC-F006-01][SYS-PR-08] cell 13 must stay at 0");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(OGRID_OBSERVED_NO, s_grid.cells[13].observed,
                                    "[TC-F006-01][SYS-PR-08] cell 13 must not be observed");
}

/** @req SYS-PR-08 @contract post POST-1 sin eco todas las celdas reciben paso sin eco y observed */
void test_TC_F006_02_no_echo_all_pass(void)
{
    const uint32_t cells[] = {5U, 6U, 7U, 8U};
    const ogrid_status_t st = ogrid_update_ray(&s_grid, cells, 4U, false, 0U);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st,
                                  "[TC-F006-02][SYS-PR-08] no-echo ray must return OK");
    for (uint32_t i = 0U; i < 4U; i++)
    {
        TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, -0.40F, s_grid.cells[cells[i]].log_odds_lo,
                                         "[TC-F006-02][SYS-PR-08] every cell must get -0.40");
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(OGRID_OBSERVED_YES, s_grid.cells[cells[i]].observed,
                                        "[TC-F006-02][SYS-PR-08] every cell must be observed");
    }
}

/** @req SYS-PR-08 @contract post POST-1 sin eco echo_pos se ignora aunque sea enorme */
void test_TC_F006_03_no_echo_ignores_echo_pos(void)
{
    const uint32_t cells[] = {5U, 6U, 7U};
    const ogrid_status_t st = ogrid_update_ray(&s_grid, cells, 3U, false, UINT32_MAX);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st,
                                  "[TC-F006-03][SYS-PR-08] echo_pos must be ignored without echo");
    for (uint32_t i = 0U; i < 3U; i++)
    {
        TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, -0.40F, s_grid.cells[cells[i]].log_odds_lo,
                                         "[TC-F006-03][SYS-PR-08] every cell must get -0.40");
    }
}

/** @req SYS-PR-08 @contract post POST-2 limite echo_pos = 0: solo la primera recibe eco */
void test_TC_F006_04_echo_at_first_position(void)
{
    const uint32_t cells[] = {20U, 21U, 22U};
    const ogrid_status_t st = ogrid_update_ray(&s_grid, cells, 3U, true, 0U);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st, "[TC-F006-04][SYS-PR-08] must return OK");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, 0.85F, s_grid.cells[20].log_odds_lo,
                                     "[TC-F006-04][SYS-PR-08] first cell must get +0.85");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, 0.0F, s_grid.cells[21].log_odds_lo,
                                     "[TC-F006-04][SYS-PR-08] occluded cell 21 must stay 0");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, 0.0F, s_grid.cells[22].log_odds_lo,
                                     "[TC-F006-04][SYS-PR-08] occluded cell 22 must stay 0");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(OGRID_OBSERVED_NO, s_grid.cells[22].observed,
                                    "[TC-F006-04][SYS-PR-08] occluded cell must not be observed");
}

/** @req SYS-PR-08 @contract post POST-2 limite echo_pos = cell_count-1: todas pasan salvo la ultima
 */
void test_TC_F006_05_echo_at_last_position(void)
{
    const uint32_t cells[] = {30U, 31U, 32U};
    const ogrid_status_t st = ogrid_update_ray(&s_grid, cells, 3U, true, 2U);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st, "[TC-F006-05][SYS-PR-08] must return OK");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, -0.40F, s_grid.cells[30].log_odds_lo,
                                     "[TC-F006-05][SYS-PR-08] cell 30 must get -0.40");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, -0.40F, s_grid.cells[31].log_odds_lo,
                                     "[TC-F006-05][SYS-PR-08] cell 31 must get -0.40");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, 0.85F, s_grid.cells[32].log_odds_lo,
                                     "[TC-F006-05][SYS-PR-08] last cell must get +0.85");
}

/** @req SYS-PR-08 @contract post POST-2 un haz de una sola celda con eco recibe +0,85 */
void test_TC_F006_06_single_cell_echo(void)
{
    const uint32_t cells[] = {77U};
    const ogrid_status_t st = ogrid_update_ray(&s_grid, cells, 1U, true, 0U);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st, "[TC-F006-06][SYS-PR-08] must return OK");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, 0.85F, s_grid.cells[77].log_odds_lo,
                                     "[TC-F006-06][SYS-PR-08] single cell must get +0.85");
}

/** @req SYS-PR-08 @contract post POST-2 celdas ocultas conservan valor y observed previos */
void test_TC_F006_07_occluded_cells_keep_previous_value(void)
{
    const uint32_t cells[] = {40U, 41U, 42U, 43U};
    set_cell(42U, 1.25F, OGRID_OBSERVED_YES);
    s_grid.cells[43].log_odds_lo = -0.6F;
    s_grid.cells[43].observed = OGRID_OBSERVED_YES;
    s_ref = s_grid;
    const ogrid_status_t st = ogrid_update_ray(&s_grid, cells, 4U, true, 1U);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st, "[TC-F006-07][SYS-PR-08] must return OK");
    TEST_ASSERT_TRUE_MESSAGE(cell_same(42U),
                             "[TC-F006-07][SYS-PR-08] occluded cell 42 must be bit-identical");
    TEST_ASSERT_TRUE_MESSAGE(cell_same(43U),
                             "[TC-F006-07][SYS-PR-08] occluded cell 43 must be bit-identical");
}

/** @req SYS-PR-08 @contract post POST-3 celdas fuera de cells_in no cambian */
void test_TC_F006_08_cells_outside_beam_unchanged(void)
{
    const uint32_t cells[] = {100U, 200U, 300U};
    set_cell(99U, 0.7F, OGRID_OBSERVED_YES);
    set_cell(101U, -0.2F, OGRID_OBSERVED_YES);
    set_cell(0U, 3.0F, OGRID_OBSERVED_YES);
    set_cell(OGRID_CELL_COUNT - 1U, -0.8F, OGRID_OBSERVED_YES);
    const ogrid_status_t st = ogrid_update_ray(&s_grid, cells, 3U, true, 1U);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st, "[TC-F006-08][SYS-PR-08] must return OK");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, changes_outside(cells, 3U),
        "[TC-F006-08][SYS-PR-08] cells not in cells_in must not change");
}

/** @req SYS-PR-08 @contract post POST-3 sin eco tambien deja intactas las celdas fuera del haz */
void test_TC_F006_09_no_echo_cells_outside_unchanged(void)
{
    const uint32_t cells[] = {500U, 501U};
    set_cell(499U, 1.0F, OGRID_OBSERVED_YES);
    set_cell(502U, 2.0F, OGRID_OBSERVED_YES);
    const ogrid_status_t st = ogrid_update_ray(&s_grid, cells, 2U, false, 0U);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st, "[TC-F006-09][SYS-PR-08] must return OK");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, changes_outside(cells, 2U),
        "[TC-F006-09][SYS-PR-08] cells not in cells_in must not change");
}

/** @req SYS-PR-08 @contract post POST-1 acumulacion: cell previa 0,5 + paso = 0,1 */
void test_TC_F006_10_pass_adds_to_previous_value(void)
{
    const uint32_t cells[] = {60U};
    set_cell(60U, 0.5F, OGRID_OBSERVED_YES);
    const ogrid_status_t st = ogrid_update_ray(&s_grid, cells, 1U, false, 0U);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st, "[TC-F006-10][SYS-PR-08] must return OK");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, 0.1F, s_grid.cells[60].log_odds_lo,
                                     "[TC-F006-10][SYS-PR-08] 0.5 + (-0.40) must be 0.1");
}

/** @req SYS-PR-08 @contract post POST-2 acumulacion: celda previa 0,5 + eco = 1,35 */
void test_TC_F006_11_echo_adds_to_previous_value(void)
{
    const uint32_t cells[] = {61U};
    set_cell(61U, 0.5F, OGRID_OBSERVED_YES);
    const ogrid_status_t st = ogrid_update_ray(&s_grid, cells, 1U, true, 0U);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st, "[TC-F006-11][SYS-PR-08] must return OK");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, 1.35F, s_grid.cells[61].log_odds_lo,
                                     "[TC-F006-11][SYS-PR-08] 0.5 + 0.85 must be 1.35");
}

/** @req SYS-PR-08 @contract post POST-4 limite superior: 3,3 + eco se satura en 3,4 */
void test_TC_F006_12_echo_clamps_at_upper_limit(void)
{
    const uint32_t cells[] = {62U};
    set_cell(62U, 3.3F, OGRID_OBSERVED_YES);
    const ogrid_status_t st = ogrid_update_ray(&s_grid, cells, 1U, true, 0U);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st, "[TC-F006-12][SYS-PR-08] must return OK");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, OGRID_LOG_ODDS_MAX, s_grid.cells[62].log_odds_lo,
                                     "[TC-F006-12][SYS-PR-08] 3.3 + 0.85 must clamp at 3.4");
}

/** @req SYS-PR-08 @contract post POST-4 limite inferior: -0,6 + paso se satura en -0,8 */
void test_TC_F006_13_pass_clamps_at_lower_limit(void)
{
    const uint32_t cells[] = {63U, 64U};
    set_cell(63U, -0.6F, OGRID_OBSERVED_YES);
    set_cell(64U, OGRID_LOG_ODDS_MIN, OGRID_OBSERVED_YES);
    const ogrid_status_t st = ogrid_update_ray(&s_grid, cells, 2U, false, 0U);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st, "[TC-F006-13][SYS-PR-08] must return OK");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, OGRID_LOG_ODDS_MIN, s_grid.cells[63].log_odds_lo,
                                     "[TC-F006-13][SYS-PR-08] -0.6 - 0.40 must clamp at -0.8");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, OGRID_LOG_ODDS_MIN, s_grid.cells[64].log_odds_lo,
                                     "[TC-F006-13][SYS-PR-08] -0.8 - 0.40 must stay at -0.8");
}

/** @req SYS-PR-08 @contract post POST-4 N haces sin eco: valor = max(-0,8; -0,4 N), N = 1..4 */
void test_TC_F006_14_repeated_pass_rays_analytic(void)
{
    const uint32_t cells[] = {70U};
    const ogrid_real_t expected[] = {-0.4F, -0.8F, -0.8F, -0.8F};
    for (uint32_t k = 0U; k < 4U; k++)
    {
        const ogrid_status_t st = ogrid_update_ray(&s_grid, cells, 1U, false, 0U);
        TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st, "[TC-F006-14][SYS-PR-08] must return OK");
        TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, expected[k], s_grid.cells[70].log_odds_lo,
                                         "[TC-F006-14][SYS-PR-08] value must be max(-0.8, -0.4N)");
    }
}

/** @req SYS-PR-08 @contract post POST-4 N haces con eco: valor = min(3,4; 0,85 N), N = 1..5 */
void test_TC_F006_15_repeated_echo_rays_analytic(void)
{
    const uint32_t cells[] = {71U};
    const ogrid_real_t expected[] = {0.85F, 1.70F, 2.55F, 3.4F, 3.4F};
    for (uint32_t k = 0U; k < 5U; k++)
    {
        const ogrid_status_t st = ogrid_update_ray(&s_grid, cells, 1U, true, 0U);
        TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st, "[TC-F006-15][SYS-PR-08] must return OK");
        TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, expected[k], s_grid.cells[71].log_odds_lo,
                                         "[TC-F006-15][SYS-PR-08] value must be min(3.4, 0.85N)");
    }
}

/** @req SYS-PR-08 @contract post POST-4 mixto: eco y luego paso dan 0,85 - 0,40 = 0,45 */
void test_TC_F006_16_echo_then_pass_sequence(void)
{
    const uint32_t cells[] = {72U};
    (void)ogrid_update_ray(&s_grid, cells, 1U, true, 0U);
    const ogrid_status_t st = ogrid_update_ray(&s_grid, cells, 1U, false, 0U);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st, "[TC-F006-16][SYS-PR-08] must return OK");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, 0.45F, s_grid.cells[72].log_odds_lo,
                                     "[TC-F006-16][SYS-PR-08] 0.85 - 0.40 must be 0.45");
}

/** @req SYS-PR-08 @contract post POST-4 propiedad: tras cualquier haz valido todo log-odds esta en
 * [-0,8; 3,4] */
void test_TC_F006_17_result_always_within_limits(void)
{
    const uint32_t cells[] = {80U, 81U, 82U, 83U, 84U};
    for (uint32_t k = 0U; k < 10U; k++)
    {
        (void)ogrid_update_ray(&s_grid, cells, 5U, true, k % 5U);
        (void)ogrid_update_ray(&s_grid, cells, 5U, false, 0U);
        (void)ogrid_update_ray(&s_grid, cells, 5U, true, 4U - (k % 5U));
    }
    for (uint32_t i = 0U; i < 5U; i++)
    {
        const ogrid_real_t v = s_grid.cells[cells[i]].log_odds_lo;
        TEST_ASSERT_TRUE_MESSAGE((v >= OGRID_LOG_ODDS_MIN) && (v <= OGRID_LOG_ODDS_MAX),
                                 "[TC-F006-17][SYS-PR-08] log-odds must stay within [-0.8, 3.4]");
    }
}

/** @req SYS-PR-08 @contract post POST-6 cell_count = 0 con cells_in no nulo: OK y grid igual */
void test_TC_F006_18_zero_count_does_nothing(void)
{
    const uint32_t cells[] = {1U};
    set_cell(1U, 0.7F, OGRID_OBSERVED_YES);
    const ogrid_status_t st = ogrid_update_ray(&s_grid, cells, 0U, false, 0U);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st, "[TC-F006-18][SYS-PR-08] count 0 must return OK");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, grid_changes(),
                                     "[TC-F006-18][SYS-PR-08] count 0 must not change the grid");
}

/** @req SYS-PR-08 @contract post POST-6 cell_count = 0 con cells_in NULL es valido (PRE-2 solo
 * exige no nulo si count > 0) */
void test_TC_F006_19_zero_count_null_cells(void)
{
    const ogrid_status_t st = ogrid_update_ray(&s_grid, NULL, 0U, false, 0U);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st,
                                  "[TC-F006-19][SYS-PR-08] count 0 with NULL cells must be OK");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, grid_changes(),
                                     "[TC-F006-19][SYS-PR-08] grid must not change");
}

/** @req SYS-PR-08 @contract err ERR_PARAM con grid NULL */
void test_TC_F006_20_null_grid(void)
{
    const uint32_t cells[] = {1U, 2U};
    const ogrid_status_t st = ogrid_update_ray(NULL, cells, 2U, false, 0U);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_PARAM, st,
                                  "[TC-F006-20][SYS-PR-08] NULL grid must return ERR_PARAM");
}

/** @req SYS-PR-08 @contract err ERR_PARAM con grid NULL y count 0 */
void test_TC_F006_21_null_grid_zero_count(void)
{
    const ogrid_status_t st = ogrid_update_ray(NULL, NULL, 0U, false, 0U);
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_PARAM, st, "[TC-F006-21][SYS-PR-08] NULL grid with count 0 must be ERR_PARAM");
}

/** @req SYS-PR-08 @contract err ERR_PARAM con cells_in NULL y count > 0, grid sin cambios (POST-7)
 */
void test_TC_F006_22_null_cells_with_count(void)
{
    set_cell(3U, 1.0F, OGRID_OBSERVED_YES);
    const ogrid_status_t st = ogrid_update_ray(&s_grid, NULL, 4U, true, 1U);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_PARAM, st,
                                  "[TC-F006-22][SYS-PR-08] NULL cells must return ERR_PARAM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, grid_changes(),
                                     "[TC-F006-22][SYS-PR-08] grid must not be modified");
}

/** @req SYS-PR-08 @contract pre PRE-5 echo_pos = cell_count (justo fuera): ERR_RANGE sin cambios */
void test_TC_F006_23_echo_pos_equals_count(void)
{
    const uint32_t cells[] = {10U, 11U, 12U};
    const ogrid_status_t st = ogrid_update_ray(&s_grid, cells, 3U, true, 3U);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_RANGE, st,
                                  "[TC-F006-23][SYS-PR-08] echo_pos=count must be ERR_RANGE");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, grid_changes(),
                                     "[TC-F006-23][SYS-PR-08] grid must not be modified");
}

/** @req SYS-PR-08 @contract pre PRE-5 echo_pos = UINT32_MAX: ERR_RANGE sin cambios */
void test_TC_F006_24_echo_pos_huge(void)
{
    const uint32_t cells[] = {10U, 11U, 12U};
    const ogrid_status_t st = ogrid_update_ray(&s_grid, cells, 3U, true, UINT32_MAX);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_RANGE, st,
                                  "[TC-F006-24][SYS-PR-08] huge echo_pos must be ERR_RANGE");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, grid_changes(),
                                     "[TC-F006-24][SYS-PR-08] grid must not be modified");
}

/** @req SYS-PR-08 @contract pre PRE-3 cell_count = OGRID_CELL_COUNT + 1: ERR_RANGE sin cambios */
void test_TC_F006_25_count_just_above_max(void)
{
    const ogrid_status_t st = ogrid_update_ray(&s_grid, s_all, OGRID_CELL_COUNT + 1U, false, 0U);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_RANGE, st,
                                  "[TC-F006-25][SYS-PR-08] count=MAX+1 must be ERR_RANGE");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, grid_changes(),
                                     "[TC-F006-25][SYS-PR-08] grid must not be modified");
}

/** @req SYS-PR-08 @contract pre PRE-3 cell_count = UINT32_MAX: ERR_RANGE sin cambios */
void test_TC_F006_26_count_huge(void)
{
    const ogrid_status_t st = ogrid_update_ray(&s_grid, s_all, UINT32_MAX, true, 0U);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_RANGE, st,
                                  "[TC-F006-26][SYS-PR-08] huge count must be ERR_RANGE");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, grid_changes(),
                                     "[TC-F006-26][SYS-PR-08] grid must not be modified");
}

/** @req SYS-PR-08 @contract pre PRE-3 limite cell_count = OGRID_CELL_COUNT sin eco: todas -0,40 */
void test_TC_F006_27_count_at_max_no_echo(void)
{
    const ogrid_status_t st = ogrid_update_ray(&s_grid, s_all, OGRID_CELL_COUNT, false, 0U);
    uint32_t wrong = 0U;
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st, "[TC-F006-27][SYS-PR-08] count=MAX must return OK");
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        if ((fabsf(s_grid.cells[i].log_odds_lo - (-0.40F)) > TOL) ||
            (s_grid.cells[i].observed != OGRID_OBSERVED_YES))
        {
            wrong++;
        }
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, wrong, "[TC-F006-27][SYS-PR-08] every cell must be -0.40 and observed");
}

/** @req SYS-PR-08 @contract post POST-2 cell_count = MAX con eco en la ultima posicion */
void test_TC_F006_28_count_at_max_echo_last(void)
{
    const ogrid_status_t st =
        ogrid_update_ray(&s_grid, s_all, OGRID_CELL_COUNT, true, OGRID_CELL_COUNT - 1U);
    uint32_t wrong = 0U;
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st, "[TC-F006-28][SYS-PR-08] must return OK");
    for (uint32_t i = 0U; i < (OGRID_CELL_COUNT - 1U); i++)
    {
        if (fabsf(s_grid.cells[i].log_odds_lo - (-0.40F)) > TOL)
        {
            wrong++;
        }
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, wrong,
                                     "[TC-F006-28][SYS-PR-08] cells before echo must be -0.40");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, 0.85F, s_grid.cells[OGRID_CELL_COUNT - 1U].log_odds_lo,
                                     "[TC-F006-28][SYS-PR-08] last cell must be +0.85");
}

/** @req SYS-PR-08 @contract pre PRE-4 indice = OGRID_CELL_COUNT al final: ERR_RANGE y atomicidad */
void test_TC_F006_29_index_equals_count_at_end(void)
{
    const uint32_t cells[] = {10U, 11U, OGRID_CELL_COUNT};
    const ogrid_status_t st = ogrid_update_ray(&s_grid, cells, 3U, false, 0U);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_RANGE, st,
                                  "[TC-F006-29][SYS-PR-08] index=COUNT must be ERR_RANGE");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, grid_changes(),
        "[TC-F006-29][SYS-SAF-07] earlier valid cells must not be modified (POST-7)");
}

/** @req SYS-PR-08 @contract post POST-7 indice invalido en medio tras el eco (oculto): ERR_RANGE
 * sin cambios */
void test_TC_F006_30_invalid_index_in_occluded_part(void)
{
    const uint32_t cells[] = {10U, 11U, 12U, UINT32_MAX};
    const ogrid_status_t st = ogrid_update_ray(&s_grid, cells, 4U, true, 1U);
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_RANGE, st,
        "[TC-F006-30][SYS-PR-08] invalid index behind the echo must be ERR_RANGE");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, grid_changes(),
                                     "[TC-F006-30][SYS-SAF-07] grid must not be modified (POST-7)");
}

/** @req SYS-PR-08 @contract post POST-7 indice invalido al principio: ERR_RANGE sin cambios */
void test_TC_F006_31_invalid_index_first(void)
{
    const uint32_t cells[] = {OGRID_CELL_COUNT + 1U, 11U, 12U};
    const ogrid_status_t st = ogrid_update_ray(&s_grid, cells, 3U, true, 2U);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_RANGE, st,
                                  "[TC-F006-31][SYS-PR-08] invalid first index is ERR_RANGE");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, grid_changes(),
                                     "[TC-F006-31][SYS-SAF-07] grid must not be modified (POST-7)");
}

/** @req SYS-PR-08 @contract pre PRE-4 limites validos de indice 0 y COUNT-1 se actualizan */
void test_TC_F006_32_index_bounds_valid(void)
{
    const uint32_t cells[] = {0U, OGRID_CELL_COUNT - 1U};
    const ogrid_status_t st = ogrid_update_ray(&s_grid, cells, 2U, true, 1U);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st, "[TC-F006-32][SYS-PR-08] bounds must be OK");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, -0.40F, s_grid.cells[0].log_odds_lo,
                                     "[TC-F006-32][SYS-PR-08] cell 0 must get -0.40");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, 0.85F, s_grid.cells[OGRID_CELL_COUNT - 1U].log_odds_lo,
                                     "[TC-F006-32][SYS-PR-08] last cell must get +0.85");
}

/** @req SYS-PR-08 @contract post POST-7 error de eco con grid ya con datos: nada cambia */
void test_TC_F006_33_range_error_on_populated_grid(void)
{
    const uint32_t cells[] = {200U, 201U};
    set_cell(200U, 1.1F, OGRID_OBSERVED_YES);
    set_cell(201U, -0.3F, OGRID_OBSERVED_YES);
    const ogrid_status_t st = ogrid_update_ray(&s_grid, cells, 2U, true, 5U);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_RANGE, st,
                                  "[TC-F006-33][SYS-SAF-07] bad echo_pos must be ERR_RANGE");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, grid_changes(),
                                     "[TC-F006-33][SYS-SAF-07] populated grid must not change");
}

/** @req SYS-SAF-07 @contract err ERR_STATE celda observed corrupto en medio: se detiene ahi y las
 * posteriores no cambian */
void test_TC_F006_34_corrupt_observed_stops_ray(void)
{
    const uint32_t cells[] = {10U, 11U, 12U, 13U};
    set_cell(11U, 0.0F, 2U);
    const ogrid_status_t st = ogrid_update_ray(&s_grid, cells, 4U, false, 0U);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_STATE, st,
                                  "[TC-F006-34][SYS-SAF-07] corrupt cell must return ERR_STATE");
    TEST_ASSERT_TRUE_MESSAGE(cell_same(12U),
                             "[TC-F006-34][SYS-SAF-07] cell after the corrupt one must not change");
    TEST_ASSERT_TRUE_MESSAGE(cell_same(13U), "[TC-F006-34][SYS-SAF-07] last cell must not change");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, changes_outside(cells, 4U),
        "[TC-F006-34][SYS-SAF-07] cells outside the beam must not change");
}

/** @req SYS-SAF-07 @contract err ERR_STATE celda con log-odds NaN en la celda del eco */
void test_TC_F006_35_corrupt_nan_on_echo_cell(void)
{
    const uint32_t cells[] = {20U, 21U, 22U};
    set_cell(21U, NAN, OGRID_OBSERVED_YES);
    const ogrid_status_t st = ogrid_update_ray(&s_grid, cells, 3U, true, 1U);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_STATE, st,
                                  "[TC-F006-35][SYS-SAF-07] NaN echo cell must return ERR_STATE");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, changes_outside(cells, 3U),
        "[TC-F006-35][SYS-SAF-07] cells outside the beam must not change");
}

/** @req SYS-SAF-07 @contract err ERR_STATE corrupcion en la primera celda: ninguna otra cambia */
void test_TC_F006_36_corrupt_first_cell(void)
{
    const uint32_t cells[] = {30U, 31U, 32U};
    set_cell(30U, 0.0F, 9U);
    const ogrid_status_t st = ogrid_update_ray(&s_grid, cells, 3U, false, 0U);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_STATE, st,
                                  "[TC-F006-36][SYS-SAF-07] corrupt first cell must be ERR_STATE");
    TEST_ASSERT_TRUE_MESSAGE(cell_same(31U),
                             "[TC-F006-36][SYS-SAF-07] cell 31 must not change after stop");
    TEST_ASSERT_TRUE_MESSAGE(cell_same(32U),
                             "[TC-F006-36][SYS-SAF-07] cell 32 must not change after stop");
}

/** @req SYS-SAF-07 @contract err una celda sana y un haz valido no devuelven ERR_STATE */
void test_TC_F006_37_healthy_grid_never_err_state(void)
{
    const uint32_t cells[] = {50U, 51U, 52U};
    for (uint32_t k = 0U; k < 6U; k++)
    {
        const ogrid_status_t st = ogrid_update_ray(&s_grid, cells, 3U, (k % 2U) == 0U, k % 3U);
        TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, st,
                                      "[TC-F006-37][SYS-SAF-07] healthy grid must give OK");
    }
}

/** @req SYS-PR-08 @contract perf bucle acotado: haz de COUNT celdas termina y es determinista
 * (dos grids iguales dan el mismo resultado) */
void test_TC_F006_38_deterministic_full_ray(void)
{
    ogrid_t other;
    (void)ogrid_reset_grid(&other);
    const ogrid_status_t a = ogrid_update_ray(&s_grid, s_all, OGRID_CELL_COUNT, true, 1000U);
    const ogrid_status_t b = ogrid_update_ray(&other, s_all, OGRID_CELL_COUNT, true, 1000U);
    uint32_t diff = 0U;
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        if ((s_grid.cells[i].log_odds_lo != other.cells[i].log_odds_lo) ||
            (s_grid.cells[i].observed != other.cells[i].observed))
        {
            diff++;
        }
    }
    TEST_ASSERT_TRUE_MESSAGE((a == OGRID_OK) && (b == OGRID_OK),
                             "[TC-F006-38][SYS-PR-08] both calls must return OK");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, diff,
                                     "[TC-F006-38][SYS-PR-08] same input must give same grid");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, 0.85F, s_grid.cells[1000].log_odds_lo,
                                     "[TC-F006-38][SYS-PR-08] echo cell 1000 must be +0.85");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL, 0.0F, s_grid.cells[1001].log_odds_lo,
                                     "[TC-F006-38][SYS-PR-08] cell 1001 behind echo stays 0");
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_TC_F006_01_example_from_contract);
    RUN_TEST(test_TC_F006_02_no_echo_all_pass);
    RUN_TEST(test_TC_F006_03_no_echo_ignores_echo_pos);
    RUN_TEST(test_TC_F006_04_echo_at_first_position);
    RUN_TEST(test_TC_F006_05_echo_at_last_position);
    RUN_TEST(test_TC_F006_06_single_cell_echo);
    RUN_TEST(test_TC_F006_07_occluded_cells_keep_previous_value);
    RUN_TEST(test_TC_F006_08_cells_outside_beam_unchanged);
    RUN_TEST(test_TC_F006_09_no_echo_cells_outside_unchanged);
    RUN_TEST(test_TC_F006_10_pass_adds_to_previous_value);
    RUN_TEST(test_TC_F006_11_echo_adds_to_previous_value);
    RUN_TEST(test_TC_F006_12_echo_clamps_at_upper_limit);
    RUN_TEST(test_TC_F006_13_pass_clamps_at_lower_limit);
    RUN_TEST(test_TC_F006_14_repeated_pass_rays_analytic);
    RUN_TEST(test_TC_F006_15_repeated_echo_rays_analytic);
    RUN_TEST(test_TC_F006_16_echo_then_pass_sequence);
    RUN_TEST(test_TC_F006_17_result_always_within_limits);
    RUN_TEST(test_TC_F006_18_zero_count_does_nothing);
    RUN_TEST(test_TC_F006_19_zero_count_null_cells);
    RUN_TEST(test_TC_F006_20_null_grid);
    RUN_TEST(test_TC_F006_21_null_grid_zero_count);
    RUN_TEST(test_TC_F006_22_null_cells_with_count);
    RUN_TEST(test_TC_F006_23_echo_pos_equals_count);
    RUN_TEST(test_TC_F006_24_echo_pos_huge);
    RUN_TEST(test_TC_F006_25_count_just_above_max);
    RUN_TEST(test_TC_F006_26_count_huge);
    RUN_TEST(test_TC_F006_27_count_at_max_no_echo);
    RUN_TEST(test_TC_F006_28_count_at_max_echo_last);
    RUN_TEST(test_TC_F006_29_index_equals_count_at_end);
    RUN_TEST(test_TC_F006_30_invalid_index_in_occluded_part);
    RUN_TEST(test_TC_F006_31_invalid_index_first);
    RUN_TEST(test_TC_F006_32_index_bounds_valid);
    RUN_TEST(test_TC_F006_33_range_error_on_populated_grid);
    RUN_TEST(test_TC_F006_34_corrupt_observed_stops_ray);
    RUN_TEST(test_TC_F006_35_corrupt_nan_on_echo_cell);
    RUN_TEST(test_TC_F006_36_corrupt_first_cell);
    RUN_TEST(test_TC_F006_37_healthy_grid_never_err_state);
    RUN_TEST(test_TC_F006_38_deterministic_full_ray);
    return UNITY_END();
}
