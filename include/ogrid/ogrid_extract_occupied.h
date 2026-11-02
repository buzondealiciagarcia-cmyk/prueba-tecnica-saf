/**
 * @file ogrid_extract_occupied.h
 * @brief Mark in a mask the cells that count as occupied (p > threshold).
 * @details Input of the grouping into objects. It uses the same rule as ogrid_get_cell_state, so
 * both outputs never disagree.
 * @ingroup ogrid
 */
#ifndef OGRID_OGRID_EXTRACT_OCCUPIED_H
#define OGRID_OGRID_EXTRACT_OCCUPIED_H

#include "ogrid/ogrid.h"

/**
 * @brief Mark in a mask the cells that count as occupied (p > threshold).
 * @param[in] grid Grid to read; must not be NULL.
 * @param[out] mask_out Array of OGRID_CELL_COUNT bool, true on occupied cells; must not be NULL.
 * @return OGRID_OK when the whole mask was written.
 * @return OGRID_ERR_PARAM when grid or mask_out is NULL.
 * @return OGRID_ERR_STATE when some cell is corrupt; the mask is complete with false on those
 * cells.
 * @pre grid != NULL and mask_out != NULL pointing to OGRID_CELL_COUNT bool.
 * @post On OGRID_OK mask_out[idx] is true exactly where ogrid_get_cell_state gives occupied, for
 * every idx. With OGRID_ERR_PARAM mask_out is not modified. The grid is never modified.
 * @req SYS-PR-04, SYS-PR-08
 * @contract F-007
 */
ogrid_status_t ogrid_extract_occupied(const ogrid_t *grid, bool *mask_out);

#endif /* OGRID_OGRID_EXTRACT_OCCUPIED_H */
