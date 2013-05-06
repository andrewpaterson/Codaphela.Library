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
#include "BaseLib/IntegerHelper.h"
#include "BaseLib/PointerFunctions.h"
#include "CoreLib/TypeConverter.h"
#include "ImageAccessor.h"
#include "Image.h"
#include "SubImage.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageAccessor::Init(CImage* pcImage, CChannelsAccessor* pcAccessor)
{
	mpcImage = pcImage;
	if (mpcImage)
	{
		miWidth = pcImage->miWidth;
	}
	else
	{
		miWidth = -1;
	}
	mpcAccessor = pcAccessor;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageAccessor::Kill(void)
{
	mpcAccessor = NULL;
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CImageAccessor::Get(int x, int y)
{
	return mpcAccessor->Get(x + y * miWidth);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageAccessor::Set(int x, int y, void* pvSource)
{
	mpcAccessor->Set(x + y * miWidth, pvSource);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CImageAccessor::GetBufferSize(void)
{
	return mpcAccessor->GetBufferSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CImageAccessor::SafeGet(int x, int y)
{
	if (IsValid(x, y))
	{
		return Get(x, y);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageAccessor::SafeSet(int x, int y, void* pvData)
{
	if (IsValid(x, y))
	{
		Set(x, y, pvData);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageAccessor::Get(int x, int y, void* pvDest)
{
	void*	pvData;
	pvData = Get(x, y);
	memcpy_fast(pvDest, pvData, mpcAccessor->GetBufferSize());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CImageAccessor::IsValid(int x, int y)
{
	if (x >= 0 && y >= 0 && x < miWidth && y < mpcImage->miHeight)
	{
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CImageAccessor::MakeColour(SImageColour* psDest, CImageColour* pcColour)
{
	int						i;
	float					fValue;
	SImageColour			sTemp;
	BOOL					bValid;
	void*					pvData;
	CChannelAccessor*		pcAccessor;
	int						iSize;
	CArrayChannelAccessor*	pacAccessors;

	if (GetBufferSize() > sizeof(SImageColour))
	{
		//The colour representation is too big to fit.
		psDest->Zero();
		return FALSE;
	}

	bValid = TRUE;
	pvData = psDest;

	if (mpcAccessor->GetByteSize() == -1)
	{
		return FALSE;
	}

	pacAccessors = mpcAccessor->GetAccessors();
	for (i = 0; i < pacAccessors->NumElements(); i++)
	{
		pcAccessor = pacAccessors->Get(i);
		fValue = pcColour->GetValue(pcAccessor->miChannel);
		iSize = pcAccessor->miAccessByteSize;
		if (iSize < 0)
		{
			iSize = 1;
		}
		if (!IsNotANumber(&fValue))
		{
			gcTypeConverter.Do(pcAccessor->meAccessType, &sTemp, PT_float, &fValue);

			ga_memcpy_fast[pcAccessor->miAccessByteSize](pvData, &sTemp);
		}
		else
		{
			//There are more channels in the accessor than are provided by the colour.
			//The destination will not be usable.
			bValid = FALSE;
		}
		pvData = (void*)(size_t)(((int)(size_t)pvData) + iSize);
	}
	return bValid;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CImageAccessor::IsEmpty(void)
{
	return mpcAccessor->GetAccessors()->NumElements()== 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageAccessor::SyncDataCache(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EPrimitiveTypes	CImageAccessor::GetType(void)
{
	return mpcAccessor->GetType();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CImageAccessor::GetBitSize(void)
{
	return mpcAccessor->GetBitSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CImageAccessor::GetByteSize(void)
{
	return mpcAccessor->GetByteSize();
}


CChannelsAccessor* CImageAccessor::GetAccessor(void) { return mpcAccessor; }
CImage* CImageAccessor::GetImage(void) { return mpcImage; }
int CImageAccessor::GetWidth(void) { return miWidth; }
