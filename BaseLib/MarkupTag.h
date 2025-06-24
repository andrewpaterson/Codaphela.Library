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
#ifndef __MARKUP_TAG_H__
#define __MARKUP_TAG_H__
#include "ArrayTemplate.h"
#include "Number.h"
#include "MarkupBase.h"
#include "MapStringIntAndPointer.h"
#include "ArrayIntAndPointer.h"


typedef CArrayTemplate<CMarkupBase*>	CArrayMarkupBase;


struct STagIterator
{
	size			iIndex;
	CMarkupTag*		mpcCurrent;
	bool			bNamed;
};


class CMarkupText;
class CMarkupNamedRef;
class CMarkupTag : public CMarkupBase
{
protected:
	CArrayMarkupBase			macBases;
	CMapStringIntAndPointer		mcNamedAttributes;
	CArrayIntAndPointer			mcOrderedAttributes;
	CChars						mszName;

public:
	void				Init(CMarkupTag* pcParent);
	void				Init(char* szName, CMarkupTag* pcParent);
	void				Kill(void);
	bool				IsEmpty(void);
	void*				GetNamedAttribute(char* szAttribute, uint* puiType);
	size				GetNumOrderedAttributes(void);
	void*				GetOrderedAttribute(size uiIndex, uint* puiType);
	CMarkupTag*			GetTag(char* szTagName, STagIterator* psIter = NULL);
	CMarkupTag*			GetTag(char* szTagName, size iTagNumber);
	CMarkupTag*			GetNextTag(STagIterator* psIter);
	CMarkupTag*			GetTagFromIndex(char* szTagName, STagIterator* psIter);
	CMarkupTag*			GetTagFromIndex(STagIterator* psIter);
	CMarkupTag*			GetTag(STagIterator* psIter = NULL);
	bool				Is(char* szName);
	bool				GetText(CChars* psz, bool bFirstContiguous = true, bool bFirstTag = false);
	void				SetName(char* szName);
	char*				GetName(void);
	CMarkupTag*			AppendTag(void);
	CMarkupTag*			AppendTag(char* szName);
	CMarkupText*		AppendText(void);
	CMarkupText*		AppendText(char* szText);
	CMarkupNamedRef*	AppendNamedReference(char* szIdentifier);
	bool				AddStringAttribute(char* szString);
	bool				AddStringAttribute(char* szAttribute, char* szString);
	bool				AddNumberAttribute(CNumber* pcNumber);
	bool				AddNumberAttribute(char* szAttribute, CNumber* pcNumber);
	bool				AddBooleanAttribute(bool bValue);
	bool				AddBooleanAttribute(char* szAttribute, bool bValue);
	bool				AddNullAttribute(void);
	bool				AddNullAttribute(char* szAttribute);
	bool				ContainsOnlyText(void);
	bool				Swap(CMarkupBase* pcNew, CMarkupBase* pcOld);
	CMallocator*		GetMalloc(void);
	void				Print(CChars* psz);
	size				Print(CChars* psz, size iDepth, size iLine);
	void				Dump(void);

protected:
	void CMarkupTag::PrintType(CChars* psz, char* szValue, uint uiType);
};


#endif // __MARKUP_TAG_H__

