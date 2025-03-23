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
	mcTags.Init(sizeof(CMarkupTag));
	mcTexts.Init(sizeof(CMarkupText));
	mcSubDocs.Init(sizeof(CMarkupSubDoc));
	mcSubTexts.Init(sizeof(CMarkupSubText));
	mcRefDocs.Init(sizeof(CMarkupRefDoc));
	mcRefTexts.Init(sizeof(CMarkupRefText));
	mcNamedRefs.Init(sizeof(CMarkupNamedRef));
	mcDocs.Init(sizeof(CMarkupDoc));

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

	mcTexts.Kill();
	mcTags.Kill();
	mcSubDocs.Kill();
	mcSubTexts.Kill();
	mcRefDocs.Kill();
	mcRefTexts.Kill();
	mcNamedRefs.Kill();
	mcDocs.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupTag* CMarkup::AllocateTag(CMarkupDoc* pcDoc)
{
	CMarkupTag*					pcTag;
	CPostMalloc<CMarkupTag>		cPostMalloc;

	pcTag = (CMarkupTag*)mcTags.Add();
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

	pcText = (CMarkupText*)mcTexts.Add();
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

	pcSubDoc = (CMarkupSubDoc*)mcSubDocs.Add();
	cPostMalloc.PostMalloc(pcSubDoc);

	pcSubDoc->mpcMarkup = this;
	return pcSubDoc;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupSubText* CMarkup::AllocateSubText(void)
{
	CMarkupSubText*				pcSubText;
	CPostMalloc<CMarkupSubText>	cPostMalloc;

	pcSubText = (CMarkupSubText*)mcSubTexts.Add();
	cPostMalloc.PostMalloc(pcSubText);

	pcSubText->mpcMarkup = this;
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

	pcRefDoc = (CMarkupRefDoc*)mcRefDocs.Add();
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
	CMarkupRefText*				pcRefText;
	CPostMalloc<CMarkupRefText>	cPostMalloc;

	pcRefText = (CMarkupRefText*)mcRefTexts.Add();
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

	pcNamedRef = (CMarkupNamedRef*)mcNamedRefs.Add();
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
	CMarkupDoc* pcDoc;

	pcDoc = (CMarkupDoc*)mcDocs.Add();
	return pcDoc;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkup::Deallocated(CMarkupNamedRef* pc)
{
	mcNamedRefs.Remove(pc);
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
	SFreeListIterator	sIter;
	CMarkupSubDoc*		pcDoc;
	CMarkupSubText*		pcText;

	pcDoc = (CMarkupSubDoc*)mcSubDocs.StartIteration(&sIter);
	while (pcDoc)
	{
		if (pcDoc->Is(szName))
		{
			return pcDoc;
		}
		pcDoc = (CMarkupSubDoc*)mcSubDocs.Iterate(&sIter);
	}	

	pcText = (CMarkupSubText*)mcSubTexts.StartIteration(&sIter);
	while (pcText)
	{
		if (pcText->Is(szName))
		{
			return pcText;
		}
		pcText = (CMarkupSubText*)mcSubTexts.Iterate(&sIter);
	}
	return NULL;
}

