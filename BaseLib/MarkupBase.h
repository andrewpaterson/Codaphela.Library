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

	int				miLine;
	int				miColumn;

public:
			void 	Init(EMarkupType eType, CMarkupTag* pcParent);

			BOOL 	IsTag(void);
			BOOL 	IsText(void);
			BOOL	IsRefDoc(void);
			BOOL	IsRefText(void);
			BOOL	IsNamedRef(void);

			int		GetLine(void);
			int		GetColumn(void);
			void	SetLineAndColumn(int iLine, int iColumn);

			void	ReplaceIllegalChars(CChars* pszMutableSource);
			int		ToString(CChars* pszDest, CChars* pszMutableSource, int iDepth, int iLine, BOOL bAllowSameLine);

	virtual int		ToString(CChars* psz, int iDepth, int iLine) =0;
};


#endif // !__MARKUP_BASE_H__

