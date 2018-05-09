/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
void CRectangleLinePacker::Init(BOOL bLeftToRight, BOOL bPow2, int iOutsideEdgeWidth, int iInnerEdgeWidth)
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
BOOL CRectangleLinePacker::AddRectangle(CRectangle* pcRect, void* pvUserData)
{
	CPackedRectangle*	psPacked;

	if (!(pcRect->GetWidth() == 0))
	{
		psPacked = macRectangles.Add();
		psPacked->Init(pcRect, pvUserData);
		return TRUE;
	}
	else
	{
		//Don't feed the packer empty rectangles.
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CRectangleLinePacker::AddRectangle(CSubImage* pcSubImage)
{
	return AddRectangle(&pcSubImage->mcImageRect, pcSubImage);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CRectangleLinePacker::AddRectangle(CImageCel* pcImageCel)
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
SInt2 CRectangleLinePacker::GetPackedImageSize(void)
{
	return miSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CRectangleLinePacker::GetWidestRect(void)
{
	int				i;
	int				iMaxWidth;
	CRectangle*		pcRect;
	int				iWidth;

	iMaxWidth = 0;
	for (i = 0; i < macRectangles.NumElements(); i++)
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
	int				i;
	int				iMaxHeight;
	CRectangle*		pcRect;
	int				iHeight;

	iMaxHeight = 0;
	for (i = 0; i < macRectangles.NumElements(); i++)
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
	int				i;
	CRectangle*		pcRect;
	int				iLeft;

	iLeft = miOutsideEdgeWidth;
	for (i = 0; i < macRectangles.NumElements(); i++)
	{
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
	int				i;
	CRectangle*		pcRect;
	int				iTop;

	iTop = miOutsideEdgeWidth;
	for (i = 0; i < macRectangles.NumElements(); i++)
	{
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
BOOL CRectangleLinePacker::IsLeftToRight(void) {return mbLeftToRight;}
SInt2 CRectangleLinePacker::GetSize(void) {return miSize;}
BOOL CRectangleLinePacker::IsPow2(void) {return mbPow2;}
