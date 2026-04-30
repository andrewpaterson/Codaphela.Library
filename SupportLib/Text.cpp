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
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/UTF16.h"
#include "StandardLib/ClassDefines.h"
#include "TextUTF16Long.h"
#include "TextUTF16Short.h"
#include "TextUTF16Multi.h"
#include "TextNewLine.h"
#include "Text.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CText::Init(CMallocator* pcMalloc)
{
	macText.Init();

	if (pcMalloc == NULL)
	{
		pcMalloc = &gcSystemAllocator;
	}
	mpcMalloc = pcMalloc;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CText::Free(void)
{
	size		ui;
	size		uiNumElements;
	CTextRun*	pcTextRun;

	uiNumElements = macText.NumElements();
	for (ui = 0; ui < uiNumElements; ui++)
	{
		pcTextRun = macText.Get(ui);
		pcTextRun->Kill();
	}
	macText.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CText::Class(void)
{
	U_Data(CArrayTextRun, macText);
	U_Pointer(mpcMalloc);
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
bool CText::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CText::AddUTF16(Ptr<CFont> pFont, uint16* szText, size uiLength)
{
	CUTF16	cUTF16;

	cUTF16.Init(szText, uiLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMallocator* CText::GetMalloc(void)
{
	return mpcMalloc;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTextRun* CText::AllocateRun(CFont* pcFont)
{
	CTextRun*	pcTextRun;

	pcTextRun = macText.Add();
	pcTextRun->Init(pcFont, this);
	return pcTextRun;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTextUTF16Short* CText::AllocateUTF16Short(size uiNumShorts, uint16* puiData)
{
	CTextUTF16Short*		pcUTF16Short;

	pcUTF16Short = (CTextUTF16Short*)mpcMalloc->Malloc(sizeof(CTextUTF16Short) + (uiNumShorts * sizeof(uint16)));
	if (pcUTF16Short)
	{
		New<CTextUTF16Short>(pcUTF16Short);
		pcUTF16Short->Init(uiNumShorts);
		pcUTF16Short->Copy(puiData);
		return pcUTF16Short;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTextUTF16Long* CText::AllocateUTF16Long(size uiNumLongs, uint32* puiData)
{
	CTextUTF16Long*		pcUTF16Long;

	pcUTF16Long = (CTextUTF16Long*)mpcMalloc->Malloc(sizeof(CTextUTF16Long) + (uiNumLongs * sizeof(uint32)));
	if (pcUTF16Long)
	{
		New<CTextUTF16Long>(pcUTF16Long);
		pcUTF16Long->Init(uiNumLongs);
		pcUTF16Long->Copy(puiData);
		return pcUTF16Long;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTextUTF16Multi* CText::AllocateUTF16Multi(size uiByteSize, void* pvData)
{
	CTextUTF16Multi*		pcUTF16Multi;

	pcUTF16Multi = (CTextUTF16Multi*)mpcMalloc->Malloc(sizeof(CTextUTF16Multi) + uiByteSize);
	if (pcUTF16Multi)
	{
		New<CTextUTF16Multi>(pcUTF16Multi);
		pcUTF16Multi->Init(uiByteSize);
		pcUTF16Multi->Copy(pvData);
		return pcUTF16Multi;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTextNewLine* CText::AllocateNewLine(void)
{
	CTextNewLine*	pcNewLine;

	pcNewLine = (CTextNewLine*)mpcMalloc->Malloc(sizeof(CTextUTF16Multi));
	if (pcNewLine)
	{
		New<CTextNewLine>(pcNewLine);
		pcNewLine->Init();
		return pcNewLine;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CText::NumRuns(void)
{
	return macText.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTextRun* CText::GetRun(size uiIndex)
{
	return macText.Get(uiIndex);
}


