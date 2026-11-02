/**
 * @file ogrid_clamp_log_odds.c
 * @brief Limit a log-odds value to [OGRID_LOG_ODDS_MIN, OGRID_LOG_ODDS_MAX].
 * @details Keeps every cell value inside the configured bounds so a saturated cell leaves the
 * occupied state quickly and three echoes always confirm an obstacle. Constant time.
 * @ingroup ogrid
 */
#include "ogrid/ogrid_clamp_log_odds.h"

#include <float.h>
#include <stddef.h>

ogrid_status_t ogrid_clamp_log_odds(ogrid_real_t value_lo, ogrid_real_t *clamped_lo_out)
{
    ogrid_status_t status = OGRID_OK;

    if (clamped_lo_out == NULL)
    {
        status = OGRID_ERR_PARAM;
    }
    else if (!((value_lo >= -FLT_MAX) && (value_lo <= FLT_MAX)))
    {
        /* Negated form also rejects NaN, which fails every ordered comparison. */
        status = OGRID_ERR_RANGE;
    }
    else if (value_lo < OGRID_LOG_ODDS_MIN)
    {
        *clamped_lo_out = OGRID_LOG_ODDS_MIN;
    }
    else if (value_lo > OGRID_LOG_ODDS_MAX)
    {
        *clamped_lo_out = OGRID_LOG_ODDS_MAX;
    }
    else
    {
        *clamped_lo_out = value_lo;
    }

    return status;
}
