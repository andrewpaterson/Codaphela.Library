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
#include "Label.h"
#include "ComponentFactory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLabel::Init(CViewport* pcViewport, char* szString, BOOL bDefaultBorder, BOOL bDefaultBackground)
{
	CFillContainer*		mpcFill1;

	CComponent::Init(pcViewport);
	mbCanGetFocus = TRUE;

	mpcWidget = GetFactory()->CreateWidget(bDefaultBorder, bDefaultBackground);
	mpcFill1 = GetFactory()->CreateFillContainer();
	mpcFill1->AddComponent(mpcWidget);

	mpcText = GetFactory()->CreateText();
	mpcText->SetText(szString);
	mpcFill = GetFactory()->CreateFillContainer();
	mpcFill->AddComponent(mpcText);
	mpcFill->SetInsets(&mpcWidget->GetBorderWidths());

	AddComponent(mpcFill1);
	AddComponent(mpcFill);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLabel::Kill(void)
{
	CComponent::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLabel::Draw(void)
{
	return CComponent::Draw();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLabel::Layout(SInt2 sPosition, SInt2 sAreaSize)
{
	CComponent::Layout(sPosition, sAreaSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLabel::SetText(char* szText)
{
	mpcText->SetText(szText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLabel::SetText(CChars cString)
{
	mpcText->SetText(cString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLabel::SetAlpha(float fAlpha)
{
	CBorder*	pcBorder;
	CBlock*		pcBackground;

	pcBackground = mpcWidget->mpcBackground;
	pcBorder = mpcWidget->mpcBorder;

	if (pcBackground)
	{
		pcBackground->SetAlpha(fAlpha);
	}
	if (pcBorder)
	{
		pcBorder->SetAlpha(fAlpha);
	}

	mpcText->SetAlpha(fAlpha);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLabel::SetFillStyle(EFillStyleHorizontal eH, EFillStyleVertical eV)
{
	mpcFill->SetFillStyle(eH, eV);
}


