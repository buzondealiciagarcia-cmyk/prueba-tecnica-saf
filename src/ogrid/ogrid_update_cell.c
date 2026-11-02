/**
 * @file ogrid_update_cell.c
 * @brief Apply the vote of one radar beam to one cell of the occupancy grid.
 * @details Echo adds OGRID_VOTE_ECHO, pass without echo adds OGRID_VOTE_PASS and an occluded cell
 * is left untouched. The result is limited with ogrid_clamp_log_odds so that one isolated echo
 * never confirms an obstacle and three consecutive ones always do. Constant time.
 * @ingroup ogrid
 */
#include "ogrid/ogrid_update_cell.h"

#include "ogrid/ogrid_clamp_log_odds.h"

#include <stddef.h>

/**
 * @brief Check that a stored log-odds is a finite value inside the clamp limits.
 * @param[in] value_lo Stored log-odds.
 * @return true when value_lo lies in [OGRID_LOG_ODDS_MIN, OGRID_LOG_ODDS_MAX].
 */
static bool is_stored_value_valid(ogrid_real_t value_lo)
{
    bool is_valid = false;

    /* Positive form of the comparison is false for NaN, so NaN is rejected too. */
    if ((value_lo >= OGRID_LOG_ODDS_MIN) && (value_lo <= OGRID_LOG_ODDS_MAX))
    {
        is_valid = true;
    }

    return is_valid;
}

/**
 * @brief Compute and store the new value of a cell for an echo or pass vote.
 * @param[in,out] cell_io Cell to update; must not be NULL.
 * @param[in] delta_lo Log-odds increment of the vote.
 * @return OGRID_OK, or OGRID_ERR_STATE when the stored value is corrupt (cell unchanged).
 */
static ogrid_status_t apply_delta(ogrid_cell_t *cell_io, ogrid_real_t delta_lo)
{
    ogrid_status_t status = OGRID_OK;
    ogrid_real_t new_lo = 0.0F;

    if (!is_stored_value_valid(cell_io->log_odds_lo) ||
        (ogrid_clamp_log_odds(cell_io->log_odds_lo + delta_lo, &new_lo) != OGRID_OK))
    {
        /* The clamp cannot fail for a validated value; kept as a side-effect free path. */
        status = OGRID_ERR_STATE;
    }
    else
    {
        cell_io->log_odds_lo = new_lo;
        cell_io->observed = true;
    }

    return status;
}

ogrid_status_t ogrid_update_cell(ogrid_t *grid_io, uint32_t cell_idx, ogrid_vote_kind_t vote)
{
    ogrid_status_t status = OGRID_OK;

    if ((grid_io == NULL) || ((vote != OGRID_VOTE_KIND_ECHO) && (vote != OGRID_VOTE_KIND_PASS) &&
                              (vote != OGRID_VOTE_KIND_OCCLUDED)))
    {
        status = OGRID_ERR_PARAM;
    }
    else if (cell_idx >= OGRID_CELL_COUNT)
    {
        status = OGRID_ERR_RANGE;
    }
    else if (!is_stored_value_valid(grid_io->cells[cell_idx].log_odds_lo))
    {
        /* Corrupt cells are reported even for occluded votes (PRE-4). */
        status = OGRID_ERR_STATE;
    }
    else if (vote != OGRID_VOTE_KIND_OCCLUDED)
    {
        const ogrid_real_t delta_lo =
            (vote == OGRID_VOTE_KIND_ECHO) ? OGRID_VOTE_ECHO : OGRID_VOTE_PASS;

        status = apply_delta(&grid_io->cells[cell_idx], delta_lo);
    }
    else
    {
        /* Occluded: the beam did not observe the cell, nothing changes. */
    }

    return status;
}
