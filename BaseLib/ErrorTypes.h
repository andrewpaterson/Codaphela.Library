/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __ERROR_TYPES_H__
#define __ERROR_TYPES_H__
#include "Define.h"

#if (TRUE == -1)
#error True must have a value of 1
#endif


enum TRISTATE
{
	TRITRUE	= TRUE,
	TRIFALSE = FALSE,
	TRIERROR = -1,

	FORCE_DWORD = 0xffffffff
};


//Bad global right here.  Dunno how to get rid of it though.
extern TRISTATE	gTriState; 
#define TTV(line) gTriState


#define InvertTrueAndFalse(p)			if (p == TRITRUE) { p = TRIFALSE; } else if (p == TRIFALSE) { p = TRITRUE; } 
#define ReturnOnErrorBreakOnFalse(p)	TTV(__LINE__) = p;  if (TTV(__LINE__) == TRIERROR) { return TRIERROR; } else if (TTV(__LINE__) == TRIFALSE) { break; }
#define ReturnOnErrorBreakOnTrue(p)		TTV(__LINE__) = p;  if (TTV(__LINE__) == TRIERROR) { return TRIERROR; } else if (TTV(__LINE__) == TRITRUE) { break; }
#define ReturnOnErrorAndFalse(p)		TTV(__LINE__) = p;  if ((TTV(__LINE__) == TRIERROR) || (TTV(__LINE__) == TRIFALSE)) { return TTV(__LINE__); }
#define ReturnOnErrorAndTrue(p)			TTV(__LINE__) = p;  if ((TTV(__LINE__) == TRIERROR) || (TTV(__LINE__) == TRITRUE)) { return TTV(__LINE__); }
#define ReturnOnError(p)				if (p == TRIERROR) { return TRIERROR; }
#define ReturnOnFalse(p)				if (p == TRIFALSE) { return TRIFALSE; }
#define ReturnOnTrue(p)					if (p == TRITRUE)  { return TRITRUE; }
#define ReturnErrorOnFalse(p)			if (p == TRIFALSE) { return TRIERROR; }
#define ReturnFalseOnError(p)			if (p == TRIERROR) { return TRIFALSE; }
#define ReturnFalseOnErrorAndFalse(p)	TTV(__LINE__) = p;  if ((TTV(__LINE__) == TRIERROR) || (TTV(__LINE__) == TRIFALSE)) { return TRIFALSE; }
#define ReturnErrorOnErrorAndFalse(p)	TTV(__LINE__) = p;  if ((TTV(__LINE__) == TRIERROR) || (TTV(__LINE__) == TRIFALSE)) { return TRIERROR; }
#define BreakOnErrorAndFalse(p)			if (p != TRITRUE) { break; }
#define BreakOnError(p)					if (p == TRIERROR) { break; }
#define BreakOnFalse(p)					if (p == TRIFALSE) { break; }
#define BreakOnTrue(p)					if (p == TRITRUE) { break; }
#define ContinueOnTrue(p)				if (p == TRITRUE) { continue; }
TRISTATE Negate(TRISTATE tristate);
TRISTATE Fix(TRISTATE tristate);

//Not specifically TRISTATE related but errors none the less.
#define ReturnZeroOnZero(p)				if (p == 0)  { return 0; }
#define ReturnMinusOneOnMinusOne(p)		if (p == -1)  { return -1; }


#endif //__ERROR_TYPES_H__

