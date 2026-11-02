/**
 * @file test_ogrid_clamp_log_odds.c
 * @brief Unit tests of ogrid_clamp_log_odds (F-003).
 * @details The contract requires exact results (no tolerance): outputs are compared bit by bit.
 *          Limits are [-0.8, 3.4] (OGRID_LOG_ODDS_MIN / OGRID_LOG_ODDS_MAX).
 */
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ogrid/ogrid.h"
#include "ogrid/ogrid_clamp_log_odds.h"
#include "unity.h"

/** Sentinel written in the output before calling, to detect modifications. */
#define SENTINEL (0.123456F)
/** Number of steps of the sweep over [-5, 5]. */
#define SWEEP_STEPS (200U)

static ogrid_real_t s_out;

/** True when a and b have identical bit patterns. */
static bool same_bits(ogrid_real_t a, ogrid_real_t b)
{
    return memcmp(&a, &b, sizeof(a)) == 0;
}

/** Neighbour of a nonzero float: away from zero (up = true) or towards zero (up = false). */
static ogrid_real_t step_bits(ogrid_real_t v, bool away)
{
    uint32_t bits = 0U;
    ogrid_real_t r = 0.0F;
    (void)memcpy(&bits, &v, sizeof(bits));
    bits = away ? (bits + 1U) : (bits - 1U);
    (void)memcpy(&r, &bits, sizeof(r));
    return r;
}

/** True when s_out still holds exactly the sentinel bit pattern. */
static bool out_is_sentinel(void)
{
    return same_bits(s_out, SENTINEL);
}

void setUp(void)
{
    s_out = SENTINEL;
}

void tearDown(void)
{
}

/** @req SYS-PR-08 @contract err OGRID_OK con parametros validos (valor dentro de limites) */
void test_TC_F003_01_valid_input_returns_ok(void)
{
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_clamp_log_odds(0.85F, &s_out),
                                  "[TC-F003-01][SYS-PR-08] valid input must return OGRID_OK");
}

/** @req SYS-PR-08 @contract post POST-1 valor 0 dentro de limites se devuelve identico */
void test_TC_F003_02_zero_unchanged(void)
{
    (void)ogrid_clamp_log_odds(0.0F, &s_out);
    TEST_ASSERT_TRUE_MESSAGE(s_out == 0.0F, "[TC-F003-02][SYS-PR-08] clamp(0) must be 0 exactly");
}

/** @req SYS-PR-08 @contract post POST-1 valores interiores (0,85; 1,70; 2,55; -0,4) exactos */
void test_TC_F003_03_interior_values_unchanged(void)
{
    const ogrid_real_t vs[] = {0.85F, 1.70F, 2.55F, -0.4F, 3.0F};
    uint32_t bad = 0U;
    for (size_t i = 0U; i < 5U; i++)
    {
        ogrid_real_t o = SENTINEL;
        if ((ogrid_clamp_log_odds(vs[i], &o) != OGRID_OK) || !same_bits(o, vs[i]))
        {
            bad++;
        }
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, bad, "[TC-F003-03][SYS-PR-08] interior values must be returned bit-exact");
}

/** @req SYS-SAF-07 @contract post POST-1 limite minimo inclusivo: clamp(MIN) = MIN exacto */
void test_TC_F003_04_min_boundary_unchanged(void)
{
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_clamp_log_odds(OGRID_LOG_ODDS_MIN, &s_out),
                                  "[TC-F003-04][SYS-SAF-07] MIN must return OGRID_OK");
    TEST_ASSERT_TRUE_MESSAGE(same_bits(s_out, OGRID_LOG_ODDS_MIN),
                             "[TC-F003-04][SYS-SAF-07] clamp(MIN) must be MIN exactly");
}

/** @req SYS-SAF-07 @contract post POST-1 limite maximo inclusivo: clamp(MAX) = MAX exacto */
void test_TC_F003_05_max_boundary_unchanged(void)
{
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_clamp_log_odds(OGRID_LOG_ODDS_MAX, &s_out),
                                  "[TC-F003-05][SYS-SAF-07] MAX must return OGRID_OK");
    TEST_ASSERT_TRUE_MESSAGE(same_bits(s_out, OGRID_LOG_ODDS_MAX),
                             "[TC-F003-05][SYS-SAF-07] clamp(MAX) must be MAX exactly");
}

/** @req SYS-SAF-07 @contract post POST-1 un ulp dentro de cada limite se devuelve sin cambio */
void test_TC_F003_06_just_inside_limits_unchanged(void)
{
    const ogrid_real_t in_lo = step_bits(OGRID_LOG_ODDS_MIN, false);
    const ogrid_real_t in_hi = step_bits(OGRID_LOG_ODDS_MAX, false);
    ogrid_real_t o = SENTINEL;
    (void)ogrid_clamp_log_odds(in_lo, &o);
    TEST_ASSERT_TRUE_MESSAGE(same_bits(o, in_lo),
                             "[TC-F003-06][SYS-SAF-07] one ulp above MIN must be unchanged");
    o = SENTINEL;
    (void)ogrid_clamp_log_odds(in_hi, &o);
    TEST_ASSERT_TRUE_MESSAGE(same_bits(o, in_hi),
                             "[TC-F003-06][SYS-SAF-07] one ulp below MAX must be unchanged");
}

/** @req SYS-SAF-07 @contract post POST-2 un ulp por debajo de MIN -> MIN exacto, OK */
void test_TC_F003_07_just_below_min_gives_min(void)
{
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_OK, ogrid_clamp_log_odds(step_bits(OGRID_LOG_ODDS_MIN, true), &s_out),
        "[TC-F003-07][SYS-SAF-07] one ulp below MIN must return OGRID_OK");
    TEST_ASSERT_TRUE_MESSAGE(same_bits(s_out, OGRID_LOG_ODDS_MIN),
                             "[TC-F003-07][SYS-SAF-07] one ulp below MIN must give MIN exactly");
}

/** @req SYS-SAF-07 @contract post POST-3 un ulp por encima de MAX -> MAX exacto, OK */
void test_TC_F003_08_just_above_max_gives_max(void)
{
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_OK, ogrid_clamp_log_odds(step_bits(OGRID_LOG_ODDS_MAX, true), &s_out),
        "[TC-F003-08][SYS-SAF-07] one ulp above MAX must return OGRID_OK");
    TEST_ASSERT_TRUE_MESSAGE(same_bits(s_out, OGRID_LOG_ODDS_MAX),
                             "[TC-F003-08][SYS-SAF-07] one ulp above MAX must give MAX exactly");
}

/** @req SYS-PR-08 @contract post POST-2 valores claramente bajos (-1,0 y -100) -> MIN */
void test_TC_F003_09_clearly_below_gives_min(void)
{
    ogrid_real_t o = SENTINEL;
    (void)ogrid_clamp_log_odds(-1.0F, &o);
    TEST_ASSERT_TRUE_MESSAGE(same_bits(o, OGRID_LOG_ODDS_MIN),
                             "[TC-F003-09][SYS-PR-08] -1.0 must clamp to MIN");
    o = SENTINEL;
    (void)ogrid_clamp_log_odds(-100.0F, &o);
    TEST_ASSERT_TRUE_MESSAGE(same_bits(o, OGRID_LOG_ODDS_MIN),
                             "[TC-F003-09][SYS-PR-08] -100 must clamp to MIN");
}

/** @req SYS-PR-08 @contract post POST-3 valores claramente altos (3,5; 4,0; 100) -> MAX */
void test_TC_F003_10_clearly_above_gives_max(void)
{
    const ogrid_real_t vs[] = {3.5F, 4.0F, 100.0F};
    uint32_t bad = 0U;
    for (size_t i = 0U; i < 3U; i++)
    {
        ogrid_real_t o = SENTINEL;
        if ((ogrid_clamp_log_odds(vs[i], &o) != OGRID_OK) || !same_bits(o, OGRID_LOG_ODDS_MAX))
        {
            bad++;
        }
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, bad,
                                     "[TC-F003-10][SYS-PR-08] values above MAX must clamp to MAX");
}

/** @req SYS-SAF-07 @contract post POST-2 y POST-3 extremos finitos +-FLT_MAX son admisibles */
void test_TC_F003_11_float_extremes_are_clamped(void)
{
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_clamp_log_odds(FLT_MAX, &s_out),
                                  "[TC-F003-11][SYS-SAF-07] FLT_MAX must return OGRID_OK");
    TEST_ASSERT_TRUE_MESSAGE(same_bits(s_out, OGRID_LOG_ODDS_MAX),
                             "[TC-F003-11][SYS-SAF-07] FLT_MAX must clamp to MAX");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_clamp_log_odds(-FLT_MAX, &s_out),
                                  "[TC-F003-11][SYS-SAF-07] -FLT_MAX must return OGRID_OK");
    TEST_ASSERT_TRUE_MESSAGE(same_bits(s_out, OGRID_LOG_ODDS_MIN),
                             "[TC-F003-11][SYS-SAF-07] -FLT_MAX must clamp to MIN");
}

/** @req SYS-SAF-07 @contract post POST-2 y POST-3 los limites valen -0,8 y 3,4 (diseno 6.2) */
void test_TC_F003_12_limits_are_design_values(void)
{
    ogrid_real_t o = SENTINEL;
    (void)ogrid_clamp_log_odds(-10.0F, &o);
    TEST_ASSERT_TRUE_MESSAGE(same_bits(o, (ogrid_real_t)-0.8F),
                             "[TC-F003-12][SYS-SAF-07] lower limit must be -0.8");
    o = SENTINEL;
    (void)ogrid_clamp_log_odds(10.0F, &o);
    TEST_ASSERT_TRUE_MESSAGE(same_bits(o, (ogrid_real_t)3.4F),
                             "[TC-F003-12][SYS-SAF-07] upper limit must be 3.4");
}

/** @req SYS-PR-08 @contract post POST-4 idempotencia: clamp(clamp(x)) = clamp(x) */
void test_TC_F003_13_idempotent(void)
{
    const ogrid_real_t vs[] = {-50.0F, -0.8F, -0.5F, 0.0F, 1.7F, 3.4F, 3.41F, 50.0F};
    uint32_t bad = 0U;
    for (size_t i = 0U; i < 8U; i++)
    {
        ogrid_real_t once = SENTINEL;
        ogrid_real_t twice = SENTINEL;
        if ((ogrid_clamp_log_odds(vs[i], &once) != OGRID_OK) ||
            (ogrid_clamp_log_odds(once, &twice) != OGRID_OK) || !same_bits(once, twice))
        {
            bad++;
        }
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, bad,
                                     "[TC-F003-13][SYS-PR-08] clamping twice must equal once");
}

/** @req SYS-SAF-07 @contract post POST-2 acotacion: MIN <= salida <= MAX en barrido [-5, 5] paso
 * 0,05 */
void test_TC_F003_14_sweep_output_bounded(void)
{
    uint32_t bad = 0U;
    for (uint32_t i = 0U; i <= SWEEP_STEPS; i++)
    {
        const ogrid_real_t x = -5.0F + ((ogrid_real_t)i * 0.05F);
        ogrid_real_t o = SENTINEL;
        if ((ogrid_clamp_log_odds(x, &o) != OGRID_OK) || !(o >= OGRID_LOG_ODDS_MIN) ||
            !(o <= OGRID_LOG_ODDS_MAX))
        {
            bad++;
        }
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, bad, "[TC-F003-14][SYS-SAF-07] every output must lie within [MIN, MAX]");
}

/** @req SYS-PR-08 @contract post POST-1 monotonia: x1 <= x2 implica clamp(x1) <= clamp(x2) en el
 * barrido [-5, 5] */
void test_TC_F003_15_sweep_monotonic_non_decreasing(void)
{
    uint32_t bad = 0U;
    ogrid_real_t prev = SENTINEL;
    (void)ogrid_clamp_log_odds(-5.0F, &prev);
    for (uint32_t i = 1U; i <= SWEEP_STEPS; i++)
    {
        const ogrid_real_t x = -5.0F + ((ogrid_real_t)i * 0.05F);
        ogrid_real_t cur = SENTINEL;
        (void)ogrid_clamp_log_odds(x, &cur);
        if (!(cur >= prev))
        {
            bad++;
        }
        prev = cur;
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, bad,
                                     "[TC-F003-15][SYS-PR-08] clamp must be non-decreasing");
}

/** @req SYS-SAF-07 @contract err OGRID_ERR_RANGE para NaN (PRE-2) */
void test_TC_F003_16_nan_is_range_error(void)
{
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_RANGE, ogrid_clamp_log_odds(NAN, &s_out),
                                  "[TC-F003-16][SYS-SAF-07] NaN must return OGRID_ERR_RANGE");
}

/** @req SYS-SAF-07 @contract err OGRID_ERR_RANGE para +inf y -inf (PRE-2) */
void test_TC_F003_17_infinity_is_range_error(void)
{
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_RANGE, ogrid_clamp_log_odds(INFINITY, &s_out),
                                  "[TC-F003-17][SYS-SAF-07] +inf must return OGRID_ERR_RANGE");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_RANGE, ogrid_clamp_log_odds(-INFINITY, &s_out),
                                  "[TC-F003-17][SYS-SAF-07] -inf must return OGRID_ERR_RANGE");
}

/** @req SYS-SAF-07 @contract post POST-5 NaN: la salida no se modifica */
void test_TC_F003_18_nan_keeps_output(void)
{
    (void)ogrid_clamp_log_odds(NAN, &s_out);
    TEST_ASSERT_TRUE_MESSAGE(out_is_sentinel(),
                             "[TC-F003-18][SYS-SAF-07] output must be untouched for NaN");
}

/** @req SYS-SAF-07 @contract post POST-5 infinitos: la salida no se modifica */
void test_TC_F003_19_infinity_keeps_output(void)
{
    (void)ogrid_clamp_log_odds(INFINITY, &s_out);
    TEST_ASSERT_TRUE_MESSAGE(out_is_sentinel(),
                             "[TC-F003-19][SYS-SAF-07] output must be untouched for +inf");
    (void)ogrid_clamp_log_odds(-INFINITY, &s_out);
    TEST_ASSERT_TRUE_MESSAGE(out_is_sentinel(),
                             "[TC-F003-19][SYS-SAF-07] output must be untouched for -inf");
}

/** @req SYS-SAF-07 @contract err OGRID_ERR_PARAM cuando clamped_lo_out es NULL (PRE-1), valor
 * valido */
void test_TC_F003_20_null_out_is_param_error(void)
{
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_PARAM, ogrid_clamp_log_odds(0.85F, NULL),
                                  "[TC-F003-20][SYS-SAF-07] NULL output must return ERR_PARAM");
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_PARAM, ogrid_clamp_log_odds(10.0F, NULL),
        "[TC-F003-20][SYS-SAF-07] NULL output (value > MAX) is ERR_PARAM");
}

/** @req SYS-SAF-07 @contract post POST-5 con NULL no se toca ningun dato ajeno */
void test_TC_F003_21_null_out_no_side_effect(void)
{
    (void)ogrid_clamp_log_odds(0.0F, NULL);
    TEST_ASSERT_TRUE_MESSAGE(out_is_sentinel(),
                             "[TC-F003-21][SYS-SAF-07] unrelated output must stay untouched");
}

/** @req SYS-PR-08 @contract post POST-5 tras un error previo, una llamada valida escribe la salida
 */
void test_TC_F003_22_valid_call_after_error_works(void)
{
    (void)ogrid_clamp_log_odds(NAN, &s_out);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_clamp_log_odds(1.0F, &s_out),
                                  "[TC-F003-22][SYS-PR-08] valid call after error must be OK");
    TEST_ASSERT_TRUE_MESSAGE(same_bits(s_out, 1.0F),
                             "[TC-F003-22][SYS-PR-08] valid call after error gives 1.0");
}

/** @req SYS-SAF-07 @contract perf funcion pura sin estado: misma entrada, mismo resultado exacto
 * con otra llamada intercalada */
void test_TC_F003_23_pure_and_deterministic(void)
{
    ogrid_real_t a = 0.0F;
    ogrid_real_t b = 0.0F;
    ogrid_real_t other = 0.0F;
    (void)ogrid_clamp_log_odds(7.0F, &a);
    (void)ogrid_clamp_log_odds(-7.0F, &other);
    (void)ogrid_clamp_log_odds(7.0F, &b);
    TEST_ASSERT_TRUE_MESSAGE(same_bits(a, b),
                             "[TC-F003-23][SYS-SAF-07] same input must give identical output");
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_TC_F003_01_valid_input_returns_ok);
    RUN_TEST(test_TC_F003_02_zero_unchanged);
    RUN_TEST(test_TC_F003_03_interior_values_unchanged);
    RUN_TEST(test_TC_F003_04_min_boundary_unchanged);
    RUN_TEST(test_TC_F003_05_max_boundary_unchanged);
    RUN_TEST(test_TC_F003_06_just_inside_limits_unchanged);
    RUN_TEST(test_TC_F003_07_just_below_min_gives_min);
    RUN_TEST(test_TC_F003_08_just_above_max_gives_max);
    RUN_TEST(test_TC_F003_09_clearly_below_gives_min);
    RUN_TEST(test_TC_F003_10_clearly_above_gives_max);
    RUN_TEST(test_TC_F003_11_float_extremes_are_clamped);
    RUN_TEST(test_TC_F003_12_limits_are_design_values);
    RUN_TEST(test_TC_F003_13_idempotent);
    RUN_TEST(test_TC_F003_14_sweep_output_bounded);
    RUN_TEST(test_TC_F003_15_sweep_monotonic_non_decreasing);
    RUN_TEST(test_TC_F003_16_nan_is_range_error);
    RUN_TEST(test_TC_F003_17_infinity_is_range_error);
    RUN_TEST(test_TC_F003_18_nan_keeps_output);
    RUN_TEST(test_TC_F003_19_infinity_keeps_output);
    RUN_TEST(test_TC_F003_20_null_out_is_param_error);
    RUN_TEST(test_TC_F003_21_null_out_no_side_effect);
    RUN_TEST(test_TC_F003_22_valid_call_after_error_works);
    RUN_TEST(test_TC_F003_23_pure_and_deterministic);
    return UNITY_END();
}
