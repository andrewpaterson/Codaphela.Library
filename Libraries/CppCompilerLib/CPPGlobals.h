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
#ifndef __C_P_P_GLOBALS_H__
#define __C_P_P_GLOBALS_H__
#include "BaseLib/Chars.h"
#include "CPPSyntaxes.h"
#include "CPPSymbol.h"
#include "CPPVariable.h"
#include "CPPUnitData.h"


class CCPPUnit;
class CCPPClass;
class CCPPDiagnostics;
class CCPPDeclarators;
class CCPPContext;
class CCPP65Types;
struct SCPP65Type;


//The unit's data of static storage (CCPPUnit::GetGlobals): global variables, static data members and static locals,
//as data with their constant values and the R_Initialise and R_Finalise that make and destroy what isn't constant;
//references bound to temporaries; the exit lists objects are destroyed from at the end; and the routines written once
//the unit's functions are, the static locals' R_Finalise, R_RunExitLists and R_DynamicCast.  [basic.stc.static],
//[basic.start.init], [basic.start.term], [stmt.dcl] 4
class CCPPGlobals
{
protected:
	CCPPUnit*				mpcUnit;
	CCPPDiagnostics*		mpcDiagnostics;
	CCPPDeclarators*		mpcDeclarators;
	CCPPContext*			mpcContext;
	CCPP65Types*			mpcTypes;
	CCPPUnitData*			mpcData;
	CArrayCPPVariables		mapcStaticLocals;	//Each static local as last defined, which a local class's member functions name (CCPPClosures::EnclosingStatic).
	int						miStatics;			//Static locals in the unit.
	CArrayCPPUnitRoutines	masExitFinalisers;	//The static locals destroyed at exit whose R_Finalise is still to write (WriteExitRoutines).
	bool					mbExitLists;		//Something is pushed on an exit list: the unit writes R_RunExitLists (UseExitLists).
	bool					mbExitListsWritten;
	bool					mbDynamicCast;		//Something casts with dynamic_cast: the unit writes R_DynamicCast (UseDynamicCast).
	bool					mbDynamicCastWritten;

public:
	void					Init(CCPPUnit* pcUnit);
	void					Kill(void);

	bool					GlobalDeclaration(CCPPSyntaxSimpleDeclaration* pcDeclaration, CCPPVariable* pcDefining = NULL);
	bool					ExternLocal(CCPPSyntaxInitDeclarator* pcInit, CCPPSyntaxDeclSpecifiers* pcSpecifiers);
	bool					StaticLocal(CCPPSyntaxInitDeclarator* pcInit, CCPPSyntaxDeclSpecifiers* pcSpecifiers);
	CArrayCPPVariables*		GetStaticLocals(void);
	void					UseExitLists(void);
	void					UseDynamicCast(void);
	bool					WriteExitRoutines(void);
	bool					WriteDynamicCast(void);

protected:
	bool					GlobalInitialiser(SCPP65Type* psType, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode, bool* pbInitialise);
	bool					AnonymousGlobal(CCPPSyntaxClassSpecifier* pcSpecifier);
	CCPPVariable*			RedeclaredVariable(CCPPSymbol* pcSymbol);
	CCPPVariable*			NamespaceVariable(CCPPSymbol* pcSymbol);
	bool					SameVariableType(SCPP65Type* psFirst, SCPP65Type* psSecond);
	void					CompleteDeclaredArrays(CCPPVariable* pcDefined);
	bool					BoundTemporary(SCPP65Type* psReference, CCPPSyntax* pcInitialiser, CChars* pszLabel, bool bGlobal, const char* szOptions, CChars* pszBound, int* piOffset, bool* pbResult);
	bool					GlobalStartUpRoutines(SCPP65Type* psType, CCPPSyntax* pcInitialiser, CChars* pszData, bool bConstruct, bool bDestroy, bool bThread);
	bool					StartUpRoutine(bool bFinalise, SCPP65Type* psType, CCPPSyntax* pcInitialiser, CChars* pszData, CChars* pszNode, CChars* pszFinaliser, bool bEntry, bool bThread);
	bool					IsBss(CCPPSyntax* pcInitialiser, SCPP65Type* psType);
	int						DataAlignment(SCPP65Type* psType, bool bClass);
	int						VariableDataAlignment(CCPPVariable* pcVariable, bool bClass);
	int						ExitNodeBytes(void);
};


#endif // __C_P_P_GLOBALS_H__

