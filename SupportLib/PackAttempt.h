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
#ifndef __PACK_ATTEMPT_H__
#define __PACK_ATTEMPT_H__
#include "PackedLine.h"
#include "PackedRectangle.h"


class CRectangleBestPacker;
class CPackAttempt
{
public:
	int							miMaxWidth;
	int							miWidth;
	int							miHeight;
	int							miArea;
	CArrayPackedLine			macLines;
	CArrayPackRectangleAttempt	macRectangles;
	CRectangleBestPacker*		mpcRectanglePacker;
	BOOL						mbValid;

	void 					Init(CRectangleBestPacker* pcRectanglePacker, int MaxWidth);
	void 					Kill(void);

	void 					AddRectanglesFromPacker(void);
	void					Pack(void);
	void 					Done(void);
	CPackRectangleAttempt*	GetLastRectangle(void);
	void					RemoveLastRectangle(void);
	BOOL					GetLargestRectangleFittingAndRemoveIt(CRectangle* pcBounds, CPackRectangleAttempt* pcDest);
	int						GetWastedArea(void);
	void					GetPackedRectangles(CArrayPackedRectangle* pacPackedRectangles);
	SInt2					GetPackedImageSize(void);
	void					ToString(CChars* psz);
	void					Dump(void);
};


typedef CArrayTemplate<CPackAttempt>	CArrayPackAttempt;
int ComparePackAttempt(const void* pv1, const void* pv2);


#endif // __PACK_ATTEMPT_H__

