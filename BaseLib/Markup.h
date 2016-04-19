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
#ifndef __MARKUP_H__
#define __MARKUP_H__
#include "FreeList.h"
#include "MarkupTag.h"
#include "MarkupText.h"
#include "MarkupSubstitute.h"
#include "MarkupReference.h"


class CMarkup
{
public:
	CFreeList	mcTags;
	CFreeList	mcTexts;
	CFreeList	mcSubDocs;
	CFreeList	mcSubTexts;
	CFreeList	mcRefDocs;
	CFreeList	mcRefTexts;
	CFreeList	mcNamedRefs;
	CFreeList	mcDocs;

	CMarkupDoc*	mpcDoc;

	void				Init(void);
	void				Kill(void);

	CMarkupTag*			AllocateTag(CMarkupDoc* pcDoc);
	CMarkupText*		AllocateText(CMarkupDoc* pcDoc);
	CMarkupSubDoc*		AllocateSubDoc(void);
	CMarkupSubText*		AllocateSubText(void);
	CMarkupRefDoc*		AllocateRefDoc(CMarkupDoc* pcDoc);
	CMarkupRefText*		AllocateRefText(CMarkupDoc* pcDoc);
	CMarkupNamedRef*	AllocateNamedRef(CMarkupDoc* pcDoc);
	CMarkupDoc*			AllocateDoc(void);

	void				Deallocated(CMarkupNamedRef* pc);

	CMarkupTag*			SetRootTag(char* szTagName);
	CMarkupTag*			GetRootTag(void);
	CMarkupSubstitute*	GetSubstitute(char* szName);
};


#endif // __MARKUP_H__

