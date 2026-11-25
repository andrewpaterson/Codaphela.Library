/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __I_R_SOURCE_MARKERS_H__
#define __I_R_SOURCE_MARKERS_H__
#include "BaseLib/ArrayChars.h"
#include "BaseLib/ArrayTemplate.h"


#define IR_NO_MARKERS	(-1)	//A line with no markers before it.


bool	IRIsMarkerLine(const char* szLine);


//The marker lines of a function's code (the source markers, IR_SOURCE_MARKER, and the debugger's place markers,
//IR_PLACE_MARKER), kept as fields of the code's lines: each line has the list of markers written before it (a list
//here, IR_NO_MARKERS for none), and they are printed before it.  The markers written since the last line (Add) are
//the next line's (TakePending).  A line a pass takes out gives its markers to the line after it (Join); a list is
//never changed, so two lines may have one.  Of the source markers before one line only the last is printed (the
//code of the others went); every place marker is, in order (each says something of its own).
class CIRLineMarkers
{
protected:
	CArrayChars				maszMarkers;	//Each marker's line, as written.
	CArrayTemplate<int32>	maiLists;		//The lists: at each one's index its count, then that many of maszMarkers.
	CArrayTemplate<int32>	maiPending;		//The markers written since the last line.

public:
	void	Init(void);
	void	Kill(void);
	void	Clear(void);

	void	Add(const char* szMarker);
	int32	TakePending(void);
	void	PutBack(int32 iList);
	int32	Join(int32 iFirst, int32 iSecond);
	void	Print(int32 iList, CChars* pszOut);
};


#endif // __I_R_SOURCE_MARKERS_H__
