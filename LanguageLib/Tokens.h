/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LanguageLib

Codaphela LanguageLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LanguageLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LanguageLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __TOKENS_H__
#define __TOKENS_H__
#include <string.h>
#include "ElementTypes.h"


struct STokenSingle
{
	EElement	eElementType;  //ET_Error, ET_Unknown, ET_Node

	void Init(EElement eElementType)
	{
		this->eElementType = eElementType;
	}

	int Size(void)
	{
		return (sizeof(STokenSingle)+3)/4;
	}
};


struct STokenKeywordText
{
	EElement	eElementType;	//ET_Text
	int			iID;  //Enumeration identifier.

	void Init(int iID)
	{
		eElementType = ET_Text;
		this->iID = iID;
	}

	int Size(void)
	{
		return (sizeof(STokenKeywordText)+3)/4;;
	}
};


struct STokenIdentifier
{
	EElement	eElementType;  //ET_Identifier
	int			iID;

	void Init(int iID)
	{
		eElementType = ET_Identifier;
		this->iID = iID;
	}

	int Size(void)
	{
		return (sizeof(STokenIdentifier)+3)/4;
	}
};


struct STokenKeywordChar
{
	EElement	eElementType;	//ET_Char
	char		cValue;

	void Init(char cValue)
	{
		eElementType = ET_Char;
		this->cValue = cValue;
	}

	int Size(void)
	{
		return (sizeof(STokenKeywordChar)+3)/4;
	}
};


struct STokenInteger
{
	EElement	eElementType;	//PT_IntegerType
	int			iValue;

	void Init(int iValue)
	{
		eElementType = PT_IntegerType;
		this->iValue = iValue;
	}

	int Size(void)
	{
		return (sizeof(STokenInteger)+3)/4;
	}
};


struct STokenReal
{
	EElement	eElementType;	//PT_RealType
	float		fValue;

	void Init(float fValue)
	{
		eElementType = PT_RealType;
		this->fValue = fValue;
	}

	int Size(void)
	{
		return (sizeof(STokenReal)+3)/4;
	}
};


struct STokenUnknownText
{
	EElement	eElementType;	//PT_StringType
	int			iNumIntsInString;
	char		szStringCharacters[1];  //Characters without quotes padded to nearest int and zero terminated.

	void Init(char* szText)
	{
		int len;

		eElementType = PT_StringType;
		len = (int)strlen(szText)+1;
		iNumIntsInString = len/4;
		strcpy(szStringCharacters, szText);
	}

	int Size(void)
	{
		return (sizeof(STokenUnknownText)+3)/4 + iNumIntsInString;
	}

	int Size(char* szText)
	{
		int len;

		len = (int)strlen(szText);
		return (sizeof(STokenUnknownText)+3)/4 + len/4;
	}
};


struct STokenUnknownChar
{
	EElement	eElementType;	//PT_CharType
	char		cValue;


	void Init(char cValue)
	{
		eElementType = PT_CharType;
		this->cValue = cValue;
	}

	int Size(void)
	{
		return (sizeof(STokenUnknownChar)+3)/4;
	}
};


int GetTokenSize(int* piCurrent);
int* GetNextToken(int* piCurrent);

#endif //__TOKENS_H__

