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
// 'C'-style script compiler
//
//=============================================================================

#ifndef __CC_OPTIONS_H
#define __CC_OPTIONS_H

#define SCOPT_EXPORTALL        (1<<0)   // export all functions automatically
#define SCOPT_SHOWWARNINGS     (1<<1)   // printf warnings to console
#define SCOPT_LINENUMBERS      (1<<2)   // include line numbers in compiled code
#define SCOPT_AUTOIMPORT       (1<<3)   // when creating instance, export funcs to other scripts
#define SCOPT_DEBUGRUN         (1<<4)   // write instructions as they are procssed to log file
#define SCOPT_NOIMPORTOVERRIDE (1<<5)   // do not allow an import to be re-declared
#define SCOPT_LEFTTORIGHT      (1<<6)   // left-to-right operator precedance
#define SCOPT_OLDSTRINGS       (1<<7)   // allow old-style strings

extern void ccSetOption(int, int);
extern int ccGetOption(int);

#endif
