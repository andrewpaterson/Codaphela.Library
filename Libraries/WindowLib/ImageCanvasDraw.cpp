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
#include "Canvas.h"
#include "ImageCanvasDraw.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCanvasDraw::Init(Ptr<CImageCel> pCel, bool bStretch)
{
	PreInit();
	CBlitCanvasDraw::Init();
	mpCel = pCel;
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
	CBlitCanvasDraw::Class();
	M_Pointer(mpCel);
	U_Bool(mbStretch);
	U_Enum(meHorizontal);
	U_Enum(meVertical);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageCanvasDraw::Save(CObjectWriter* pcFile)
{
	CBlitCanvasDraw::Save(pcFile);
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageCanvasDraw::Load(CObjectReader* pcFile)
{
	CBlitCanvasDraw::Load(pcFile);
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageCanvasDraw::Draw(Ptr<CCanvas> pCanvas)
{
	SInt32Vec2			sSize;
	SInt32Vec2			sCelSize;
	SInt32Vec2			sOrigin;
	bool				bResult;
	Ptr<CImage>			pImage;
	Ptr<CImageBlitter>	pBlitter;

	if (!pCanvas->IsValid() || mpCel.IsNull())
	{
		return true;
	}
	pImage = StartBlit(pCanvas);
	if (pImage.IsNull())
	{
		return true;
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

	//The image holds the part of the canvas from its native origin.
	sSize = pCanvas->GetActualSize();
	sOrigin = pCanvas->GetNativeOrigin();
	if (mbStretch)
	{
		bResult = pBlitter->Blit(-sOrigin.x, -sOrigin.y, sSize.x, sSize.y);
	}
	else
	{
		sCelSize = GetDesiredSize();
		bResult = pBlitter->Blit(AlignComponentHorizontal(meHorizontal, sCelSize.x, sSize.x) - sOrigin.x, AlignComponentVertical(meVertical, sCelSize.y, sSize.y) - sOrigin.y);
	}

	FinishBlit(pCanvas);
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

