#ifndef __AC_PLATFORM_H
#define __AC_PLATFORM_H

// platform definitions. Not intended for replacing types or checking for libraries.

#if defined(_WIN32)
    //define something for Windows (32-bit and 64-bit)
    #define AGS_PLATFORM_OS_WINDOWS (1)
#else
    #define AGS_PLATFORM_OS_WINDOWS (0)
#endif

#include "endianness.h"

#if ENDIANNESS_LE
    #define AGS_PLATFORM_ENDIAN_LITTLE  (1)
    #define AGS_PLATFORM_ENDIAN_BIG     (0)
#else
    #define AGS_PLATFORM_ENDIAN_LITTLE  (0)
    #define AGS_PLATFORM_ENDIAN_BIG     (1)
#endif

#if defined(_DEBUG)
    #define AGS_PLATFORM_DEBUG  (1)
#elif ! defined(NDEBUG)
    #define AGS_PLATFORM_DEBUG  (1)
#else
    #define AGS_PLATFORM_DEBUG  (0)
#endif

#endif // __AC_PLATFORM_H
