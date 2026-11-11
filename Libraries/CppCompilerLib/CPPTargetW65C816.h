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
#ifndef __C_P_P_TARGET_W_65_C_816_H__
#define __C_P_P_TARGET_W_65_C_816_H__
#include "CPPTarget.h"


//The W65C816: int is 16 bits, a pointer is 24 bits and a zero byte, and the linker keeps sections inside 64KB
//banks.  Its numbers are W65C816.txt's.
class CCPPTargetW65C816 : public CCPPTarget
{
public:
	ECPPTarget	GetTarget(void) override;
	const char*	GetConfigurationName(void) override;
	int			GetIntBytes(void) override;
	int			GetPointerBytes(void) override;
	uint32		GetBankSize(void) override;
	uint32		GetStackTop(void) override;
	void		AddDefines(CConfig* pcConfig, bool bAssemblyUnits) override;
	void		AppendStartUp(CChars* pszDest) override;
	void		AppendRuntime(CCPPRuntimeWriter* pcWriter) override;
	void		AppendAdjustThunk(CChars* pszDest, int iAdjust, const char* szFunction) override;
	void		AppendVirtualCallThunk(CChars* pszDest, int iVtableOffset, int iEntryOffset) override;
	bool		HasZeroedBss(void) override;
	bool		NarrowsSubscripts(void) override;
	bool		HasDecimalMode(void) override;
	const char*	GetCountRoutine(bool bLeading, int iBytes) override;
	const char*	GetCountTable(bool bLeading) override;
	bool		InlinesAcrossUnits(void) override;
	const char*	GetAssemblyUnitSuffix(void) override;
	const char**	GetLibc65Units(void) override;
	const char**	GetRuntimeUnits(bool bMathTables) override;
	const char**	GetMathTableUnits(void) override;
	int			GetStubOpcode(void) override;
	const char*	GetRuntimePrefix(void) override;
};


#endif // __C_P_P_TARGET_W_65_C_816_H__

