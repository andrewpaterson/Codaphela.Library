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



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int ComparePackAttempt(const void* pv1, const void* pv2)
{
	CPackAttempt*	pcPack1 = (CPackAttempt*)pv1;
	CPackAttempt*	pcPack2 = (CPackAttempt*)pv2;

	if (!pcPack1->mbValid && !pcPack2->mbValid)
	{
		return 0;
	}
	if (!pcPack1->mbValid)
	{
		return 1;
	}
	if (!pcPack2->mbValid)
	{
		return -1;
	}

	if (pcPack1->miArea < pcPack2->miArea)
	{
		return -1;
	}
	if (pcPack1->miArea > pcPack2->miArea)
	{
		return 1;
	}

	if (pcPack1->miWidth > pcPack2->miWidth)
	{
		return -1;
	}
	if (pcPack1->miWidth  < pcPack2->miWidth)
	{
		return 1;
	}
	if (pcPack1->miHeight < pcPack2->miHeight)
	{
		return -1;
	}
	if (pcPack1->miHeight > pcPack2->miHeight)
	{
		return 1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackAttempt::Init(CRectangleBestPacker* pcRectanglePacker, int iMaxWidth, EPackRotation eRotation, bool bRotateToFit)
{
	macLines.Init();
	mpcRectanglePacker = pcRectanglePacker;
	miWidth = 0;
	miHeight = 0;
	miArea = 0;
	mbValid = false;
	miMaxWidth = iMaxWidth;
	meRotation = eRotation;
	mbRotateToFit = bRotateToFit;
	macRectangles.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackAttempt::Kill(void)
{
	size			i;
	CPackedLine*	pcLine;

	for (i = 0; i < macLines.NumElements(); i++)
	{
		pcLine = macLines.Get(i);
		pcLine->Kill();
	}
	macLines.Kill();

	macRectangles.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackAttempt::AddRectanglesFromPacker(void)
{
	size					i;
	size					iNumRectangles;
	CPackSourceRectangle*	pcPackedRect;
	CPackRectangleAttempt*	pcRectAttempt;

	iNumRectangles = mpcRectanglePacker->GetSourceRectangles()->NumElements();
	for (i = 0; i < iNumRectangles; i++)
	{
		pcPackedRect = mpcRectanglePacker->GetSourceRectangles()->Get(i);
		pcRectAttempt = macRectangles.Add();
		pcRectAttempt->Init(pcPackedRect, pcPackedRect->IsRotatedBy(meRotation));
	}

	//Sort from smallest to largest.  This is so that large rectangles can be removed from the end.
	macRectangles.QuickSort(&ComparePackRectangleAttempt);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackAttempt::Pack(void)
{
	size			i;
	CPackedLine*	pcLine;

	//The rectangles are taken from the packer here rather than in Init so that an attempt can be added before its rectangles.
	for (i = 0; i < macLines.NumElements(); i++)
	{
		pcLine = macLines.Get(i);
		pcLine->Kill();
	}
	macLines.Kill();
	macLines.Init();
	macRectangles.Kill();
	macRectangles.Init();
	miWidth = 0;
	miHeight = 0;
	miArea = 0;
	mbValid = false;
	AddRectanglesFromPacker();

	while (macRectangles.NumElements() > 0)
	{
		pcLine = macLines.Add();
		pcLine->Init();
		if (!pcLine->Pack(this))
		{
			mbValid = false;
			break;
		}
	}
	Done();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackAttempt::Done(void)
{
	size			i;
	CPackedLine*	pcLine;

	if (macRectangles.NumElements() > 0)
	{
		//Not actually done.  Bugger off.
		return;
	}

	if (macLines.NumElements() == 0)
	{
		//Nothing was packed.
		return;
	}

	for (i = 0; i < macLines.NumElements(); i++)
	{
		pcLine = macLines.Get(i);
		pcLine->Done();
	}

	macLines.QuickSort(&ComparePackedLine);

	pcLine = macLines.Get(0);
	miWidth = pcLine->miWidth;

	miHeight = 0;
	for (i = 0; i < macLines.NumElements(); i++)
	{
		pcLine = macLines.Get(i);
		pcLine->miy = miHeight;
		miHeight += pcLine->miHeight + mpcRectanglePacker->GetInnerEdgeWidth();
	}

	miHeight -= mpcRectanglePacker->GetInnerEdgeWidth();

	miArea = (int64)miHeight * (int64)miWidth;
	mbValid = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPackRectangleAttempt* CPackAttempt::GetLastRectangle(void)
{
	//This is also the largest rectangle.
	return macRectangles.Tail();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackAttempt::RemoveLastRectangle(void)
{
	macRectangles.RemoveTail();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPackAttempt::GetLargestRectangleFittingAndRemoveIt(CInt32Rect* pcBounds, CPackRectangleAttempt* pcDest)
{
	size					i;
	CPackRectangleAttempt*	pcRect;

	i = macRectangles.NumElements();
	if (i != 0)
	{
		do
		{
			i--;
			pcRect = macRectangles.Get(i);
			if ((pcBounds->GetWidth() >= pcRect->mcSourceRectangle.miWidth) &&
				(pcBounds->GetHeight() >= pcRect->mcSourceRectangle.miHeight))
			{
				*pcDest = *pcRect;
				macRectangles.RemoveAt(i, true);
				return true;
			}
			if (mbRotateToFit &&
				(pcBounds->GetWidth() >= pcRect->mcSourceRectangle.miHeight) &&
				(pcBounds->GetHeight() >= pcRect->mcSourceRectangle.miWidth))
			{
				*pcDest = *pcRect;
				pcDest->Rotate();
				macRectangles.RemoveAt(i, true);
				return true;
			}
		}
		while (i != 0);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CPackAttempt::GetWastedArea(void)
{
	return miArea - mpcRectanglePacker->GetSourceArea();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackAttempt::GetPackedRectangles(CArrayPackedRectangle* pacPackedRectangles)
{
	size			i;
	CPackedLine*	pcLine;

	for (i = 0; i < macLines.NumElements(); i++)
	{
		pcLine = macLines.Get(i);
		pcLine->GetPackedRectangles(pacPackedRectangles, mpcRectanglePacker->GetOutsideEdgeWidth());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CPackAttempt::GetPackedImageSize(void)
{
	SInt32Vec2	sSize;

	sSize.Init(miWidth + mpcRectanglePacker->GetOutsideEdgeWidth()*2, miHeight + mpcRectanglePacker->GetOutsideEdgeWidth()*2);
	return sSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackAttempt::Print(CChars* psz)
{
	size			i;
	CPackedLine*	pcLine;

	psz->Append("Lines[");
	psz->Append(macLines.NumElements());
	psz->Append("] Size[");
	psz->Append(miWidth);
	psz->Append(", ");
	psz->Append(miHeight);
	psz->Append("]\n");

	for (i = 0; i < macLines.NumElements(); i++)
	{
		pcLine = macLines.Get(i);
		pcLine->Print(psz);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackAttempt::Dump(void)
{
	CChars	sz;

	sz.Init();
	Print(&sz);
	sz.Dump();
	sz.Kill();
}

