/**
 * @file ogrid_update_ray.h
 * @brief Apply one radar beam to the cells it crosses, nearest first.
 * @details Cells before the echo get the pass vote, the echo cell gets the echo vote and cells
 * behind it are occluded and unchanged. Which cells a beam crosses is computed by the caller.
 * @ingroup ogrid
 */
#ifndef OGRID_OGRID_UPDATE_RAY_H
#define OGRID_OGRID_UPDATE_RAY_H

#include "ogrid/ogrid.h"

/**
 * @brief Apply one radar beam to the cells it crosses, nearest first.
 * @param[in,out] grid_io Grid to update; must not be NULL.
 * @param[in] cells_in Cell indices (each 0 to OGRID_CELL_COUNT - 1) crossed by the beam, nearest
 * first, no repeats; not NULL when cell_count > 0.
 * @param[in] cell_count Number of cells in cells_in, 0 to OGRID_CELL_COUNT; 0 without echo does
 * nothing, 0 with echo is OGRID_ERR_RANGE.
 * @param[in] has_echo True when the beam returned an echo among those cells.
 * @param[in] echo_pos Position (index into cells_in) of the echo cell, 0 to cell_count - 1; ignored
 * without echo.
 * @return OGRID_OK when the beam was applied.
 * @return OGRID_ERR_PARAM when grid_io is NULL, or cells_in is NULL with cell_count > 0; nothing is
 * modified.
 * @return OGRID_ERR_RANGE when cell_count, any index of cells_in (cells behind the echo included)
 * or echo_pos is out of range; nothing is modified.
 * @return OGRID_ERR_STATE when a cell up to the echo has invalid log-odds. Earlier cells may
 * already be updated; the corrupt cell and later ones are unchanged. Cells behind the echo are
 * never read. The caller must go to a safe state and not reuse the grid until ogrid_reset_grid
 * (C-ERR-02).
 * @pre grid_io != NULL; cells_in valid for cell_count entries, each < OGRID_CELL_COUNT; echo_pos <
 * cell_count when has_echo.
 * @post On OGRID_OK cells before the echo got the pass vote, the echo cell got the echo vote and
 * the rest are unchanged. Checks run in the order param, range, state and the first failure is
 * returned; on PARAM or RANGE errors the grid is not modified.
 * @req SYS-PR-08, SYS-SAF-07
 * @contract F-006
 */
ogrid_status_t ogrid_update_ray(ogrid_t *grid_io, const uint32_t *cells_in, uint32_t cell_count,
                                bool has_echo, uint32_t echo_pos);

#endif /* OGRID_OGRID_UPDATE_RAY_H */
