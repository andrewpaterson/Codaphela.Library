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
#ifndef __C_P_P_CLASS_DATA_H__
#define __C_P_P_CLASS_DATA_H__
#include "BaseLib/Chars.h"
#include "CPPVtable.h"


class CCPPUnit;
class CCPPClass;
class CCPPFunction;
struct SCPP65Type;


//Writes a class's data for the unit (CCPPUnit::GetClassData), each once, as its label is asked for: its vtables and
//VTT (Itanium's, each a COMDAT that every unit with the class writes alike), its type info and a type's typeid name,
//and the thunks that call a virtual function through a pointer to a member.
class CCPPClassData
{
protected:
	CCPPUnit*				mpcUnit;

public:
	void					Init(CCPPUnit* pcUnit);
	void					Kill(void);

	void					EnsureVtable(CCPPClass* pcClass);
	void					AppendVtableParts(CCPPClass* pcClass, CChars* pszLabel, CArrayCPPVtableEntries* pasEntries);
	void					EnsureVTT(CCPPClass* pcClass);
	void					AppendVTTEntries(CCPPClass* pcComplete, CCPPClass* pcClass, int iOffset, CChars* pszWords);
	size					VTTEntries(CCPPClass* pcClass);
	int						VTTIndex(CCPPClass* pcClass, CCPPClass* pcBase);
	void					AppendVtableHeader(CCPPClass* pcClass, int iVtablePointer);
	void					EnsureTypeInfo(CCPPClass* pcClass);
	void					AppendTypeInfoBases(CCPPClass* pcClass, int iOffset, uint32 uiFlags, uint32 uiHigh);
	void					AppendTypeInfoHeader(CChars* pszName, uint32 uiKind);
	void					AppendTypeInfoName(CChars* pszInfo, SCPP65Type* psType, bool bShared, CChars* pszName);
	void					AsmRoutine(const char* szText, CChars* pszLabel);
	void					AppendVtableEntry(CCPPFunction* pcFunction, int iAdjust);
	void					AppendVirtualCallThunk(CChars* psz, CCPPFunction* pcFunction);
	void					AppendThunkLabel(CChars* pszLabel, bool bComdat);
	void					AppendTypeidLabel(CChars* psz, SCPP65Type* psType);
};


#endif // __C_P_P_CLASS_DATA_H__

