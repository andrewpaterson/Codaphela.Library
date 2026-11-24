/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __CPP65_DEBUG_WRITER_H__
#define __CPP65_DEBUG_WRITER_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/HashMapPrimitiveTemplate.h"
#include "BaseLib/HashMapString.h"
#include "CPP65Type.h"


class CCPPUnit;
class CCPPClass;
class CCPPSymbol;
class CCPPVariable;


//A type the unit describes (.dtype), in the order it was first asked for: its index is its place.
struct SCPP65DebugType
{
	CChars		szKey;
	CChars		szName;
	const char*	szKind;		//The directive's word: integer, pointer, class...
	uint8		uiFlags;	//Assembler65Lib's ADTF_.
	int			iBytes;
	int32		iTarget;	//The type it is of, -1 for none.
	int			iCount;
	CChars		szMembers;	//Its .dmember lines.
};


typedef CArrayTemplate<SCPP65DebugType>	CArrayCPP65DebugTypes;


class CIRWrittenFunction;


//Writes the debug directives a unit's assembly describes itself with for a debugger (Assembler65Lib's
//CAsm65DebugRecords, the .d* directives): what the front end knows of a function (its name and its variables) and the
//types they have.  A unit's types are described once each, at the unit's end (AppendTypes), every one the variables
//need (TypeIndex) and the ones those need.  Where a function's frame is made is the back end's (CIRBackend::
//DescribeFrame).
class CCPP65DebugWriter
{
protected:
	CCPPUnit*					mpcUnit;
	CArrayCPP65DebugTypes		masTypes;
	CHashMapLongTemplate<int32>	mcIndices;		//Each type's index, by its SCPP65Type...
	CHashMapString<int32>		mcKeys;			//...and by its key: two of the compiler's types can be one (an enum's int and int).

public:
	void			Init(CCPPUnit* pcUnit);
	void			Kill(void);

	int32			TypeIndex(SCPP65Type* psType);
	void			AppendVariable(CChars* pszDest, const char* szName, SCPP65Type* psType, bool bParameter, const char* szWhere, int32 iOffset, int iFirstLine, int iLastLine);
	void			AppendGlobal(CChars* pszDest, CCPPVariable* pcVariable, const char* szLabel, bool bStatic);
	void			AppendTypes(CChars* pszDest);

	static void		AppendString(CChars* pszDest, const char* szText);
	static bool		DescribeFunction(CChars* pszCode, const char* szLabel, const char* szName, int32 iResult, CIRWrittenFunction* pcWritten);
	static size		FindAfterLabel(const char* szCode, const char* szLabel);

protected:
	void			Describe(int32 iIndex, SCPP65Type* psType, CChars* pszKey);
	void			DescribeClass(int32 iIndex, CCPPClass* pcClass, CChars* pszMembers);
	void			DescribeEnum(CCPPSymbol* pcEnum, CChars* pszMembers);
	void			AppendKey(SCPP65Type* psType, CChars* pszDest);
	void			AppendPlace(int iToken, CChars* pszDest);
	void			AppendMember(CChars* pszDest, const char* szKind, const char* szName, int32 iType, int64 iValue, int iBitOffset, int iBits);
};


#endif // __CPP65_DEBUG_WRITER_H__
