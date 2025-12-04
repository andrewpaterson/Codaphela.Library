/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "StandardLib/ChannelsAccessorContiguous.h"
#include "ImageAccessorByte.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageAccessorByte::Init(CImage* pcImage, CChannelsAccessorContiguous* pcAccessor, bool bKillChannelAccessor)
{
	CImageAccessor::Init(pcImage, pcAccessor, bKillChannelAccessor);

	miOffset = pcAccessor->miByteOffset;
	miPixelStride = mpcImage->GetChannels()->GetByteStride();
	miImageStride = miPixelStride * miWidth;
	SyncDataCache();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageAccessorByte::Kill(void)
{
	CImageAccessor::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CImageAccessorByte::Get(int x, int y)
{
	return &mpucImageData[x * miPixelStride + miOffset + y * miImageStride];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageAccessorByte::Set(int x, int y, void* pvSource)
{
	mpucImageData[x * miPixelStride + miOffset + y * miImageStride] = *(uint8*)pvSource;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageAccessorByte::SyncDataCache(void)
{
	mpucImageData = (uint8*)mpcImage->mcChannels.GetData();
}

