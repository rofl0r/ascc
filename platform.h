#ifndef __AC_PLATFORM_H
#define __AC_PLATFORM_H

// platform definitions. Not intended for replacing types or checking for libraries.

#include "endianness.h"

#if ENDIANNESS_LE
    #define AGS_PLATFORM_ENDIAN_LITTLE  (1)
    #define AGS_PLATFORM_ENDIAN_BIG     (0)
#else
    #define AGS_PLATFORM_ENDIAN_LITTLE  (0)
    #define AGS_PLATFORM_ENDIAN_BIG     (1)
#endif

#endif // __AC_PLATFORM_H
