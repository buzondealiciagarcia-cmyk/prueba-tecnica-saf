/**
 * @file ogrid_clamp_prob.c
 * @brief Probability clamping for occupancy grid cells.
 * @details Implements contract F-001.
 * @ingroup ogrid
 */
#include "ogrid/ogrid.h"

#include <stddef.h>

ogrid_status_t ogrid_clamp_prob(float p, float *p_out)
{
    ogrid_status_t status = OGRID_OK;

    if (p_out == NULL)
    {
        status = OGRID_ERR_PARAM;
    }
    else if (p != p)
    {
        status = OGRID_ERR_RANGE;
    }
    else if (p < 0.0F)
    {
        *p_out = 0.0F;
    }
    else if (p > 1.0F)
    {
        *p_out = 1.0F;
    }
    else
    {
        *p_out = p;
    }
    return status;
}
