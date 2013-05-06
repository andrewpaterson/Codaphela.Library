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
#ifndef __P_P_TOKEN_H__
#define __P_P_TOKEN_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Define.h"
#include "BaseLib/MemoryStackExtended.h"
#include "GeneralToken.h"


class CPPToken
{
public:
	int		miLine;
	int		miColumn;
	
	virtual void Init(int iLine, int iColumn) = 0;
	virtual void Kill(void) = 0;
	virtual BOOL IsDirective(void);
	virtual BOOL IsText(void);
	virtual BOOL IsWhiteSpace(void);
	virtual BOOL IsReplacement(void);
	virtual BOOL IsHash(void);
	virtual BOOL IsAbstractHolder(void);
	virtual BOOL IsLine(void);
	virtual BOOL IsHolder(void);
	virtual BOOL IsFile(void);
	virtual BOOL IsBlock(void);
	virtual BOOL IsEmpty(void) = 0;
			void Set(int iLine, int iColumn);
	virtual void Append(CChars* psz) = 0;
	virtual void Copy(CPPToken* pcSource, CMemoryStackExtended* pcStack) = 0;
	virtual BOOL Equals(CPPToken* pcOther) = 0;
};


typedef CArrayTemplate<CPPToken*> CArrayPPTokens;


#endif // __P_P_TOKEN_H__

