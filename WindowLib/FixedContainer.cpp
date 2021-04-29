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
#include "BaseLib/Int2.h"
#include "FixedContainer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFixedContainer::Init(CViewport* pcViewport)
{
	CComponent::Init(pcViewport);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFixedContainer::Kill(void)
{
	CComponent::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFixedContainer::Layout(SInt2 sPosition, SInt2 sAreaSize)
{
	CComponent*		pcComponent;
	SInt2			sSize;
	int				i;

	SetPosition(sPosition);
	SetActualSize(sAreaSize);
	SetBounds(sPosition, sAreaSize);

	//Fixed containers are a bit special.  They assume that the actual position and size have already been set.
	for (i = 0; i <	mcComponents.Size(); i++)
	{
		pcComponent = mcComponents.Get(i);
		sSize = pcComponent->GetBestSize();
		pcComponent->Layout(pcComponent->GetPosition(), sSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFixedContainer::SetRequiredSize(void)
{
	CComponent*		pcComponent;
	SInt2			sPosition;
	SInt2			sSize;
	SInt2			sTemp;
	int				i;

	msRequiredSize.x = 0;
	msRequiredSize.y = 0;
	for (i = 0; i <	mcComponents.Size(); i++)
	{
		pcComponent = mcComponents.Get(i);
		sSize = pcComponent->GetBestSize();
		sPosition = pcComponent->GetPosition();
		sTemp = sPosition + sSize;
		if (sTemp.x > msRequiredSize.x)
		{
			msRequiredSize.x = sTemp.x;
		}
		if (sTemp.y > msRequiredSize.y)
		{
			msRequiredSize.y = sTemp.y;
		}
	}
}
