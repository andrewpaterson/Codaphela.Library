/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#ifndef __I_R_IMPORTS_H__
#define __I_R_IMPORTS_H__
#include "BaseLib/ArrayChars.h"
#include "BaseLib/Mutex.h"
#include "BaseLib/ConditionVariable.h"
#include "IRModule.h"


//Functions other translation units may inline (CIRInliner::SetImports): a copy, in its own module, of each small
//function with external linkage (not local, weak or variadic) that calls no named function and refers to no symbol
//local to its unit (globals and the like may be named: they are the same symbols in any unit, and the inliner maps
//them by name).  Units add their functions after they are optimised (Export); a unit compiled before a function it
//calls was exported can be compiled again (the builder's CompileUnits).
//
//Units compiled at the same time take turns (BeginTurn, EndTurn): each finds and exports functions only in its own
//turn, its place in the order the units are compiled, so it sees what it would if they were compiled one after
//another.
class CIRImports
{
protected:
	CIRModule			mcModule;
	size				muiMaxInstructions;
	CMutex				mcMutex;
	CConditionVariable	mcTurnChanged;
	size				muiTurn;		//The unit whose turn it is.

public:
	void			Init(size uiMaxInstructions);
	void			Kill(void);

	void			ResetTurns(void);
	void			BeginTurn(size uiTurn);
	void			EndTurn(size uiTurn);

	bool			Export(CIRModule* pcModule, CIRFunction* pcFunction);
	CIRFunction*	Find(const char* szName);
	size			NumFunctions(void);
	CIRModule*		GetModule(void);

protected:
	bool			IsExportable(CIRModule* pcModule, CIRFunction* pcFunction);
	bool			IsLocalSymbol(CIRModule* pcModule, SIRReference sValue);
};


#endif // __I_R_IMPORTS_H__
