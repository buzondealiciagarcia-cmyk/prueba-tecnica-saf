/**
 * @file test_ogrid_clamp_prob.c
 * @brief Unit tests for contract F-001 (ogrid_clamp_prob).
 */
#include "ogrid/ogrid.h"
#include "unity.h"
#include <math.h>

void setUp(void)
{
}

void tearDown(void)
{
}

/** @req REQ-FUN-002 @contract post */
void test_TC_F001_01_inside_range_unchanged(void)
{
    float out = -1.0F;
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_OK, ogrid_clamp_prob(0.25F, &out),
                                  "[TC-F001-01][REQ-FUN-002] status OK");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(1e-6F, 0.25F, out,
                                     "[TC-F001-01][REQ-FUN-002] value unchanged");
}

/** @req REQ-FUN-002 @contract post */
void test_TC_F001_02_above_one_clamped(void)
{
    float out = -1.0F;
    (void)ogrid_clamp_prob(1.5F, &out);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(1e-6F, 1.0F, out, "[TC-F001-02][REQ-FUN-002] p>1 clamps to 1");
}

/** @req REQ-FUN-002 @contract post */
void test_TC_F001_03_below_zero_clamped(void)
{
    float out = -1.0F;
    (void)ogrid_clamp_prob(-0.5F, &out);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(1e-6F, 0.0F, out, "[TC-F001-03][REQ-FUN-002] p<0 clamps to 0");
}

/** @req REQ-FUN-002 @contract err */
void test_TC_F001_04_null_output(void)
{
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_PARAM, ogrid_clamp_prob(0.5F, NULL),
                                  "[TC-F001-04][REQ-FUN-002] NULL rejected");
}

/** @req REQ-FUN-002 @contract err */
void test_TC_F001_05_nan_rejected(void)
{
    float out = 0.3F;
    TEST_ASSERT_EQUAL_INT_MESSAGE(OGRID_ERR_RANGE, ogrid_clamp_prob(NAN, &out),
                                  "[TC-F001-05][REQ-FUN-002] NaN rejected");
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_TC_F001_01_inside_range_unchanged);
    RUN_TEST(test_TC_F001_02_above_one_clamped);
    RUN_TEST(test_TC_F001_03_below_zero_clamped);
    RUN_TEST(test_TC_F001_04_null_output);
    RUN_TEST(test_TC_F001_05_nan_rejected);
    return UNITY_END();
}
