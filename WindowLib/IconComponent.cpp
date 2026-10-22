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
#include "StandardLib/Objects.h"
#include "SupportLib/SubImage.h"
#include "IconComponent.h"


//////////////////////////////////////////////////////////////////////////
//
// pCel can be NULL, the canvas then shows nothing but still takes its
// fixed size.  -1 on an axis takes the cel's size on that axis.
//
//////////////////////////////////////////////////////////////////////////
void CIconComponent::Init(Ptr<CWindow> pWindow, Ptr<CImageCel> pCel, int32 iWidth, int32 iHeight)
{
	PreInit();

	mpCelDraw = OMalloc<CImageCanvasDraw>(pCel);
	mpCelDraw->SetCelAlignment(CSH_Center, CSV_Center);
	CCanvas::Init(pWindow, mpCelDraw);
	SetTransparent(true);
	SetDesiredSize(iWidth, iHeight);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIconComponent::Class(void)
{
	CCanvas::Class();
	M_Pointer(mpCelDraw);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIconComponent::Free(void)
{
	CCanvas::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIconComponent::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIconComponent::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Changing the cel only changes the desired size on an axis that isn't
// fixed.
//
//////////////////////////////////////////////////////////////////////////
void CIconComponent::SetCel(Ptr<CImageCel> pCel)
{
	mpCelDraw->SetCel(pCel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIconComponent::SetCelAlignment(EContainerStyleHorizontal eHorizontal, EContainerStyleVertical eVertical)
{
	mpCelDraw->SetCelAlignment(eHorizontal, eVertical);
}


//////////////////////////////////////////////////////////////////////////
//
// Unlike a plain canvas each axis is worked out on its own, so a fixed
// width doesn't make the height variable.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CIconComponent::GetDesiredSize(void)
{
	SInt32Vec2		sSize;
	Ptr<CImageCel>	pCel;
	CSubImage*		pcSubImage;
	int32			iCelWidth;
	int32			iCelHeight;

	pCel = mpCelDraw->GetCel();
	if (pCel.IsNotNull())
	{
		pcSubImage = pCel->GetSubImage();
		iCelWidth = pcSubImage->GetFullWidth();
		iCelHeight = pcSubImage->GetFullHeight();
	}
	else
	{
		iCelWidth = 0;
		iCelHeight = 0;
	}

	sSize.x = msDesiredSize.x >= 0 ? msDesiredSize.x : iCelWidth;
	sSize.y = msDesiredSize.y >= 0 ? msDesiredSize.y : iCelHeight;
	return sSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CIconComponent::GetCel(void) { return mpCelDraw->GetCel(); }

