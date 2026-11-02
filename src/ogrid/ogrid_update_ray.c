/**
 * @file ogrid_update_ray.c
 * @brief Apply one radar beam to the cells it crosses, nearest first.
 * @details Validates the whole beam before touching the grid, then votes pass on the cells before
 * the echo, echo on the echo cell and leaves the cells behind it unchanged (occluded).
 * @ingroup ogrid
 */
#include "ogrid/ogrid_update_ray.h"

#include "ogrid/ogrid_update_cell.h"

#include <stddef.h>

/**
 * @brief Check the range of the beam parameters and of every cell index.
 * @param[in] cells_in Cell indices; valid for cell_count entries.
 * @param[in] cell_count Number of cells.
 * @param[in] has_echo True when the beam has an echo.
 * @param[in] echo_pos Position of the echo in cells_in.
 * @return OGRID_OK when all values are in range, OGRID_ERR_RANGE otherwise.
 */
static ogrid_status_t validate_ray(const uint32_t *cells_in, uint32_t cell_count, bool has_echo,
                                   uint32_t echo_pos)
{
    ogrid_status_t status = OGRID_OK;

    if ((cell_count > OGRID_CELL_COUNT) || (has_echo && (echo_pos >= cell_count)))
    {
        status = OGRID_ERR_RANGE;
    }
    else
    {
        for (uint32_t i = 0U; i < cell_count; i++)
        {
            if (cells_in[i] >= OGRID_CELL_COUNT)
            {
                status = OGRID_ERR_RANGE;
                break;
            }
        }
    }

    return status;
}

/**
 * @brief Vote on the validated cells of the beam, stopping at the first error.
 * @param[in,out] grid_io Grid to update.
 * @param[in] cells_in Validated cell indices.
 * @param[in] cell_count Number of cells to process (cells behind the echo excluded).
 * @param[in] echo_idx Position that gets the echo vote, or cell_count when there is none.
 * @return Status of the first failing ogrid_update_cell call, otherwise OGRID_OK.
 */
static ogrid_status_t apply_votes(ogrid_t *grid_io, const uint32_t *cells_in, uint32_t cell_count,
                                  uint32_t echo_idx)
{
    ogrid_status_t status = OGRID_OK;

    for (uint32_t i = 0U; (i < cell_count) && (status == OGRID_OK); i++)
    {
        const ogrid_vote_kind_t vote =
            (i == echo_idx) ? OGRID_VOTE_KIND_ECHO : OGRID_VOTE_KIND_PASS;
        status = ogrid_update_cell(grid_io, cells_in[i], vote);
    }

    return status;
}

ogrid_status_t ogrid_update_ray(ogrid_t *grid_io, const uint32_t *cells_in, uint32_t cell_count,
                                bool has_echo, uint32_t echo_pos)
{
    ogrid_status_t status = OGRID_OK;

    if ((grid_io == NULL) || ((cells_in == NULL) && (cell_count > 0U)))
    {
        status = OGRID_ERR_PARAM;
    }
    else
    {
        status = validate_ray(cells_in, cell_count, has_echo, echo_pos);
    }

    if (status == OGRID_OK)
    {
        /* Cells behind the echo are occluded, so processing ends at the echo cell. */
        const uint32_t process_count = has_echo ? (echo_pos + 1U) : cell_count;
        const uint32_t echo_idx = has_echo ? echo_pos : cell_count;

        status = apply_votes(grid_io, cells_in, process_count, echo_idx);
    }

    return status;
}
