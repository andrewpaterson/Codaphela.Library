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
void CPackedLine::Init(CPackAttempt* pcPackAttempt)
{
	mpcPackAttempt = pcPackAttempt;
	macRectangles.Init(16);
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
BOOL CPackedLine::Pack(void)
{
	CPackRectangleAttempt*	pcUnusedRect;
	CRectangle				cBounds;

	pcUnusedRect = mpcPackAttempt->GetLastRectangle();
	cBounds.Init(0, 0, mpcPackAttempt->miMaxWidth, pcUnusedRect->mpcPackedRectangle->miHeight + mpcPackAttempt->mpcRectanglePacker->GetInnerEdgeWidth());

	return RecursePack(&cBounds);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPackedLine::RecursePack(CRectangle* pcBounds)
{
	CRectangle				cBoundsRight;
	CRectangle				cBoundsBelow;
	BOOL					bAnyFit;
	CPackRectangleAttempt	cFits;
	CPackRectangleAttempt*	pcUsedRect;
	
	if (!pcBounds->IsValid())
	{
		return FALSE;
	}

	bAnyFit = mpcPackAttempt->GetLargestRectangleFittingAndRemoveIt(pcBounds, &cFits);
	if (!bAnyFit)
	{
		return FALSE;
	}
	
	pcUsedRect = macRectangles.Add(&cFits);
	pcUsedRect->Set(this, pcBounds->miLeft, pcBounds->miTop);

	cBoundsRight.Init(	pcBounds->miLeft + pcUsedRect->mpcPackedRectangle->miWidth + mpcPackAttempt->mpcRectanglePacker->GetInnerEdgeWidth(),
						pcBounds->miTop, 
						pcBounds->miRight, 
						pcBounds->miBottom);

	cBoundsBelow.Init(	pcBounds->miLeft,
						pcBounds->miTop + pcUsedRect->mpcPackedRectangle->miHeight + mpcPackAttempt->mpcRectanglePacker->GetInnerEdgeWidth(), 
						pcBounds->miLeft + pcUsedRect->mpcPackedRectangle->miWidth + mpcPackAttempt->mpcRectanglePacker->GetInnerEdgeWidth(),
						pcBounds->miBottom);

	RecursePack(&cBoundsBelow);
	RecursePack(&cBoundsRight);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackedLine::Done(void)
{
	int						i;
	CPackRectangleAttempt*	pcRect;
	int						iTemp;

	miWidth = 0;
	miHeight = 0;
	for (i = 0; i < macRectangles.NumElements(); i++)
	{
		pcRect = macRectangles.Get(i);

		iTemp = pcRect->mix + pcRect->mpcPackedRectangle->miWidth;
		if (iTemp > miWidth)
		{
			miWidth = iTemp;
		}

		iTemp = pcRect->miy + pcRect->mpcPackedRectangle->miHeight;
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
	int						i;
	CPackRectangleAttempt*	pcRect;

	for (i = 0; i < macRectangles.NumElements(); i++)
	{
		pcRect = macRectangles.Get(i);

		if (pcRect->mpcPackedRectangle->mpvUserData == pvUserData)
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
void CPackedLine::GetPackedRectangles(CArrayPackedRectangle* pacPackedRectangles)
{
	int						i;
	CPackRectangleAttempt*	pcRect;
	CPackedRectangle*		pcPackedRect;
	int						iLeft;
	int						iTop;

	for (i = 0; i < macRectangles.NumElements(); i++)
	{
		pcRect = macRectangles.Get(i);
		pcPackedRect = pacPackedRectangles->Add();
		iLeft = pcRect->mix + mpcPackAttempt->mpcRectanglePacker->GetOutsideEdgeWidth();
		iTop = pcRect->miy + miy + mpcPackAttempt->mpcRectanglePacker->GetOutsideEdgeWidth();
		pcPackedRect->Init(	iLeft, 
							iTop, 
							iLeft + pcRect->mpcPackedRectangle->miWidth, 
							iTop + pcRect->mpcPackedRectangle->miHeight, 
							pcRect->mpcPackedRectangle->mpvUserData);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackedLine::ToString(CChars* psz)
{
	int						i;
	CPackRectangleAttempt*	pcRect;

	for (i = 0; i < macRectangles.NumElements(); i++)
	{
		pcRect = macRectangles.Get(i);
		
		psz->Append("[(");
		psz->Append(pcRect->mix);
		psz->Append(", ");
		psz->Append(pcRect->miy);
		psz->Append(") ");
		psz->Append(pcRect->mpcPackedRectangle->miWidth);
		psz->Append(":");
		psz->Append(pcRect->mpcPackedRectangle->miHeight);
		psz->Append("]");
		if (i != macRectangles.NumElements()-1)
		{
			psz->Append(", ");
		}
	}
	psz->AppendNewLine();
}

