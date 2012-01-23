/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "FlowContainer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlowContainer::Init(CViewport* pcViewport)
{
	CComponent::Init(pcViewport);
	meWrap = FSW_Wrap;
	meDirection = FSD_Right;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlowContainer::Kill(void)
{
	CComponent::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlowContainer::SetRequiredSize(void)
{
	CComponent*		pcComponent;
	SInt2			sSize;
	int				i;
	SInt2			sTotalSize;

	sTotalSize.Init(0, 0);
	for (i = 0; i <	mcComponents.Size(); i++)
	{
		pcComponent = mcComponents.Get(i);
		sSize = pcComponent->GetBestSize();
		if (meDirection == FSD_Right)
		{
			sTotalSize.x += sSize.x;
			if (sTotalSize.y < sSize.y)
			{
				sTotalSize.y = sSize.y;
			}
		}
		else if (meDirection == FSD_Down)
		{
			sTotalSize.y += sSize.y;
			if (sTotalSize.x < sSize.x)
			{
				sTotalSize.x = sSize.x;
			}
		}
	}
	msRequiredSize = sTotalSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlowContainer::SetFlowStyle(EFlowStyleDirection eD, EFlowStyleWrap eW)
{
	meWrap = eW;
	meDirection = eD;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlowContainer::Layout(SInt2 sPosition, SInt2 sAreaSize)
{
	CComponent*		pcComponent;
	SInt2			sSize;
	int				i;
	int				fXPos;
	int				fYPos;
	SInt2			sOffset;

	SetBounds(sPosition, sAreaSize);

	fXPos = 0;
	fYPos = 0;
	for (i = 0; i <	mcComponents.Size(); i++)
	{
		pcComponent = mcComponents.Get(i);
		sSize = pcComponent->GetBestSize();
		if (meDirection == FSD_Right)
		{
			if ((meWrap == FSW_Wrap) && ((fXPos + sSize.x) > sAreaSize.x))
			{
				fXPos = 0;
				fYPos += sSize.y;
			}
			sOffset.Init(fXPos + sPosition.x, fYPos + sPosition.y);
			fXPos += sSize.x;
		}
		else if (meDirection == FSD_Down)
		{
			if ((meWrap == FSW_Wrap) && ((fYPos + sSize.y) > sAreaSize.y))
			{
				fYPos = 0;
				fXPos += sSize.x;
			}
			sOffset.Init(fXPos + sPosition.x, fYPos + sPosition.y);
			fYPos += sSize.y;
		}
		
		pcComponent->Layout(sOffset, sSize);
	}
}

