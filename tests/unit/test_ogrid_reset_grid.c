/**
 * @file test_ogrid_reset_grid.c
 * @brief Unit tests of ogrid_reset_grid (F-001).
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ogrid/ogrid.h"
#include "ogrid/ogrid_reset_grid.h"
#include "unity.h"

static ogrid_t s_grid;
static ogrid_t s_ref;

/** Fill a grid with a deterministic dirty pattern (observed alternating, values varied). */
static void fill_dirty(ogrid_t *g)
{
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        g->cells[i].log_odds_lo = ((i % 3U) == 0U)   ? OGRID_LOG_ODDS_MAX
                                  : ((i % 3U) == 1U) ? OGRID_LOG_ODDS_MIN
                                                     : OGRID_VOTE_ECHO;
        g->cells[i].observed = ((i % 2U) == 0U) ? OGRID_OBSERVED_YES : OGRID_OBSERVED_NO;
    }
}

/** Fill every cell with the same value and flag. */
static void fill_uniform(ogrid_t *g, ogrid_real_t v, uint8_t obs)
{
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        g->cells[i].log_odds_lo = v;
        g->cells[i].observed = obs;
    }
}

/** True when the float has exactly the bit pattern of OGRID_LOG_ODDS_INIT (exact, no tolerance). */
static bool is_init_bits(ogrid_real_t v)
{
    const ogrid_real_t init = OGRID_LOG_ODDS_INIT;
    return memcmp(&v, &init, sizeof(v)) == 0;
}

/** Count cells that violate the reset state. */
static uint32_t count_bad_log_odds(const ogrid_t *g)
{
    uint32_t bad = 0U;
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        if (!is_init_bits(g->cells[i].log_odds_lo))
        {
            bad++;
        }
    }
    return bad;
}

/** Count cells whose guaranteed fields (log_odds_lo bits, observed) differ; padding is ignored. */
static uint32_t count_field_diffs(const ogrid_t *a, const ogrid_t *b)
{
    uint32_t diffs = 0U;
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        const bool same_lo =
            (memcmp(&a->cells[i].log_odds_lo, &b->cells[i].log_odds_lo, sizeof(ogrid_real_t)) == 0);
        const bool same_obs = (a->cells[i].observed == b->cells[i].observed);
        if (!(same_lo && same_obs))
        {
            diffs++;
        }
    }
    return diffs;
}

static uint32_t count_observed(const ogrid_t *g)
{
    uint32_t n = 0U;
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        if (g->cells[i].observed != OGRID_OBSERVED_NO)
        {
            n++;
        }
    }
    return n;
}

void setUp(void)
{
    fill_dirty(&s_grid);
    fill_dirty(&s_ref);
}

void tearDown(void)
{
}

/** @req SYS-PR-08 @contract err OGRID_OK cuando grid_out no es nulo */
void test_TC_F001_01_returns_ok_for_valid_grid(void)
{
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_reset_grid(&s_grid),
                                  "[TC-F001-01][SYS-PR-08] valid grid must return OGRID_OK");
}

/** @req SYS-PR-08 @contract post POST-1 todas las celdas con log-odds exactamente 0 (contenido
 * previo sucio) */
void test_TC_F001_02_all_log_odds_exactly_init_from_dirty(void)
{
    (void)ogrid_reset_grid(&s_grid);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, count_bad_log_odds(&s_grid),
        "[TC-F001-02][SYS-PR-08] every cell log_odds_lo must equal OGRID_LOG_ODDS_INIT exactly");
}

/** @req SYS-PR-08 @contract post POST-2 todas las celdas con observed == OGRID_OBSERVED_NO */
void test_TC_F001_03_all_cells_not_observed(void)
{
    (void)ogrid_reset_grid(&s_grid);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, count_observed(&s_grid),
        "[TC-F001-03][SYS-PR-08] every cell must have observed == false");
}

/** @req SYS-PR-08 @contract post POST-1 limites de indice: primera y ultima celda (0 y COUNT-1) */
void test_TC_F001_04_first_and_last_cell_reset(void)
{
    (void)ogrid_reset_grid(&s_grid);
    TEST_ASSERT_TRUE_MESSAGE(is_init_bits(s_grid.cells[0].log_odds_lo),
                             "[TC-F001-04][SYS-PR-08] first cell log-odds must be 0");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(OGRID_OBSERVED_NO, s_grid.cells[0].observed,
                                    "[TC-F001-04][SYS-PR-08] first cell must be not observed");
    TEST_ASSERT_TRUE_MESSAGE(is_init_bits(s_grid.cells[OGRID_CELL_COUNT - 1U].log_odds_lo),
                             "[TC-F001-04][SYS-PR-08] last cell log-odds must be 0");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(OGRID_OBSERVED_NO, s_grid.cells[OGRID_CELL_COUNT - 1U].observed,
                                    "[TC-F001-04][SYS-PR-08] last cell must be not observed");
}

/** @req SYS-PR-08 @contract post POST-1 el grid tiene 3520 celdas (880 x 4) */
void test_TC_F001_05_cell_count_is_3520(void)
{
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3520U, (uint32_t)OGRID_CELL_COUNT,
                                     "[TC-F001-05][SYS-PR-08] OGRID_CELL_COUNT must be 3520");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3520U,
                                     (uint32_t)(sizeof(s_grid.cells) / sizeof(s_grid.cells[0])),
                                     "[TC-F001-05][SYS-PR-08] grid array must hold 3520 cells");
}

/** @req SYS-SAF-07 @contract post POST-3 desde celdas ocupadas (max) no queda ninguna con evidencia
 */
void test_TC_F001_06_reset_from_all_max_occupied(void)
{
    fill_uniform(&s_grid, OGRID_LOG_ODDS_MAX, OGRID_OBSERVED_YES);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_reset_grid(&s_grid),
                                  "[TC-F001-06][SYS-SAF-07] reset must return OGRID_OK");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, count_bad_log_odds(&s_grid),
        "[TC-F001-06][SYS-SAF-07] no cell may keep occupied log-odds after reset");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, count_observed(&s_grid),
        "[TC-F001-06][SYS-SAF-07] no cell may stay observed (would not be UNSURVEYED)");
}

/** @req SYS-SAF-07 @contract post POST-3 desde celdas de probabilidad baja (min) vuelven a no
 * estudiadas */
void test_TC_F001_07_reset_from_all_min_low_prob(void)
{
    fill_uniform(&s_grid, OGRID_LOG_ODDS_MIN, OGRID_OBSERVED_YES);
    (void)ogrid_reset_grid(&s_grid);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, count_bad_log_odds(&s_grid),
        "[TC-F001-07][SYS-SAF-07] no cell may keep low-probability log-odds after reset");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, count_observed(&s_grid),
        "[TC-F001-07][SYS-SAF-07] no cell may stay observed after reset");
}

/** @req SYS-SAF-07 @contract post POST-3 ninguna celda ocupada: log-odds 0 implica p=0,5 < 0,85 y
 * observed false */
void test_TC_F001_08_no_cell_can_be_occupied_or_low_prob(void)
{
    uint32_t not_unsurveyed = 0U;
    (void)ogrid_reset_grid(&s_grid);
    for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
    {
        /* Derived state is UNSURVEYED iff observed == false; log-odds 0 means no evidence. */
        if ((s_grid.cells[i].observed != OGRID_OBSERVED_NO) ||
            !is_init_bits(s_grid.cells[i].log_odds_lo))
        {
            not_unsurveyed++;
        }
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, not_unsurveyed,
        "[TC-F001-08][SYS-SAF-07] every cell must be derivable as OGRID_CELL_UNSURVEYED");
}

/** @req SYS-PR-08 @contract post POST-1 contenido previo con valores extremos (NaN, inf, grande) se
 * sobrescribe */
void test_TC_F001_09_reset_from_garbage_bit_patterns(void)
{
    /* All-ones bytes give a NaN float and a out-of-range observed value (255) in the source memory.
     */
    memset(&s_grid, 0xFF, sizeof(s_grid));
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_reset_grid(&s_grid),
                                  "[TC-F001-09][SYS-PR-08] garbage grid must return OGRID_OK");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, count_bad_log_odds(&s_grid),
        "[TC-F001-09][SYS-PR-08] log-odds must be exactly 0 after reset from garbage");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, count_observed(&s_grid),
        "[TC-F001-09][SYS-PR-08] observed must be false after reset from garbage");
}

/** @req SYS-PR-08 @contract post POST-4 idempotencia: dos llamadas seguidas dejan el mismo
 * resultado */
void test_TC_F001_10_idempotent_double_call(void)
{
    (void)ogrid_reset_grid(&s_grid);
    (void)memcpy(&s_ref, &s_grid, sizeof(s_grid));
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_reset_grid(&s_grid),
                                  "[TC-F001-10][SYS-PR-08] second call must return OGRID_OK");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, count_bad_log_odds(&s_grid),
        "[TC-F001-10][SYS-PR-08] log-odds must stay 0 after second call");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, count_observed(&s_grid),
        "[TC-F001-10][SYS-PR-08] observed must stay false after second call");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, count_field_diffs(&s_ref, &s_grid),
        "[TC-F001-10][SYS-PR-08] second call must leave the same state as the first");
}

/** @req SYS-PR-08 @contract post POST-4 idempotencia tras dato nuevo entre reinicios: vuelve a 0 */
void test_TC_F001_11_reset_after_modification_restores_state(void)
{
    (void)ogrid_reset_grid(&s_grid);
    s_grid.cells[100].log_odds_lo = OGRID_VOTE_ECHO;
    s_grid.cells[100].observed = OGRID_OBSERVED_YES;
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_reset_grid(&s_grid),
                                  "[TC-F001-11][SYS-PR-08] reset must return OGRID_OK");
    TEST_ASSERT_TRUE_MESSAGE(is_init_bits(s_grid.cells[100].log_odds_lo),
                             "[TC-F001-11][SYS-PR-08] modified cell log-odds must return to 0");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(
        OGRID_OBSERVED_NO, s_grid.cells[100].observed,
        "[TC-F001-11][SYS-PR-08] modified cell must be not observed again");
}

/** @req SYS-PR-08 @contract err OGRID_ERR_PARAM cuando grid_out es NULL (PRE-1) */
void test_TC_F001_12_null_returns_err_param(void)
{
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_PARAM, ogrid_reset_grid(NULL),
        "[TC-F001-12][SYS-PR-08] NULL grid_out must return OGRID_ERR_PARAM");
}

/** @req SYS-PR-08 @contract post POST-5 con error no se modifica ningun dato (grid ajeno intacto)
 */
void test_TC_F001_13_null_does_not_modify_other_data(void)
{
    (void)ogrid_reset_grid(NULL);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, count_field_diffs(&s_ref, &s_grid),
        "[TC-F001-13][SYS-PR-08] a NULL call must not modify any other grid");
}

/** @req SYS-PR-08 @contract err el error no se confunde con otros codigos: NULL no devuelve OK ni
 * RANGE/STATE */
void test_TC_F001_14_null_status_is_not_ok(void)
{
    ogrid_status_t st = ogrid_reset_grid(NULL);
    TEST_ASSERT_NOT_EQUAL_MESSAGE(OGRID_OK, st,
                                  "[TC-F001-14][SYS-PR-08] NULL must not return OGRID_OK");
    TEST_ASSERT_NOT_EQUAL_MESSAGE(OGRID_ERR_RANGE, st,
                                  "[TC-F001-14][SYS-PR-08] NULL must not return OGRID_ERR_RANGE");
    TEST_ASSERT_NOT_EQUAL_MESSAGE(OGRID_ERR_STATE, st,
                                  "[TC-F001-14][SYS-PR-08] NULL must not return OGRID_ERR_STATE");
}

/** @req SYS-IMP-01 @contract perf sin memoria dinamica: el grid es de tamano fijo y la funcion solo
 * toca *grid_out */
void test_TC_F001_15_only_grid_out_is_modified(void)
{
    /* Two adjacent grids in one struct: resetting the first must leave the second untouched. */
    static struct
    {
        ogrid_t a;
        ogrid_t b;
    } pair;
    fill_dirty(&pair.a);
    fill_dirty(&pair.b);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_reset_grid(&pair.a),
                                  "[TC-F001-15][SYS-IMP-01] reset must return OGRID_OK");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, count_bad_log_odds(&pair.a),
                                     "[TC-F001-15][SYS-IMP-01] target grid must be reset");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, count_field_diffs(&s_ref, &pair.b),
        "[TC-F001-15][SYS-IMP-01] neighbouring grid must not be modified");
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_TC_F001_01_returns_ok_for_valid_grid);
    RUN_TEST(test_TC_F001_02_all_log_odds_exactly_init_from_dirty);
    RUN_TEST(test_TC_F001_03_all_cells_not_observed);
    RUN_TEST(test_TC_F001_04_first_and_last_cell_reset);
    RUN_TEST(test_TC_F001_05_cell_count_is_3520);
    RUN_TEST(test_TC_F001_06_reset_from_all_max_occupied);
    RUN_TEST(test_TC_F001_07_reset_from_all_min_low_prob);
    RUN_TEST(test_TC_F001_08_no_cell_can_be_occupied_or_low_prob);
    RUN_TEST(test_TC_F001_09_reset_from_garbage_bit_patterns);
    RUN_TEST(test_TC_F001_10_idempotent_double_call);
    RUN_TEST(test_TC_F001_11_reset_after_modification_restores_state);
    RUN_TEST(test_TC_F001_12_null_returns_err_param);
    RUN_TEST(test_TC_F001_13_null_does_not_modify_other_data);
    RUN_TEST(test_TC_F001_14_null_status_is_not_ok);
    RUN_TEST(test_TC_F001_15_only_grid_out_is_modified);
    return UNITY_END();
}
