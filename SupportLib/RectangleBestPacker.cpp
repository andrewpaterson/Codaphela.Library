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

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#include <math.h>
#include "BaseLib/DataTypes.h"
#include "BaseLib/FloatHelper.h"
#include "BaseLib/Int32Vec2.h"
#include "RectangleBestPacker.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRectangleBestPacker::Init(int iOutsideEdgeWidth, int iInnerEdgeWidth)
{
	masSourceRectangles.Init();
	miOutsideEdgeWidth = iOutsideEdgeWidth;
	miInnerEdgeWidth = iInnerEdgeWidth;
	macPackAttempts.Init();
	miSourceArea = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRectangleBestPacker::Kill(void)
{
	size			i;
	CPackAttempt*	pcAttempt;
	size			uiNumElements;

	uiNumElements = macPackAttempts.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcAttempt = macPackAttempts.Get(i);
		pcAttempt->Kill();
	}
	macPackAttempts.Kill();
	masSourceRectangles.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRectangleBestPacker::AddRectangle(CInt32Rect* pcRect, void* pvUserData)
{
	CPackSourceRectangle*	psPacked;

	if ((pcRect->GetWidth() > 0) && (pcRect->GetHeight() > 0))
	{
		psPacked = masSourceRectangles.Add();
		psPacked->Init(pcRect, pvUserData);
		miSourceArea += pcRect->GetArea();
		return true;
	}
	else
	{
		//Don't feed the packer empty rectangles.
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRectangleBestPacker::AddRectangle(CSubImage* pcSubImage)
{
	return AddRectangle(&pcSubImage->mcImageRect, pcSubImage);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRectangleBestPacker::AddRectangle(CImageCel* pcImageCel)
{
	return AddRectangle(&pcImageCel->GetSubImage()->mcImageRect, pcImageCel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRectangleBestPacker::Pack(void)
{
	size			i;
	CPackAttempt*	pcAttempt;
	size			uiNumElements;

	if (masSourceRectangles.NumElements() == 0)
	{
		return;
	}

	if (macPackAttempts.NumElements() == 0)
	{
		AddDefaultAttempts();
	}

	uiNumElements = macPackAttempts.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcAttempt = macPackAttempts.Get(i);
		pcAttempt->Pack();
	}

	//Sort of try and put the best attempt at the front of the list.
	macPackAttempts.BubbleSort(&ComparePackAttempt);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CRectangleBestPacker::GetSourcesMaxWidth(EPackRotation eRotation)
{
	size					i;
	CPackSourceRectangle*	pcRect;
	int						iWidth;
	int						iHeight;
	int						iTotalWidth;
	size					uiNumElements;

	//The width of every rectangle in a single row.
	if (masSourceRectangles.NumElements() == 0)
	{
		return 0;
	}

	iTotalWidth = 0;
	uiNumElements = masSourceRectangles.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcRect = masSourceRectangles.Get(i);
		pcRect->GetSize(eRotation, &iWidth, &iHeight);
		iTotalWidth += iWidth;
	}
	iTotalWidth += ((int)masSourceRectangles.NumElements() - 1) * miInnerEdgeWidth;

	return iTotalWidth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CRectangleBestPacker::GetSourcesMaxWidth(void)
{
	return GetSourcesMaxWidth(PR_AsGiven);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CRectangleBestPacker::GetSourcesMinWidth(void)
{
	return GetSourcesMinWidth(PR_AsGiven, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CRectangleBestPacker::GetSourcesMinWidth(EPackRotation eRotation, bool bRotateToFit)
{
	size					i;
	CPackSourceRectangle*	pcRect;
	int						iMaxWidth;
	int						iWidth;
	int						iHeight;
	size					uiNumElements;

	//The narrowest attempt that can hold every rectangle is as wide as the widest rectangle.  Turning to fit lets a rectangle use its shorter side.
	iMaxWidth = 0;
	uiNumElements = masSourceRectangles.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcRect = masSourceRectangles.Get(i);
		pcRect->GetSize(eRotation, &iWidth, &iHeight);
		if (bRotateToFit && (iHeight < iWidth))
		{
			iWidth = iHeight;
		}
		if (iWidth > iMaxWidth)
		{
			iMaxWidth = iWidth;
		}
	}
	return iMaxWidth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRectangleBestPacker::AddAttempt(int iMaxWidth, EPackRotation eRotation, bool bRotateToFit)
{
	CPackAttempt*	pcAttempt;

	pcAttempt = macPackAttempts.Add();
	pcAttempt->Init(this, iMaxWidth, eRotation, bRotateToFit);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRectangleBestPacker::AddHorizontalAttempt(void)
{
	AddAttempt(GetSourcesMaxWidth());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRectangleBestPacker::AddVerticalAttempt(void)
{
	AddAttempt(GetSourcesMinWidth());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRectangleBestPacker::AddSquareAttempt(void)
{
	size					i;
	CPackSourceRectangle*	pcRect;
	float32					fArea;
	int						iMaxWidth;
	int						iMinWidth;
	size					uiNumElements;

	//Aim for a square: the side of a square with the area of the rectangles and the gaps after them.
	fArea = 0.0f;
	uiNumElements = masSourceRectangles.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcRect = masSourceRectangles.Get(i);
		fArea += (float32)(pcRect->miWidth + miInnerEdgeWidth) * (float32)(pcRect->miHeight + miInnerEdgeWidth);
	}
	iMaxWidth = (int)ceilf(sqrtf(fArea)) - miInnerEdgeWidth;

	iMinWidth = GetSourcesMinWidth();
	if (iMaxWidth < iMinWidth)
	{
		iMaxWidth = iMinWidth;
	}

	AddAttempt(iMaxWidth);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRectangleBestPacker::AddDefaultAttempts(void)
{
	AddWidthAttempts(PR_AsGiven, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRectangleBestPacker::AddWidthAttempts(EPackRotation eRotation, bool bRotateToFit)
{
	int						i;
	float32					fStartWidth;
	float32					fEndWidth;
	int						iLastWidth;
	int						iWidth;
	float32					fDest;
	float32					fPos;

	fStartWidth = (float32)GetSourcesMinWidth(eRotation, bRotateToFit);
	fEndWidth = (float32)GetSourcesMaxWidth(eRotation);

	iLastWidth = -1;

	//InterpolateFloat goes from fEndWidth at 0 to fStartWidth at 1, so the attempts run from the single row width down to the widest rectangle, both included.
	for (i = 0; i < 25; i++)
	{
		fPos = i/24.0f;
		InterpolateFloat(&fDest, fStartWidth, fEndWidth, fPos);
		iWidth = (int)(fDest + 0.5f);
		if (iWidth != iLastWidth)
		{
			AddAttempt(iWidth, eRotation, bRotateToFit);
		}
		iLastWidth = iWidth;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPackAttempt* CRectangleBestPacker::GetBestAttempt(void)
{
	CPackAttempt*	pcAttempt;

	if (macPackAttempts.NumElements() > 0)
	{
		pcAttempt = macPackAttempts.Get(0);
		if (pcAttempt->mbValid)
		{
			return pcAttempt;
		}
		return NULL;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPackAttempt* CRectangleBestPacker::GetWorstAttempt(void)
{
	if (macPackAttempts.NumElements() > 0)
	{
		return macPackAttempts.Tail();
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRectangleBestPacker::GetPackedRectangles(CArrayPackedRectangle* pacPackedRectangles)
{
	CPackAttempt*	pcBest;

	pcBest = GetBestAttempt();
	if (pcBest)
	{
		pcBest->GetPackedRectangles(pacPackedRectangles);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CRectangleBestPacker::GetPackedImageSize(void)
{
	CPackAttempt*	pcBest;
	SInt32Vec2		sSize;

	pcBest = GetBestAttempt();
	if (pcBest)
	{
		return pcBest->GetPackedImageSize();
	}
	else
	{
		sSize.Zero();
		return sSize;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayPackSourceRectangle* CRectangleBestPacker::GetSourceRectangles(void) {return &masSourceRectangles;}
int CRectangleBestPacker::GetOutsideEdgeWidth(void) {return miOutsideEdgeWidth;}
int CRectangleBestPacker::GetInnerEdgeWidth(void) {return miInnerEdgeWidth;}
CArrayPackAttempt* CRectangleBestPacker::GetPackAttempts(void) {return &macPackAttempts;}
int64 CRectangleBestPacker::GetSourceArea(void) {return miSourceArea;}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRectangleBestPacker::Dump(void)
{
}

