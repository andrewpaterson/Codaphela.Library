/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __IMAGE_RESAMPLER_H__
#define __IMAGE_RESAMPLER_H__
#include "Image.h"
#include "ImageModifier.h"


enum EPow2Style
{
	RS_Unknown,
	RS_Up,
	RS_Down,
	RS_Auto
};


enum EImageResample
{
	IR_NearestNeighbour,
	IR_Bilinear,
	IR_Bicubic,
	IR_Smart,  //http://www.hiend3d.com/smartflt.html
};


class CImage;
class CImageResampler : public CImageModifier
{
CONSTRUCTABLE(CImageResampler);
private:
	void GetPowerOf2Size(EPow2Style eStyle, int iSourceWidth, int iSourceHeight, int* piDestWidth, int* piDestHeight);
	void ResampleTo(CImage* pcDest, CImage* pcSource);

public:
	EImageResample	meResample;
	int				miWidth;
	int				miHeight;
	EPow2Style		meStyle;

	void Init(EImageResample eResample, EPow2Style eStyle);
	void Init(EImageResample eResample, int iWidth, int iHeight);
	void Kill(void);
	bool Modify(CImage* pcImage);
};


#endif // __IMAGE_RESAMPLER_H__

