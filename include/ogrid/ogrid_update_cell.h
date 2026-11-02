/**
 * @file ogrid_update_cell.h
 * @brief Apply the vote of one radar beam to one cell.
 * @details Echo adds OGRID_VOTE_ECHO, pass without echo adds OGRID_VOTE_PASS, occluded leaves the
 * cell unchanged; the result is limited to the log-odds range.
 * @ingroup ogrid
 */
#ifndef OGRID_OGRID_UPDATE_CELL_H
#define OGRID_OGRID_UPDATE_CELL_H

#include "ogrid/ogrid.h"

/**
 * @brief Apply the vote of one radar beam to one cell.
 * @param[in,out] grid_io Grid to update; must not be NULL.
 * @param[in] cell_idx Cell index, 0 to OGRID_CELL_COUNT - 1 (s_index * OGRID_WIDTH_CELLS +
 * d_index).
 * @param[in] vote One of OGRID_VOTE_KIND_ECHO, OGRID_VOTE_KIND_PASS, OGRID_VOTE_KIND_OCCLUDED.
 * @return OGRID_OK when the cell was updated or intentionally left unchanged (occluded).
 * @return OGRID_ERR_PARAM when grid_io is NULL or vote is invalid.
 * @return OGRID_ERR_RANGE when cell_idx is out of range.
 * @return OGRID_ERR_STATE when the stored log-odds is corrupt (NaN, infinite or out of limits),
 * with any vote, occluded included. When several preconditions fail, the first in the order
 * param, range, state decides.
 * @pre grid_io != NULL, vote valid, cell_idx < OGRID_CELL_COUNT, stored log-odds finite and within
 * the limits.
 * @post On OGRID_OK the cell holds clamp(previous + vote) and is observed for echo and pass,
 * nothing else changes; on error the grid is not modified.
 * @req SYS-PR-08, SYS-SAF-07
 * @contract F-004
 * @note Constant time, no loops (contract perf); it is called many times per 50 ms cycle from
 * ogrid_update_ray.
 */
ogrid_status_t ogrid_update_cell(ogrid_t *grid_io, uint32_t cell_idx, ogrid_vote_kind_t vote);

#endif /* OGRID_OGRID_UPDATE_CELL_H */
