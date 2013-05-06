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
#ifndef __RECTANGLE_PACKER_H__
#define __RECTANGLE_PACKER_H__
#include "BaseLib/DataTypes.h"
#include "StandardLib/Unknown.h"
#include "Rectangle.h"
#include "PackedRectangle.h"
#include "SubImage.h"
#include "ImageCel.h"


class CRectanglePacker : public CUnknown
{
BASE_FUNCTIONS(CRectanglePacker);
public:
	virtual void	Kill(void) =0;

	virtual void	Pack(void) =0;

	virtual BOOL	AddRectangle(CRectangle* pcRect, void* pvUserData) =0;
	virtual BOOL	AddRectangle(CSubImage* pcSubImage) =0;
	virtual BOOL	AddRectangle(CImageCel* pcImageCel) =0;

	virtual void	GetPackedRectangles(CArrayPackedRectangle* pacPackedRectangles) =0;
	virtual SInt2	GetPackedImageSize(void) =0;
};


#endif //__RECTANGLE_PACKER_H__
