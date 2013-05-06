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
#ifndef __MARKUP_SUBSTITUTE_H__
#define __MARKUP_SUBSTITUTE_H__
#include "Chars.h"
#include "MarkupDoc.h"


enum EMarkupSubstituteType
{
	MST_Text,
	MST_Doc,
};


class CMarkupSubstitute
{
public:
	CChars					mszName;
	EMarkupSubstituteType	meType;

	void Init(char* szName, EMarkupSubstituteType eType);
	void Kill(void);

	BOOL Is(char* szName);
	BOOL IsText(void);
	BOOL IsDoc(void);
};


class CMarkup;
class CMarkupSubText : public CMarkupSubstitute
{
public:
	CChars		mszText;
	int			miReferences;
	CMarkup*	mpcMarkup;

	void Init(char* szName, char* szText);
	void Kill(void);
	void AddRef(void);
	BOOL RemoveRef(void);
	void RemoveAndKill(void);  //Remove a reference and kill the MarkupSubText if the reference count goes to zero.
};


class CMarkupSubDoc : public CMarkupSubstitute
{
public:
	CMarkupDoc*		mpcDoc;
	int				miReferences;
	CMarkup*		mpcMarkup;

	void Init(char* szName, CMarkupDoc* pcDoc);
	void Kill(void);
	void AddRef(void);
	BOOL RemoveRef(void);
	void RemoveAndKill(void);
};


#endif // __MARKUP_SUBSTITUTE_H__

