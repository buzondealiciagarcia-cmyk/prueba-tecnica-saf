/**
 * @file ogrid.h
 * @brief Public entry point of the ogrid module (occupancy grid in log-odds).
 * @details Declares the status codes shared by every public function and pulls in the
 *          module types and limits. Per-function headers are added to the list at the end
 *          as the functions of the implementation plan are integrated.
 * @ingroup ogrid
 */
#ifndef OGRID_OGRID_H
#define OGRID_OGRID_H

#include "ogrid/ogrid_limits.h"
#include "ogrid/ogrid_types.h"

/**
 * @brief Result of every public ogrid function (C-ERR-01).
 */
typedef enum
{
    OGRID_OK = 0,    /**< Operation completed. */
    OGRID_ERR_PARAM, /**< Null pointer or argument out of the contract range; no side effects. */
    OGRID_ERR_RANGE, /**< Cell index or log-odds value outside the grid or its limits. */
    OGRID_ERR_STATE, /**< Operation not allowed in the current state of the grid. */
    OGRID_STATUS_COUNT
} ogrid_status_t;

#endif /* OGRID_OGRID_H */
