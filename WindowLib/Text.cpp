/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "StandardLib/ClassDefines.h"
#include "Text.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CText> CText::Init(Ptr<CWindow> pWindow)
{
	PreInit();

	CBasicComponent::Init(pWindow);

	mcTextData.Init();
	mbCaretVisible = false;
	msCaretPos.Init(0, 0);

	mpCaret = OMalloc<CCaret>(pWindow);
	AddComponent(mpCaret);

	PostInit();

	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CText::Class(void)
{
	CBasicComponent::Class();
	U_Data(CTextData, mcTextData);
	U_Bool(mbCaretVisible);
	M_Pointer(mpCaret);
	U_2Int32(msCaretPos);

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CText::Free(void)
{
	mcTextData.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CText::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CText::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CText::Layout(SInt2 sPosition, SInt2 sAreaSize)
{
	CRectangle	cRect;
	bool		bValid;

	mcTextData.Layout();


	bValid = mcTextData.GetBounds(msCaretPos.x, msCaretPos.y, &cRect);

	if (bValid)
	{
		mpCaret->SetPosition(cRect.miLeft + sPosition.x, cRect.miTop + sPosition.y);
		mpCaret->SetActualSize(1, cRect.GetHeight());
	}
	else
	{
		mbCaretVisible = false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CText::Draw(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CText::SetText(char* sz, Ptr<CFont> pFont)
{
	int					iLen;

	iLen = (int)strlen(sz);
	mcTextData.Kill();
	mcTextData.Init(&pFont, sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CText::SetText(CChars sz, Ptr<CFont> pFont)
{
	mcTextData.Kill();
	mcTextData.Init(&pFont, &sz);
}


////////////////////////////////////////////////////////////////////////////
////
////
////////////////////////////////////////////////////////////////////////////
//void CText::SetRequiredSize(void)
//{
//	SInt2		sSize;
//
//	sSize = mcTextData.GetRequiredSize();
//	msRequiredSize.x = sSize.x;
//	msRequiredSize.y = sSize.y;
//}


