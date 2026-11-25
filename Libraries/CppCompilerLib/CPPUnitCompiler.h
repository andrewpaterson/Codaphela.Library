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
#ifndef __C_P_P_UNIT_COMPILER_H__
#define __C_P_P_UNIT_COMPILER_H__
#include "CppTokeniser.h"
#include "CppParser.h"
#include "CPPUnit.h"
#include "CPP65Assembly.h"
#include "CPP65UnitWorkspace.h"
#include "CPPUnitSchedule.h"


//One unit of the project compiled from its source to its object, by the schedule (CCPPUnitSchedule::CompileUnit):
//preprocessed, tokenised, parsed, generated in its turn with the imports, its listing written and assembled.  Kept
//while the unit waits for its turn without a thread; the schedule frees it when it has finished (Compiled).
class CCPPUnitCompiler
{
protected:
	CCPPUnitSchedule*		mpcSchedule;		//What compiles the units, the imports and the workspaces...
	CCPPBuild*				mpcBuild;			//...what they are built for and how...
	CCPPProgramLinker*		mpcProgram;			//...where their objects go...
	CProject*				mpcProject;			//...and the project they are in.
	SCPP65CompiledUnit*		mpsUnit;
	CCPPTokeniser			mcTokeniser;
	CCPPSyntaxTree			mcTree;
	CCPPParser				mcParser;
	CCPPUnit				mcGenerator;
	CCPP65Assembly			mcAssembly;
	CCPP65UnitWorkspace*	mpcWorkspace;
	bool					mbResult;		//Preprocessed, tokenised and parsed.
	bool					mbGenerated;	//Its generation before its turn went well.
	size					muiAttempts;	//Times it was generated again before its turn (stale).
	bool					mbFirst;		//Compiled for the first time, not again.

public:
	void	Init(CCPPUnitSchedule* pcSchedule, SCPP65CompiledUnit* psUnit, bool bFirst);
	void	Compile(void);
	void	Continue(void);
	void	Restart(void);
	SCPP65CompiledUnit*	GetUnit(void);

protected:
	void	InitGenerator(void);
	void	StartGenerating(void);
	void	Finish(bool bGenerated);
};


#endif // __C_P_P_UNIT_COMPILER_H__
