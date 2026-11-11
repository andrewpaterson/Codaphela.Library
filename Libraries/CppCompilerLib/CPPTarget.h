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
#ifndef __C_P_P_TARGET_H__
#define __C_P_P_TARGET_H__
#include "BaseLib/PrimitiveTypes.h"


//The targets there are, to choose one by (GetCPPTarget).
enum ECPPTarget
{
	CPPT_65C64,
	CPPT_W65C816,
	CPPT_Host,			//Code that runs on the PC (MSVC's x64 sizes), only checked (CCPPSemanticPass): never generated.
	CPPT_NumTargets
};


class CConfig;
class CChars;
class CArrayChars;


//What a target writes its runtime routines through (CCPPTarget::AppendRuntime): the unit's code, as the generator has
//it.
class CCPPRuntimeWriter
{
public:
	virtual CArrayChars*	GetRuntimeCalls(void) =0;				//The runtime routines the unit's code calls, by name.
	virtual bool			BeginRuntime(const char* szName) =0;	//A routine's section and label, in a COMDAT section so the linker keeps one; false if the unit has it already.
	virtual CChars*			GetRuntimeCode(void) =0;				//Where the routine's code goes, after BeginRuntime.
	virtual void			RuntimeError(const char* szMessage) =0;
};


//What the C++ compiler needs to know about the processor it compiles for, and what differs between the processors.
//The generator, the IR front end and CCPP65Builder ask it rather than test which processor it is.  A target has no
//state: there is one of each (GetCPPTarget), used by every unit compiled for it.  What isn't overridden is the answer
//for a target with nothing special.
class CCPPTarget
{
public:
	virtual ECPPTarget	GetTarget(void) =0;
	virtual const char*	GetConfigurationName(void) =0;	//The project configuration units are preprocessed with.

	//Sizes.
	virtual int			GetIntBytes(void) =0;			//int and unsigned int.  long is 4 on every target.
	virtual int			GetPointerBytes(void) =0;		//A pointer's (and a reference's) size in memory.
	virtual bool		WidensResults(void);			//Results narrower than 32 bits are returned as i32.
	virtual uint32		GetBankSize(void);				//The linker keeps sections inside banks of this size (0 for none).
	virtual uint32		GetStackTop(void);				//S as the start up sets it.

	//The preprocessor's configuration.
	virtual void		AddDefines(CConfig* pcConfig, bool bAssemblyUnits);

	//What the generator and the IR front end write differently.
	virtual void		AppendStartUp(CChars* pszDest);			//A program's start up, as an object linked first.
	virtual void		AppendRuntime(CCPPRuntimeWriter* pcWriter);	//The runtime routines the unit's code calls that the unit writes itself.
	virtual void		AppendAdjustThunk(CChars* pszDest, int iAdjust, const char* szFunction);	//After a thunk's label: this moved by iAdjust, then a jump to szFunction.
	virtual void		AppendVirtualCallThunk(CChars* pszDest, int iVtableOffset, int iEntryOffset);	//After its label: a jump to the function at iEntryOffset in the vtable whose pointer is at iVtableOffset in the object.
	virtual bool		HasZeroedBss(void);				//Globals with no initialiser go in bss, which the start up clears.
	virtual bool		NarrowsSubscripts(void);		//An index into an array of less than 64KB is scaled as 16 bits.
	virtual bool		HasDecimalMode(void);			//__builtin_bcd_add and __builtin_bcd_subtract.
	virtual const char*	GetCountRoutine(bool bLeading, int iBytes);		//The runtime routine that counts an integer's leading or trailing zeros, or NULL.
	virtual const char*	GetCountTable(bool bLeading);	//The runtime's table of each byte's leading or trailing zeros, or NULL.

	//What a program is built with.
	virtual bool		InlinesAcrossUnits(void);		//Units import each other's small functions, in turns, and may be compiled again.
	virtual bool		HasAssemblyUnits(void);			//A unit X.cpp may have the target's assembly beside it.
	virtual const char*	GetAssemblyUnitSuffix(void);	//What that unit is called after X: "816.asm".
	virtual const char**	GetLibc65Units(void);			//Libc65's units for the target, ended by NULL, or NULL for none.
	virtual const char**	GetRuntimeUnits(bool bMathTables);	//The back end's runtime routines' units in Libc65, or NULL.
	virtual const char**	GetMathTableUnits(void);		//The maths tables' units in Libc65/MathTbls, or NULL.
	virtual int			GetStubOpcode(void);			//The one byte a missing label's stub is, or -1 for the linker's own.
	virtual const char*	GetRuntimePrefix(void);			//What the names of the runtime routines that must be linked start with, or NULL.
};


CCPPTarget*	GetCPPTarget(ECPPTarget eTarget);


#endif // __C_P_P_TARGET_H__

