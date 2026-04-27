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
#include "TextRunUTF16Long.h"
#include "TextRunUTF16Short.h"
#include "TextRunUTF16Multi.h"
#include "Text.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CText::Init(ETextRunRelativeDirection eRunDirection, CMallocator* pcMalloc)
{
	meRunDirection = eRunDirection;
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
	U_Enum(meRunDirection);
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
CTextRunUTF16Short* CText::AllocateUTF16Short(size uiNumShorts)
{
	CTextRunUTF16Short*		pcUTF16Short;

	pcUTF16Short = (CTextRunUTF16Short*)mpcMalloc->Malloc(sizeof(CTextRunUTF16Short) + (uiNumShorts * sizeof(uint16)));
	New<CTextRunUTF16Short>(pcUTF16Short);
	pcUTF16Short->Init(uiNumShorts);
	return pcUTF16Short;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTextRunUTF16Long* CText::AllocateUTF16Long(size uiNumLongs)
{
	CTextRunUTF16Long* pcUTF16Long;

	pcUTF16Long = (CTextRunUTF16Long*)mpcMalloc->Malloc(sizeof(CTextRunUTF16Long) + (uiNumLongs * sizeof(uint16)));
	New<CTextRunUTF16Long>(pcUTF16Long);
	pcUTF16Long->Init(uiNumLongs);
	return pcUTF16Long;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTextRunUTF16Multi* CText::AllocateUTF16Multi(size uiNumShorts)
{
	CTextRunUTF16Multi* pcUTF16Multi;

	pcUTF16Multi = (CTextRunUTF16Multi*)mpcMalloc->Malloc(sizeof(CTextRunUTF16Multi) + (uiNumShorts * sizeof(uint16)));
	New<CTextRunUTF16Multi>(pcUTF16Multi);
	pcUTF16Multi->Init(uiNumShorts);
	return pcUTF16Multi;
}

