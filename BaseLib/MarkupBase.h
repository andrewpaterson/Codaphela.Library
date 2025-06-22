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
#ifndef __MARKUP_BASE_H__
#define __MARKUP_BASE_H__
#include "Chars.h"
#include "Define.h"


enum EMarkupType
{
	MUT_Unknown,
	MUT_Tag,
	MUT_Text,
	MUT_RefDoc,
	MUT_RefText,
	MUT_NamedRef,
};


class CMarkupTag;
class CMarkupDoc;
class CMarkupBase : public CKillable
{
public:
	EMarkupType		meType;
	CMarkupTag*		mpcParent;
	CMarkupDoc*		mpcDoc;

	size			miLine;
	size			miColumn;

public:
			void 	Init(EMarkupType eType, CMarkupTag* pcParent);

			bool 	IsTag(void);
			bool 	IsText(void);
			bool	IsRefDoc(void);
			bool	IsRefText(void);
			bool	IsNamedRef(void);

			size	GetLine(void);
			size	GetColumn(void);
			void	SetLineAndColumn(size iLine, size iColumn);

			void	ReplaceIllegalChars(CChars* pszMutableSource);
			size	Print(CChars* pszDest, CChars* pszMutableSource, size iDepth, size iLine, bool bAllowSameLine);

	virtual size	Print(CChars* psz, size iDepth, size iLine) =0;
};


#endif // __MARKUP_BASE_H__

