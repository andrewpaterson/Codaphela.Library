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
#ifndef __I_R_IMPORTS_H__
#define __I_R_IMPORTS_H__
#include "BaseLib/ArrayChars.h"
#include "BaseLib/Mutex.h"
#include "BaseLib/ConditionVariable.h"
#include "IRModule.h"
#include "NameSet.h"


//How a unit arrives at its turn (CIRImports::Arrive).
enum EIRArrival
{
	IRA_Turn,		//It is the unit's turn: it goes on.
	IRA_Parked,		//It waits, without a thread: it is resumed (IRImportsResume) in its turn, or when it is stale.
	IRA_Stale,		//A function it looked for and didn't find has been exported: it does its work again first.
};


//Resumes a parked unit (pvUnit, given to Arrive): in its turn, or (bStale) to do its work again before its turn.
typedef void (*IRImportsResume)(void* pvContext, void* pvUnit, bool bStale);


//A unit waiting for its turn (CIRImports::Arrive).
struct SIRParkedUnit
{
	size		uiTurn;
	void*		pvUnit;
	CNameSet*	pcNotFound;		//The functions it looked for and didn't find, NULL if it isn't resumed when one is exported.
};


typedef CArrayTemplate<SIRParkedUnit> CArrayIRParkedUnits;


//Functions other translation units may inline (CIRInliner::SetImports): a copy, in its own module, of each small
//function with external linkage (not local, weak or variadic) that refers to no symbol local to its unit (globals,
//the functions it calls and the like may be named: they are the same symbols in any unit, and the inliner maps them
//by name, with whether they are restrict or called in registers).  Units add their functions after they are optimised (Export); a unit compiled before a function it
//calls was exported can be compiled again (the builder's CompileUnits).
//
//Units compiled at the same time take turns (BeginTurn, EndTurn): each exports functions only in its own turn, its
//place in the order the units are compiled, so it is as if they were compiled one after another.  A unit may find
//functions before its turn (between BeginRead and EndRead), keeping the names it didn't find: in its turn, if none
//of them has been exported since (Missed), it found what it would have in its turn (an exported function never
//changes).  Otherwise it does its work again, in its turn.
//
//A unit that arrives before its turn (Arrive) doesn't hold a thread waiting: it is parked, and resumed (SetResume's
//function) when the turn before it ends, or as soon as a function it didn't find is exported (it does its work again
//then, so it misses fewer in its turn).
class CIRImports
{
protected:
	CIRModule			mcModule;
	size				muiMaxInstructions;
	CMutex				mcMutex;
	CConditionVariable	mcTurnChanged;
	size				muiTurn;		//The unit whose turn it is.
	CMutex				mcModuleLock;	//Held shared by units finding functions before their turn, exclusive to export.
	size				muiMissed;		//Turns a unit missed a function exported after it looked (Missed).
	size				muiStale;		//Units that found they had missed one before their turn (Arrive).
	CArrayIRParkedUnits	masParked;		//Units waiting for their turns (Arrive).
	IRImportsResume		mfResume;
	void*				mpvResumeContext;
	bool				mbForceMisses;	//Every unit misses once (SetForceMisses), for testing.
	size				muiForcedTurn;	//The turn last made to miss.

public:
	void			Init(size uiMaxInstructions);
	void			Kill(void);

	void			ResetTurns(void);
	void			BeginTurn(size uiTurn);
	void			EndTurn(size uiTurn);
	void			BeginRead(void);
	void			EndRead(void);
	bool			Missed(CArrayChars* paszNotFound);
	void			SetResume(IRImportsResume fResume, void* pvContext);
	EIRArrival		Arrive(size uiTurn, void* pvUnit, CArrayChars* paszNotFound);
	size			NumMissed(void);
	size			NumStale(void);
	void			SetForceMisses(bool bForceMisses);

	bool			Export(CIRModule* pcModule, CIRFunction* pcFunction);
	CIRFunction*	Find(const char* szName);
	size			NumFunctions(void);
	CIRModule*		GetModule(void);

protected:
	bool			IsExportable(CIRModule* pcModule, CIRFunction* pcFunction);
	bool			AnyFound(CArrayChars* paszNames);
	void			Exported(const char* szName);
	bool			IsLocalSymbol(CIRModule* pcModule, SIRReference sValue);
};


#endif // __I_R_IMPORTS_H__
