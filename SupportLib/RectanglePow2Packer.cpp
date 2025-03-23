/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
void CRectanglePow2Packer::Init(int iOutsideEdgeWidth, int iInnerEdgeWidth)
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
bool CRectanglePow2Packer::AddRectangle(CRectangle* pcRect, void* pvUserData)
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
	int		iStartWidth;
	int		iEndWidth;
	int		iWidth;


	iStartWidth = mcPacker.GetSourcesMinWidth();;
	iEndWidth = mcPacker.GetSourcesMaxWidth();

	if (iEndWidth > 0)
	{
		iStartWidth += mcPacker.GetOutsideEdgeWidth()*2;
		iEndWidth += mcPacker.GetOutsideEdgeWidth()*2;

		iStartWidth = GetPowerOf2SizeUp(iStartWidth);
		iEndWidth = GetPowerOf2SizeUp(iEndWidth);

		for (iWidth = iStartWidth; iWidth <= iEndWidth; iWidth *= 2)
		{
			mcPacker.AddAttempt(iWidth-mcPacker.GetOutsideEdgeWidth()*2);
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
	int				i;
	CPackAttempt*	pcAttempt;
	int				iNumAttempts;

	iNumAttempts = mcPacker.GetPackAttempts()->NumElements();
	for (i = 0; i < iNumAttempts; i++)
	{
		pcAttempt = mcPacker.GetPackAttempts()->Get(i);
		pcAttempt->miWidth = GetPowerOf2SizeUp(pcAttempt->miWidth + mcPacker.GetOutsideEdgeWidth()*2);
		pcAttempt->miHeight = GetPowerOf2SizeUp(pcAttempt->miHeight + mcPacker.GetOutsideEdgeWidth()*2);
		pcAttempt->miArea = pcAttempt->miWidth * pcAttempt->miHeight;
	}
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
SInt2 CRectanglePow2Packer::GetPackedImageSize(void)
{
	SInt2			sSize;
	CPackAttempt*	pcBest;

	pcBest = mcPacker.GetBestAttempt();
	if (pcBest)
	{
		sSize.Init(pcBest->miWidth, pcBest->miHeight);
	}
	else
	{
		sSize.Zero();
	}
	return sSize;
}
