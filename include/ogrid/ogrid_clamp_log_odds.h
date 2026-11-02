/**
 * @file ogrid_clamp_log_odds.h
 * @brief Limit a log-odds value to [OGRID_LOG_ODDS_MIN, OGRID_LOG_ODDS_MAX].
 * @details The upper limit keeps a saturated cell from staying occupied for long; the lower limit
 * guarantees that three echoes confirm an obstacle.
 * @ingroup ogrid
 */
#ifndef OGRID_OGRID_CLAMP_LOG_ODDS_H
#define OGRID_OGRID_CLAMP_LOG_ODDS_H

#include "ogrid/ogrid.h"

/**
 * @brief Limit a log-odds value to [OGRID_LOG_ODDS_MIN, OGRID_LOG_ODDS_MAX].
 * @param[in] value_lo Log-odds (natural logarithm) to limit; any finite value is admissible.
 * @param[out] clamped_lo_out Limited value; must not be NULL.
 * @return OGRID_OK when the value was written.
 * @return OGRID_ERR_PARAM when clamped_lo_out is NULL; checked first, so it wins over a non-finite
 * value_lo.
 * @return OGRID_ERR_RANGE when value_lo is NaN or infinite.
 * @pre clamped_lo_out != NULL and value_lo finite.
 * @post On OGRID_OK the result equals value_lo inside the limits, otherwise the nearest limit, and
 * clamping twice gives the same value; on error *clamped_lo_out is not modified.
 * @req SYS-PR-08, SYS-SAF-07
 * @contract F-003
 * @note Constant time, two comparisons (contract perf); safe to call inside the 50 ms cycle.
 */
ogrid_status_t ogrid_clamp_log_odds(ogrid_real_t value_lo, ogrid_real_t *clamped_lo_out);

#endif /* OGRID_OGRID_CLAMP_LOG_ODDS_H */
