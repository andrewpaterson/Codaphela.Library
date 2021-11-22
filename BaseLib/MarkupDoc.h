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
#ifndef __MARKUP_DOC_H__
#define __MARKUP_DOC_H__
#include "FreeList.h"
#include "MarkupTag.h"
#include "ArrayTemplate.h"


class CMarkupReference;
typedef CArrayTemplate<CMarkupReference*>	CArrayMarkupReference;
class CMarkupSubstitute;
typedef CArrayTemplate<CMarkupSubstitute*>	CArrayMarkupSubstitute;


class CMarkup;
class CMarkupDoc
{
public:
	CMarkupTag*				mpcRootTag;
	CArrayMarkupReference	macRefs;
	CArrayMarkupSubstitute	macSubstitutes;
	CMarkup*				mpcMarkup;

	void			Init(CMarkup* pcMarkup);
	void			Kill(void);

	CMarkupTag*		GetRootTag(void);
	CMarkupTag*		SetRootTag(char* szTagName);
	BOOL			Is(char* szName);
	void			AddReference(CMarkupReference* pcRef);
	BOOL			FixReferences(void);

	void			AddSubstitute(CMarkupSubstitute* pcSubstitute);

	int				Print(CChars* psz);
	int				Print(CChars* psz, int iDepth, int iLine);
	void			Dump(void);
};


#endif // !__MARKUP_DOC_H__


