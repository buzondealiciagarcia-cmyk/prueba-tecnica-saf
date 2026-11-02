/**
 * @file ogrid_get_cell_state.c
 * @brief Derive the state of a cell: occupied, low probability or unsurveyed.
 * @details There is no free state: absence of echo does not prove absence of obstacle
 * (SYS-SAF-07). Corrupt log-odds of an observed cell yield OGRID_ERR_STATE and the unsurveyed
 * state, never a state that could be read as safe.
 * @ingroup ogrid
 */
#include "ogrid/ogrid_get_cell_state.h"

#include <stddef.h>

#include "ogrid/ogrid_log_odds_to_prob.h"

/**
 * @brief Derive the state of an observed cell from its log-odds.
 * @param[in] log_odds_lo Stored log-odds of the cell.
 * @param[out] state_out Derived state; written in every case. Must not be NULL.
 * @return OGRID_OK when the state is occupied or low probability, OGRID_ERR_STATE otherwise.
 */
static ogrid_status_t derive_observed_state(ogrid_real_t log_odds_lo, ogrid_cell_state_t *state_out)
{
    ogrid_real_t prob = 0.0F;
    ogrid_status_t status = OGRID_OK;

    /* Any failure of the conversion (NaN, infinity, out of clamp) is treated as corrupt data. */
    if (ogrid_log_odds_to_prob(log_odds_lo, &prob) != OGRID_OK)
    {
        *state_out = OGRID_CELL_UNSURVEYED;
        status = OGRID_ERR_STATE;
    }
    else if (prob > OGRID_OCCUPIED_PROB_THRESHOLD)
    {
        *state_out = OGRID_CELL_OCCUPIED;
    }
    else
    {
        *state_out = OGRID_CELL_LOW_PROB;
    }

    return status;
}

ogrid_status_t ogrid_get_cell_state(const ogrid_t *grid, uint32_t cell_idx,
                                    ogrid_cell_state_t *state_out)
{
    ogrid_status_t status = OGRID_OK;

    if ((grid == NULL) || (state_out == NULL))
    {
        status = OGRID_ERR_PARAM;
    }
    else if (cell_idx >= OGRID_CELL_COUNT)
    {
        status = OGRID_ERR_RANGE;
    }
    else if (!grid->cells[cell_idx].observed)
    {
        *state_out = OGRID_CELL_UNSURVEYED;
    }
    else
    {
        status = derive_observed_state(grid->cells[cell_idx].log_odds_lo, state_out);
    }

    return status;
}
