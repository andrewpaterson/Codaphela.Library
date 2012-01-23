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
#include "FillContainer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFillContainer::Init(CViewport* pcViewport)
{
	CComponent::Init(pcViewport);
	meHorizontal = FSH_Stretch;
	meVertical = FSV_Stretch;
	msInsets.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFillContainer::Kill(void)
{
	CComponent::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFillContainer::SetFillStyle(EFillStyleHorizontal eH, EFillStyleVertical eV)
{
	meHorizontal = eH;
	meVertical = eV;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFillContainer::Layout(SInt2 sPosition, SInt2 sAreaSize)
{
	SInt2		sSize;
	int			i;
	CComponent*	pcComponent;
	SInt2		sOrigin;
	SInt2		sArea;

	sOrigin = sPosition;
	sArea = sAreaSize;
	sOrigin.x += msInsets.mafWidths[BEI_Left];
	sOrigin.y += msInsets.mafWidths[BEI_Top];
	sArea.x -= (msInsets.mafWidths[BEI_Left] + msInsets.mafWidths[BEI_Right]);
	sArea.y -= (msInsets.mafWidths[BEI_Bottom] + msInsets.mafWidths[BEI_Top]);

	SetBounds(sOrigin, sArea);

	//There should only be one componet, any more will be laid out into the same area.
	for (i = 0; i < mcComponents.Size(); i++)
	{
		sPosition = sOrigin;
		sAreaSize = sArea;

		pcComponent = mcComponents.Get(i);

		sSize = pcComponent->GetBestSize();
		if (sSize.x == -1)
		{
			sSize = sAreaSize;
		}

		//Horizontal.
		if (meHorizontal == FSH_Stretch)
		{
		}
		else if (meHorizontal == FSH_Left)
		{
			sAreaSize.x = sSize.x;
		}
		else if (meHorizontal == FSH_Center)
		{
			sPosition.x = sPosition.x + (sAreaSize.x - sSize.x) / 2;
			sAreaSize.x = sSize.x;
		}
		else if (meHorizontal == FSH_Right)
		{
			sPosition.x = sPosition.x + sAreaSize.x - sSize.x;
			sAreaSize.x = sSize.x;
		}

		//Vertical.
		if (meVertical == FSV_Stretch)
		{
		}
		else if (meVertical == FSV_Top)
		{
			sAreaSize.y = sSize.y;
		}
		else if (meVertical == FSV_Center)
		{
			sPosition.y = sPosition.y + (sAreaSize.y - sSize.y) / 2;
			sAreaSize.y = sSize.y;
		}
		else if (meVertical == FSV_Bottom)
		{
			sPosition.y = sPosition.y + sAreaSize.y - sSize.y;
			sAreaSize.y = sSize.y;
		}
		pcComponent->Layout(sPosition, sAreaSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFillContainer::SetInsets(SInsets* psInsets)
{
	memcpy(&msInsets, psInsets, sizeof(SInsets));
}
