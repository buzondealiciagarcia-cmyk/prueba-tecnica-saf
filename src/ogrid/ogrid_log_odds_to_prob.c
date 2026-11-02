/**
 * @file ogrid_log_odds_to_prob.c
 * @brief Convert log-odds to occupancy probability p = 1 / (1 + e^-l).
 * @details Only conversion of the module; the cell state and the occupied mask compare this
 * probability with the threshold. The probability is internal and never published.
 * @ingroup ogrid
 */
#include "ogrid/ogrid_log_odds_to_prob.h"

#include <math.h>
#include <stddef.h>

/** Neutral element of the denominator 1 + e^-l. */
#define OGRID_PROB_ONE (1.0F)

ogrid_status_t ogrid_log_odds_to_prob(ogrid_real_t log_odds_lo, ogrid_real_t *prob_out)
{
    ogrid_status_t status = OGRID_OK;

    if (prob_out == NULL)
    {
        status = OGRID_ERR_PARAM;
    }
    else if (!((log_odds_lo >= OGRID_LOG_ODDS_MIN) && (log_odds_lo <= OGRID_LOG_ODDS_MAX)))
    {
        /* Negated form also rejects NaN, which fails every ordered comparison. */
        status = OGRID_ERR_RANGE;
    }
    else
    {
        /* Domain is bounded above, so expf cannot overflow. */
        *prob_out = OGRID_PROB_ONE / (OGRID_PROB_ONE + expf(-log_odds_lo));
    }

    return status;
}
