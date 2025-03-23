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
#include "BaseLib/FastFunctions.h"
#include "BaseLib/IntegerHelper.h"
#include "ImageFiller.h"
#include "ImageColour.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageFiller::Init(void* pvBorderColour, CImageAccessor* pcSource, CImage* pcDestImage)
{
	mcStack.Init();
	mpvBorderColour = pvBorderColour;
	mpcDestImage = pcDestImage;
	mpcSourceAccessor = pcSource;

	mpcDestImage->ReInit(	mpcSourceAccessor->GetWidth(), 
							mpcSourceAccessor->GetImage()->GetHeight(), 
							PT_int16, 
							IMAGE_MASK, 
							CHANNEL_ZERO);

	mpcDestImage->Clear();
	mpcDestMaskAccessor = CImageAccessorCreator::Create(mpcDestImage, IMAGE_MASK, CHANNEL_ZERO);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageFiller::Kill(void)
{
	mcStack.Kill();
	mpcDestMaskAccessor->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageFiller::Fill(int x, int y, CFillRectangle* pcDestSubImage, uint16 iMask)
{
	SImageFill	sImageFill;

	mpcDestSubImage = pcDestSubImage;
	mpcDestSubImage->Init(iMask, x, y, x, y);
	miMask = iMask;

	if (!Push(x, y, FILL_NONE))
	{
		return false;
	}

	while (mcStack.NumElements() != 0)
	{
		mcStack.Pop(&sImageFill);

		if (sImageFill.iFill & FILL_RIGHT)
		{
			Push(sImageFill.x+1, sImageFill.y, FILL_LEFT);
		}
		if (sImageFill.iFill & FILL_DOWN)
		{
			Push(sImageFill.x, sImageFill.y+1, FILL_UP);
		}
		if (sImageFill.iFill & FILL_LEFT)
		{
			Push(sImageFill.x-1, sImageFill.y, FILL_RIGHT);
		}
		if (sImageFill.iFill & FILL_UP)
		{
			Push(sImageFill.x, sImageFill.y-1, FILL_DOWN);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageFiller::Push(int x, int y, size iRemove)
{
	SImageColour*	psColour;
	SImageFill*		psImageFill;
	short			iTested;

	if ((x < 0) || (x >= mpcSourceAccessor->GetWidth()) || (y < 0) || (y >= mpcSourceAccessor->GetImage()->GetHeight()))
	{
		return false;
	}

	iTested = *((short*)mpcDestMaskAccessor->Get(x, y));

	if (iTested != 0)
	{
		return false;
	}

	psColour = (SImageColour*)mpcSourceAccessor->Get(x, y);
	if (psColour->Equals(mpvBorderColour, mpcSourceAccessor->GetBufferSize()))
	{
		return false;
	}

	psImageFill = mcStack.Push();
	psImageFill->iFill = FILL_ALL;
	SetFlagSize(&psImageFill->iFill, iRemove, false);
	psImageFill->x = x;
	psImageFill->y = y;
	mpcDestMaskAccessor->Set(x, y, &miMask);
	mpcDestSubImage->GrowToContain(x, y);
	mpcDestSubImage->miFilled++;
	return true;
}

