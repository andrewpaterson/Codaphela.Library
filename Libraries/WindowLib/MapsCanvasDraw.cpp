/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "StandardLib/ClassDefines.h"
#include "Canvas.h"
#include "MapsCanvasDraw.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapsCanvasDraw::Init(Ptr<CMaps> pMaps)
{
	PreInit();
	CCanvasDraw::Init();
	mpMaps = pMaps;
	mpDestImage = NULL;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapsCanvasDraw::Class(void)
{
	CCanvasDraw::Class();
	M_Pointer(mpMaps);
	M_Pointer(mpDestImage);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapsCanvasDraw::Free(void)
{
	CCanvasDraw::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapsCanvasDraw::Save(CObjectWriter* pcFile)
{
	CCanvasDraw::Save(pcFile);
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapsCanvasDraw::Load(CObjectReader* pcFile)
{
	CCanvasDraw::Load(pcFile);
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapsCanvasDraw::Draw(Ptr<CCanvas> pCanvas)
{
	SInt32Vec2      sSize;
	bool            bResult;
	Ptr<CImage>     pImage;
	
	if (pCanvas->IsValid())
	{
		sSize = pCanvas->GetActualSize();
		pImage = pCanvas->GetImageOrNull();
		if (pImage.IsNull())
		{
			if (mpDestImage.IsNull() || (mpDestImage->GetWidth() != (uint32)sSize.x) || (mpDestImage->GetHeight() != (uint32)sSize.y) ||
				(mpDestImage->GetColourFormat() != pCanvas->GetColourFormat()) || (mpDestImage->GetAlphaBits() != pCanvas->GetAlphaBits()))
			{
				mpDestImage = OMalloc<CImage>(sSize.x, sSize.y, pCanvas->GetColourFormat(), pCanvas->GetColourOrder(), pCanvas->GetColourBits(), pCanvas->GetAlphaBits());
				mpDestImage->Black();  //A native canvas starts black so this image must too.
			}
			else if (pCanvas->IsTransparent())
			{
				//Like the transparent canvas this image is copied onto, it must start each draw empty.
				mpDestImage->Black();
			}
			pImage = mpDestImage;
		}
		else
		{
			if (mpDestImage.IsNotNull())
			{
				mpDestImage = NULL;
			}
		}

		//The image being drawn onto keeps the blitters that draw onto it, so a new image starts with new blitters.
		mpMaps->SetCacheAndViewport(pImage->GetBlitterCache(), pImage);

		if (!mpMaps->HasCelBlitters())
		{
			bResult = mpMaps->CreateCelBlitters();
		}
		else
		{
			bResult = true;
		}
		if (bResult)
		{
			bResult = mpMaps->Blit(false);
		}

		if (mpDestImage.IsNotNull())
		{
			pCanvas->DrawImage(0, 0, mpDestImage);
		}
	}
	return true;
}

