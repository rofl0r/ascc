//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================
//
// Various utility bit and byte operations
//
//=============================================================================
#ifndef __AGS_CN_UTIL__BBOP_H
#define __AGS_CN_UTIL__BBOP_H

#define ENDIANNESS_PORTABLE_CONVERSION 1
#include "endianness.h"
#include "core_types.h"

namespace AGS
{
namespace Common
{

namespace BitByteOperations
{
    inline int16_t Int16FromLE(const int16_t val)
    {
        return end_le16toh(val);
    }

    inline int32_t Int32FromLE(const int32_t val)
    {
        return end_le32toh(val);
    }

    inline int64_t Int64FromLE(const int64_t val)
    {
        return end_le64toh(val);
    }

} // namespace BitByteOperations


// Aliases for easier calling
namespace BBOp  = BitByteOperations;


} // namespace Common
} // namespace AGS

#endif // __AGS_CN_UTIL__BBOP_H
