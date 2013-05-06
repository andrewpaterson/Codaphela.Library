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
#include "Widget.h"
#include "BaseLib/PointerFunctions.h"
#include "ComponentFactory.h"
#include "FillContainer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWidget::Init(CViewport* pcViewport, BOOL bDefaultBorder, BOOL bDefaultBackground)
{
	CFillContainer*	mpcFill;

	CComponent::Init(pcViewport);

	mpcBackground = NULL;
	mpcBorder = NULL;
	if (bDefaultBorder)
	{
		mpcBorder = GetFactory()->CreateBorder();
		AddComponent(mpcBorder);
	}
	if (bDefaultBackground)
	{
		mpcBackground = GetFactory()->CreateBlock();

		mpcFill = GetFactory()->CreateFillContainer();
		mpcFill->SetInsets(&GetBorderWidths());
		mpcFill->AddComponent(mpcBackground);
		AddComponent(mpcFill);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWidget::Kill(void)
{
	CComponent::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CWidget::Draw(void)
{
	return CComponent::Draw();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWidget::Layout(SInt2 sPosition, SInt2 sAreaSize)
{
	CComponent::Layout(sPosition, sAreaSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBorder* CWidget::GetBorder(void)
{
	return mpcBorder;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBlock* CWidget::GetBackground(void)
{
	return mpcBackground;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWidget::GetUsableArea(SInt2* psPosition, SInt2* psSize)
{
	if (mpcBorder)
	{
		mpcBorder->GetUsableArea(psPosition, psSize, &msActualSize);
	}
	else
	{
		*psSize = msActualSize;
		*psPosition = msPosition;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInsets CWidget::GetBorderWidths(void)
{
	SInsets		sTemp;

	if (mpcBorder)
	{
		sTemp.Init(mpcBorder->mpcBorderParameters->maiEdgeWidths);
		return sTemp;
	}
	else
	{
		sTemp.Init();
		return sTemp;
	}
}

