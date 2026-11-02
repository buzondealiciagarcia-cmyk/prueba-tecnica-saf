/**
 * @file ogrid_log_odds_to_prob.h
 * @brief Convert log-odds to occupancy probability p = 1 / (1 + e^-l).
 * @details Only conversion of the module; the cell state and the occupied mask compare this
 * probability with the threshold. The probability is internal and never published.
 * @ingroup ogrid
 */
#ifndef OGRID_OGRID_LOG_ODDS_TO_PROB_H
#define OGRID_OGRID_LOG_ODDS_TO_PROB_H

#include "ogrid/ogrid.h"

/**
 * @brief Convert log-odds to occupancy probability p = 1 / (1 + e^-l).
 * @param[in] log_odds_lo Log-odds in natural logarithm; finite, within [OGRID_LOG_ODDS_MIN,
 * OGRID_LOG_ODDS_MAX].
 * @param[out] prob_out Probability, dimensionless, in (0, 1); must not be NULL.
 * @return OGRID_OK when the probability was written.
 * @return OGRID_ERR_PARAM when prob_out is NULL.
 * @return OGRID_ERR_RANGE when log_odds_lo is NaN, infinite or outside the limits.
 * @pre prob_out != NULL and log_odds_lo finite within the limits.
 * @post On OGRID_OK *prob_out equals 1 / (1 + e^-l) within 1e-6 (contract POST-1) and l = 0 gives
 * exactly 0.5 (POST-4); on error *prob_out is not modified (POST-5).
 * @req SYS-PR-08
 * @contract F-002
 */
ogrid_status_t ogrid_log_odds_to_prob(ogrid_real_t log_odds_lo, ogrid_real_t *prob_out);

#endif /* OGRID_OGRID_LOG_ODDS_TO_PROB_H */
