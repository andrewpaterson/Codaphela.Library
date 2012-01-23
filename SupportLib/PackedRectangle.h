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
#ifndef __PACKED_RECTANGLE_H__
#define __PACKED_RECTANGLE_H__
#include "BaseLib/ArrayTemplate.h"
#include "Rectangle.h"


class CPackSourceRectangle
{
public:
	int			miWidth;
	int			miHeight;
	void*		mpvUserData;

	void Init(CRectangle* pcRect, void* pvUserData);
};


typedef CArrayTemplate<CPackSourceRectangle> CArrayPackSourceRectangle;


class CPackedLine;
class CPackRectangleAttempt
{
public:
	CPackSourceRectangle*	mpcPackedRectangle;
	CPackedLine*			mpcLine;
	int						mix;	//Relative to the CPackedLine.
	int						miy;

	void Init(CPackSourceRectangle* pcPackedRectangle);
	void Set(CPackedLine* pcLine, int x, int y);
};


typedef CArrayTemplate<CPackRectangleAttempt> CArrayPackRectangleAttempt;
int ComparePackRectangleAttempt(const void* pv1, const void* pv2);


class CPackedRectangle : public CRectangle
{
public:
	void*		mpvUserData;

	void Init(int iLeft, int iTop, int iRight, int iBottom, void* pvUserData);
	void Init(CRectangle* pcRect, void* pvUserData);
};


typedef CArrayTemplate<CPackedRectangle> CArrayPackedRectangle;


#endif // __PACKED_RECTANGLE_H__

