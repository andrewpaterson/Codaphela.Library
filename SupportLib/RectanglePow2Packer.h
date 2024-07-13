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
#ifndef __RECTANGLE_POW2_PACKER_H__
#define __RECTANGLE_POW2_PACKER_H__
#include "RectangleBestPacker.h"


class CRectanglePow2Packer : public CRectanglePacker
{
CONSTRUCTABLE(CRectanglePow2Packer);
protected:
	CRectangleBestPacker	mcPacker;

public:
	void	Init(int iOutsideEdgeWidth, int iInnerEdgeWidth);
	void	Kill(void);

	void	Pack(void);

	bool	AddRectangle(CRectangle* pcRect, void* pvUserData);
	bool	AddRectangle(CSubImage* pcSubImage);
	bool	AddRectangle(CImageCel* pcImageCel);

	void	GetPackedRectangles(CArrayPackedRectangle* pacPackedRectangles);
	SInt2	GetPackedImageSize(void);

private:
	void	AdjustAttemptAreas(void);
};


#endif // __RECTANGLE_POW2_PACKER_H__
