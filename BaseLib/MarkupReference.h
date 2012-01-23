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
#ifndef __MARKUP_REFERENCE_H__
#define __MARKUP_REFERENCE_H__
#include "MarkupSubstitute.h"
#include "MarkupBase.h"


class CMarkupReference : public CMarkupBase
{
public:
};


class CMarkupRefText : public CMarkupReference
{
public:
	CMarkupSubText*	mpcRef;

	void 	Init(CMarkupSubText* pcRef, CMarkupTag* pcParent);
	void 	Kill(void);
	int		ToString(CChars* psz, int iDepth, int iLine);
};


class CMarkupTag;
class CMarkupRefDoc : public CMarkupReference
{
public:
	CMarkupSubDoc*	mpcRef;

	void 	Init(CMarkupSubDoc* pcRef, CMarkupTag* pcParent);
	void 	Kill(void);

	BOOL		Is(char* szName);
	CMarkupTag*	GetRootTag(void);
	int			ToString(CChars* psz, int iDepth, int iLine);
};


class CMarkupNamedRef : public CMarkupReference
{
public:
	CChars	mszName;

	void 	Init(char* szName, CMarkupTag* pcParent);
	void 	Kill(void);
	int		ToString(CChars* psz, int iDepth, int iLine);
};


#endif // __MARKUP_REFERENCE_H__

