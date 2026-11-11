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
#ifndef __C_P_P_TARGET_65_C_64_H__
#define __C_P_P_TARGET_65_C_64_H__
#include "CPPTarget.h"


//The 65C64: int is 32 bits and A is 32 bits wide, so narrow results are returned widened.  Its numbers are the
//generator's (CPP65Generator.txt Memory map).
class CCPPTarget65C64 : public CCPPTarget
{
public:
	ECPPTarget	GetTarget(void) override;
	const char*	GetConfigurationName(void) override;
	int			GetIntBytes(void) override;
	int			GetPointerBytes(void) override;
	bool		WidensResults(void) override;
	uint32		GetStackTop(void) override;
	void		AddDefines(CConfig* pcConfig, bool bAssemblyUnits) override;
	void		AppendStartUp(CChars* pszDest) override;
	void		AppendRuntime(CCPPRuntimeWriter* pcWriter) override;
	void		AppendAdjustThunk(CChars* pszDest, int iAdjust, const char* szFunction) override;
	void		AppendVirtualCallThunk(CChars* pszDest, int iVtableOffset, int iEntryOffset) override;

protected:
	void		AppendDivideSigned(CChars* pszCode, const char* szName, int iBytes);
};


#endif // __C_P_P_TARGET_65_C_64_H__

