/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __I_R_SOURCE_MARKERS_H__
#define __I_R_SOURCE_MARKERS_H__
#include "BaseLib/ArrayChars.h"
#include "BaseLib/ArrayTemplate.h"


//The source markers (IR_SOURCE_MARKER lines) of a function's code, taken out before a back end's text passes so the
//passes see the code they would without them (a listing's code is the same as the one without), and put back after:
//each before the first line it was before that is still there.
class CIRSourceMarkers
{
protected:
	CArrayChars				maszLines;		//The code's lines without the markers, as Take left them.
	CArrayTemplate<int32>	maiBefore;		//Per marker: the line (in maszLines) it was before.
	CArrayChars				maszMarkers;	//Per marker: its line.

public:
	void	Init(void);
	void	Kill(void);

	void	Take(CChars* pszCode, size uiStart);
	void	Put(CChars* pszCode, size uiStart);

protected:
	void	Match(CArrayChars* pasLines, CArrayTemplate<int32>* paiFound);
};


#endif // __I_R_SOURCE_MARKERS_H__

