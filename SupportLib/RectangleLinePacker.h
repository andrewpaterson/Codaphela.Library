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
#ifndef __RECTANGLE_LINE_PACKER_H__
#define __RECTANGLE_LINE_PACKER_H__
#include "RectanglePacker.h"


class CRectangleLinePacker : public CRectanglePacker
{
CONSTRUCTABLE(CRectangleLinePacker);
protected:
	CArrayPackedRectangle	macRectangles;
	int						miOutsideEdgeWidth;
	int						miInnerEdgeWidth;
	BOOL					mbLeftToRight;
	SInt2					miSize;
	BOOL					mbPow2;

public:
	void					Init(BOOL bLeftToRight = TRUE, BOOL bPow2 = FALSE, int iOutsideEdgeWidth = 0, int iInnerEdgeWidth = 0);
	void					Kill(void);

	void					Pack(void);

	BOOL					AddRectangle(CRectangle* pcRect, void* pvUserData);
	BOOL					AddRectangle(CSubImage* pcSubImage);
	BOOL					AddRectangle(CImageCel* pcImageCel);

	void					GetPackedRectangles(CArrayPackedRectangle* pacPackedRectangles);
	SInt2					GetPackedImageSize(void);

	int						GetWidestRect(void);
	int						GetHighestRect(void);
	int						PackLeftToRight(void);
	int						PackTopToBottom(void);

	CArrayPackedRectangle*	GetRectangles(void);
	int						GetOutsideEdgeWidth(void);
	int						GetInnerEdgeWidth(void);
	BOOL					IsLeftToRight(void);
	SInt2					GetSize(void);
	BOOL					IsPow2(void);
};


#endif // !__RECTANGLE_LINE_PACKER_H__

