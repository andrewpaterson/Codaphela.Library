/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __LINE_ZONE_SOURCE_H__
#define __LINE_ZONE_SOURCE_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Chars.h"


typedef CArrayTemplate<size>	CArrayLineZoneLines;


//What CLineZones shows under the lines of a code editor's code: for each line that has something, pages of text with
//a title each, such as the places a line's code went to in a build's listings.  Lines and pages count from 0.
class CLineZoneSource
{
public:
	virtual void	GetLines(CArrayLineZoneLines* pauiDest) =0;  //The lines with pages, in order, each once.
	virtual size	NumPages(size uiLine) =0;
	virtual void	GetTitle(size uiLine, size uiPage, CChars* pszDest) =0;
	virtual void	GetText(size uiLine, size uiPage, CChars* pszDest) =0;
};


#endif // __LINE_ZONE_SOURCE_H__

