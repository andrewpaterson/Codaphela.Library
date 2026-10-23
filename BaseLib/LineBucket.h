/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#ifndef __LINE_BUCKET_H__
#define __LINE_BUCKET_H__
#include "Line.h"


//Lines on the same horizontal or vertical merged into one line covering them all (see MinimiseLines).
class CLineBucket
{
protected:
	CLine		mcLine;
	CArrayLine	macLines;

public:
	void		Init(CLine* pcLine);
	void		Kill(void);

	void		Merge(CLineBucket* pcOther);
	bool		Overlaps(CLineBucket* pcOther);

	CLine*		GetLine(void);
	CArrayLine*	GetLines(void);
};


typedef CArrayTemplate<CLineBucket>	CArrayLineBucket;


#endif // __LINE_BUCKET_H__

