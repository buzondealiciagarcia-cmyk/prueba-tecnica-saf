/**
 * @file ogrid_platform.h
 * @brief Platform assumptions checked at compile time (C-LNG-06).
 * @details The host simulation must match the target hardware: every assumption about
 *          type sizes or representation is asserted here.
 * @ingroup ogrid
 */
#ifndef OGRID_OGRID_PLATFORM_H
#define OGRID_OGRID_PLATFORM_H

#include <stdint.h>
#include <limits.h>

#define OGRID_PLAT_CHAR_BITS   (8)
#define OGRID_PLAT_FLOAT_SIZE  (4U)
#define OGRID_PLAT_DOUBLE_SIZE (8U)
#define OGRID_PLAT_PTR_MIN     (4U)

_Static_assert(CHAR_BIT == OGRID_PLAT_CHAR_BITS, "8-bit bytes required");
_Static_assert(sizeof(float) == OGRID_PLAT_FLOAT_SIZE, "IEEE-754 binary32 float required");
_Static_assert(sizeof(double) == OGRID_PLAT_DOUBLE_SIZE, "IEEE-754 binary64 double required");
_Static_assert(sizeof(void *) >= OGRID_PLAT_PTR_MIN, "32-bit or wider pointers required");

#endif /* OGRID_OGRID_PLATFORM_H */
