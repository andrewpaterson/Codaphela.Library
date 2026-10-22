/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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

** ------------------------------------------------------------------------ **/
#ifndef __RLE_BITMAP_DECODER_H__
#define __RLE_BITMAP_DECODER_H__
#include "BaseLib/FileReader.h"


//Decodes Windows bitmap RLE8 and RLE4 data into one palette index per pixel.  Pairs of bytes are either a count
//and a value to repeat, or an escape (zero count) for end of line, end of bitmap, a delta or an absolute run.
class CRLEBitmapDecoder
{
protected:
	int		miWidth;
	int		miHeight;
	bool	mbFourBit;

public:
	void	Init(int iWidth, int iHeight, bool bFourBit);
	void	Kill(void);

	bool	Decode(CFileReader* pcFile, uint8* puiIndices);

protected:
	void	SetIndex(uint8* puiIndices, int x, int y, uint8 uiIndex);
};


#endif // __RLE_BITMAP_DECODER_H__
