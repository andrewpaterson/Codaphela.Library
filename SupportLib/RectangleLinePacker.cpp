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
#include "RectangleLinePacker.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRectangleLinePacker::Init(bool bLeftToRight, bool bPow2, int iOutsideEdgeWidth, int iInnerEdgeWidth)
{
	miOutsideEdgeWidth = iOutsideEdgeWidth;
	miInnerEdgeWidth = iInnerEdgeWidth;
	macRectangles.Init();
	mbLeftToRight = bLeftToRight;
	miSize.Init(-1, -1);
	mbPow2 = bPow2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRectangleLinePacker::Kill(void)
{
	macRectangles.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRectangleLinePacker::Pack(void)
{
	if (macRectangles.NumElements() == 0)
	{
		//Nothing to pack, the same as the other packers.
		miSize.Zero();
		return;
	}

	if (mbLeftToRight)
	{
		miSize.y = GetHighestRect() + miOutsideEdgeWidth*2;
		miSize.x = PackLeftToRight();
	}
	else
	{
		miSize.x = GetWidestRect() + miOutsideEdgeWidth*2;
		miSize.y = PackTopToBottom();
	}

	if (mbPow2)
	{
		miSize.x = GetPowerOf2SizeUp(miSize.x);
		miSize.y = GetPowerOf2SizeUp(miSize.y);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRectangleLinePacker::AddRectangle(CInt32Rect* pcRect, void* pvUserData)
{
	CPackedRectangle*	psPacked;

	if ((pcRect->GetWidth() > 0) && (pcRect->GetHeight() > 0))
	{
		psPacked = macRectangles.Add();
		psPacked->Init(pcRect, pvUserData);
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
bool CRectangleLinePacker::AddRectangle(CSubImage* pcSubImage)
{
	return AddRectangle(&pcSubImage->mcImageRect, pcSubImage);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRectangleLinePacker::AddRectangle(CImageCel* pcImageCel)
{
	return AddRectangle(&pcImageCel->GetSubImage()->mcImageRect, pcImageCel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRectangleLinePacker::GetPackedRectangles(CArrayPackedRectangle* pacPackedRectangles)
{
	pacPackedRectangles->InsertArrayAfterEnd(&macRectangles);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CRectangleLinePacker::GetPackedImageSize(void)
{
	return miSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CRectangleLinePacker::GetWidestRect(void)
{
	size			i;
	int				iMaxWidth;
	CInt32Rect*		pcRect;
	int				iWidth;
	size			uiNumElements;

	iMaxWidth = 0;
	uiNumElements = macRectangles.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcRect = macRectangles.Get(i);
		iWidth = pcRect->GetWidth();

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
int CRectangleLinePacker::GetHighestRect(void)
{
	size			i;
	int				iMaxHeight;
	CInt32Rect*		pcRect;
	int				iHeight;
	size			uiNumElements;

	iMaxHeight = 0;
	uiNumElements = macRectangles.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcRect = macRectangles.Get(i);
		iHeight = pcRect->GetHeight();

		if (iHeight > iMaxHeight)
		{
			iMaxHeight = iHeight;
		}
	}
	return iMaxHeight;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CRectangleLinePacker::PackLeftToRight(void)
{
	size			i;
	CInt32Rect*		pcRect;
	int				iLeft;
	size			uiNumElements;

	iLeft = miOutsideEdgeWidth;
	uiNumElements = macRectangles.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		if (i != 0)
		{
			iLeft += miInnerEdgeWidth;
		}
		pcRect = macRectangles.Get(i);
		pcRect->SetPos(iLeft, miOutsideEdgeWidth);
		iLeft += pcRect->GetWidth();
	}
	return iLeft + miOutsideEdgeWidth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CRectangleLinePacker::PackTopToBottom(void)
{
	size			i;
	CInt32Rect*		pcRect;
	int				iTop;
	size			uiNumElements;

	iTop = miOutsideEdgeWidth;
	uiNumElements = macRectangles.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		if (i != 0)
		{
			iTop += miInnerEdgeWidth;
		}
		pcRect = macRectangles.Get(i);
		pcRect->SetPos(miOutsideEdgeWidth, iTop);
		iTop += pcRect->GetHeight();
	}
	return iTop + miOutsideEdgeWidth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayPackedRectangle* CRectangleLinePacker::GetRectangles(void) {return &macRectangles;}
int CRectangleLinePacker::GetOutsideEdgeWidth(void) {return miOutsideEdgeWidth;}
int CRectangleLinePacker::GetInnerEdgeWidth(void) {return miInnerEdgeWidth;}
bool CRectangleLinePacker::IsLeftToRight(void) {return mbLeftToRight;}
SInt32Vec2 CRectangleLinePacker::GetSize(void) {return miSize;}
bool CRectangleLinePacker::IsPow2(void) {return mbPow2;}
