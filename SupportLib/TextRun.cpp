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
}


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
void CTextRun::Done(void)
{
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

