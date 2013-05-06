/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __DEFINE_MAP_H__
#define __DEFINE_MAP_H__
#include "BaseLib/ASCIITree.h"
#include "BaseLib/ArrayTemplate.h"
#include "PPLine.h"
#include "ExternalString.h"


#define DEFINE_FLAGS_BRACKETED	0x01
#define DEFINE_FLAGS_SPECIAL	0x02
#define DEFINE_FLAGS_IN_MAP		0x04


class CDefineMap;
class CDefine
{
public:
	int				miIndex;  //Index back into the ASCIITree (for the name).
	CPPLine			mcReplacement;
	CArrayString	mcArguments;  //This is unnecessary but useful for reference.
	unsigned int	muiID;  //If a token is undef'd and redef'd it's muiID will change.
	CDefineMap*		mpcDefineMap;
	int				miFlags;

	void 	Init(int iIndex, int uiID, CDefineMap* pcDefineMap);
	void 	Kill(void);
	void 	AddArgument(CExternalString* pcName);
	void 	AddReplacmentToken(CPPToken* pcToken);
	BOOL 	Equals(CDefine* pcOther);
	char*	GetName(void);
	BOOL	IsBacketed(void);
	BOOL	IsSpecial(void);
	BOOL	IsInMap(void);
	void	SetSpecial(BOOL b);
	void	SetBracketed(BOOL b);
	void	SetDefineMap(CDefineMap* pcDefineMap);
	void	Dump(void);
};


typedef CArrayTemplate<CDefine>	CArrayDefines;


class CDefineMap
{
public:
	CArrayDefines	mcDefinesArray;
	CASCIITree		mcDefinesTree;
	unsigned int	muiID;

	void 		Init(int iChunkSize);
	void 		Kill(void);
	CDefine*	AddDefine(CExternalString* pcName);
	CDefine*	AddDefine(char* szName);
	CDefine*	AddDefine(CExternalString* pcName, CDefine* pcDefine);
	void		RemoveDefine(CExternalString* pcName);
	void		RemoveDefine(char* szName);
	CDefine*	GetDefine(CExternalString* pcName);
	CDefine*	GetDefine(CChars* pszName);
	CDefine*	GetDefine(char* szName);
	char*		GetName(CDefine* pcDefine);
	void		Dump(void);
};


#endif // __DEFINE_MAP_H__

