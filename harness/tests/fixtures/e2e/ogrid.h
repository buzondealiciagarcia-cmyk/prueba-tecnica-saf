/**
 * @file ogrid.h
 * @brief Public API of the occupancy grid module.
 * @details Dynamic occupancy grid for static obstacle detection.
 * @ingroup ogrid
 */
#ifndef OGRID_OGRID_H
#define OGRID_OGRID_H

#include "ogrid/ogrid_platform.h"

/** @brief Module status codes. */
typedef enum
{
    OGRID_OK = 0,
    OGRID_ERR_PARAM,
    OGRID_ERR_RANGE
} ogrid_status_t;

/**
 * @brief Clamp a probability to the closed interval [0, 1].
 * @param[in] p Probability to clamp (any finite value).
 * @param[out] p_out Clamped probability.
 * @return OGRID_OK on success, OGRID_ERR_PARAM if p_out is NULL, OGRID_ERR_RANGE if p is NaN.
 * @pre p_out != NULL.
 * @post 0 <= *p_out <= 1 when OGRID_OK.
 * @req SYS-PR-01
 * @contract F-001
 */
ogrid_status_t ogrid_clamp_prob(float p, float *p_out);

#endif /* OGRID_OGRID_H */
