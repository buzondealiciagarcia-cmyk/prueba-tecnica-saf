/**
 * @file ogrid_reset_grid.h
 * @brief Reset of the whole occupancy grid to "unsurveyed".
 * @details Used at start-up and when returning from FALLO, where earlier measurements are
 *          no longer trustworthy (diseno.md 6.3).
 * @ingroup ogrid
 */
#ifndef OGRID_OGRID_RESET_GRID_H
#define OGRID_OGRID_RESET_GRID_H

#include "ogrid/ogrid.h"

/**
 * @brief Set every cell to OGRID_LOG_ODDS_INIT and "not observed".
 * @param[out] grid_out Grid to reset; must not be NULL. Previous content is not read.
 * @return OGRID_OK when all OGRID_CELL_COUNT cells were reset.
 * @return OGRID_ERR_PARAM when grid_out is NULL; nothing is modified.
 * @pre grid_out != NULL.
 * @post On OGRID_OK every cell has log_odds_lo == OGRID_LOG_ODDS_INIT and observed ==
 * OGRID_OBSERVED_NO, so its derived state is OGRID_CELL_UNSURVEYED; calling it twice gives the same
 * result.
 * @req SYS-PR-08, SYS-SAF-07, SYS-IMP-01
 * @contract F-001
 */
ogrid_status_t ogrid_reset_grid(ogrid_t *grid_out);

#endif /* OGRID_OGRID_RESET_GRID_H */
