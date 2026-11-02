/**
 * @file test_ogrid_log_odds_to_prob.c
 * @brief Unit tests of ogrid_log_odds_to_prob (F-002).
 * @details Reference values are computed by hand from p = 1 / (1 + e^-l) (7 significant digits,
 *          hand error below 2e-7). Contract tolerance is 1e-6 (float32).
 */
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ogrid/ogrid.h"
#include "ogrid/ogrid_log_odds_to_prob.h"
#include "unity.h"

/** Contract tolerance for POST-1/POST-2/POST-4. */
#define TOL_CONTRACT (1.0e-6F)
/** Sentinel written in the output before calling, to detect modifications. */
#define SENTINEL (0.123456F)
/** Number of steps of the monotonicity sweep over [MIN, MAX]. */
#define SWEEP_STEPS (84U)

static ogrid_real_t s_out;

/** Next representable float away from v towards larger bit pattern (for same-sign v). */
static ogrid_real_t next_bits(ogrid_real_t v)
{
    uint32_t bits = 0U;
    ogrid_real_t r = 0.0F;
    (void)memcpy(&bits, &v, sizeof(bits));
    bits++;
    (void)memcpy(&r, &bits, sizeof(r));
    return r;
}

/** True when s_out still holds exactly the sentinel bit pattern. */
static bool out_is_sentinel(void)
{
    const ogrid_real_t s = SENTINEL;
    return memcmp(&s_out, &s, sizeof(s)) == 0;
}

/** Log-odds of sweep point i: MIN at 0, MAX at SWEEP_STEPS, step 0.05. */
static ogrid_real_t sweep_value(uint32_t i)
{
    if (i == 0U)
    {
        return OGRID_LOG_ODDS_MIN;
    }
    if (i >= SWEEP_STEPS)
    {
        return OGRID_LOG_ODDS_MAX;
    }
    return OGRID_LOG_ODDS_MIN + ((ogrid_real_t)i * 0.05F);
}

void setUp(void)
{
    s_out = SENTINEL;
}

void tearDown(void)
{
}

/** @req SYS-PR-08 @contract err OGRID_OK con parametros validos (l = 0) */
void test_TC_F002_01_valid_input_returns_ok(void)
{
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_log_odds_to_prob(0.0F, &s_out),
                                  "[TC-F002-01][SYS-PR-08] valid input must return OGRID_OK");
}

/** @req SYS-PR-08 @contract post POST-4 l = 0 da exactamente 0,5 (tol 1e-6, contrato) */
void test_TC_F002_02_zero_gives_half(void)
{
    (void)ogrid_log_odds_to_prob(0.0F, &s_out);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL_CONTRACT, 0.5F, s_out,
                                     "[TC-F002-02][SYS-PR-08] p(0) must be 0.5");
}

/** @req SYS-PR-08 @contract post POST-2 l = 0,85 -> 1/(1+e^-0,85) = 0,7005671 (tol 1e-6) */
void test_TC_F002_03_ref_echo_vote(void)
{
    (void)ogrid_log_odds_to_prob(0.85F, &s_out);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL_CONTRACT, 0.7005671F, s_out,
                                     "[TC-F002-03][SYS-PR-08] p(0.85) must be 0.7005671");
}

/** @req SYS-PR-08 @contract post POST-2 l = 1,70 -> 0,8455347 (tol 1e-6) */
void test_TC_F002_04_ref_two_echoes(void)
{
    (void)ogrid_log_odds_to_prob(1.70F, &s_out);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL_CONTRACT, 0.8455347F, s_out,
                                     "[TC-F002-04][SYS-PR-08] p(1.70) must be 0.8455347");
}

/** @req SYS-PR-08 @contract post POST-2 l = 2,55 -> 0,9275735 (tol 1e-6) */
void test_TC_F002_05_ref_three_echoes(void)
{
    (void)ogrid_log_odds_to_prob(2.55F, &s_out);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL_CONTRACT, 0.9275735F, s_out,
                                     "[TC-F002-05][SYS-PR-08] p(2.55) must be 0.9275735");
}

/** @req SYS-PR-08 @contract post POST-2 limite maximo l = 3,4 -> 0,9677045 (tol 1e-6) */
void test_TC_F002_06_ref_upper_limit(void)
{
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_OK, ogrid_log_odds_to_prob(OGRID_LOG_ODDS_MAX, &s_out),
        "[TC-F002-06][SYS-PR-08] OGRID_LOG_ODDS_MAX must be accepted (inclusive limit)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL_CONTRACT, 0.9677045F, s_out,
                                     "[TC-F002-06][SYS-PR-08] p(3.4) must be 0.9677045");
}

/** @req SYS-PR-08 @contract post POST-2 limite minimo l = -0,8 -> 0,3100255 (tol 1e-6) */
void test_TC_F002_07_ref_lower_limit(void)
{
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_OK, ogrid_log_odds_to_prob(OGRID_LOG_ODDS_MIN, &s_out),
        "[TC-F002-07][SYS-PR-08] OGRID_LOG_ODDS_MIN must be accepted (inclusive limit)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL_CONTRACT, 0.3100255F, s_out,
                                     "[TC-F002-07][SYS-PR-08] p(-0.8) must be 0.3100255");
}

/** @req SYS-PR-08 @contract post POST-2 valores literales del contrato (4 decimales); tolerancia
 * 5e-5 = media unidad del ultimo decimal citado (0,00005) */
void test_TC_F002_08_contract_quoted_values_rounded(void)
{
    const ogrid_real_t tol = 5.0e-5F;
    (void)ogrid_log_odds_to_prob(0.85F, &s_out);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(tol, 0.7006F, s_out,
                                     "[TC-F002-08][SYS-PR-08] p(0.85) must round to 0.7006");
    (void)ogrid_log_odds_to_prob(1.70F, &s_out);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(tol, 0.8455F, s_out,
                                     "[TC-F002-08][SYS-PR-08] p(1.70) must round to 0.8455");
    (void)ogrid_log_odds_to_prob(2.55F, &s_out);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(tol, 0.9276F, s_out,
                                     "[TC-F002-08][SYS-PR-08] p(2.55) must round to 0.9276");
    (void)ogrid_log_odds_to_prob(3.4F, &s_out);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(tol, 0.9677F, s_out,
                                     "[TC-F002-08][SYS-PR-08] p(3.4) must round to 0.9677");
    (void)ogrid_log_odds_to_prob(-0.8F, &s_out);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(tol, 0.3100F, s_out,
                                     "[TC-F002-08][SYS-PR-08] p(-0.8) must round to 0.3100");
}

/** @req SYS-PR-08 @contract post POST-1 puntos interiores con valores analiticos conocidos de la
 * sigmoide (tol 1e-6) */
void test_TC_F002_09_formula_interior_points(void)
{
    (void)ogrid_log_odds_to_prob(1.0F, &s_out);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL_CONTRACT, 0.7310586F, s_out,
                                     "[TC-F002-09][SYS-PR-08] p(1.0) must be 0.7310586");
    (void)ogrid_log_odds_to_prob(2.0F, &s_out);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL_CONTRACT, 0.8807971F, s_out,
                                     "[TC-F002-09][SYS-PR-08] p(2.0) must be 0.8807971");
    (void)ogrid_log_odds_to_prob(3.0F, &s_out);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL_CONTRACT, 0.9525741F, s_out,
                                     "[TC-F002-09][SYS-PR-08] p(3.0) must be 0.9525741");
    (void)ogrid_log_odds_to_prob(-0.4F, &s_out);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL_CONTRACT, 0.4013123F, s_out,
                                     "[TC-F002-09][SYS-PR-08] p(-0.4) must be 0.4013123");
}

/** @req SYS-PR-08 @contract post POST-3 acotacion 0 < p < 1 en todo el barrido [MIN, MAX] */
void test_TC_F002_10_sweep_bounded_open_interval(void)
{
    uint32_t bad = 0U;
    for (uint32_t i = 0U; i <= SWEEP_STEPS; i++)
    {
        ogrid_real_t p = SENTINEL;
        if ((ogrid_log_odds_to_prob(sweep_value(i), &p) != OGRID_OK) || !(p > 0.0F) || !(p < 1.0F))
        {
            bad++;
        }
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, bad, "[TC-F002-10][SYS-PR-08] every sweep point must be OK with 0 < p < 1");
}

/** @req SYS-PR-08 @contract post POST-3 monotonia estrictamente creciente en el barrido (paso 0,05;
 * incremento minimo de p ~1,5e-3 >> epsilon float32) */
void test_TC_F002_11_sweep_strictly_increasing(void)
{
    uint32_t bad = 0U;
    ogrid_real_t prev = 0.0F;
    (void)ogrid_log_odds_to_prob(sweep_value(0U), &prev);
    for (uint32_t i = 1U; i <= SWEEP_STEPS; i++)
    {
        ogrid_real_t cur = 0.0F;
        (void)ogrid_log_odds_to_prob(sweep_value(i), &cur);
        if (!(cur > prev))
        {
            bad++;
        }
        prev = cur;
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        0U, bad, "[TC-F002-11][SYS-PR-08] p must be strictly increasing with log-odds");
}

/** @req SYS-PR-08 @contract post POST-3 monotonia entre los valores de voto del diseno: p(MIN) <
 * p(0) < p(0,85) < p(1,70) < p(2,55) < p(MAX) */
void test_TC_F002_12_monotonic_reference_chain(void)
{
    const ogrid_real_t ls[] = {OGRID_LOG_ODDS_MIN, 0.0F, 0.85F, 1.70F, 2.55F, OGRID_LOG_ODDS_MAX};
    ogrid_real_t ps[6] = {0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F};
    for (size_t i = 0U; i < 6U; i++)
    {
        (void)ogrid_log_odds_to_prob(ls[i], &ps[i]);
    }
    TEST_ASSERT_TRUE_MESSAGE((ps[0] < ps[1]) && (ps[1] < ps[2]) && (ps[2] < ps[3]) &&
                                 (ps[3] < ps[4]) && (ps[4] < ps[5]),
                             "[TC-F002-12][SYS-PR-08] p must increase along the reference chain");
}

/** @req SYS-PR-08 @contract post POST-1 simetria p(l) + p(-l) = 1 para l en [0, 0,8]; tolerancia
 * 2e-6 (suma de dos errores de 1e-6) */
void test_TC_F002_13_symmetry(void)
{
    const ogrid_real_t ls[] = {0.0F, 0.1F, 0.4F, 0.8F};
    uint32_t bad = 0U;
    for (size_t i = 0U; i < 4U; i++)
    {
        ogrid_real_t pp = 0.0F;
        ogrid_real_t pn = 0.0F;
        (void)ogrid_log_odds_to_prob(ls[i], &pp);
        (void)ogrid_log_odds_to_prob(-ls[i], &pn);
        if (fabsf((pp + pn) - 1.0F) > 2.0e-6F)
        {
            bad++;
        }
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, bad, "[TC-F002-13][SYS-PR-08] p(l) + p(-l) must equal 1");
}

/** @req SYS-PR-08 @contract err OGRID_ERR_RANGE justo por debajo del minimo (un ulp mas negativo)
 */
void test_TC_F002_14_just_below_min_is_range_error(void)
{
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_RANGE, ogrid_log_odds_to_prob(next_bits(OGRID_LOG_ODDS_MIN), &s_out),
        "[TC-F002-14][SYS-PR-08] one ulp below MIN must return OGRID_ERR_RANGE");
}

/** @req SYS-PR-08 @contract err OGRID_ERR_RANGE justo por encima del maximo (un ulp mayor) */
void test_TC_F002_15_just_above_max_is_range_error(void)
{
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        OGRID_ERR_RANGE, ogrid_log_odds_to_prob(next_bits(OGRID_LOG_ODDS_MAX), &s_out),
        "[TC-F002-15][SYS-PR-08] one ulp above MAX must return OGRID_ERR_RANGE");
}

/** @req SYS-PR-08 @contract err OGRID_ERR_RANGE claramente fuera (-1,0 y 4,0) */
void test_TC_F002_16_clearly_outside_is_range_error(void)
{
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_RANGE, ogrid_log_odds_to_prob(-1.0F, &s_out),
                                  "[TC-F002-16][SYS-PR-08] -1.0 must return OGRID_ERR_RANGE");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_RANGE, ogrid_log_odds_to_prob(4.0F, &s_out),
                                  "[TC-F002-16][SYS-PR-08] 4.0 must return OGRID_ERR_RANGE");
}

/** @req SYS-PR-08 @contract err OGRID_ERR_RANGE para NaN (PRE-2) */
void test_TC_F002_17_nan_is_range_error(void)
{
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_RANGE, ogrid_log_odds_to_prob(NAN, &s_out),
                                  "[TC-F002-17][SYS-PR-08] NaN must return OGRID_ERR_RANGE");
}

/** @req SYS-PR-08 @contract err OGRID_ERR_RANGE para +infinito y -infinito (PRE-2) */
void test_TC_F002_18_infinity_is_range_error(void)
{
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_RANGE, ogrid_log_odds_to_prob(INFINITY, &s_out),
                                  "[TC-F002-18][SYS-PR-08] +inf must return OGRID_ERR_RANGE");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_RANGE, ogrid_log_odds_to_prob(-INFINITY, &s_out),
                                  "[TC-F002-18][SYS-PR-08] -inf must return OGRID_ERR_RANGE");
}

/** @req SYS-PR-08 @contract err OGRID_ERR_RANGE para magnitudes extremas finitas (FLT_MAX) */
void test_TC_F002_19_float_extremes_are_range_error(void)
{
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_RANGE, ogrid_log_odds_to_prob(FLT_MAX, &s_out),
                                  "[TC-F002-19][SYS-PR-08] FLT_MAX must return OGRID_ERR_RANGE");
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_RANGE, ogrid_log_odds_to_prob(-FLT_MAX, &s_out),
                                  "[TC-F002-19][SYS-PR-08] -FLT_MAX must return OGRID_ERR_RANGE");
}

/** @req SYS-PR-08 @contract post POST-5 bajo MIN: la salida no se modifica */
void test_TC_F002_20_below_min_keeps_output(void)
{
    (void)ogrid_log_odds_to_prob(next_bits(OGRID_LOG_ODDS_MIN), &s_out);
    TEST_ASSERT_TRUE_MESSAGE(out_is_sentinel(),
                             "[TC-F002-20][SYS-PR-08] output must be untouched below MIN");
}

/** @req SYS-PR-08 @contract post POST-5 sobre MAX: la salida no se modifica */
void test_TC_F002_21_above_max_keeps_output(void)
{
    (void)ogrid_log_odds_to_prob(next_bits(OGRID_LOG_ODDS_MAX), &s_out);
    TEST_ASSERT_TRUE_MESSAGE(out_is_sentinel(),
                             "[TC-F002-21][SYS-PR-08] output must be untouched above MAX");
}

/** @req SYS-PR-08 @contract post POST-5 NaN: la salida no se modifica */
void test_TC_F002_22_nan_keeps_output(void)
{
    (void)ogrid_log_odds_to_prob(NAN, &s_out);
    TEST_ASSERT_TRUE_MESSAGE(out_is_sentinel(),
                             "[TC-F002-22][SYS-PR-08] output must be untouched for NaN");
}

/** @req SYS-PR-08 @contract post POST-5 infinitos: la salida no se modifica */
void test_TC_F002_23_infinity_keeps_output(void)
{
    (void)ogrid_log_odds_to_prob(INFINITY, &s_out);
    TEST_ASSERT_TRUE_MESSAGE(out_is_sentinel(),
                             "[TC-F002-23][SYS-PR-08] output must be untouched for +inf");
    (void)ogrid_log_odds_to_prob(-INFINITY, &s_out);
    TEST_ASSERT_TRUE_MESSAGE(out_is_sentinel(),
                             "[TC-F002-23][SYS-PR-08] output must be untouched for -inf");
}

/** @req SYS-PR-08 @contract err OGRID_ERR_PARAM cuando prob_out es NULL (PRE-1), l valido */
void test_TC_F002_24_null_out_is_param_error(void)
{
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_PARAM, ogrid_log_odds_to_prob(0.85F, NULL),
                                  "[TC-F002-24][SYS-PR-08] NULL prob_out must return ERR_PARAM");
}

/** @req SYS-PR-08 @contract err NULL no devuelve OK ni RANGE y no toca datos ajenos (POST-5) */
void test_TC_F002_25_null_out_status_and_no_side_effect(void)
{
    ogrid_status_t st = ogrid_log_odds_to_prob(0.0F, NULL);
    TEST_ASSERT_NOT_EQUAL_MESSAGE(OGRID_OK, st,
                                  "[TC-F002-25][SYS-PR-08] NULL must not return OGRID_OK");
    TEST_ASSERT_NOT_EQUAL_MESSAGE(OGRID_ERR_RANGE, st,
                                  "[TC-F002-25][SYS-PR-08] NULL must not return OGRID_ERR_RANGE");
    TEST_ASSERT_TRUE_MESSAGE(out_is_sentinel(),
                             "[TC-F002-25][SYS-PR-08] unrelated output must stay untouched");
}

/** @req SYS-PR-08 @contract post POST-5 tras un error previo, una llamada valida escribe la salida
 * (sin estado residual) */
void test_TC_F002_26_valid_call_after_error_works(void)
{
    (void)ogrid_log_odds_to_prob(NAN, &s_out);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_log_odds_to_prob(0.0F, &s_out),
                                  "[TC-F002-26][SYS-PR-08] valid call after error must be OK");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(TOL_CONTRACT, 0.5F, s_out,
                                     "[TC-F002-26][SYS-PR-08] valid call after error gives 0.5");
}

/** @req SYS-PR-08 @contract perf funcion pura sin estado: misma entrada, mismo resultado exacto,
 * con otra llamada intercalada */
void test_TC_F002_27_pure_and_deterministic(void)
{
    ogrid_real_t a = 0.0F;
    ogrid_real_t b = 0.0F;
    ogrid_real_t other = 0.0F;
    (void)ogrid_log_odds_to_prob(1.70F, &a);
    (void)ogrid_log_odds_to_prob(3.0F, &other);
    (void)ogrid_log_odds_to_prob(1.70F, &b);
    TEST_ASSERT_TRUE_MESSAGE(memcmp(&a, &b, sizeof(a)) == 0,
                             "[TC-F002-27][SYS-PR-08] same input must give identical output");
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_TC_F002_01_valid_input_returns_ok);
    RUN_TEST(test_TC_F002_02_zero_gives_half);
    RUN_TEST(test_TC_F002_03_ref_echo_vote);
    RUN_TEST(test_TC_F002_04_ref_two_echoes);
    RUN_TEST(test_TC_F002_05_ref_three_echoes);
    RUN_TEST(test_TC_F002_06_ref_upper_limit);
    RUN_TEST(test_TC_F002_07_ref_lower_limit);
    RUN_TEST(test_TC_F002_08_contract_quoted_values_rounded);
    RUN_TEST(test_TC_F002_09_formula_interior_points);
    RUN_TEST(test_TC_F002_10_sweep_bounded_open_interval);
    RUN_TEST(test_TC_F002_11_sweep_strictly_increasing);
    RUN_TEST(test_TC_F002_12_monotonic_reference_chain);
    RUN_TEST(test_TC_F002_13_symmetry);
    RUN_TEST(test_TC_F002_14_just_below_min_is_range_error);
    RUN_TEST(test_TC_F002_15_just_above_max_is_range_error);
    RUN_TEST(test_TC_F002_16_clearly_outside_is_range_error);
    RUN_TEST(test_TC_F002_17_nan_is_range_error);
    RUN_TEST(test_TC_F002_18_infinity_is_range_error);
    RUN_TEST(test_TC_F002_19_float_extremes_are_range_error);
    RUN_TEST(test_TC_F002_20_below_min_keeps_output);
    RUN_TEST(test_TC_F002_21_above_max_keeps_output);
    RUN_TEST(test_TC_F002_22_nan_keeps_output);
    RUN_TEST(test_TC_F002_23_infinity_keeps_output);
    RUN_TEST(test_TC_F002_24_null_out_is_param_error);
    RUN_TEST(test_TC_F002_25_null_out_status_and_no_side_effect);
    RUN_TEST(test_TC_F002_26_valid_call_after_error_works);
    RUN_TEST(test_TC_F002_27_pure_and_deterministic);
    return UNITY_END();
}
