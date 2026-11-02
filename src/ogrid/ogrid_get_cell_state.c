/**
 * @file ogrid_get_cell_state.c
 * @brief Derive the state of a grid cell: occupied, low probability or unsurveyed.
 * @details A cell never observed is unsurveyed whatever its log-odds. An observed cell is
 * occupied when its probability is strictly above OGRID_OCCUPIED_PROB_THRESHOLD and low
 * probability otherwise. There is no free state (SYS-SAF-07). Corrupt cells are reported with
 * OGRID_ERR_STATE and the safe state unsurveyed. Constant time.
 * @ingroup ogrid
 */
#include "ogrid/ogrid_get_cell_state.h"

#include "ogrid/ogrid_log_odds_to_prob.h"

#include <stddef.h>

/**
 * @brief Derive the state of an observed cell from its stored log-odds.
 * @param[in] log_odds_lo Stored log-odds of the cell.
 * @param[out] state_out Derived state; must not be NULL.
 * @return OGRID_OK when the state was derived, OGRID_ERR_STATE when the log-odds is corrupt.
 */
static ogrid_status_t derive_observed_state(ogrid_real_t log_odds_lo, ogrid_cell_state_t *state_out)
{
    ogrid_real_t prob = 0.0F;
    ogrid_status_t status = OGRID_OK;

    if (ogrid_log_odds_to_prob(log_odds_lo, &prob) != OGRID_OK)
    {
        /* Safe state on corrupt data: never free nor low probability. */
        status = OGRID_ERR_STATE;
        *state_out = OGRID_CELL_UNSURVEYED;
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
    else if (grid->cells[cell_idx].observed == OGRID_OBSERVED_NO)
    {
        *state_out = OGRID_CELL_UNSURVEYED;
    }
    else if (grid->cells[cell_idx].observed == OGRID_OBSERVED_YES)
    {
        status = derive_observed_state(grid->cells[cell_idx].log_odds_lo, state_out);
    }
    else
    {
        /* Observed flag neither 0 nor 1: corrupt cell. */
        status = OGRID_ERR_STATE;
        *state_out = OGRID_CELL_UNSURVEYED;
    }

    return status;
}
