/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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


class CMarkupSubstitute : public CKillable
{
public:
	CChars					mszName;
	EMarkupSubstituteType	meType;
	int						miReferences;

			void Init(char* szName, EMarkupSubstituteType eType);
			void Kill(void);

			void AddRef(void);
			bool RemoveRef(void);
	virtual void RemoveAndKill(void);  //Remove a reference and kill the MarkupSubText if the reference count goes to zero.

			bool Is(char* szName);
			bool IsText(void);
			bool IsDoc(void);
};


class CMarkup;
class CMarkupSubText : public CMarkupSubstitute
{
public:
	CChars		mszText;
	CMarkup*	mpcMarkup;

	void Init(char* szName, char* szText);
	void Kill(void);
};


class CMarkupSubDoc : public CMarkupSubstitute
{
public:
	CMarkupDoc*		mpcDoc;
	CMarkup*		mpcMarkup;

	void Init(char* szName, CMarkupDoc* pcDoc);
	void Kill(void);
};


#endif // __MARKUP_SUBSTITUTE_H__

