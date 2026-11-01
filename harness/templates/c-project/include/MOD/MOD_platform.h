/**
 * @file @MOD@_platform.h
 * @brief Platform assumptions checked at compile time (C-LNG-06).
 * @details The host simulation must match the target hardware: every assumption about
 *          type sizes or representation is asserted here.
 * @ingroup @MOD@
 */
#ifndef @MOD_UPPER@_@MOD_UPPER@_PLATFORM_H
#define @MOD_UPPER@_@MOD_UPPER@_PLATFORM_H

#include <stdint.h>
#include <limits.h>

#define @MOD_UPPER@_PLAT_CHAR_BITS   (8)
#define @MOD_UPPER@_PLAT_FLOAT_SIZE  (4U)
#define @MOD_UPPER@_PLAT_DOUBLE_SIZE (8U)
#define @MOD_UPPER@_PLAT_PTR_MIN     (4U)

_Static_assert(CHAR_BIT == @MOD_UPPER@_PLAT_CHAR_BITS, "8-bit bytes required");
_Static_assert(sizeof(float) == @MOD_UPPER@_PLAT_FLOAT_SIZE, "IEEE-754 binary32 float required");
_Static_assert(sizeof(double) == @MOD_UPPER@_PLAT_DOUBLE_SIZE, "IEEE-754 binary64 double required");
_Static_assert(sizeof(void *) >= @MOD_UPPER@_PLAT_PTR_MIN, "32-bit or wider pointers required");

#endif /* @MOD_UPPER@_@MOD_UPPER@_PLATFORM_H */
