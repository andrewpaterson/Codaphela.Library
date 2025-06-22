/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "ConstructorCall.h"
#include "Markup.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkup::Init(void)
{
	mcMemory.Init();
	mapcSubDocs.Init();
	mapcSubTexts.Init();

	mpcDoc = AllocateDoc();
	mpcDoc->Init(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkup::Kill(void)
{
	mpcDoc->Kill();
	mapcSubDocs.Kill();
	mapcSubTexts.Kill();

	mcMemory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupTag* CMarkup::AllocateTag(CMarkupDoc* pcDoc)
{
	CMarkupTag*					pcTag;
	CPostMalloc<CMarkupTag>		cPostMalloc;

	pcTag = (CMarkupTag*)mcMemory.Malloc(sizeof(CMarkupTag));
	cPostMalloc.PostMalloc(pcTag);

	pcTag->mpcDoc = pcDoc;
	return pcTag;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupText* CMarkup::AllocateText(CMarkupDoc* pcDoc)
{
	CMarkupText*				pcText;
	CPostMalloc<CMarkupText>	cPostMalloc;

	pcText = (CMarkupText*)mcMemory.Malloc(sizeof(CMarkupText));
	cPostMalloc.PostMalloc(pcText);

	pcText->mpcDoc = pcDoc;
	return pcText;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupSubDoc* CMarkup::AllocateSubDoc(void)
{
	CMarkupSubDoc*				pcSubDoc;
	CPostMalloc<CMarkupSubDoc>	cPostMalloc;

	pcSubDoc = (CMarkupSubDoc*)mcMemory.Malloc(sizeof(CMarkupSubDoc));
	cPostMalloc.PostMalloc(pcSubDoc);

	pcSubDoc->mpcMarkup = this;

	mapcSubDocs.Add(pcSubDoc);

	return pcSubDoc;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupSubText* CMarkup::AllocateSubText(void)
{
	CMarkupSubText*					pcSubText;
	CPostMalloc<CMarkupSubText>		cPostMalloc;

	pcSubText = (CMarkupSubText*)mcMemory.Malloc(sizeof(CMarkupSubText));
	cPostMalloc.PostMalloc(pcSubText);

	pcSubText->mpcMarkup = this;

	mapcSubTexts.Add(pcSubText);

	return pcSubText;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupRefDoc* CMarkup::AllocateRefDoc(CMarkupDoc* pcDoc)
{
	CMarkupRefDoc*				pcRefDoc;
	CPostMalloc<CMarkupRefDoc>	cPostMalloc;

	pcRefDoc = (CMarkupRefDoc*)mcMemory.Malloc(sizeof(CMarkupRefDoc));
	cPostMalloc.PostMalloc(pcRefDoc);

	pcRefDoc->mpcDoc = pcDoc;
	return pcRefDoc;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupRefText* CMarkup::AllocateRefText(CMarkupDoc* pcDoc)
{
	CMarkupRefText*					pcRefText;
	CPostMalloc<CMarkupRefText>		cPostMalloc;

	pcRefText = (CMarkupRefText*)mcMemory.Malloc(sizeof(CMarkupRefText));
	cPostMalloc.PostMalloc(pcRefText);

	pcRefText->mpcDoc = pcDoc;
	return pcRefText;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupNamedRef* CMarkup::AllocateNamedRef(CMarkupDoc* pcDoc)
{
	CMarkupNamedRef*				pcNamedRef;
	CPostMalloc<CMarkupNamedRef>	cPostMalloc;

	pcNamedRef = (CMarkupNamedRef*)mcMemory.Malloc(sizeof(CMarkupNamedRef));
	cPostMalloc.PostMalloc(pcNamedRef);

	pcNamedRef->mpcDoc = pcDoc;
	return pcNamedRef;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupDoc* CMarkup::AllocateDoc(void)
{
	CMarkupDoc*		pcDoc;

	pcDoc = (CMarkupDoc*)mcMemory.Malloc(sizeof(CMarkupDoc));
	return pcDoc;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkup::Deallocated(CMarkupNamedRef* pc)
{
	mcMemory.Free(pc);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupTag* CMarkup::SetRootTag(char* szTagName)
{
	return mpcDoc->SetRootTag(szTagName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupTag* CMarkup::GetRootTag(void)
{
	return mpcDoc->GetRootTag();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupSubstitute* CMarkup::GetSubstitute(char* szName)
{
	CMarkupSubDoc*		pcDoc;
	CMarkupSubText*		pcText;
	size				ui;
	size				uiNumElements;
	
	uiNumElements = mapcSubDocs.NumElements();
	for (ui = 0; ui < uiNumElements; ui++)
	{
		pcDoc = mapcSubDocs.GetPtr(ui);
		if (pcDoc->Is(szName))
		{
			return pcDoc;
		}
	}

	uiNumElements = mapcSubTexts.NumElements();
	for (ui = 0; ui < uiNumElements; ui++)
	{
		pcText = mapcSubTexts.GetPtr(ui);
		if (pcText->Is(szName))
		{
			return pcText;
		}
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDataMemoryAllocator* CMarkup::GetMemory(void)
{
	return &mcMemory;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkup::Print(CChars* psz)
{
	if (mpcDoc)
	{
		mpcDoc->Print(psz);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkup::Dump(void)
{
	CChars	sz;

	sz.Init();
	Print(&sz);
	sz.DumpKill();
}


