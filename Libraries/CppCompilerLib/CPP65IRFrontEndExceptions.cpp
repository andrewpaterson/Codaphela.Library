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
#define CPP65_FREE_EXCEPTION		"___FreeException"
#define CPP65_UNEXPECTED			"___Unexpected"
#define CPP65_THROW_BAD_EXCEPTION	"___ThrowBadException"


//////////////////////////////////////////////////////////////////////////
//
// Whether a call of pcFunction (NULL: through a pointer, whose function
// isn't known) may be left with an exception: in a program built with
// exceptions, any function not declared noexcept.  [except.spec]
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CallMayThrow(CCPPFunction* pcFunction)
{
	return mpcUnit->GetOptions()->mbExceptions && !(pcFunction && mpcUnit->GetExceptions()->IsNoexcept(pcFunction));
}


//////////////////////////////////////////////////////////////////////////
//
// Straight after a call's instruction: if the call may throw, the test of
// whether it did, and the way an exception goes from here (Landing).  The
// code goes on in a new block, for the way that didn't.  While a cleanup is
// being written for an exception on its way (mbUnwinding), a second one is
// std::terminate.  [except.ctor] 3  sCallee is what was called: a function
// called by name is noted (GetTestedCallees), but for the unit's own (a
// static function), which is C++ compiled here and has no label the linker
// sees.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::TestThrown(CCPPFunction* pcFunction, SIRReference sCallee)
{
	SIRReference	sThrown;
	CIRBlock*		pcLanding;
	CIRBlock*		pcNext;

	if (!CallMayThrow(pcFunction) || mbUnsupported)
	{
		return;
	}
	if (sCallee.IsSymbol() && !mcModule.IsLocal(sCallee.Index()))
	{
		maszTested.AddIfUnique(mcModule.GetSymbolName(sCallee.Index()));
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
// way out.  While a normal way out destroys an entry (muiLandingTop), from
// the entry below it.  In a function no exception leaves, outside any try,
// straight to std::terminate: whether the stack is unwound first is the
// implementation's to say.  [except.terminate] 2
//
//////////////////////////////////////////////////////////////////////////
CIRBlock* CCPP65IRFrontEnd::Landing(void)
{
	size	uiBase;
	size	uiTop;

	if ((masTries.NumElements() == 0) && FunctionTerminates())
	{
		return TerminateBlock();
	}
	uiBase = (masTries.NumElements() > 0) ? masTries.Tail()->uiBase : 0;
	uiTop = (mcCleanups.NumEntries() < muiLandingTop) ? mcCleanups.NumEntries() : muiLandingTop;
	if (uiTop > uiBase)
	{
		return UndoBlock(uiTop - 1);
	}
	return LandingBelow();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether no exception leaves the function being written: one declared
// noexcept, a destructor that isn't noexcept(false), and the routines the
// compiler writes for itself (mbRoutine).  [except.spec] 9
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::FunctionTerminates(void)
{
	return mbRoutine || !mpcCPPFunction || (mpcUnit->GetExceptions()->IsNoexcept(mpcCPPFunction) && !mpcUnit->GetExceptions()->ExceptionsOf(mpcCPPFunction)->mbDynamic);
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
		CallNoThrow(CPP65_END_CATCH, IRType(IRTK_None), NULL, NULL, 0);
		return true;
	case CPP65IRC_Part:
		return UndoPart(psEntry);
	default:
		return Destroy(psEntry->sPlace.sAddress, psEntry->sPlace.psType);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A normal way out's undoing of the entry at uiIndex (a scope's end, a
// jump, a full expression's end): an exception from its destructor goes to
// the entries below it, not to itself again.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::UndoEntryBelow(SCPP65IRCleanup* psEntry, size uiIndex)
{
	size	uiTop;
	bool	bResult;

	uiTop = muiLandingTop;
	muiLandingTop = uiIndex;
	bResult = UndoEntry(psEntry);
	muiLandingTop = uiTop;
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// What an exception undoes of something half made (ECPP65IRPart).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::UndoPart(SCPP65IRCleanup* psPart)
{
	CCPPClass*		pcBaseObject;
	SIRReference	asArguments[1];
	SIRType			asTypes[1];
	bool			bResult;

	switch (psPart->ePart)
	{
	case CPP65IRP_Destroy:
		pcBaseObject = mpcBaseObject;
		mpcBaseObject = psPart->pcBaseObject;
		bResult = Destroy(psPart->sPlace.sAddress, psPart->sPlace.psType);
		mpcBaseObject = pcBaseObject;
		return bResult;
	case CPP65IRP_FreeException:
		asTypes[0] = IRType(IRTK_Pointer);
		asArguments[0] = psPart->sPlace.sAddress;
		CallNoThrow(CPP65_FREE_EXCEPTION, IRType(IRTK_None), asTypes, asArguments, 1);
		return true;
	case CPP65IRP_Guard:
		mpcFunction->Store(mpcBlock, IRType(IRTK_I8), mpcFunction->AddInteger(IRType(IRTK_I8), 0), psPart->sPlace.sAddress);
		return true;
	case CPP65IRP_Elements:
		return UndoElements(psPart);
	case CPP65IRP_Deallocate:
		return UndoDeallocate(psPart);
	default:
		return Unsupported(__FILE__, __LINE__);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// new T[n]'s elements made so far (the count in the part's slot), the last
// first.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::UndoElements(SCPP65IRCleanup* psPart)
{
	SIRType				sSize;
	SIRReference		sCount;
	SIRReference		sIndex;
	SIRReference		sNext;
	SIRReference		sElement;
	SCPP65IRCountedLoop	sLoop;

	sSize = IRIntegerType(mcModule.GetPointerSize(0));
	sCount = mpcFunction->Load(mpcBlock, sSize, psPart->sCount);

	//loop: the index counts down from the count.
	sIndex = BeginCountedLoop(&sLoop, sSize, sCount, IRO_Ne, mpcFunction->AddInteger(sSize, 0));
	sNext = mpcFunction->Add(mpcBlock, IRO_Sub, sSize, sIndex, mpcFunction->AddInteger(sSize, 1));
	sElement = mpcFunction->Add(mpcBlock, IRO_PtrAdd, IRType(IRTK_Pointer), psPart->sPlace.sAddress, mpcFunction->Add(mpcBlock, IRO_Mul, sSize, sNext, mpcFunction->AddInteger(sSize, psPart->sPlace.psType->iBytes)));
	if (!Destroy(sElement, psPart->sPlace.psType))
	{
		return false;
	}
	EndCountedLoop(&sLoop, sNext);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A new expression's memory given back, its object not made: to the
// operator delete that matches the allocation function, called with the
// pointer then the values the part keeps (the placement arguments, or the
// size), or the runtime's.  [expr.new] 20
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::UndoDeallocate(SCPP65IRCleanup* psPart)
{
	CArraySyntax				apcArguments;
	CCPPSyntaxImplicitArgument*	pcArgument;
	SCPP65IRValue				sValue;
	SCPP65IRValue				sResult;
	size						ui;
	bool						bResult;

	if (!psPart->pcFunction)
	{
		CallRuntime(psPart->szRuntime, IRType(IRTK_None), IRType(IRTK_Pointer), psPart->sPlace.sAddress);
		return true;
	}
	apcArguments.Init();
	pcArgument = mpcUnit->GetDesugarings()->ImplicitValue(psPart->pcFor, mpcTypes->Pointer(&mpcTypes->msVoid), 1);
	apcArguments.Add(pcArgument);
	sValue.psType = pcArgument->mpsType;
	sValue.sValue = psPart->sPlace.sAddress;
	mcImplicitValues.Put((int64)(size)pcArgument, &sValue);
	for (ui = 0; ui < psPart->uiValues; ui++)
	{
		pcArgument = mpcUnit->GetDesugarings()->ImplicitValue(psPart->pcFor, psPart->apsValueTypes[ui], (int)(2 + ui));
		apcArguments.Add(pcArgument);
		sValue.psType = psPart->apsValueTypes[ui];
		sValue.sValue = psPart->asValues[ui];
		mcImplicitValues.Put((int64)(size)pcArgument, &sValue);
	}
	bResult = Call(psPart->pcFunction, FunctionSymbol(psPart->pcFunction), NULL, &apcArguments, &sResult, NULL, false, NULL, false, IRNone());
	for (ui = 0; ui < apcArguments.NumElements(); ui++)
	{
		mcImplicitValues.Remove((int64)(size)apcArguments.GetPtr(ui));
	}
	apcArguments.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether what is half made is to be undone by an exception (parts are
// pushed): in a program built with exceptions.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::PartsWanted(void)
{
	return mpcUnit->GetOptions()->mbExceptions && !mbConstants;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an object of psType (or each element of an array of them) has a
// destructor to call.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::NeedsDestroying(SCPP65Type* psType)
{
	psType = ElementType(psType);
	return psType->IsClass() && (psType->pcClass->mpcDestructor != NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// A part made, to be destroyed (pcBaseObject: a base, with the base version
// of its destructor) if an exception comes before what it is part of is
// made: when parts are wanted and it has a destructor.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::PushDestroyPart(SIRReference sAddress, SCPP65Type* psType, CCPPClass* pcBaseObject, size uiAt)
{
	SCPP65IRCleanup	sPart;

	if (!PartsWanted() || !NeedsDestroying(psType))
	{
		return;
	}
	sPart.Clear(CPP65IRC_Part);
	sPart.ePart = CPP65IRP_Destroy;
	sPart.sPlace.sAddress = sAddress;
	sPart.sPlace.psType = psType;
	sPart.pcBaseObject = pcBaseObject;
	if (uiAt < mcCleanups.NumEntries())
	{
		mcCleanups.InsertPart(uiAt, &sPart);
	}
	else
	{
		mcCleanups.PushPart(&sPart);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A base or member made by a constructor's member initialiser (or by
// default), whose temporaries are the ones since uiTemporaries: it is a part
// from uiAt, where the cleanup stack was when it began (under those
// temporaries), and they are destroyed, its initialiser being a full
// expression.  [intro.execution] 10, [except.ctor] 2
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::MemberInitialised(size uiAt, size uiTemporaries, SIRReference sAddress, SCPP65Type* psType, CCPPClass* pcBaseObject)
{
	PushDestroyPart(sAddress, psType, pcBaseObject, uiAt);
	return DestroyTemporaries(uiTemporaries);
}


//////////////////////////////////////////////////////////////////////////
//
// A destructor's end takes out the part DestructorStart pushed for the
// part of psType it is about to destroy (there is one when it has a
// destructor).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::TakeDestroyPart(SCPP65Type* psType)
{
	if (PartsWanted() && NeedsDestroying(psType))
	{
		mcCleanups.RemoveNewestPart();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A part that is an address: a thrown object being made (to free) or a
// guard byte (to clear).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::PushAddressPart(ECPP65IRPart ePart, SIRReference sAddress)
{
	SCPP65IRCleanup	sPart;

	if (!PartsWanted())
	{
		return;
	}
	sPart.Clear(CPP65IRC_Part);
	sPart.ePart = ePart;
	sPart.sPlace.sAddress = sAddress;
	mcCleanups.PushPart(&sPart);
}


//////////////////////////////////////////////////////////////////////////
//
// A destructor's start, with exceptions: its fields and bases are parts, so
// that an exception from its body (it is noexcept(false)) or from one of
// their destructors still destroys the rest.  They are pushed in the order
// they were made; DestructorEnd takes each out as it destroys it.
// [except.ctor] 2
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::DestructorStart(void)
{
	CCPPClass*			pcClass;
	SCPPField*			psField;
	SCPPBase*			psBase;
	SCPPVirtualBase*	psVirtual;
	size				ui;

	pcClass = mpcCPPFunction->mpcClass;
	for (ui = 0; !mbBaseVersion && (ui < pcClass->macVirtualBases.NumElements()); ui++)
	{
		psVirtual = pcClass->macVirtualBases.Get(ui);
		PushDestroyPart(AddressAt(msThis, psVirtual->iOffset), psVirtual->pcClass->mpsType, psVirtual->pcClass);
	}
	for (ui = 0; ui < pcClass->macBases.NumElements(); ui++)
	{
		psBase = pcClass->macBases.Get(ui);
		if (!psBase->bVirtual)
		{
			PushDestroyPart(AddressAt(msThis, psBase->iOffset), psBase->pcClass->mpsType, psBase->pcClass);
		}
	}
	for (ui = 0; ui < pcClass->macFields.NumElements(); ui++)
	{
		psField = pcClass->macFields.Get(ui);
		PushDestroyPart(AddressAt(msThis, psField->iOffset), psField->psType, NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// How a new expression's memory is given back if its object's construction
// throws (*psFree, a Deallocate part; neither pcFunction nor szRuntime if it
// isn't).  A placement allocation function's is the operator delete whose
// parameters after the first are the same (none: nothing), given the
// placement arguments, which are worked out here, before the allocation
// call, and given to it as values (papcArguments' are swapped for them).
// Another's is what delete would call: the class's own operator delete
// (with the size if it takes one), else the runtime's.  new (address) T
// allocates nothing.  [expr.new] 20 to 22
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Deallocation(CCPPSyntaxNewExpression* pcNew, CCPPFunction* pcAllocation, SCPP65Type* psType, CArraySyntax* papcArguments, SCPP65IRValue* psBytes, SCPP65IRCleanup* psFree)
{
	CCPPSyntaxImplicitArgument*	pcArgument;
	CCPPFunction*				pcDeallocation;
	SCPP65IRValue				sValue;
	size						ui;
	bool						bArray;

	psFree->Clear(CPP65IRC_Part);
	psFree->ePart = CPP65IRP_Deallocate;
	psFree->pcFor = pcNew;
	bArray = pcNew->macpArraySizes.NumElements() > 0;
	if (!pcAllocation)
	{
		psFree->szRuntime = pcNew->mbPlacement ? NULL : (bArray ? "opDeleteArray" : "opDelete");
		return true;
	}
	if (papcArguments->NumElements() > 1)
	{
		pcDeallocation = PlacementDeallocation(pcAllocation, bArray);
		if (!pcDeallocation || (papcArguments->NumElements() - 1 > CPP65_IR_PART_VALUES))
		{
			return true;
		}
		for (ui = 1; ui < papcArguments->NumElements(); ui++)
		{
			if (pcAllocation->mapsParameters.GetPtr(ui)->IsReference())
			{
				return true;
			}
		}
		for (ui = 1; ui < papcArguments->NumElements(); ui++)
		{
			if (!Value(papcArguments->GetPtr(ui), &sValue))
			{
				return false;
			}
			pcArgument = mpcUnit->GetDesugarings()->ImplicitValue(pcNew, sValue.psType, (int)(1 + ui));
			mcImplicitValues.Put((int64)(size)pcArgument, &sValue);
			papcArguments->SetPtr(ui, pcArgument);
			psFree->asValues[ui - 1] = sValue.sValue;
			psFree->apsValueTypes[ui - 1] = sValue.psType;
		}
		psFree->uiValues = papcArguments->NumElements() - 1;
		psFree->pcFunction = pcDeallocation;
		return true;
	}
	pcDeallocation = psType->IsClass() && !pcNew->mbGlobal ? mpcUnit->GetBinder()->FindDeallocation(psType->pcClass, bArray) : NULL;
	if (!pcDeallocation)
	{
		psFree->szRuntime = bArray ? "opDeleteArray" : "opDelete";
		return true;
	}
	psFree->pcFunction = pcDeallocation;
	if (pcDeallocation->mapsParameters.NumElements() == 2)
	{
		psFree->asValues[0] = psBytes->sValue;
		psFree->apsValueTypes[0] = psBytes->psType;
		psFree->uiValues = 1;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The operator delete (or delete[]) that matches a placement allocation
// function: of the same class (or both global), its parameters after the
// pointer the same as the allocation function's after the size.  NULL if
// there is none.  [expr.new] 20
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPP65IRFrontEnd::PlacementDeallocation(CCPPFunction* pcAllocation, bool bArray)
{
	CCPPFunction*	pcFunction;
	int64			lliName;
	size			uiParameters;
	size			ui;
	bool			bSame;

	lliName = mpcUnit->GetOperatorName(CT_delete, bArray);
	uiParameters = pcAllocation->mapsParameters.NumElements();
	for (pcFunction = mpcUnit->FirstFunctionOfName(lliName); pcFunction; pcFunction = pcFunction->mpcNextOfName)
	{
		if ((pcFunction->mpcClass != pcAllocation->mpcClass) || (pcFunction->mapsParameters.NumElements() != uiParameters))
		{
			continue;
		}
		bSame = pcFunction->mapsParameters.GetPtr(0)->Unqualified()->IsPointer();
		for (ui = 1; bSame && (ui < uiParameters); ui++)
		{
			bSame = pcFunction->mapsParameters.GetPtr(ui)->Equals(pcAllocation->mapsParameters.GetPtr(ui));
		}
		if (bSame)
		{
			return pcFunction;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The function's way out for an exception nothing in it handles: a block
// that ends in throw (the caller's thrown is true), one for the function.
// A function declared noexcept, a destructor that isn't noexcept(false) and
// the routines the compiler writes for itself (an object's initialiser run
// at start up, a finaliser: mbRoutine) have none: std::terminate.
// [except.spec] 9  One with a throw(A, B) list, or throw(), first checks
// that the list allows the exception (UnexpectedBlock).
//
//////////////////////////////////////////////////////////////////////////
CIRBlock* CCPP65IRFrontEnd::WayOut(void)
{
	if (!mbRoutine && mpcCPPFunction && mpcUnit->GetExceptions()->ExceptionsOf(mpcCPPFunction)->mbDynamic)
	{
		return UnexpectedBlock();
	}
	if (FunctionTerminates())
	{
		return TerminateBlock();
	}
	return ThrowExit();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a function's label is weak, so that a program's definition takes
// its place: __attribute__((weak)) on its definition, or on the
// declaration it is known by (Libc65's replaceable operator new and delete
// say it where they are defined, so that a program's own, declared by the
// same header, isn't).  [replacement.functions]
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::IsWeak(CCPPFunction* pcFunction)
{
	return mpcUnit->IsWeakFunction(pcFunction);
}


//////////////////////////////////////////////////////////////////////////
//
// The block that ends in throw, one for the function.
//
//////////////////////////////////////////////////////////////////////////
CIRBlock* CCPP65IRFrontEnd::ThrowExit(void)
{
	if (!mpcThrowExit)
	{
		mpcThrowExit = NewBlock();
		mpcThrowExit->Throw();
	}
	return mpcThrowExit;
}


//////////////////////////////////////////////////////////////////////////
//
// A throw(A, B) list's check of the exception leaving the function, one
// for it: one the list allows (one of the types, or a class publicly
// derived from one: the runtime's match) goes on.  One it doesn't is being
// handled while std::unexpected's handler is called (__Unexpected), which
// may throw: what it throws goes on if the list allows it, is replaced by
// std::bad_exception if the list has that, and is std::terminate otherwise.
// A handler that returns is std::terminate (the runtime's).  throw() allows
// nothing.  [except.unexpected]
//
//////////////////////////////////////////////////////////////////////////
CIRBlock* CCPP65IRFrontEnd::UnexpectedBlock(void)
{
	CCPPExceptionSpec*	pcSpec;
	CIRBlock*			pcBlock;
	CIRBlock*			pcThrown;
	CIRBlock*			pcReplace;
	SCPP65Type*			psType;
	size				ui;
	bool				bBadException;

	if (mpcUnexpected)
	{
		return mpcUnexpected;
	}
	pcSpec = mpcUnit->GetExceptions()->ExceptionsOf(mpcCPPFunction);
	mpcUnexpected = NewBlock();
	pcBlock = mpcBlock;
	StartBlock(mpcUnexpected);
	MatchListed(pcSpec, ThrowExit());

	//Not allowed: it is being handled while the handler runs, and ended when the handler throws.
	CallNoThrow(CPP65_BEGIN_CATCH, IRType(IRTK_Pointer), NULL, NULL, 0);
	pcThrown = NewBlock();
	TestedRoutine(CPP65_UNEXPECTED, pcThrown, TerminateBlock());
	StartBlock(pcThrown);
	CallNoThrow(CPP65_END_CATCH, IRType(IRTK_None), NULL, NULL, 0);
	MatchListed(pcSpec, ThrowExit());
	bBadException = false;
	for (ui = 0; ui < pcSpec->mapsTypes.NumElements(); ui++)
	{
		psType = pcSpec->mapsTypes.GetPtr(ui);
		bBadException |= psType->IsClass() && psType->pcClass->mpcSymbol && mpcUnit->GetBinder()->IsStdName(psType->pcClass->mlliName, psType->pcClass->mpcSymbol->mpcScope, "bad_exception");
	}
	if (bBadException)
	{
		pcReplace = NewBlock();
		JumpTo(pcReplace);
		StartBlock(pcReplace);
		TestedRoutine(CPP65_THROW_BAD_EXCEPTION, ThrowExit(), TerminateBlock());
	}
	else
	{
		JumpTo(TerminateBlock());
	}
	StartBlock(pcBlock);
	return mpcUnexpected;
}


//////////////////////////////////////////////////////////////////////////
//
// For each type a throw(A, B) list allows, the runtime's match of the
// exception in flight: to pcAllowed if it takes it.  What follows is for
// one none of them takes.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::MatchListed(CCPPExceptionSpec* pcSpec, CIRBlock* pcAllowed)
{
	SIRType			sPointer;
	SIRReference	sTypeInfo;
	SIRReference	sMatched;
	SIRReference	sCondition;
	CIRBlock*		pcNext;
	size			ui;

	sPointer = IRType(IRTK_Pointer);
	for (ui = 0; ui < pcSpec->mapsTypes.NumElements(); ui++)
	{
		sTypeInfo = TypeInfoSymbol(ThrownType(pcSpec->mapsTypes.GetPtr(ui)));
		sMatched = CallNoThrow(CPP65_MATCH, sPointer, &sPointer, &sTypeInfo, 1);
		sCondition = mpcFunction->Add(mpcBlock, IRO_Ne, sPointer, sMatched, mpcFunction->AddConstant(sPointer, 0));
		pcNext = NewBlock();
		mpcBlock->Branch(sCondition, pcAllowed, pcNext);
		StartBlock(pcNext);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Calls one of the runtime's routines that may throw (C++ in Libc65, with no
// arguments), and goes to pcThrown if it did, else to pcReturned.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::TestedRoutine(const char* szLabel, CIRBlock* pcThrown, CIRBlock* pcReturned)
{
	SIRReference	sSymbol;
	SIRReference	sThrown;

	sSymbol = mcModule.Symbol(szLabel);
	maszTested.AddIfUnique(mcModule.GetSymbolName(sSymbol.Index()));
	mpcFunction->Call(mpcBlock, IRType(IRTK_None), sSymbol);
	sThrown = mpcFunction->AddTemporary(IRType(IRTK_I1));
	mpcBlock->AddInstruction(IRO_Thrown, IRType(IRTK_I1), sThrown, IRNone(), IRNone());
	mpcBlock->Branch(sThrown, pcThrown, pcReturned);
}


//////////////////////////////////////////////////////////////////////////
//
// Calls one of the runtime's routines that throws what the language does
// (std::bad_cast, std::bad_typeid, std::bad_array_new_length): the
// exception goes on its way from here (Landing).  The code after it isn't
// reached.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::ThrowRoutine(const char* szLabel)
{
	CIRBlock*	pcLanding;

	pcLanding = mbUnwinding ? TerminateBlock() : Landing();
	TestedRoutine(szLabel, pcLanding, TerminateBlock());
	Terminated();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a new expression's allocation function may give it NULL, so that
// nothing is made and the result is NULL: one with a non-throwing exception
// specification.  Not the reserved placement form, new (address) Type,
// which is given its address back (one that is NULL is the program's
// mistake: CWG 1748).  [expr.new] 13
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::IsNullable(CCPPFunction* pcAllocation)
{
	SCPP65Type*	psSecond;

	if (!pcAllocation || !mpcUnit->GetExceptions()->IsNoexcept(pcAllocation))
	{
		return false;
	}
	if (!pcAllocation->mpcClass && (pcAllocation->mapsParameters.NumElements() == 2))
	{
		psSecond = pcAllocation->mapsParameters.GetPtr(1)->Unqualified();
		if (psSecond->IsPointer() && psSecond->psBase->IsVoid())
		{
			return false;
		}
	}
	return true;
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
		CallNoThrow(CPP65_TERMINATE, IRType(IRTK_None), NULL, NULL, 0);
		mpcBlock->Unreachable();
		StartBlock(pcBlock);
	}
	return mpcTerminate;
}


//////////////////////////////////////////////////////////////////////////
//
// Calls one of the runtime's routines by its label (Libc65's exception.cpp's,
// or one the unit writes: R_DynamicCast) with its arguments on the stack,
// as any caller of a C function may.  None of them throws.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CCPP65IRFrontEnd::CallNoThrow(const char* szLabel, SIRType sResult, SIRType* pasTypes, SIRReference* pasArguments, int iArguments)
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
		return mpcTypes->Pointer(psType->ElementType());
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
	mpcUnit->GetClassData()->AppendTypeidLabel(&szLabel, psType);
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
	size			uiMark;
	bool			bThrown;

	psValue->psType = &mpcTypes->msVoid;
	psValue->sValue = IRNone();
	if (!mpcUnit->GetOptions()->mbExceptions)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (!pcThrow->mpcOperand)
	{
		CallNoThrow(CPP65_RETHROW, IRType(IRTK_None), NULL, NULL, 0);
		JumpTo(Landing());
		return Terminated();
	}

	psType = mpcUnit->GetTypeChecker()->TypeOf(pcThrow->mpcOperand);
	if (!psType)
	{
		mpcUnit->GetTypeChecker()->ReportTypeOf(pcThrow->mpcOperand);
		return Unsupported(__FILE__, __LINE__);
	}

	//What can't be thrown is said aloud (the front end works quietly).
	mpcDiagnostics->BeginLoud();
	bThrown = mpcUnit->GetExceptions()->CheckThrown(pcThrow);
	mpcDiagnostics->EndLoud();
	if (!bThrown)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	psType = ThrownType(psType);
	if (psType->IsVoid() || (psType->IsClass() && !mpcUnit->CompleteClass(psType->pcClass, pcThrow)) || (!psType->IsClass() && !IsSupportedType(psType)))
	{
		return Unsupported(__FILE__, __LINE__);
	}

	asTypes[0] = IRTypeOf(&mpcTypes->msUnsignedInt);
	asArguments[0] = mpcFunction->AddConstant(asTypes[0], (uint64)psType->iBytes);
	sPlace.sAddress = CallNoThrow(CPP65_ALLOCATE_EXCEPTION, IRType(IRTK_Pointer), asTypes, asArguments, 1);
	sPlace.psType = psType;
	if (psType->IsClass())
	{
		uiMark = mcCleanups.NumEntries();
		PushAddressPart(CPP65IRP_FreeException, sPlace.sAddress);
		if (!Construct(&sPlace, NULL, pcThrow->mpcOperand))
		{
			return false;
		}
		mcCleanups.RemovePartsFrom(uiMark);
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
	CallNoThrow(CPP65_THROW, IRType(IRTK_None), asTypes, asArguments, 3);
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
	mpcDiagnostics->BeginQuiet();
	bResult = Destroy(sObject, psType);
	mpcDiagnostics->EndQuiet();
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
	SCPP65IRTry*	psTry;
	CIRBlock*		pcDispatch;
	CIRBlock*		pcAfter;

	if (!mpcUnit->GetOptions()->mbExceptions)
	{
		return Unsupported(__FILE__, __LINE__);
	}
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
	return TryHandlers(pcTry, pcDispatch, pcAfter, false);
}


//////////////////////////////////////////////////////////////////////////
//
// A try's handlers, at pcDispatch, where an exception from its block comes
// once what the block made is undone: tried in the order they are written,
// and on to pcAfter from the end of one.  With none that takes it the
// exception goes on.  bRethrow: they are a constructor's or destructor's
// function-try-block's, and one that reaches its end rethrows.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::TryHandlers(CCPPSyntaxTryStatement* pcTry, CIRBlock* pcDispatch, CIRBlock* pcAfter, bool bRethrow)
{
	CCPPSyntaxCatchHandler*	pcHandler;
	CIRBlock*				pcHandlerBlock;
	CIRBlock*				pcNext;
	SCPP65Type*				psHandled;
	SIRReference			sMatched;
	SIRReference			sCondition;
	SIRType					sPointer;
	SIRReference			sTypeInfo;
	size					ui;
	bool					bAll;

	//The handlers, tried in the order they are written.
	sPointer = IRType(IRTK_Pointer);
	StartBlock(pcDispatch);
	bAll = false;
	pcNext = NULL;
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
			sMatched = CallNoThrow(CPP65_MATCH, sPointer, &sPointer, &sTypeInfo, 1);
			sCondition = mpcFunction->Add(mpcBlock, IRO_Ne, sPointer, sMatched, mpcFunction->AddConstant(sPointer, 0));
			pcNext = NewBlock();
			mpcBlock->Branch(sCondition, pcHandlerBlock, pcNext);
		}
		StartBlock(pcHandlerBlock);
		if (!Handler(pcHandler, sMatched, bRethrow))
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
// [except.handle] 15, 16  bRethrow: reaching its end is throw; (a
// constructor's or destructor's function-try-block's, [except.handle] 15).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Handler(CCPPSyntaxCatchHandler* pcHandler, SIRReference sMatched, bool bRethrow)
{
	size	uiLiving;

	uiLiving = mcCleanups.NumLocals();
	CallNoThrow(CPP65_BEGIN_CATCH, IRType(IRTK_Pointer), NULL, NULL, 0);
	mcCleanups.PushCatch();
	if (pcHandler->mpcParameter && !HandlerParameter(pcHandler, sMatched))
	{
		return false;
	}
	if (!Statement(pcHandler->mpcBlock))
	{
		return false;
	}
	if (!bRethrow)
	{
		return LeaveScope(uiLiving);
	}
	CallNoThrow(CPP65_RETHROW, IRType(IRTK_None), NULL, NULL, 0);
	JumpTo(Landing());
	Terminated();
	mcCleanups.CutLocals(uiLiving);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A function's body that is a function-try-block: the try begins before a
// constructor's member initialisers and bases, and before a destructor's
// parts, and covers the body and a destructor's destruction of its members
// and bases ([except] 4).  What the try made is undone before its handlers
// are entered ([except.handle] 11: so its parts come off the stack, for
// they are undone).  A constructor's or destructor's handler that reaches
// its end rethrows, a constructor's can't return (14, 15), and a
// destructor's that returns doesn't destroy the members again.  The
// parameters are destroyed after the handlers (CPP65IRFrontEnd's WriteBody
// isn't to write the destructor's end again).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::FunctionTryBlock(CCPPFunction* pcFunction, CCPPSyntaxTryStatement* pcTry)
{
	SCPP65IRTry*	psTry;
	CIRBlock*		pcDispatch;
	CIRBlock*		pcAfter;
	size			uiBase;
	bool			bConstructor;
	bool			bDestructor;
	bool			bResult;

	if (!mpcUnit->GetOptions()->mbExceptions)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	bConstructor = pcFunction->meKind == CPPFK_Constructor;
	bDestructor = pcFunction->meKind == CPPFK_Destructor;
	pcDispatch = NewBlock();
	pcAfter = NewBlock();
	uiBase = mcCleanups.NumEntries();
	psTry = masTries.Add();
	psTry->uiBase = uiBase;
	psTry->pcDispatch = pcDispatch;
	if (bConstructor && !ConstructorStart(pcFunction->mpcDefinition))
	{
		return false;
	}
	if (bDestructor)
	{
		mpcExit = NewBlock();
		if (!StoreVtables(msThis, pcFunction->mpcClass, mbBaseVersion))
		{
			return false;
		}
		DestructorStart();
	}
	if (!ScopedStatement(pcTry->mpcBlock))
	{
		return false;
	}
	if (bDestructor)
	{
		JumpTo(mpcExit);
		StartBlock(mpcExit);
		if (!DestructorEnd())
		{
			return false;
		}
	}
	masTries.RemoveTail();
	mcCleanups.RemovePartsFrom(uiBase);
	JumpTo(pcAfter);

	//A destructor's handler that returns has had its members destroyed.
	mpcExit = NULL;
	mbConstructorHandler = bConstructor;
	bResult = TryHandlers(pcTry, pcDispatch, pcAfter, bConstructor || bDestructor);
	mbConstructorHandler = false;
	if (!bResult)
	{
		return false;
	}
	StartBlock(pcAfter);
	if (!DestroyLiving(0))
	{
		return false;
	}
	if (bDestructor)
	{
		mpcBlock->Return();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The handler's parameter, a local of the function's: a reference refers to
// what the match gave; an object is copied from it, and destroyed with the
// handler's scope; anything else is read from it.  A parameter with no name
// is nothing, unless it is an object.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::HandlerParameter(CCPPSyntaxCatchHandler* pcHandler, SIRReference sMatched)
{
	CCPPVariable*	pcVariable;
	SCPP65IRPlace	sPlace;
	SCPP65IRPlace	sSource;
	SCPP65IRValue	sValue;
	bool			bUnwinding;
	bool			bResult;

	if (!pcHandler->mpcParameter->mpcSymbol)
	{
		//With no name: an object is made all the same (its copy constructor and destructor run), in a place of its own.
		sPlace.psType = mpcUnit->HandlerType(pcHandler);
		if (!sPlace.psType || !sPlace.psType->IsClass())
		{
			return sPlace.psType != NULL;
		}
		sPlace.sAddress = mpcFunction->Slot(mpcFunction->GetBlock(0), IRType(IRTK_Pointer), (uint32)sPlace.psType->iBytes, 4);
	}
	else
	{
		pcVariable = mpcUnit->GetVariable(pcHandler->mpcParameter->mpcSymbol);
		if (!pcVariable || (pcVariable->meStorage != CPPVS_Local))
		{
			return Unsupported(__FILE__, __LINE__);
		}
		sPlace.sAddress = *mcAddresses.Get((int64)(size)pcVariable);
		sPlace.psType = pcVariable->mpsType;
	}
	if (sPlace.psType->IsReference())
	{
		mpcFunction->Store(mpcBlock, IRType(IRTK_Pointer), sMatched, sPlace.sAddress);
		return true;
	}
	if (sPlace.psType->IsClass())
	{
		//The handler isn't active until its parameter is made: an exception from the copy is std::terminate.
		//[except.terminate] 1, [except.handle] 7
		bUnwinding = mbUnwinding;
		mbUnwinding = true;
		bResult = CopyFromAddress(&sPlace, sMatched);
		mbUnwinding = bUnwinding;
		if (!bResult)
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
		return CopyPart(pcClass, psPlace->sAddress, sSource, false, IRNone());
	}
	if (pcClass->IsCopyDeleted())
	{
		return CopyDeleted(mpcValue);
	}
	sFrom.sAddress = sSource;
	sFrom.psType = psPlace->psType->Unqualified();
	return CopyPlace(psPlace, &sFrom, &sAddress) && (pcClass->IsTriviallyCopyable() || CopyParts(pcClass, psPlace->sAddress, sAddress, false, IRNone())) &&
		   StoreVtables(psPlace->sAddress, pcClass);
}
