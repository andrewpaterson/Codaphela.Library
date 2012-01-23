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
#include <math.h>
#include "BaseLib/DataTypes.h"
#include "BaseLib/FloatHelper.h"
#include "RectangleBestPacker.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRectangleBestPacker::Init(int iOutsideEdgeWidth, int iInnerEdgeWidth)
{
	masSourceRectangles.Init(32);
	miOutsideEdgeWidth = iOutsideEdgeWidth;
	miInnerEdgeWidth = iInnerEdgeWidth;
	macPackAttempts.Init(8);
	miSourceArea = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRectangleBestPacker::Kill(void)
{
	int				i;
	CPackAttempt*	pcAttempt;

	for (i = 0; i < macPackAttempts.NumElements(); i++)
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
BOOL CRectangleBestPacker::AddRectangle(CRectangle* pcRect, void* pvUserData)
{
	CPackSourceRectangle*	psPacked;

	if (!(pcRect->GetWidth() == 0))
	{
		psPacked = masSourceRectangles.Add();
		psPacked->Init(pcRect, pvUserData);
		miSourceArea += pcRect->GetArea();
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
BOOL CRectangleBestPacker::AddRectangle(CSubImage* pcSubImage)
{
	return AddRectangle(&pcSubImage->mcImageRect, pcSubImage);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CRectangleBestPacker::AddRectangle(CImageCel* pcImageCel)
{
	return AddRectangle(&pcImageCel->GetSubImage()->mcImageRect, pcImageCel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRectangleBestPacker::Pack(void)
{
	int				i;
	CPackAttempt*	pcAttempt;

	if (macPackAttempts.NumElements() == 0)
	{
		AddDefaultAttempts();
	}

	for (i = 0; i < macPackAttempts.NumElements(); i++)
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
SInt2 CRectangleBestPacker::GetSourcesSizes(void)
{
	int						i;
	CPackSourceRectangle*	pcRect;
	SInt2					sSize;

	sSize.Zero();
	for (i = 0; i < masSourceRectangles.NumElements(); i++)
	{
		pcRect = masSourceRectangles.Get(i);
		sSize.x += pcRect->miWidth;
		sSize.y += pcRect->miHeight;
	}
	sSize.x += (masSourceRectangles.NumElements() - 1) * miInnerEdgeWidth ;
	sSize.y += (masSourceRectangles.NumElements() - 1) * miInnerEdgeWidth ;

	return sSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CRectangleBestPacker::GetSourcesMaxWidth(void)
{
	SInt2			sSize;

	sSize = GetSourcesSizes();
	return sSize.x;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CRectangleBestPacker::GetSourcesMinWidth(void)
{
	CPackSourceRectangle*	pcRect;

	pcRect = masSourceRectangles.Get(0);
	return pcRect->miWidth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRectangleBestPacker::AddAttempt(int iMaxWidth)
{
	CPackAttempt*	pcAttempt;
	SInt2			sSize;

	sSize = GetSourcesSizes();

	pcAttempt = macPackAttempts.Add();
	pcAttempt->Init(this, iMaxWidth);
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
	CPackAttempt*	pcAttempt;
	SInt2			sSize;
	int				iMaxWidth;

	sSize = GetSourcesSizes();
	iMaxWidth = (int)(ceilf((sSize.x + sSize.y)/4.0f))-1;

	pcAttempt = macPackAttempts.Add();
	pcAttempt->Init(this, iMaxWidth);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRectangleBestPacker::AddDefaultAttempts(void)
{
	int						i;
	CPackAttempt*			pcAttempt;
	float					fStartWidth;
	float					fEndWidth;
	int						iLastWidth;
	int						iWidth;
	float					fDest;
	float					fPos;

	fStartWidth = (float)GetSourcesMinWidth();
	fEndWidth = (float)GetSourcesMaxWidth();

	iLastWidth = -1;

	for (i = 0; i < 25; i++)
	{
		fPos = i/25.0f;
		InterpolateFloat(&fDest, fStartWidth, fEndWidth, fPos);
		iWidth = (int)(fDest + 0.5f);
		if (iWidth != iLastWidth)
		{
			pcAttempt = macPackAttempts.Add();
			pcAttempt->Init(this, iWidth);
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
	pcBest->GetPackedRectangles(pacPackedRectangles);

	
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt2 CRectangleBestPacker::GetPackedImageSize(void)
{
	CPackAttempt*	pcBest;

	pcBest = GetBestAttempt();
	return pcBest->GetPackedImageSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayPackSourceRectangle* CRectangleBestPacker::GetSourceRectangles(void) {return &masSourceRectangles;}
int CRectangleBestPacker::GetOutsideEdgeWidth(void) {return miOutsideEdgeWidth;}
int CRectangleBestPacker::GetInnerEdgeWidth(void) {return miInnerEdgeWidth;}
CArrayPackAttempt* CRectangleBestPacker::GetPackAttempts(void) {return &macPackAttempts;}
int CRectangleBestPacker::GetSourceArea(void) {return miSourceArea;}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRectangleBestPacker::Dump(void)
{
}

