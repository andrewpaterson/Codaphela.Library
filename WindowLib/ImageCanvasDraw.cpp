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
void CImageCanvasDraw::Init(Ptr<CImageCel> pCel, bool bStretch)
{
	PreInit();
	CCanvasDraw::Init();
	mpCel = pCel;
	mpDestImage = NULL;
	mbStretch = bStretch;
	meHorizontal = CSH_Left;
	meVertical = CSV_Top;
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
	U_Bool(mbStretch);
	U_Enum(meHorizontal);
	U_Enum(meVertical);
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
	SInt32Vec2		sCelSize;
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
		if (mpDestImage.IsNull() || (mpDestImage->GetWidth() != (uint32)sSize.x) || (mpDestImage->GetHeight() != (uint32)sSize.y))
		{
			mpDestImage = OMalloc<CImage>(sSize.x, sSize.y, pCanvas->GetColourFormat(), pCanvas->GetColourOrder(), pCanvas->GetColourBits(), pCanvas->GetAlphaBits());
			mpDestImage->Black();  //A native canvas starts black so this image must too.
		}
		else if (pCanvas->IsTransparent())
		{
			//The whole image is copied onto the canvas, so what an earlier cel left outside this one must go.
			mpDestImage->Black();
		}
		pImage = mpDestImage;
	}
	else
	{
		mpDestImage = NULL;
	}

	//The image being drawn onto keeps the blitters that draw onto it, so a new image starts with new blitters.
	if (mbStretch)
	{
		pBlitter = pImage->GetBlitterCache()->CreateSizingImageBlitter(mpCel);
	}
	else
	{
		pBlitter = pImage->GetBlitterCache()->CreateImageBlitter(mpCel);
	}
	if (pBlitter.IsNull())
	{
		return false;
	}

	if (mbStretch)
	{
		bResult = pBlitter->Blit(0, 0, sSize.x, sSize.y);
	}
	else
	{
		sCelSize = GetDesiredSize();
		bResult = pBlitter->Blit(AlignComponentHorizontal(meHorizontal, sCelSize.x, sSize.x), AlignComponentVertical(meVertical, sCelSize.y, sSize.y));
	}

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
void CImageCanvasDraw::SetCel(Ptr<CImageCel> pCel)
{
	mpCel = pCel;
}


//////////////////////////////////////////////////////////////////////////
//
// Left and top, the default, draw the cel at the canvas's top left.
//
//////////////////////////////////////////////////////////////////////////
void CImageCanvasDraw::SetCelAlignment(EContainerStyleHorizontal eHorizontal, EContainerStyleVertical eVertical)
{
	meHorizontal = eHorizontal;
	meVertical = eVertical;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CImageCanvasDraw::GetCel(void)
{
	return mpCel;
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

