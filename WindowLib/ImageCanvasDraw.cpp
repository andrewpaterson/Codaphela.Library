/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "WindowLib/Canvas.h"
#include "ImageCanvasDraw.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCanvasDraw::Init(Ptr<CImageCel> pCel)
{
	PreInit();
	CCanvasDraw::Init();
	mpCel = pCel;
	mpDestImage = NULL;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCanvasDraw::Class(void)
{
	CCanvasDraw::Class();
	M_Pointer(mpCel);
	M_Pointer(mpDestImage);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCanvasDraw::Free(void)
{
	CCanvasDraw::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageCanvasDraw::Save(CObjectWriter* pcFile)
{
	CCanvasDraw::Save(pcFile);
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageCanvasDraw::Load(CObjectReader* pcFile)
{
	CCanvasDraw::Load(pcFile);
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageCanvasDraw::Draw(Ptr<CCanvas> pCanvas)
{
	SInt32Vec2		sSize;
	bool			bResult;
	Ptr<CImage>		pImage;
	Ptr<CImageBlitter>	pBlitter;

	if (!pCanvas->IsValid() || mpCel.IsNull())
	{
		return true;
	}

	sSize = pCanvas->GetActualSize();
	if ((sSize.x <= 0) || (sSize.y <= 0))
	{
		return true;
	}

	//Blit straight into the canvas's image if it has one, otherwise into a temporary image the size of the canvas.
	pImage = pCanvas->GetImageOrNull();
	if (pImage.IsNull())
	{
		if (mpDestImage.IsNull() || (mpDestImage->GetWidth() != sSize.x) || (mpDestImage->GetHeight() != sSize.y))
		{
			mpDestImage = OMalloc<CImage>(sSize.x, sSize.y, pCanvas->GetColourFormat(), pCanvas->GetColourOrder(), pCanvas->GetColourBits(), pCanvas->GetAlphaBits());
			mpDestImage->Black();  //A native canvas starts black so this image must too.
		}
		pImage = mpDestImage;
	}
	else
	{
		mpDestImage = NULL;
	}

	//The image being drawn onto keeps the blitters that draw onto it, so a new image starts with new blitters.
	pBlitter = pImage->GetBlitterCache()->CreateImageBlitter(mpCel);
	if (pBlitter.IsNull())
	{
		return false;
	}

	bResult = pBlitter->Blit(0, 0);

	if (mpDestImage.IsNotNull())
	{
		pCanvas->DrawImage(0, 0, mpDestImage);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CImageCanvasDraw::GetDesiredSize(void)
{
	SInt32Vec2	sSize;
	CSubImage*	pcSubImage;

	if (mpCel.IsNull())
	{
		return CCanvasDraw::GetDesiredSize();
	}

	pcSubImage = mpCel->GetSubImage();
	sSize.Init(pcSubImage->GetFullWidth(), pcSubImage->GetFullHeight());
	return sSize;
}

