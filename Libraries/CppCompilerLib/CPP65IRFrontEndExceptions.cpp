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
#include "CTIdentifier.h"
#include "CPPUnit.h"
#include "CPP65IRFrontEnd.h"


//The IR front end's exceptions (ExceptionsPlan.txt): the test after a call that may throw, where an exception goes from
//any place in a function (the cleanup stack's entries undone, newest first, then the handlers of the try it is in or
//the function's own way out), throw, and try with its handlers.  Libc65's exception.cpp is the runtime: where the
//thrown object lives and which handler takes it.  None of it is written for a program built without exceptions.


//The runtime's routines: C functions, so each label is the name with an underscore.
#define CPP65_ALLOCATE_EXCEPTION	"___AllocateException"
#define CPP65_THROW					"___Throw"
#define CPP65_RETHROW				"___Rethrow"
#define CPP65_MATCH					"___Match"
#define CPP65_BEGIN_CATCH			"___BeginCatch"
#define CPP65_END_CATCH				"___EndCatch"
#define CPP65_TERMINATE				"___Terminate"


//////////////////////////////////////////////////////////////////////////
//
// Whether a call of pcFunction (NULL: through a pointer, whose function
// isn't known) may be left with an exception: in a program built with
// exceptions, any function not declared noexcept.  [except.spec]
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CallMayThrow(CCPPFunction* pcFunction)
{
	return mpcUnit->GetOptions()->mbExceptions && !(pcFunction && mpcUnit->IsNoexcept(pcFunction));
}


//////////////////////////////////////////////////////////////////////////
//
// Straight after a call's instruction: if the call may throw, the test of
// whether it did, and the way an exception goes from here (Landing).  The
// code goes on in a new block, for the way that didn't.  While a cleanup is
// being written for an exception on its way (mbUnwinding), a second one is
// std::terminate.  [except.ctor] 3
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::TestThrown(CCPPFunction* pcFunction)
{
	SIRReference	sThrown;
	CIRBlock*		pcLanding;
	CIRBlock*		pcNext;

	if (!CallMayThrow(pcFunction) || mbUnsupported)
	{
		return;
	}
	sThrown = mpcFunction->AddTemporary(IRType(IRTK_I1));
	mpcBlock->AddInstruction(IRO_Thrown, IRType(IRTK_I1), sThrown, IRNone(), IRNone());
	pcLanding = mbUnwinding ? TerminateBlock() : Landing();
	pcNext = NewBlock();
	mpcBlock->Branch(sThrown, pcLanding, pcNext);
	StartBlock(pcNext);
}


//////////////////////////////////////////////////////////////////////////
//
// Where an exception goes from here: the block that undoes the newest
// entry of the cleanup stack (which goes on to the one below's), down to the
// try being written, then that try's handlers; with no try, the function's
// way out.
//
//////////////////////////////////////////////////////////////////////////
CIRBlock* CCPP65IRFrontEnd::Landing(void)
{
	size	uiBase;

	uiBase = (masTries.NumElements() > 0) ? masTries.Tail()->uiBase : 0;
	if (mcCleanups.NumEntries() > uiBase)
	{
		return UndoBlock(mcCleanups.NumEntries() - 1);
	}
	return LandingBelow();
}


//////////////////////////////////////////////////////////////////////////
//
// Where an exception goes once every cleanup of the try being written is
// undone: its handlers; with no try, the function's way out.
//
//////////////////////////////////////////////////////////////////////////
CIRBlock* CCPP65IRFrontEnd::LandingBelow(void)
{
	return (masTries.NumElements() > 0) ? masTries.Tail()->pcDispatch : WayOut();
}


//////////////////////////////////////////////////////////////////////////
//
// The block that undoes the cleanup stack's entry uiIndex and goes on to
// what is below it: made when it is first asked for and kept in the entry,
// so a function has one for each thing it must undo however many calls can
// throw past it.  The cleanup stack forgets the block when what is below
// the entry changes (CCPP65IRCleanups::Cut).
//
//////////////////////////////////////////////////////////////////////////
CIRBlock* CCPP65IRFrontEnd::UndoBlock(size uiIndex)
{
	SCPP65IRCleanup	sEntry;
	CIRBlock*		pcUndo;
	CIRBlock*		pcBelow;
	CIRBlock*		pcBlock;
	size			uiBase;
	bool			bUnwinding;

	sEntry = *mcCleanups.GetEntry(uiIndex);
	if (sEntry.pcUndo)
	{
		return sEntry.pcUndo;
	}
	pcUndo = NewBlock();
	mcCleanups.GetEntry(uiIndex)->pcUndo = pcUndo;
	uiBase = (masTries.NumElements() > 0) ? masTries.Tail()->uiBase : 0;
	pcBelow = (uiIndex > uiBase) ? UndoBlock(uiIndex - 1) : LandingBelow();

	pcBlock = mpcBlock;
	bUnwinding = mbUnwinding;
	mbUnwinding = true;
	StartBlock(pcUndo);
	UndoEntry(&sEntry);
	JumpTo(pcBelow);
	mbUnwinding = bUnwinding;
	StartBlock(pcBlock);
	return pcUndo;
}


//////////////////////////////////////////////////////////////////////////
//
// What undoes one entry, on either way out of where it lives: a local's or
// a temporary's destructor (a conditional temporary's only if it was made),
// a handler's end.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::UndoEntry(SCPP65IRCleanup* psEntry)
{
	switch (psEntry->eKind)
	{
	case CPP65IRC_Temporary:
		return DestroyTemporary(psEntry);
	case CPP65IRC_Catch:
		ExceptionCall(CPP65_END_CATCH, IRType(IRTK_None), NULL, NULL, 0);
		return true;
	default:
		return Destroy(psEntry->sPlace.sAddress, psEntry->sPlace.psType);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The function's way out for an exception nothing in it handles: a block
// that ends in throw (the caller's thrown is true), one for the function.
// A function declared noexcept, a destructor that isn't noexcept(false) and
// the routines the compiler writes for itself (an object's initialiser run
// at start up, a finaliser: mbRoutine) have none: std::terminate.
// [except.spec] 9
//
//////////////////////////////////////////////////////////////////////////
CIRBlock* CCPP65IRFrontEnd::WayOut(void)
{
	if (mbRoutine || !mpcCPPFunction || mpcUnit->IsNoexcept(mpcCPPFunction))
	{
		return TerminateBlock();
	}
	if (!mpcThrowExit)
	{
		mpcThrowExit = NewBlock();
		mpcThrowExit->Throw();
	}
	return mpcThrowExit;
}


//////////////////////////////////////////////////////////////////////////
//
// A block that calls std::terminate, one for the function.
//
//////////////////////////////////////////////////////////////////////////
CIRBlock* CCPP65IRFrontEnd::TerminateBlock(void)
{
	CIRBlock*	pcBlock;

	if (!mpcTerminate)
	{
		mpcTerminate = NewBlock();
		pcBlock = mpcBlock;
		StartBlock(mpcTerminate);
		ExceptionCall(CPP65_TERMINATE, IRType(IRTK_None), NULL, NULL, 0);
		mpcBlock->Unreachable();
		StartBlock(pcBlock);
	}
	return mpcTerminate;
}


//////////////////////////////////////////////////////////////////////////
//
// Calls one of the runtime's routines (Libc65's exception.cpp) with its
// arguments on the stack, as any caller of a C function may.  None of them
// throws.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CCPP65IRFrontEnd::ExceptionCall(const char* szLabel, SIRType sResult, SIRType* pasTypes, SIRReference* pasArguments, int iArguments)
{
	SIRReference	sSymbol;
	int				i;

	sSymbol = mcModule.Symbol(szLabel);
	mcModule.SetNoThrow(sSymbol.Index(), true);
	for (i = 0; i < iArguments; i++)
	{
		mpcFunction->Argument(mpcBlock, pasTypes[i], pasArguments[i]);
	}
	return mpcFunction->Call(mpcBlock, sResult, sSymbol);
}


//////////////////////////////////////////////////////////////////////////
//
// The type of the object throw makes from an operand of psType: without a
// reference or top-level const and volatile, an array as a pointer to its
// first element and a function as a pointer to it.  [except.throw] 3
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65IRFrontEnd::ThrownType(SCPP65Type* psType)
{
	psType = psType->WithoutReference()->Unqualified();
	if (psType->IsArray() || psType->IsFunction())
	{
		return mpcTypes->Pointer(psType->IsArray() ? psType->psBase : psType);
	}
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// A type's type info, as a symbol (CCPPUnit::AppendTypeidLabel writes it).
//
//////////////////////////////////////////////////////////////////////////
SIRReference CCPP65IRFrontEnd::TypeInfoSymbol(SCPP65Type* psType)
{
	CChars			szLabel;
	SIRReference	sSymbol;

	szLabel.Init();
	mpcUnit->AppendTypeidLabel(&szLabel, psType);
	sSymbol = mcModule.Symbol(szLabel.Text());
	if (psType->IsClass() && psType->pcClass->IsUnnamed())
	{
		mcModule.SetLocal(sSymbol.Index());
	}
	szLabel.Kill();
	return sSymbol;
}


//////////////////////////////////////////////////////////////////////////
//
// throw operand: the exception object is made from the operand in memory
// the runtime gives (as a variable of the thrown type is made from its
// initialiser), handed to the runtime with its type and what destroys it,
// and the exception goes on its way (Landing).  throw; with no operand
// sends the exception being handled on again.  What follows isn't reached.
// The expression is void.  [except.throw]
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ThrowExpression(CCPPSyntaxThrowExpression* pcThrow, SCPP65IRValue* psValue)
{
	SCPP65Type*		psType;
	SCPP65IRPlace	sPlace;
	SCPP65IRValue	sValue;
	SIRType			asTypes[3];
	SIRReference	asArguments[3];
	CChars			szDestroyer;

	psValue->psType = &mpcTypes->msVoid;
	psValue->sValue = IRNone();
	if (!mpcUnit->GetOptions()->mbExceptions)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (!pcThrow->mpcOperand)
	{
		ExceptionCall(CPP65_RETHROW, IRType(IRTK_None), NULL, NULL, 0);
		JumpTo(Landing());
		return Terminated();
	}

	psType = mpcUnit->TypeOf(pcThrow->mpcOperand);
	if (!psType)
	{
		mpcUnit->ReportTypeOf(pcThrow->mpcOperand);
		return Unsupported(__FILE__, __LINE__);
	}
	psType = ThrownType(psType);
	if (psType->IsVoid() || (psType->IsClass() && !mpcUnit->CompleteClass(psType->pcClass, pcThrow)) || (!psType->IsClass() && !IsSupportedType(psType)))
	{
		return Unsupported(__FILE__, __LINE__);
	}

	asTypes[0] = IRTypeOf(&mpcTypes->msUnsignedInt);
	asArguments[0] = mpcFunction->AddConstant(asTypes[0], (uint64)psType->iBytes);
	sPlace.sAddress = ExceptionCall(CPP65_ALLOCATE_EXCEPTION, IRType(IRTK_Pointer), asTypes, asArguments, 1);
	sPlace.psType = psType;
	if (psType->IsClass())
	{
		if (!Construct(&sPlace, NULL, pcThrow->mpcOperand))
		{
			return false;
		}
	}
	else
	{
		if (!ValueFor(pcThrow->mpcOperand, psType, &sValue) || !ConvertImplicit(&sValue, psType, pcThrow->mpcOperand))
		{
			return false;
		}
		Store(&sPlace, &sValue);
	}

	asTypes[0] = IRType(IRTK_Pointer);
	asTypes[1] = IRType(IRTK_Pointer);
	asTypes[2] = IRType(IRTK_Pointer);
	asArguments[0] = sPlace.sAddress;
	asArguments[1] = TypeInfoSymbol(psType);
	asArguments[2] = mpcFunction->AddConstant(IRType(IRTK_Pointer), 0);
	if (psType->IsClass() && psType->pcClass->mpcDestructor)
	{
		szDestroyer.Init();
		mpcUnit->ThrownDestroyer(psType, &szDestroyer);
		asArguments[2] = mcModule.Symbol(szDestroyer.Text());
		szDestroyer.Kill();
	}
	ExceptionCall(CPP65_THROW, IRType(IRTK_None), asTypes, asArguments, 3);
	JumpTo(Landing());
	return Terminated();
}


//////////////////////////////////////////////////////////////////////////
//
// A function that destroys a thrown object of psType, which the runtime
// calls with the object's address when its last handler is left: the unit
// writes one for each class thrown (CCPPUnit::WriteThrownDestroyers).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ThrownDestroyerRoutine(const char* szLabel, SCPP65Type* psType)
{
	CCPPFunction	cFunction;
	SIRReference	sObject;
	bool			bResult;

	memset(&cFunction, 0, sizeof(CCPPFunction));
	cFunction.meKind = CPPFK_Function;
	cFunction.mpsReturn = &mpcTypes->msVoid;
	BeginWriting(&cFunction, szLabel, IRType(IRTK_None), 0);
	mbRoutine = true;
	sObject = mpcFunction->AddParameter(IRType(IRTK_Pointer));
	mpcUnit->BeginQuiet();
	bResult = Destroy(sObject, psType);
	mpcUnit->EndQuiet();
	if (bResult)
	{
		mpcBlock->Return();
	}
	return FinishWriting(bResult, szLabel);
}


//////////////////////////////////////////////////////////////////////////
//
// try block handlers: the block is written with the try on the stack of
// tries, so an exception from it goes, once what the block made is undone,
// to the handlers' block.  That asks the runtime of each handler in turn
// whether it takes the exception (__Match, with the handler's type; a catch
// (...) takes any) and goes to the first that does; if none does the
// exception goes on, as from any place outside the try.  [except.handle]
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::TryStatement(CCPPSyntaxTryStatement* pcTry)
{
	SCPP65IRTry*			psTry;
	CCPPSyntaxCatchHandler*	pcHandler;
	CIRBlock*				pcDispatch;
	CIRBlock*				pcAfter;
	CIRBlock*				pcHandlerBlock;
	CIRBlock*				pcNext;
	SCPP65Type*				psHandled;
	SIRReference			sMatched;
	SIRReference			sCondition;
	SIRType					sPointer;
	SIRReference			sTypeInfo;
	size					ui;
	bool					bAll;

	if (!mpcUnit->GetOptions()->mbExceptions)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	sPointer = IRType(IRTK_Pointer);
	pcDispatch = NewBlock();
	pcAfter = NewBlock();
	psTry = masTries.Add();
	psTry->uiBase = mcCleanups.NumEntries();
	psTry->pcDispatch = pcDispatch;
	if (!ScopedStatement(pcTry->mpcBlock))
	{
		return false;
	}
	masTries.RemoveTail();
	JumpTo(pcAfter);

	//The handlers, tried in the order they are written.
	StartBlock(pcDispatch);
	bAll = false;
	for (ui = 0; (ui < pcTry->macpHandlers.NumElements()) && !bAll; ui++)
	{
		pcHandler = (CCPPSyntaxCatchHandler*)pcTry->macpHandlers.GetPtr(ui);
		pcHandlerBlock = NewBlock();
		sMatched = IRNone();
		if (!pcHandler->mpcParameter)
		{
			bAll = true;
			JumpTo(pcHandlerBlock);
		}
		else
		{
			psHandled = mpcUnit->HandlerType(pcHandler);
			if (!psHandled)
			{
				return Unsupported(__FILE__, __LINE__);
			}
			sTypeInfo = TypeInfoSymbol(ThrownType(psHandled));
			sMatched = ExceptionCall(CPP65_MATCH, sPointer, &sPointer, &sTypeInfo, 1);
			sCondition = mpcFunction->Add(mpcBlock, IRO_Ne, sPointer, sMatched, mpcFunction->AddConstant(sPointer, 0));
			pcNext = NewBlock();
			mpcBlock->Branch(sCondition, pcHandlerBlock, pcNext);
		}
		StartBlock(pcHandlerBlock);
		if (!Handler(pcHandler, sMatched))
		{
			return false;
		}
		JumpTo(pcAfter);
		if (!bAll)
		{
			StartBlock(pcNext);
		}
	}
	if (!bAll)
	{
		//No handler takes it: it goes on.
		JumpTo(Landing());
	}
	StartBlock(pcAfter);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A handler that has taken the exception.  The exception is being handled
// from here (__BeginCatch) until the handler is left by any way out, which
// the cleanup stack sees to (an entry that calls __EndCatch).  The
// parameter is made from what the runtime's match gave (sMatched: the
// thrown object, a base of it, or a pointer it worked out): a reference is
// bound to it, an object is copied from it, and is a local like any other.
// [except.handle] 15, 16
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Handler(CCPPSyntaxCatchHandler* pcHandler, SIRReference sMatched)
{
	size	uiLiving;

	uiLiving = mcCleanups.NumLocals();
	ExceptionCall(CPP65_BEGIN_CATCH, IRType(IRTK_Pointer), NULL, NULL, 0);
	mcCleanups.PushCatch();
	if (pcHandler->mpcParameter && !HandlerParameter(pcHandler, sMatched))
	{
		return false;
	}
	return Statement(pcHandler->mpcBlock) && LeaveScope(uiLiving);
}


//////////////////////////////////////////////////////////////////////////
//
// The handler's parameter, a local of the function's: a reference refers to
// what the match gave; an object is copied from it, and destroyed with the
// handler's scope; anything else is read from it.  A parameter with no name
// is nothing.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::HandlerParameter(CCPPSyntaxCatchHandler* pcHandler, SIRReference sMatched)
{
	CCPPVariable*	pcVariable;
	SCPP65IRPlace	sPlace;
	SCPP65IRPlace	sSource;
	SCPP65IRValue	sValue;

	if (!pcHandler->mpcParameter->mpcSymbol)
	{
		return true;
	}
	pcVariable = mpcUnit->GetVariable(pcHandler->mpcParameter->mpcSymbol);
	if (!pcVariable || (pcVariable->meStorage != CPPVS_Local))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	sPlace.sAddress = *mcAddresses.Get((int64)(size)pcVariable);
	sPlace.psType = pcVariable->mpsType;
	if (sPlace.psType->IsReference())
	{
		mpcFunction->Store(mpcBlock, IRType(IRTK_Pointer), sMatched, sPlace.sAddress);
		return true;
	}
	if (sPlace.psType->IsClass())
	{
		if (!CopyFromAddress(&sPlace, sMatched))
		{
			return false;
		}
		AddLiving(&sPlace);
		return true;
	}
	sSource.sAddress = sMatched;
	sSource.psType = sPlace.psType->Unqualified();
	if (!Load(&sSource, &sValue))
	{
		return false;
	}
	Store(&sPlace, &sValue);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Copy constructs the object at psPlace from the object of its class at
// sSource, an address and no expression: with the class's copy constructor,
// or as the compiler's own does (the bytes, the parts that have copy
// constructors again with them, and the vtable pointers).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CopyFromAddress(SCPP65IRPlace* psPlace, SIRReference sSource)
{
	CCPPClass*		pcClass;
	SCPP65IRPlace	sFrom;
	SIRReference	sAddress;

	pcClass = psPlace->psType->pcClass;
	if (pcClass->GetPartCopier(false, false))
	{
		return CopyPart(pcClass, psPlace->sAddress, sSource, false, NULL, 0);
	}
	if (pcClass->IsCopyDeleted())
	{
		return CopyDeleted(mpcValue);
	}
	sFrom.sAddress = sSource;
	sFrom.psType = psPlace->psType->Unqualified();
	return CopyPlace(psPlace, &sFrom, &sAddress) && (pcClass->IsTriviallyCopyable() || CopyParts(pcClass, psPlace->sAddress, sAddress, false, NULL, 0)) &&
		   StoreVtables(psPlace->sAddress, pcClass);
}
