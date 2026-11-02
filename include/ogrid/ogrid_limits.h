/**
 * @file ogrid_limits.h
 * @brief Compile-time limits and default model constants of the occupancy grid.
 * @details Values come from diseno.md sections 6 and 10 (design v1.0). The grid size is fixed
 *          at compile time because dynamic memory is forbidden (SYS-IMP-01).
 * @ingroup ogrid
 */
#ifndef OGRID_OGRID_LIMITS_H
#define OGRID_OGRID_LIMITS_H

#include <stdint.h>

/** Cell side length in metres (diseno.md 6.1). */
#define OGRID_CELL_SIZE_M (0.25F)
/** Cells along the track: 220 m / 0.25 m (SYS-DET-01). */
#define OGRID_LENGTH_CELLS (880U)
/** Cells across the track: 1 m gauge / 0.25 m (diseno.md 6.1). */
#define OGRID_WIDTH_CELLS (4U)
/** Total cells held in memory (SYS-IMP-01). */
#define OGRID_CELL_COUNT (OGRID_LENGTH_CELLS * OGRID_WIDTH_CELLS)

/** Initial log-odds of a cell (natural logarithm, diseno.md 6.2). */
#define OGRID_LOG_ODDS_INIT (0.0F)
/** Log-odds added when the beam returns an echo in the cell. */
#define OGRID_VOTE_ECHO (0.85F)
/** Log-odds added when the beam crosses the cell without echo. */
#define OGRID_VOTE_PASS (-0.40F)
/** Upper clamp of the log-odds (p of about 0.97). */
#define OGRID_LOG_ODDS_MAX (3.4F)
/** Lower clamp of the log-odds; guarantees confirmation with 3 echoes. */
#define OGRID_LOG_ODDS_MIN (-0.8F)
/** A cell is occupied when its probability is strictly above this value. */
#define OGRID_OCCUPIED_PROB_THRESHOLD (0.85F)

#endif /* OGRID_OGRID_LIMITS_H */
