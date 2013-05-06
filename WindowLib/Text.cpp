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
#include "Text.h"
#include "Viewport.h"
#include "ComponentFactory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CText::Init(CViewport* pcViewport)
{
	CDrawable::Init(pcViewport);

	mcTextData.Init(GetFactory()->mpcDefaultFont);
	mfAlpha = 1.0f;
	mbCaretVisible = FALSE;
	msCaretPos.Init(0, 0);

	mpcCaret = gcUnknowns.Add<CCaret>();
	mpcCaret->Init(mpcViewport);
	AddComponent(mpcCaret);

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CText::Kill(void)
{
	mcTextData.Kill();
	CDrawable::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CText::Layout(SInt2 sPosition, SInt2 sAreaSize)
{
	CRectangle	cRect;
	BOOL		bValid;

	mcTextData.Layout();
	CDrawable::Layout(sPosition, sAreaSize);

	bValid = mcTextData.GetBounds(msCaretPos.x, msCaretPos.y, &cRect);

	if (bValid)
	{
		mpcCaret->SetPosition(cRect.miLeft + sPosition.x, cRect.miTop + sPosition.y);
		mpcCaret->SetActualSize(1, cRect.GetHeight());
	}
	else
	{
		mbCaretVisible = FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CText::Draw(void)
{
	mcTextData.CreateQuads(mpcViewport->mpcGraphicsObject, mpcGraphicsViewport, &msPosition, &mpcViewport->mcMap2D);
	return CDrawable::Draw();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CText::SetText(char* sz)
{
	CTextParameters*	pcTextParameters;
	int					iLen;

	iLen = (int)strlen(sz);
	pcTextParameters = mcTextData.mpcDefaultFont;
	mcTextData.Kill();
	mcTextData.Init(pcTextParameters, sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CText::SetText(CChars sz)
{
	CTextParameters*	pcTextParameters;

	pcTextParameters = mcTextData.mpcDefaultFont;
	mcTextData.Kill();
	mcTextData.Init(pcTextParameters, &sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CText::SetRequiredSize(void)
{
	SInt2		sSize;

	sSize = mcTextData.GetRequiredSize();
	msRequiredSize.x = sSize.x;
	msRequiredSize.y = sSize.y;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CText::SetAlpha(float fAlpha)
{
	mfAlpha = fAlpha;
}

