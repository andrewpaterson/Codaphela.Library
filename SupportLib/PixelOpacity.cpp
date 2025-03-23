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
#include "Image.h"
#include "PixelOpacity.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPixelOpacityBase::Kill(void)
{
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPixelOpacityChannel::Init(CImage* pcImage)
{
	mbHasOpacity = pcImage->HasChannel(IMAGE_OPACITY);

	if (!mbHasOpacity)
	{
		mpcOpacityChannel = NULL;
		return;
	}

	mpcOpacityChannel = CImageAccessorCreator::Create(pcImage, IMAGE_OPACITY, CHANNEL_ZERO);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPixelOpacityChannel::Kill(void)
{
	if (mbHasOpacity)
	{
		mpcOpacityChannel->Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPixelOpacityChannel::IsTransparent(int x, int y)
{
	SImageColour*	psAlpha;

	if (mbHasOpacity)
	{
		psAlpha = (SImageColour*)mpcOpacityChannel->Get(x, y);

		return psAlpha->IsZero(mpcOpacityChannel->GetBufferSize());
	}
	return false;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPixelOpacityTransparentColour::Init(CImage* pcImage, SImageColour* psTransparentColour)
{
	CPixelOpacityChannel::Init(pcImage);
	msTransparentColour = *psTransparentColour;

	mpcAllChannels = CImageAccessorCreator::Create(pcImage, pcImage);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPixelOpacityTransparentColour::Kill(void)
{
	mpcAllChannels->Kill();
	CPixelOpacityChannel::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPixelOpacityTransparentColour::IsTransparent(int x, int y)
{
	SImageColour*	psColour;
	bool			bResult;

	psColour = (SImageColour*)mpcAllChannels->Get(x, y);
	bResult = psColour->Equals(&msTransparentColour, mpcAllChannels->GetBufferSize());
	if (bResult)
	{
		return true;
	}
	return CPixelOpacityChannel::IsTransparent(x, y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPixelOpacityMask::Init(CImage* pcSourceImage, CImage* pcMaskImage, short iMask)
{
	CPixelOpacityChannel::Init(pcSourceImage);
	mpcMaskChannel = CImageAccessorCreator::Create(pcMaskImage, IMAGE_MASK, CHANNEL_ZERO);
	miMask = iMask;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPixelOpacityMask::Kill(void)
{
	mpcMaskChannel->Kill();
	CPixelOpacityChannel::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPixelOpacityMask::IsTransparent(int x, int y)
{
	short		iMask;

	iMask = *((short*)mpcMaskChannel->Get(x, y));

	if (iMask != miMask)
	{
		return true;
	}
	return CPixelOpacityChannel::IsTransparent(x, y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPixelOpacityMaskTransparentColour::Init(CImage* pcImage, SImageColour* psTransparentColour, CImage* pcMaskImage, short iMask)
{
	CPixelOpacityMask::Init(pcImage, pcMaskImage, iMask);
	msTransparentColour = *psTransparentColour;

	mpcAllChannels = CImageAccessorCreator::Create(pcImage, pcImage);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPixelOpacityMaskTransparentColour::Kill(void)
{
	mpcMaskChannel->Kill();
	CPixelOpacityChannel::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPixelOpacityMaskTransparentColour::IsTransparent(int x, int y)
{
	SImageColour*	psColour;
	bool			bResult;

	psColour = (SImageColour*)mpcAllChannels->Get(x, y);
	bResult = psColour->Equals(&msTransparentColour, mpcAllChannels->GetBufferSize());

	if (bResult)
	{
		return true;
	}
	return CPixelOpacityMask::IsTransparent(x, y);
}
