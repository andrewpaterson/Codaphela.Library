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
class CCPPClass;
class CCPPFunction;
class CCPPSyntax;
struct SCPP65IRPlace
{
	SIRReference	sAddress;
	SCPP65Type*		psType;
	SCPPField*		psBitField;		//The bit-field the place is (bits of the unit of psType at sAddress), or NULL.
	bool			bEither;		//c ? x : y where x or y is a bit-field: which bits of the unit it is was picked with it.
	SIRReference	sFirst;			//...all ones (a psType) when x was picked, psBitField's bits (the whole unit's for NULL), and 0 for y...
	SCPPField*		psElseBitField;	//...this bit-field's bits (or the whole unit's).

	SCPP65IRPlace(void) : psBitField(NULL), bEither(false), psElseBitField(NULL) {}
	bool	IsBitField(void) const { return (psBitField != NULL) || bEither; }
};


//What a cleanup is of.
enum ECPP65IRCleanup
{
	CPP65IRC_Local,			//An object (or array of them) of a scope being written: destroyed when the scope is left.
	CPP65IRC_Temporary,		//A temporary: destroyed at the end of its full expression.
	CPP65IRC_Catch,			//A handler being run: left (the runtime's __EndCatch) when its scope is, by any way out.  Counted with the locals.
	CPP65IRC_Part,			//What only an exception undoes (ECPP65IRPart): counted with neither, so no normal way out sees it.
};


//What an exception undoes that a normal way out doesn't (ExceptionsPlan.txt stage 7: what is half made).
enum ECPP65IRPart
{
	CPP65IRP_Destroy,		//A part made so far (a base, a member, an element, a delegated constructor's object): sPlace, with
	//pcBaseObject's base version of its destructor when it is a base.
	CPP65IRP_Deallocate,	//A new expression's memory, sPlace.sAddress, while its object is made: given back to pcFunction (the
	//matching operator delete, called as pcFor's, with asValues after the pointer), or the runtime's
	//szRuntime.  [expr.new] 20
	CPP65IRP_FreeException,	//A thrown object being made, sPlace.sAddress (the runtime's __FreeException).
	CPP65IRP_Elements,		//new T[n]'s elements of sPlace.psType from sPlace.sAddress, as many as the count in sCount's slot.
	CPP65IRP_Guard,			//A static local being made: its guard byte, sPlace.sAddress, goes back to 0.  [stmt.dcl] 4
};


#define CPP65_IR_PART_VALUES	4	//The placement arguments a Deallocate part keeps for a placement operator delete.


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

	//A part's (CPP65IRC_Part).
	ECPP65IRPart	ePart;
	CCPPClass*		pcBaseObject;
	CCPPFunction*	pcFunction;
	CCPPSyntax*		pcFor;
	const char*		szRuntime;
	SIRReference	sCount;
	SIRReference	asValues[CPP65_IR_PART_VALUES];
	SCPP65Type*		apsValueTypes[CPP65_IR_PART_VALUES];
	size			uiValues;

	void			Clear(ECPP65IRCleanup eNewKind);
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
	void				PushPart(SCPP65IRCleanup* psPart);
	void				InsertPart(size uiIndex, SCPP65IRCleanup* psPart);
	void				RemovePartsFrom(size uiIndex);
	bool				RemoveNewestPart(void);
	size				IndexOfLocal(size uiOrdinal);
	size				IndexOfTemporary(size uiOrdinal);
	SCPP65IRCleanup*	GetLocal(size uiOrdinal);
	SCPP65IRCleanup*	GetTemporary(size uiOrdinal);
	void				CutLocals(size uiCount);
	void				CutTemporaries(size uiCount);
	bool				TakeTemporary(SIRReference sAddress, SCPP65IRPlace* psPlace);

protected:
	SCPP65IRCleanup*	Get(bool bTemporary, size uiOrdinal, size* puiIndex = NULL);
	static bool			IsCounted(SCPP65IRCleanup* psEntry, bool bTemporary);
	void				Cut(bool bTemporary, size uiCount);
	void				ForgetUndoFrom(size uiIndex);
};


#endif // __C_P_P65_I_R_CLEANUPS_H__

