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
#ifndef __C_P_P65_I_R_CLEANUPS_H__
#define __C_P_P65_I_R_CLEANUPS_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/ArrayTemplate.h"
#include "IRBlock.h"
#include "CPP65Type.h"


//Where a C++ lvalue is: its address (an IR pointer) and its type.
struct SCPPField;
struct SCPP65IRPlace
{
	SIRReference	sAddress;
	SCPP65Type*		psType;
	SCPPField*		psBitField;		//The bit-field the place is (bits of the unit of psType at sAddress), or NULL.

	SCPP65IRPlace(void) : psBitField(NULL) {}
};


//What a cleanup is of.
enum ECPP65IRCleanup
{
	CPP65IRC_Local,			//An object (or array of them) of a scope being written: destroyed when the scope is left.
	CPP65IRC_Temporary,		//A temporary: destroyed at the end of its full expression.
	CPP65IRC_Catch,			//A handler being run: left (the runtime's __EndCatch) when its scope is, by any way out.  Counted with the locals.
};


//One thing to undo.  A temporary made in an operand that may not be worked out (of &&, || or ?:) is conditional: its
//place and an i8 flag (sMade: 1 once it is made) are in the entry block, and it is destroyed only if the flag says it
//was made.  pcUndo is for exceptions (ExceptionsPlan.txt stage 6): the block that undoes this entry and goes on to the
//one below's, made when it is first asked for.
struct SCPP65IRCleanup
{
	ECPP65IRCleanup	eKind;
	SCPP65IRPlace	sPlace;
	bool			bConditional;
	SIRReference	sMade;
	CIRBlock*		pcUndo;
};


typedef CArrayTemplate<SCPP65IRCleanup>	CArrayCPP65IRCleanups;


//The cleanup stack of the function being written (ExceptionsPlan.txt "The front end"): what has been made and must be
//destroyed, in the order it was registered, the oldest first.  It is one stack so that an exception, which leaves from
//the middle of anything, can undo what was made in the reverse of the order it was made.  The normal ways out don't cut
//it at one height: a scope's end destroys its locals, a full expression's end its temporaries, and a local is registered
//while its initialiser's temporaries are still to be destroyed (they are under it).  So each kind is counted by itself:
//a mark is how many locals, or how many temporaries, there were, and what is above a mark is the entries of that kind
//registered since.  CCPP65IRFrontEnd writes the destructors; this only keeps what there is to destroy.
class CCPP65IRCleanups
{
protected:
	CArrayCPP65IRCleanups	masEntries;
	size					muiLocals;
	size					muiTemporaries;

public:
	void				Init(void);
	void				Kill(void);
	void				ReInit(void);

	size				NumLocals(void) const;
	size				NumTemporaries(void) const;
	size				NumEntries(void) const;
	SCPP65IRCleanup*	GetEntry(size uiIndex);

	void				PushLocal(SCPP65IRPlace* psPlace);
	void				PushTemporary(SCPP65IRPlace* psPlace, bool bConditional, SIRReference sMade);
	void				PushCatch(void);
	SCPP65IRCleanup*	GetLocal(size uiOrdinal);
	SCPP65IRCleanup*	GetTemporary(size uiOrdinal);
	void				CutLocals(size uiCount);
	void				CutTemporaries(size uiCount);
	bool				TakeTemporary(SIRReference sAddress, SCPP65IRPlace* psPlace);

protected:
	SCPP65IRCleanup*	Get(bool bTemporary, size uiOrdinal);
	void				Cut(bool bTemporary, size uiCount);
	void				ForgetUndoFrom(size uiIndex);
};


#endif // __C_P_P65_I_R_CLEANUPS_H__

