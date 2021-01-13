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

#include "cc_options.h"
#include <string.h>

int ccCompOptions = SCOPT_LEFTTORIGHT;

static const struct {
	int bit;
	const char *name;
	const char *help;
} map[] = {
	{SCOPT_EXPORTALL, "exportall", "export all functions automatically"},
	{SCOPT_SHOWWARNINGS, "showwarnings", "printf warnings to console"},
	{SCOPT_LINENUMBERS, "linenumbers", "include line numbers in compiled code"},
	{SCOPT_AUTOIMPORT, "autoimport", "when creating instance, export funcs to other scripts"}, 
	{SCOPT_DEBUGRUN, "debugrun", "write instructions as they are processed to log file"},
	{SCOPT_NOIMPORTOVERRIDE, "noimportoverride", "do not allow an import to be re-declared"},
	{SCOPT_LEFTTORIGHT, "lefttoright", "left-to-right operator precedance"},
	{SCOPT_OLDSTRINGS, "oldstrings", "allow old-style strings"},
	{SCOPT_CHECKBOUNDS, "checkbounds", "emit array boundary check instructions. makes code slower but could be helpful for debugging."},
	{SCOPT_EXPLICITRET, "explicitret","emit explicit return 0 value in non-void functions closing without return"},
	{SCOPT_OLDNEGATE, "oldnegate", "emit old inefficient code loading a negative literal"},
	{0, 0, 0}
};

int ccOptionStringToBit(char *str) {
	for(int i = 0; map[i].name; ++i)
		if(!strcmp(str, map[i].name)) return map[i].bit;
	return 0;
}

const char* ccOptionHelpString(int optbit) {
	for(int i = 0; map[i].name; ++i)
		if(map[i].bit == optbit) return map[i].help;
	return 0;
}

const char* ccOptionNameString(int optbit) {
	for(int i = 0; map[i].name; ++i)
		if(map[i].bit == optbit) return map[i].name;
	return 0;
}

void ccSetOption(int optbit, int onoroff)
{
    if (onoroff)
        ccCompOptions |= optbit;
    else
        ccCompOptions &= ~optbit;
}

int ccGetOption(int optbit)
{
    if (ccCompOptions & optbit)
        return 1;

    return 0;
}
