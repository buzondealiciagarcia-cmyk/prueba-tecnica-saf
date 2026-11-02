/**
 * @file ogrid_reset_grid.c
 * @brief Reset of the whole occupancy grid to "unsurveyed".
 * @details Sets every cell to log-odds OGRID_LOG_ODDS_INIT and not observed, so that no cell
 *          can be reported as occupied without evidence (SYS-SAF-07). Used at start-up and when
 *          returning from FALLO.
 * @ingroup ogrid
 */
#include "ogrid/ogrid_reset_grid.h"

#include <stddef.h>
#include <stdint.h>

ogrid_status_t ogrid_reset_grid(ogrid_t *grid_out)
{
    ogrid_status_t status = OGRID_ERR_PARAM;

    if (grid_out != NULL)
    {
        /* Fields are written one by one instead of using memset: the initial value comes from
         * OGRID_LOG_ODDS_INIT, and 0.0F is not guaranteed to be an all-zero bit pattern. The
         * previous content of the grid is never read, so no stale data can leak into the result. */
        for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
        {
            grid_out->cells[i].log_odds_lo = OGRID_LOG_ODDS_INIT;
            grid_out->cells[i].observed = OGRID_OBSERVED_NO;
        }
        status = OGRID_OK;
    }

    return status;
}
