/**
 * @file ogrid_types.h
 * @brief Basic types of the occupancy grid.
 * @details Fixes the numeric representation of the log-odds (C-TYP-02): IEEE-754 binary32,
 *          asserted in ogrid_platform.h. This is an implementation decision recorded in
 *          docs/03-implementacion/implementacion.md because design v1.0 left it open.
 * @ingroup ogrid
 */
#ifndef OGRID_OGRID_TYPES_H
#define OGRID_OGRID_TYPES_H

#include <stdbool.h>
#include <stdint.h>

#include "ogrid/ogrid_limits.h"
#include "ogrid/ogrid_platform.h"

/** Real number used for log-odds and probabilities (float32). */
typedef float ogrid_real_t;

/**
 * @brief State derived from a cell; there is deliberately no "free" value (SYS-SAF-07).
 */
typedef enum
{
    OGRID_CELL_UNSURVEYED = 0, /**< Never observed or reset (zona no estudiada). */
    OGRID_CELL_LOW_PROB,       /**< Observed and p <= threshold (probabilidad baja de obstáculo). */
    OGRID_CELL_OCCUPIED,       /**< p > threshold (ocupado). */
    OGRID_CELL_STATE_COUNT
} ogrid_cell_state_t;

/**
 * @brief What one radar beam says about one cell (diseno.md 6.2).
 */
typedef enum
{
    OGRID_VOTE_KIND_ECHO = 0, /**< Echo in the cell. */
    OGRID_VOTE_KIND_PASS,     /**< Wave crossed the cell without echo. */
    OGRID_VOTE_KIND_OCCLUDED, /**< Cell behind an echo; value unchanged. */
    OGRID_VOTE_KIND_COUNT
} ogrid_vote_kind_t;

/**
 * @brief One grid cell.
 */
typedef struct
{
    ogrid_real_t log_odds_lo; /**< Log-odds in natural logarithm, within the configured clamp. */
    bool observed;            /**< False until the first vote; false again after a reset. */
} ogrid_cell_t;

/**
 * @brief The whole grid, fixed size, row-major: index = s_index * OGRID_WIDTH_CELLS + d_index.
 */
typedef struct
{
    ogrid_cell_t cells[OGRID_CELL_COUNT]; /**< Cells along s (0..879) by across d (0..3). */
} ogrid_t;

#endif /* OGRID_OGRID_TYPES_H */
