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
#include "PackAttempt.h"
#include "RectangleBestPacker.h"
#include "PackedLine.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int ComparePackedLine(const void* pv1, const void* pv2)
{
	CPackedLine*	pcLine1 = (CPackedLine*)pv1;
	CPackedLine*	pcLine2 = (CPackedLine*)pv2;

	if (pcLine1->miWidth > pcLine2->miWidth)
	{
		return -1;
	}
	if (pcLine1->miWidth  < pcLine2->miWidth)
	{
		return 1;
	}
	if (pcLine1->miHeight > pcLine2->miHeight)
	{
		return -1;
	}
	if (pcLine1->miHeight < pcLine2->miHeight)
	{
		return 1;
	}
	return 0;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackedLine::Init(void)
{
	macRectangles.Init();
	miHeight = 0;
	miWidth = 0;
	miy = -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackedLine::Kill(void)
{
	macRectangles.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPackedLine::Pack(CPackAttempt* pcPackAttempt)
{
	CPackRectangleAttempt*	pcUnusedRect;
	CInt32Rect				cBounds;

	pcUnusedRect = pcPackAttempt->GetLastRectangle();
	cBounds.Init(0, 0, pcPackAttempt->miMaxWidth, pcUnusedRect->mcSourceRectangle.miHeight + pcPackAttempt->mpcRectanglePacker->GetInnerEdgeWidth());

	return RecursePack(pcPackAttempt, &cBounds);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPackedLine::RecursePack(CPackAttempt* pcPackAttempt, CInt32Rect* pcBounds)
{
	CInt32Rect				cBoundsRight;
	CInt32Rect				cBoundsBelow;
	bool					bAnyFit;
	CPackRectangleAttempt	cFits;
	CPackRectangleAttempt*	pcUsedRect;
	
	if (!pcBounds->IsValid())
	{
		return false;
	}

	bAnyFit = pcPackAttempt->GetLargestRectangleFittingAndRemoveIt(pcBounds, &cFits);
	if (!bAnyFit)
	{
		return false;
	}
	
	pcUsedRect = macRectangles.Add(&cFits);
	pcUsedRect->SetPos(pcBounds->msTopLeft.x, pcBounds->msTopLeft.y);

	cBoundsRight.Init(	pcBounds->msTopLeft.x + pcUsedRect->mcSourceRectangle.miWidth + pcPackAttempt->mpcRectanglePacker->GetInnerEdgeWidth(),
						pcBounds->msTopLeft.y, 
						pcBounds->msBottomRight.x, 
						pcBounds->msBottomRight.y);

	//The gap to the right of the used rectangle belongs to cBoundsRight, so cBoundsBelow is only as wide as the used rectangle.
	cBoundsBelow.Init(	pcBounds->msTopLeft.x,
						pcBounds->msTopLeft.y + pcUsedRect->mcSourceRectangle.miHeight + pcPackAttempt->mpcRectanglePacker->GetInnerEdgeWidth(), 
						pcBounds->msTopLeft.x + pcUsedRect->mcSourceRectangle.miWidth,
						pcBounds->msBottomRight.y);

	RecursePack(pcPackAttempt, &cBoundsBelow);
	RecursePack(pcPackAttempt, &cBoundsRight);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackedLine::Done(void)
{
	size					i;
	CPackRectangleAttempt*	pcRect;
	int						iTemp;

	miWidth = 0;
	miHeight = 0;
	for (i = 0; i < macRectangles.NumElements(); i++)
	{
		pcRect = macRectangles.Get(i);

		iTemp = pcRect->mix + pcRect->mcSourceRectangle.miWidth;
		if (iTemp > miWidth)
		{
			miWidth = iTemp;
		}

		iTemp = pcRect->miy + pcRect->mcSourceRectangle.miHeight;
		if (iTemp > miHeight)
		{
			miHeight = iTemp;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPackRectangleAttempt* CPackedLine::Get(void* pvUserData)
{
	size					i;
	CPackRectangleAttempt*	pcRect;

	for (i = 0; i < macRectangles.NumElements(); i++)
	{
		pcRect = macRectangles.Get(i);

		if (pcRect->mcSourceRectangle.mpvUserData == pvUserData)
		{
			return pcRect;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackedLine::GetPackedRectangles(CArrayPackedRectangle* pacPackedRectangles, int iOutsideEdgeWidth)
{
	size					i;
	CPackRectangleAttempt*	pcRect;
	CPackedRectangle*		pcPackedRect;
	int						iLeft;
	int						iTop;

	for (i = 0; i < macRectangles.NumElements(); i++)
	{
		pcRect = macRectangles.Get(i);
		pcPackedRect = pacPackedRectangles->Add();
		iLeft = pcRect->mix + iOutsideEdgeWidth;
		iTop = pcRect->miy + miy + iOutsideEdgeWidth;
		pcPackedRect->Init(	iLeft, 
							iTop, 
							iLeft + pcRect->mcSourceRectangle.miWidth, 
							iTop + pcRect->mcSourceRectangle.miHeight, 
							pcRect->mcSourceRectangle.mpvUserData,
							pcRect->mbRotated);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackedLine::Print(CChars* psz)
{
	size					i;
	CPackRectangleAttempt*	pcRect;

	for (i = 0; i < macRectangles.NumElements(); i++)
	{
		pcRect = macRectangles.Get(i);
		
		psz->Append("[(");
		psz->Append(pcRect->mix);
		psz->Append(", ");
		psz->Append(pcRect->miy);
		psz->Append(") ");
		psz->Append(pcRect->mcSourceRectangle.miWidth);
		psz->Append(":");
		psz->Append(pcRect->mcSourceRectangle.miHeight);
		if (pcRect->mbRotated)
		{
			psz->Append(" R");
		}
		psz->Append("]");
		if (i != macRectangles.NumElements()-1)
		{
			psz->Append(", ");
		}
	}
	psz->AppendNewLine();
}

