/**
 * @file ogrid_get_cell_state.h
 * @brief Derive the state of a cell: occupied, low probability or unsurveyed.
 * @details There is deliberately no free state: absence of echo does not prove absence of obstacle
 * (SYS-SAF-07).
 * @ingroup ogrid
 */
#ifndef OGRID_OGRID_GET_CELL_STATE_H
#define OGRID_OGRID_GET_CELL_STATE_H

#include "ogrid/ogrid.h"

/**
 * @brief Derive the state of a cell: occupied, low probability or unsurveyed.
 * @param[in] grid Grid to read; must not be NULL.
 * @param[in] cell_idx Cell index, 0 to OGRID_CELL_COUNT - 1.
 * @param[out] state_out Derived state; must not be NULL.
 * @return OGRID_OK when the state was written.
 * @return OGRID_ERR_PARAM when grid or state_out is NULL.
 * @return OGRID_ERR_RANGE when cell_idx is out of range.
 * @return OGRID_ERR_STATE when the cell is corrupt (observed neither OGRID_OBSERVED_NO nor
 * OGRID_OBSERVED_YES, or invalid log-odds on an observed cell); *state_out is then
 * OGRID_CELL_UNSURVEYED.
 * @pre grid != NULL, state_out != NULL, cell_idx < OGRID_CELL_COUNT.
 * @post On OGRID_OK the state is unsurveyed if not observed, occupied if p >
 * OGRID_OCCUPIED_PROB_THRESHOLD, otherwise low probability; the grid is not modified.
 * @req SYS-PR-08, SYS-SAF-07
 * @contract F-005
 */
ogrid_status_t ogrid_get_cell_state(const ogrid_t *grid, uint32_t cell_idx,
                                    ogrid_cell_state_t *state_out);

#endif /* OGRID_OGRID_GET_CELL_STATE_H */
