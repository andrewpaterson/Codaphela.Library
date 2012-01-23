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
#ifndef __RECTANGLE_BEST_PACKER_H__
#define __RECTANGLE_BEST_PACKER_H__
#include "ImageCel.h"
#include "PackedRectangle.h"
#include "PackAttempt.h"
#include "RectanglePacker.h"


class CRectangleBestPacker : public CRectanglePacker
{
BASE_FUNCTIONS(CRectangleBestPacker)
private:
	SInt2			GetSourcesSizes(void);

protected:
	CArrayPackSourceRectangle	masSourceRectangles;
	int							miOutsideEdgeWidth;  //Gap around the outside of the destination image and the rectangles.
	int							miInnerEdgeWidth;  //Gap between rectangles.
	CArrayPackAttempt			macPackAttempts;
	int							miSourceArea;

public:
	void						Init(int iOutsideEdgeWidth, int iInnerEdgeWidth);
	void						Kill(void);

	void						Pack(void);
	void						Dump(void);

	BOOL						AddRectangle(CRectangle* pcRect, void* pvUserData);
	BOOL						AddRectangle(CSubImage* pcSubImage);
	BOOL						AddRectangle(CImageCel* pcImageCel);

	void						AddAttempt(int iMaxWidth);
	void						AddHorizontalAttempt(void);
	void						AddSquareAttempt(void);
	void						AddVerticalAttempt(void);
	void						AddDefaultAttempts(void);

	CPackAttempt*				GetBestAttempt(void);
	CPackAttempt*				GetWorstAttempt(void);
	int							GetSourcesMinWidth(void);
	int							GetSourcesMaxWidth(void);

	void						GetPackedRectangles(CArrayPackedRectangle* pacPackedRectangles);
	SInt2						GetPackedImageSize(void);

	CArrayPackSourceRectangle*	GetSourceRectangles(void);
	int							GetOutsideEdgeWidth(void);
	int							GetInnerEdgeWidth(void);
	CArrayPackAttempt*			GetPackAttempts(void);
	int							GetSourceArea(void);
};


#endif // __RECTANGLE_BEST_PACKER_H__

