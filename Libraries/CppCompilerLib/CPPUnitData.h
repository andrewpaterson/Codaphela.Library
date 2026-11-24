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
#ifndef __C_P_P_UNIT_DATA_H__
#define __C_P_P_UNIT_DATA_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplate.h"


typedef CArrayTemplate<CChars>	CArrayCPPStrings;


struct SCPP65Type;


//A routine the unit writes once its functions are: a static local's R_Finalise (CCPPGlobals::WriteExitRoutines) or the function
//that destroys a thrown object (WriteThrownDestroyers).
struct SCPPUnitRoutine
{
	CChars			szLabel;	//The static local's, for its R_Finalise; the destroyer's own.
	SCPP65Type*		psType;		//What it destroys.
	bool			bShared;	//A shared function's static local: its R_Finalise is a COMDAT.
};


typedef CArrayTemplate<SCPPUnitRoutine>	CArrayCPPUnitRoutines;


//What a unit has other than its code, as the assembly it is written in: each global in a section of its own
//(data.label, or bss.label for one that starts as zeros), the entries of the init and fini tables for the globals
//that are constructed and destroyed, whatever else the unit puts with them (vtables and type infos, through
//GetText), and the string literals, which are kept apart until the unit is finished (AppendStrings).  It needs
//nothing of the unit.
//An exit list node's alignment; its size is two pointers' (CCPPGlobals::ExitNodeBytes): the next node and the routine
//that destroys its object.
#define CPP65_EXIT_NODE_ALIGNMENT	4


class CCPPUnitData
{
protected:
	CChars				mszData;
	CArrayCPPStrings	maszStrings;
	int					miStartUps;		//R_Initialise and R_Finalise routines written.
	bool				mbDataBss;		//The last BeginData was bss: the global's start up routines associate with bss.label.

public:
	void		Init(void);
	void		Kill(void);

	void		BeginData(CChars* pszLabel, int iAlignment, bool bGlobal, const char* szOptions, bool bBss = false);
	void		AppendSpace(int iBytes);
	void		AppendData(int iBytes, int64 lliValue);
	void		AppendZeros(int iBytes);
	void		AppendAddress(CChars* pszLabel, int iOffset, int iBytes);
	void		AppendStartUpEntry(const char* szTable, CChars* pszLabel, CChars* pszData, bool bKeep = false);
	void		AppendExitNode(CChars* pszLabel, CChars* pszData, int iBytes);
	int			GetStartUp(void) const;
	void		NextStartUp(void) { miStartUps++; }
	CChars*		GetText(void);

	int			AddString(char* szBytes, size uiLength);
	size		NumStrings(void) const;
	void		AppendStrings(CChars* pszDest);

	static void	AppendSection(CChars* psz, const char* szKind, CChars* pszName, const char* szOptions);
	static void	AppendGlobal(CChars* psz, CChars* pszLabel);
};


#endif // __C_P_P_UNIT_DATA_H__
