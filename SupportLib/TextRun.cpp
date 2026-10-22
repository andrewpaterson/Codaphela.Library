/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MeshLib

Codaphela MeshLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MeshLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MeshLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "Text.h"
#include "TextRun.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTextRun::Init(CFont* pcFont, CText* pcText)
{
	mpcFont = pcFont;
	mpcText = pcText;
	mapcText.Init();
	miBaselineShift = 0;
	muiDecorations = 0;
	msDecorationColour = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextRun::SetBaselineShift(int16 iBaselineShift)
{
	miBaselineShift = iBaselineShift;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int16 CTextRun::GetBaselineShift(void)
{
	return miBaselineShift;
}


//////////////////////////////////////////////////////////////////////////
//
// uiDecorations are TD_ flags.  The lines are drawn in sColour.
//
//////////////////////////////////////////////////////////////////////////
void CTextRun::SetDecorations(uint8 uiDecorations, ARGB32 sColour)
{
	muiDecorations = uiDecorations;
	msDecorationColour = sColour;
}


uint8 CTextRun::GetDecorations(void) { return muiDecorations; }
ARGB32 CTextRun::GetDecorationColour(void) { return msDecorationColour; }


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTextRun::Kill(void)
{
	size			ui;
	size			uiNumElements;
	CTextElement*	pcTextElement;
	CMallocator*	pcMalloc;

	pcMalloc = GetMalloc();
	uiNumElements = mapcText.NumElements();
	for (ui = 0; ui < uiNumElements; ui++)
	{
		pcTextElement = mapcText.GetPtr(ui);
		pcTextElement->Kill();
		pcMalloc->Free(pcTextElement);
	}
	mapcText.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTextRun::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTextRun::Save(CObjectWriter* pcFile)
{
	size			ui;
	size			uiNumElements;
	CTextElement*	pcTextElement;

	//Still need to do
	//mpcFont;
	
	uiNumElements = mapcText.NumElements();
	ReturnOnFalse(pcFile->WriteSize(uiNumElements));

	for (ui = 0; ui < uiNumElements; ui++)
	{
		pcTextElement = mapcText.GetPtr(ui);
		ReturnOnFalse(pcTextElement->Save(pcFile));
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CMallocator* CTextRun::GetMalloc(void)
{
	return mpcText->GetMalloc();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTextRun::Add(CTextElement* pcElement)
{
	mapcText.Add(pcElement);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTextRun::Start(void)
{
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTextRun::End(void)
{
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CFont* CTextRun::GetFont(void)
{
	return mpcFont;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CTextRun::NumElements(void)
{
	return mapcText.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CTextElement* CTextRun::GetElement(size uiIndex)
{
	return mapcText.GetPtr(uiIndex);
}

