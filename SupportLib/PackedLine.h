/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#ifndef __PACKED_LINE_H__
#define __PACKED_LINE_H__
#include "BaseLib/ArrayTemplate.h"
#include "PackedRectangle.h"


class CPackAttempt;
class CPackedLine
{
public:
	CPackAttempt*				mpcPackAttempt;
	int							miHeight;
	int							miWidth;
	CArrayPackRectangleAttempt	macRectangles;
	int							miy;  //Relative to the CPackAttempt, not the CRectangleBestPacker.  The difference should be miOutsideEdgeWidth.

	void					Init(CPackAttempt* pcPackAttempt);
	void					Kill(void);

	bool					Pack(void);
	bool					RecursePack(CRectangle* pcBounds);
	void					Done(void);
	CPackRectangleAttempt*	Get(void* pvUserData);
	void					GetPackedRectangles(CArrayPackedRectangle* pacPackedRectangles);

	void					Print(CChars* psz);
};


typedef CArrayTemplate<CPackedLine>	CArrayPackedLine;
int ComparePackedLine(const void* pv1, const void* pv2);


#endif // !__PACKED_LINE_H__
