/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "Markup.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkup::Init(void)
{
	mcTags.Init(16, sizeof(CMarkupTag));
	mcTexts.Init(8, sizeof(CMarkupText));
	mcSubDocs.Init(8, sizeof(CMarkupSubDoc));
	mcSubTexts.Init(8, sizeof(CMarkupSubText));
	mcRefDocs.Init(8, sizeof(CMarkupRefDoc));
	mcRefTexts.Init(8, sizeof(CMarkupRefText));
	mcNamedRefs.Init(8, sizeof(CMarkupNamedRef));
	mcDocs.Init(8, sizeof(CMarkupDoc));

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
	CMarkupTag*		pcTag;

	pcTag = (CMarkupTag*)mcTags.Add();
	pcTag->mpcDoc = pcDoc;
	return pcTag;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupText* CMarkup::AllocateText(CMarkupDoc* pcDoc)
{
	CMarkupText*	pcText;

	pcText = (CMarkupText*)mcTexts.Add();
	pcText->mpcDoc = pcDoc;
	return pcText;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupSubDoc* CMarkup::AllocateSubDoc(void)
{
	CMarkupSubDoc*		pcSubDoc;

	pcSubDoc = (CMarkupSubDoc*)mcSubDocs.Add();
	pcSubDoc->mpcMarkup = this;
	return pcSubDoc;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupSubText* CMarkup::AllocateSubText(void)
{
	CMarkupSubText*		pcSubText;

	pcSubText = (CMarkupSubText*)mcSubTexts.Add();
	pcSubText->mpcMarkup = this;
	return pcSubText;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupRefDoc* CMarkup::AllocateRefDoc(CMarkupDoc* pcDoc)
{
	CMarkupRefDoc*		pcRefDoc;

	pcRefDoc = (CMarkupRefDoc*)mcRefDocs.Add();
	pcRefDoc->mpcDoc = pcDoc;
	return pcRefDoc;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupRefText* CMarkup::AllocateRefText(CMarkupDoc* pcDoc)
{
	CMarkupRefText*		pcRefText;

	pcRefText = (CMarkupRefText*)mcRefTexts.Add();
	pcRefText->mpcDoc = pcDoc;
	return pcRefText;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupNamedRef* CMarkup::AllocateNamedRef(CMarkupDoc* pcDoc)
{
	CMarkupNamedRef*		pcNamedRef;

	pcNamedRef = (CMarkupNamedRef*)mcNamedRefs.Add();
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

