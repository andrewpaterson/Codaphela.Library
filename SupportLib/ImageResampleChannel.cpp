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
#include <math.h>
#include "BaseLib/TypeNames.h"
#include "ImageResampleChannel.h"


//////////////////////////////////////////////////////////////////////////
//
// The smallest and largest raw values of eType.  Returns false if eType
// can't be resampled.
//
//////////////////////////////////////////////////////////////////////////
bool ImageResampleGetRange(EPrimitiveType eType, float64* pfMinimum, float64* pfMaximum, bool* pbFloat)
{
	*pbFloat = false;
	switch (eType)
	{
	case PT_int8:
		*pfMinimum = -128.0;
		*pfMaximum = 127.0;
		return true;
	case PT_int16:
		*pfMinimum = -32768.0;
		*pfMaximum = 32767.0;
		return true;
	case PT_int32:
		*pfMinimum = -2147483648.0;
		*pfMaximum = 2147483647.0;
		return true;
	case PT_int64:
		*pfMinimum = -9223372036854775808.0;
		*pfMaximum = 9223372036854774784.0;  //The largest float64 below 2^63.
		return true;
	case PT_uint8:
		*pfMinimum = 0.0;
		*pfMaximum = 255.0;
		return true;
	case PT_uint16:
		*pfMinimum = 0.0;
		*pfMaximum = 65535.0;
		return true;
	case PT_uint32:
		*pfMinimum = 0.0;
		*pfMaximum = 4294967295.0;
		return true;
	case PT_uint64:
		*pfMinimum = 0.0;
		*pfMaximum = 18446744073709549568.0;  //The largest float64 below 2^64.
		return true;
	case PT_float32:
	case PT_float64:
		*pfMinimum = -HUGE_VAL;
		*pfMaximum = HUGE_VAL;
		*pbFloat = true;
		return true;
	case PT_bit:
	case PT_crumb:
	case PT_tribble:
	case PT_nybble:
	case PT_nickle:
	case PT_sixbits:
		*pfMinimum = 0.0;
		*pfMaximum = (float64)((1 << gcTypeNames.GetBitSize(eType)) - 1);
		return true;
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageResampleChannel::Init(Ptr<CImage> pcImage, size uiChannelIndex)
{
	mpcChannels = pcImage->GetChannels();
	mpcChannel = pcImage->GetChannelAtIndex(uiChannelIndex);
	muiNumPixels = (size)pcImage->GetWidth() * (size)pcImage->GetHeight();

	if ((mpcChannel == NULL) || mpcChannel->bReverse || !ImageResampleGetRange(mpcChannel->eType, &mfMinimum, &mfMaximum, &mbFloat))
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageResampleChannel::Kill(void)
{
	mpcChannels = NULL;
	mpcChannel = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// True if every channel of pcImage can be resampled.
//
//////////////////////////////////////////////////////////////////////////
bool CImageResampleChannel::IsSupported(Ptr<CImage> pcImage)
{
	size					uiIndex;
	CImageResampleChannel	cChannel;

	for (uiIndex = 0; uiIndex < pcImage->NumChannels(); uiIndex++)
	{
		if (!cChannel.Init(pcImage, uiIndex))
		{
			return false;
		}
		cChannel.Kill();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if every channel of both images can be resampled and they have
// the same number of channels.
//
//////////////////////////////////////////////////////////////////////////
bool CImageResampleChannel::CanResample(Ptr<CImage> pcSource, Ptr<CImage> pcDest)
{
	return IsSupported(pcSource) && IsSupported(pcDest) && (pcSource->NumChannels() == pcDest->NumChannels());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageResampleChannel::IsSmallInteger(void)
{
	return !mbFloat && (mfMinimum >= -32768.0) && (mfMaximum <= 65535.0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageResampleChannel::IsFloat(void)
{
	return mbFloat;
}


//////////////////////////////////////////////////////////////////////////
//
// The smallest raw value.  -HUGE_VAL for floats.
//
//////////////////////////////////////////////////////////////////////////
float64 CImageResampleChannel::GetMinimum(void)
{
	return mfMinimum;
}


//////////////////////////////////////////////////////////////////////////
//
// The largest raw value.  HUGE_VAL for floats.
//
//////////////////////////////////////////////////////////////////////////
float64 CImageResampleChannel::GetMaximum(void)
{
	return mfMaximum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CImageResampleChannel::NumPixels(void)
{
	return muiNumPixels;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CImageResampleChannel::ToInteger(void* pvRaw)
{
	switch (mpcChannel->eType)
	{
	case PT_int8:
		return *((int8*)pvRaw);
	case PT_int16:
		return *((int16*)pvRaw);
	case PT_uint16:
		return *((uint16*)pvRaw);
	default:
		//uint8 and the bit sized types, which are read into the low bits of a byte.
		return *((uint8*)pvRaw);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 CImageResampleChannel::ToFloat(void* pvRaw)
{
	switch (mpcChannel->eType)
	{
	case PT_int32:
		return (float64)*((int32*)pvRaw);
	case PT_int64:
		return (float64)*((int64*)pvRaw);
	case PT_uint32:
		return (float64)*((uint32*)pvRaw);
	case PT_uint64:
		return (float64)*((uint64*)pvRaw);
	case PT_float32:
		return (float64)*((float32*)pvRaw);
	case PT_float64:
		return *((float64*)pvRaw);
	default:
		return (float64)ToInteger(pvRaw);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// iValue must be in the channel's range.
//
//////////////////////////////////////////////////////////////////////////
void CImageResampleChannel::FromInteger(int32 iValue, void* pvRaw)
{
	switch (mpcChannel->eType)
	{
	case PT_int8:
		*((int8*)pvRaw) = (int8)iValue;
		break;
	case PT_int16:
		*((int16*)pvRaw) = (int16)iValue;
		break;
	case PT_uint16:
		*((uint16*)pvRaw) = (uint16)iValue;
		break;
	default:
		*((uint8*)pvRaw) = (uint8)iValue;
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageResampleChannel::FromFloat(float64 fValue, void* pvRaw)
{
	if (!mbFloat)
	{
		fValue = floor(fValue + 0.5);
		if (fValue < mfMinimum)
		{
			fValue = mfMinimum;
		}
		else if (fValue > mfMaximum)
		{
			fValue = mfMaximum;
		}
		else if (fValue != fValue)
		{
			//NaN.
			fValue = 0.0;
		}
	}

	switch (mpcChannel->eType)
	{
	case PT_int32:
		*((int32*)pvRaw) = (int32)fValue;
		break;
	case PT_int64:
		*((int64*)pvRaw) = (int64)fValue;
		break;
	case PT_uint32:
		*((uint32*)pvRaw) = (uint32)fValue;
		break;
	case PT_uint64:
		*((uint64*)pvRaw) = (uint64)fValue;
		break;
	case PT_float32:
		*((float32*)pvRaw) = (float32)fValue;
		break;
	case PT_float64:
		*((float64*)pvRaw) = fValue;
		break;
	default:
		FromInteger((int32)fValue, pvRaw);
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageResampleChannel::GetIntegers(int32* paiDest)
{
	size	uiPixel;
	uint64	ulliRaw;

	for (uiPixel = 0; uiPixel < muiNumPixels; uiPixel++)
	{
		mpcChannels->GetNative(uiPixel, mpcChannel, &ulliRaw);
		paiDest[uiPixel] = ToInteger(&ulliRaw);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageResampleChannel::SetIntegers(int32* paiSource)
{
	size	uiPixel;
	uint64	ulliRaw;
	int32	iMinimum;
	int32	iMaximum;
	int32	iValue;

	iMinimum = (int32)mfMinimum;
	iMaximum = (int32)mfMaximum;
	for (uiPixel = 0; uiPixel < muiNumPixels; uiPixel++)
	{
		iValue = paiSource[uiPixel];
		if (iValue < iMinimum)
		{
			iValue = iMinimum;
		}
		else if (iValue > iMaximum)
		{
			iValue = iMaximum;
		}
		ulliRaw = 0;
		FromInteger(iValue, &ulliRaw);
		mpcChannels->SetNative(uiPixel, mpcChannel, &ulliRaw);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageResampleChannel::GetFloats(float64* pafDest)
{
	size	uiPixel;
	uint64	ulliRaw;

	for (uiPixel = 0; uiPixel < muiNumPixels; uiPixel++)
	{
		mpcChannels->GetNative(uiPixel, mpcChannel, &ulliRaw);
		pafDest[uiPixel] = ToFloat(&ulliRaw);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageResampleChannel::SetFloats(float64* pafSource)
{
	size	uiPixel;
	uint64	ulliRaw;

	for (uiPixel = 0; uiPixel < muiNumPixels; uiPixel++)
	{
		ulliRaw = 0;
		FromFloat(pafSource[uiPixel], &ulliRaw);
		mpcChannels->SetNative(uiPixel, mpcChannel, &ulliRaw);
	}
}

