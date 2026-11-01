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
#include "RectanglePow2Packer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRectanglePow2Packer::Init(int32 iOutsideEdgeWidth, int32 iInnerEdgeWidth)
{
	mcPacker.Init(iOutsideEdgeWidth, iInnerEdgeWidth);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRectanglePow2Packer::Kill(void)
{
	mcPacker.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRectanglePow2Packer::AddRectangle(CInt32Rect* pcRect, void* pvUserData)
{
	return mcPacker.AddRectangle(pcRect, pvUserData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRectanglePow2Packer::AddRectangle(CSubImage* pcSubImage)
{
	return mcPacker.AddRectangle(pcSubImage);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRectanglePow2Packer::AddRectangle(CImageCel* pcImageCel)
{
	return mcPacker.AddRectangle(pcImageCel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRectanglePow2Packer::Pack(void)
{
	int32	iStartWidth;
	int32	iEndWidth;
	uint32	iWidth;

	if (mcPacker.GetSourceRectangles()->NumElements() == 0)
	{
		return;
	}

	iStartWidth = mcPacker.GetSourcesMinWidth();
	iEndWidth = mcPacker.GetSourcesMaxWidth();

	if (iEndWidth > 0)
	{
		iStartWidth += mcPacker.GetOutsideEdgeWidth()*2;
		iEndWidth += mcPacker.GetOutsideEdgeWidth()*2;

		iStartWidth = GetPowerOf2SizeUp(iStartWidth);
		iEndWidth = GetPowerOf2SizeUp(iEndWidth);

		for (iWidth = (uint32)iStartWidth; iWidth <= (uint32)iEndWidth; iWidth *= 2)
		{
			mcPacker.AddAttempt((int32)iWidth-mcPacker.GetOutsideEdgeWidth()*2);
		}

		mcPacker.Pack();
		AdjustAttemptAreas();
		mcPacker.GetPackAttempts()->BubbleSort(&ComparePackAttempt);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRectanglePow2Packer::AdjustAttemptAreas(void)
{
	size			i;
	CPackAttempt*	pcAttempt;
	size			iNumAttempts;
	SInt32Vec2		sSize;

	//Only the area changes so that the attempts can be compared by the size of their power of 2 images.  The width and height stay the packed size.
	iNumAttempts = mcPacker.GetPackAttempts()->NumElements();
	for (i = 0; i < iNumAttempts; i++)
	{
		pcAttempt = mcPacker.GetPackAttempts()->Get(i);
		if (pcAttempt->mbValid)
		{
			sSize = GetPowerOf2Size(pcAttempt);
			pcAttempt->miArea = (int64)sSize.x * (int64)sSize.y;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CRectanglePow2Packer::GetPowerOf2Size(CPackAttempt* pcAttempt)
{
	SInt32Vec2	sSize;

	sSize = pcAttempt->GetPackedImageSize();
	sSize.Init(GetPowerOf2SizeUp(sSize.x), GetPowerOf2SizeUp(sSize.y));
	return sSize;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRectanglePow2Packer::GetPackedRectangles(CArrayPackedRectangle* pacPackedRectangles)
{
	CPackAttempt*	pcBest;

	pcBest = mcPacker.GetBestAttempt();
	if (pcBest)
	{
		pcBest->GetPackedRectangles(pacPackedRectangles);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CRectanglePow2Packer::GetPackedImageSize(void)
{
	SInt32Vec2			sSize;
	CPackAttempt*	pcBest;

	pcBest = mcPacker.GetBestAttempt();
	if (pcBest)
	{
		sSize = GetPowerOf2Size(pcBest);
	}
	else
	{
		sSize.Zero();
	}
	return sSize;
}
