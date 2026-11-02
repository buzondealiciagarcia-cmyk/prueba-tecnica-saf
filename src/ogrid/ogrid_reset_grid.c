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

ogrid_status_t ogrid_reset_grid(ogrid_t *grid_out)
{
    ogrid_status_t status = OGRID_ERR_PARAM;

    if (grid_out != NULL)
    {
        for (uint32_t i = 0U; i < OGRID_CELL_COUNT; i++)
        {
            grid_out->cells[i].log_odds_lo = OGRID_LOG_ODDS_INIT;
            grid_out->cells[i].observed = false;
        }
        status = OGRID_OK;
    }

    return status;
}
