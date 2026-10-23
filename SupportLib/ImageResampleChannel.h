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
#ifndef __IMAGE_RESAMPLE_CHANNEL_H__
#define __IMAGE_RESAMPLE_CHANNEL_H__
#include "Image.h"


//Reads and writes every pixel of one image channel as raw values (a uint8 is 0 to 255, a nybble 0 to 15) so the resamplers
//can interpolate it.  Byte aligned channels are read directly; bit sized and unaligned channels are read bit by bit.
//Reversed channels are not supported.
class CImageResampleChannel
{
protected:
	CChannels*		mpcChannels;
	CChannel*		mpcChannel;
	size			muiNumPixels;
	bool			mbAligned;
	uint8*			mpuiData;
	size			muiByteStride;
	size			muiBitStride;
	float64			mfMinimum;
	float64			mfMaximum;
	bool			mbFloat;

public:
	bool			Init(Ptr<CImage> pcImage, size uiChannelIndex);
	void			Kill(void);

	bool			IsSmallInteger(void);  //An integer of 16 bits or fewer.  These can be read with GetIntegers.
	bool			IsFloat(void);
	float64			GetMinimum(void);
	float64			GetMaximum(void);
	size			NumPixels(void);

	void			GetIntegers(int32* paiDest);
	void			SetIntegers(int32* paiSource);
	void			GetFloats(float64* pafDest);
	void			SetFloats(float64* pafSource);  //Integer channels are rounded to the nearest value and clamped to their range.

	static bool		IsSupported(Ptr<CImage> pcImage);

protected:
	void			GetRaw(size uiPixel, void* pvDest);
	void			SetRaw(size uiPixel, void* pvSource);
	int32			ToInteger(void* pvRaw);
	float64			ToFloat(void* pvRaw);
	void			FromInteger(int32 iValue, void* pvRaw);
	void			FromFloat(float64 fValue, void* pvRaw);
};


#endif // __IMAGE_RESAMPLE_CHANNEL_H__

