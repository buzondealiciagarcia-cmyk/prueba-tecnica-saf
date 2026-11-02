/**
 * @file ogrid_extract_occupied.c
 * @brief Mark in a mask the grid cells that count as occupied (p > threshold).
 * @details Every cell is classified through ogrid_get_cell_state, so the mask and the cell state
 * never disagree. Corrupt cells are false in the mask and make the function report
 * OGRID_ERR_STATE once the whole grid has been scanned. Linear in OGRID_CELL_COUNT.
 * @ingroup ogrid
 */
#include "ogrid/ogrid_extract_occupied.h"

#include "ogrid/ogrid_get_cell_state.h"

#include <stddef.h>

ogrid_status_t ogrid_extract_occupied(const ogrid_t *grid, bool *mask_out)
{
    ogrid_status_t status = OGRID_OK;

    if ((grid == NULL) || (mask_out == NULL))
    {
        status = OGRID_ERR_PARAM;
    }
    else
    {
        for (uint32_t idx = 0U; idx < OGRID_CELL_COUNT; idx++)
        {
            ogrid_cell_state_t state = OGRID_CELL_UNSURVEYED;

            /* Indices are always valid, so the only failure is a corrupt cell.
             * The contract requires a complete mask and a single OGRID_ERR_STATE at the end,
             * so the scan does not stop at the first corrupt cell: the error is recorded and
             * the mask entry is still written, remaining false for that cell (state keeps its
             * non-occupied initial value). */
            if (ogrid_get_cell_state(grid, idx, &state) != OGRID_OK)
            {
                status = OGRID_ERR_STATE;
            }

            mask_out[idx] = (state == OGRID_CELL_OCCUPIED);
        }
    }

    return status;
}
