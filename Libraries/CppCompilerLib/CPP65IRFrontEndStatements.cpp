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
#include "CTString.h"
#include "CTReservedWord.h"
#include "CPPUnit.h"
#include "CPP65IRFrontEnd.h"


//The IR front end's statements: compound, declarations and their initialisers, if, loops, switch, return, break,
//goto and labels, and the scopes whose objects are destroyed as they are left.


//////////////////////////////////////////////////////////////////////////
//
// A statement (StatementOf), after which none of the temporaries it made
// may still need destroying.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Statement(CCPPSyntax* pcStatement)
{
	size	uiTemporaries;
	uint32	uiSource;
	bool	bResult;

	//The instructions are for the statement's first token (its line), and the rest of the statement it is in for its
	//own (a for's condition and step after its body).
	uiSource = mcModule.muiSource;
	if (pcStatement && (pcStatement->miFirstToken >= 0))
	{
		mcModule.muiSource = (uint32)pcStatement->miFirstToken + 1;
	}

	//Expression, declaration and return statements destroy their temporaries; one left (made in a condition, say) isn't
	//supported.
	uiTemporaries = mcCleanups.NumTemporaries();
	bResult = StatementOf(pcStatement) && ((mcCleanups.NumTemporaries() <= uiTemporaries) || Unsupported(__FILE__, __LINE__));
	mcModule.muiSource = uiSource;
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Each kind of statement; an expression statement destroys its
// temporaries.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::StatementOf(CCPPSyntax* pcStatement)
{
	CCPPSyntaxExpressionStatement*	pcExpression;
	SCPP65IRValue					sValue;

	if (!pcStatement)
	{
		return true;
	}
	switch (pcStatement->GetKind())
	{
	case CPPS_CompoundStatement:
		return CompoundStatement((CCPPSyntaxCompoundStatement*)pcStatement);
	case CPPS_DeclarationStatement:
		return DeclarationStatement((CCPPSyntaxDeclarationStatement*)pcStatement);
	case CPPS_ExpressionStatement:
		pcExpression = (CCPPSyntaxExpressionStatement*)pcStatement;
		return !pcExpression->mpcExpression || FullExpression(pcExpression->mpcExpression, &sValue);
	case CPPS_IfStatement:
		return IfStatement((CCPPSyntaxIfStatement*)pcStatement);
	case CPPS_WhileStatement:
		return WhileStatement((CCPPSyntaxWhileStatement*)pcStatement);
	case CPPS_DoStatement:
		return DoStatement((CCPPSyntaxDoStatement*)pcStatement);
	case CPPS_ForStatement:
		return ForStatement((CCPPSyntaxForStatement*)pcStatement);
	case CPPS_SwitchStatement:
		return SwitchStatement((CCPPSyntaxSwitchStatement*)pcStatement);
	case CPPS_CaseStatement:
		return CaseStatement((CCPPSyntaxCaseStatement*)pcStatement);
	case CPPS_RangeForStatement:
		return RangeForStatement((CCPPSyntaxRangeForStatement*)pcStatement);
	case CPPS_ReturnStatement:
		return ReturnStatement((CCPPSyntaxReturnStatement*)pcStatement);
	case CPPS_BreakStatement:
		return BreakStatement(pcStatement, false);
	case CPPS_ContinueStatement:
		return BreakStatement(pcStatement, true);
	case CPPS_GotoStatement:
		return GotoStatement((CCPPSyntaxGotoStatement*)pcStatement);
	case CPPS_LabelStatement:
		return LabelStatement((CCPPSyntaxLabelStatement*)pcStatement);
	case CPPS_SkippedStatement:
		return AsmStatement(pcStatement);
	case CPPS_AttributedStatement:
		//[[fallthrough]], [[likely]] and the like change nothing.
		return StatementOf(((CCPPSyntaxAttributedStatement*)pcStatement)->mpcStatement);
	case CPPS_TryStatement:
		//In a program built without exceptions it is an error (ExceptionsPlan.txt).
		if (!mpcUnit->GetOptions()->mbExceptions)
		{
			mpcDiagnostics->ReportError(pcStatement, CPP_EXCEPTIONS_OFF_TRY);
			return Unsupported(__FILE__, __LINE__);
		}
		return TryStatement((CCPPSyntaxTryStatement*)pcStatement);
	default:
		return Unsupported(__FILE__, __LINE__);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CompoundStatement(CCPPSyntaxCompoundStatement* pcCompound)
{
	size	ui;
	size	uiLiving;

	uiLiving = mcCleanups.NumLocals();
	for (ui = 0; ui < pcCompound->macpStatements.NumElements(); ui++)
	{
		if (!Statement(pcCompound->macpStatements.GetPtr(ui)))
		{
			return false;
		}
	}
	return LeaveScope(uiLiving);
}


//////////////////////////////////////////////////////////////////////////
//
// A loop's or switch's body: break goes to pcBreak and continue to
// pcContinue (NULL in a switch, which passes it out), destroying on the
// way the objects declared since.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::JumpStatement(CCPPSyntax* pcStatement, CIRBlock* pcBreak, CIRBlock* pcContinue, size uiBreakLiving, size uiContinueLiving)
{
	SCPP65IRJump*	psJump;
	bool			bResult;

	//A break and a continue destroy what is living since the statement began, or since what a loop's condition
	//declares (uiBreakLiving, uiContinueLiving).
	psJump = masJumps.Add();
	psJump->uiLiving = (uiBreakLiving == (size)-1) ? mcCleanups.NumLocals() : uiBreakLiving;
	psJump->uiContinueLiving = (uiContinueLiving == (size)-1) ? mcCleanups.NumLocals() : uiContinueLiving;
	psJump->pcBreak = pcBreak;
	psJump->pcContinue = pcContinue;
	bResult = ScopedStatement(pcStatement);
	masJumps.Pop();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A statement that is a scope of its own (an if's, a loop's or a switch's
// body): objects it declares are destroyed after it.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ScopedStatement(CCPPSyntax* pcStatement)
{
	size	uiLiving;

	uiLiving = mcCleanups.NumLocals();
	return Statement(pcStatement) && LeaveScope(uiLiving);
}


//////////////////////////////////////////////////////////////////////////
//
// An object (or array of them) that is destroyed when its scope is left, if
// its class has a destructor.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::AddLiving(SCPP65IRPlace* psPlace)
{
	if (ElementType(psPlace->psType)->pcClass->mpcDestructor)
	{
		mcCleanups.PushLocal(psPlace);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A local reference bound to a temporary its initialiser made (at
// sReferred) keeps it: it is destroyed when the reference's scope is left,
// not at the end of the full expression.  [class.temporary] 6
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::ExtendTemporary(SIRReference sReferred)
{
	SCPP65IRPlace	sPlace;

	if (mcCleanups.TakeTemporary(sReferred, &sPlace))
	{
		AddLiving(&sPlace);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Destroys the objects declared since uiLiving (the last first) and
// forgets them: the end of their scope.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::LeaveScope(size uiLiving)
{
	bool	bResult;

	bResult = DestroyLiving(uiLiving);
	mcCleanups.CutLocals(uiLiving);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Destroys the objects living since uiLiving, the last first, for a jump
// out of their scopes (they stay living for the code after the jump).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::DestroyLiving(size uiLiving)
{
	SCPP65IRCleanup	sEntry;
	size			ui;

	for (ui = mcCleanups.NumLocals(); ui > uiLiving; ui--)
	{
		//A copy: writing a destructor's call may register something, which moves the entries.  A handler's entry is
		//among the locals: leaving its scope ends it (UndoEntry).
		sEntry = *mcCleanups.GetLocal(ui - 1);
		if (!UndoEntryBelow(&sEntry, mcCleanups.IndexOfLocal(ui - 1)))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// An expression statement's expression, a for statement's step and the
// like: a full expression, whose temporaries are destroyed at its end.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::FullExpression(CCPPSyntax* pcExpression, SCPP65IRValue* psValue)
{
	size	uiTemporaries;

	uiTemporaries = mcCleanups.NumTemporaries();
	return Expression(pcExpression, psValue) && DestroyTemporaries(uiTemporaries);
}


//////////////////////////////////////////////////////////////////////////
//
// The temporaries made since uiTemporaries, at the end of their full
// expression, last made first.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::DestroyTemporaries(size uiTemporaries)
{
	if (!DestroyTemporaryPlaces(uiTemporaries))
	{
		return false;
	}
	mcCleanups.CutTemporaries(uiTemporaries);
	MarkFullExpression();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A full-expression has been written: in a constant's IR the last
// instruction of the block it ends in is marked, and the interpreter counts
// those.  One that wrote nothing there (a constant) isn't counted.
// [implimits]
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::MarkFullExpression(void)
{
	if (mbConstants && mpcBlock && (mpcBlock->masInstructions.NumElements() > 0))
	{
		mpcBlock->masInstructions.Tail()->uiFlags |= IRF_FullExpression;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Destroys the temporaries made since uiFrom, last made first
// (DestroyTemporary).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::DestroyTemporaryPlaces(size uiFrom)
{
	SCPP65IRCleanup	sTemporary;
	size			ui;

	for (ui = mcCleanups.NumTemporaries(); ui > uiFrom; ui--)
	{
		//A copy: writing a destructor's call may register something, which moves the entries.
		sTemporary = *mcCleanups.GetTemporary(ui - 1);
		if (!UndoEntryBelow(&sTemporary, mcCleanups.IndexOfTemporary(ui - 1)))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Destroys a temporary; one that may not have been made only if its flag
// says it was (the flag is cleared for the next time round a loop).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::DestroyTemporary(SCPP65IRCleanup* psTemporary)
{
	CIRBlock*		pcDestroy;
	CIRBlock*		pcNext;
	SCPP65IRValue	sMade;
	SIRReference	sCondition;
	SIRType			sFlag;

	if (!psTemporary->bConditional)
	{
		return Destroy(psTemporary->sPlace.sAddress, psTemporary->sPlace.psType);
	}
	sFlag = IRTypeOf(&mpcTypes->msBool);
	pcDestroy = NewBlock();
	pcNext = NewBlock();
	sMade.sValue = mpcFunction->Load(mpcBlock, sFlag, psTemporary->sMade);
	sMade.psType = &mpcTypes->msBool;
	if (!Truth(&sMade, &sCondition))
	{
		return false;
	}
	mpcBlock->Branch(sCondition, pcDestroy, pcNext);
	StartBlock(pcDestroy);
	if (!Destroy(psTemporary->sPlace.sAddress, psTemporary->sPlace.psType))
	{
		return false;
	}
	mpcFunction->Store(mpcBlock, sFlag, mpcFunction->AddConstant(sFlag, 0), psTemporary->sMade);
	JumpTo(pcNext);
	StartBlock(pcNext);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Locals already have their slots; this runs their initialisers.  Local
// function declarations, typedefs, static_asserts and what only changes what
// names mean were dealt with by the generator's set up, or need nothing.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::DeclarationStatement(CCPPSyntaxDeclarationStatement* pcStatement)
{
	bool	bResult;

	if (pcStatement->mpcDeclaration && (pcStatement->mpcDeclaration->GetKind() == CPPS_StaticAssert))
	{
		//Checked when it is compiled; nothing is written.
		mpcDiagnostics->BeginLoud();
		bResult = mpcUnit->StaticAssert((CCPPSyntaxStaticAssert*)pcStatement->mpcDeclaration);
		mpcDiagnostics->EndLoud();
		return bResult;
	}
	if (pcStatement->mpcDeclaration && mpcUnit->IsScopeDeclaration(pcStatement->mpcDeclaration))
	{
		return true;
	}
	if (!pcStatement->mpcDeclaration || (pcStatement->mpcDeclaration->GetKind() != CPPS_SimpleDeclaration))
	{
		return !mpcUnit->GetCheck()->CheckBlockDeclaration(pcStatement->mpcDeclaration) && Unsupported(__FILE__, __LINE__);
	}
	if (!mpcUnit->GetCheck()->CheckLocalClass(pcStatement->mpcDeclaration))
	{
		return false;
	}
	return Declaration((CCPPSyntaxSimpleDeclaration*)pcStatement->mpcDeclaration, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// Runs a declaration's initialisers.  ppsLast is the last variable
// declared (NULL if it isn't wanted).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Declaration(CCPPSyntaxSimpleDeclaration* pcDeclaration, CCPPVariable** ppcLast)
{
	CCPPSyntaxInitDeclarator*		pcInit;
	CCPPVariable*					pcVariable;
	SCPP65IRPlace					sPlace;
	size							ui;
	size							uiTemporaries;
	bool							bResult;

	if (pcDeclaration->mpcSpecifiers && (pcDeclaration->mpcSpecifiers->muiFlags & CPPDS_TYPEDEF))
	{
		return true;
	}
	for (ui = 0; ui < pcDeclaration->macpDeclarators.NumElements(); ui++)
	{
		pcInit = (CCPPSyntaxInitDeclarator*)pcDeclaration->macpDeclarators.GetPtr(ui);
		if (pcInit->mpcDeclarator->IsFunction())
		{
			mpcUnit->GetFunctionDeclarer()->DeclareBlockFunction(pcDeclaration->mpcSpecifiers, pcInit->mpcDeclarator);
			continue;
		}
		pcVariable = pcInit->mpcSymbol ? mpcUnit->GetVariable(pcInit->mpcSymbol) : NULL;
		if (!pcVariable)
		{
			return Unsupported(__FILE__, __LINE__);
		}
		if ((pcVariable->meStorage != CPPVS_Global) && !mpcUnit->CheckDeclared(pcVariable->mpsType, pcInit, pcInit->mpcInitializer))
		{
			//(An extern declaration in a function only names a global.)
			return false;
		}
		if ((pcVariable->meStorage == CPPVS_Static) && pcVariable->mbGuarded)
		{
			if (!GuardedStaticLocal(pcVariable, pcInit->mpcInitializer))
			{
				return false;
			}
			continue;
		}
		if ((pcVariable->meStorage == CPPVS_Static) || (pcVariable->meStorage == CPPVS_Global))
		{
			//A static local is made in the data, and an extern one only names a global.
			continue;
		}
		if (pcVariable->meStorage != CPPVS_Local)
		{
			return Unsupported(__FILE__, __LINE__);
		}
		if (ppcLast)
		{
			*ppcLast = pcVariable;
		}
		if (pcDeclaration->mpcSpecifiers && (pcDeclaration->mpcSpecifiers->muiFlags & CPPDS_CONSTEXPR) && !mpcUnit->CheckConstexprLocal(pcVariable, pcInit))
		{
			return false;
		}
		sPlace.sAddress = *mcAddresses.Get((int64)(size)pcVariable);
		sPlace.psType = pcVariable->mpsType;
		uiTemporaries = mcCleanups.NumTemporaries();
		if (ElementType(pcVariable->mpsType)->IsClass())
		{
			//Objects are constructed (by default without an initialiser) and destroyed when their scope is left; a
			//std::initializer_list's array lives as long as it does.
			mbListLocal = mpcUnit->GetDesugarings()->InitializerListElement(pcVariable->mpsType) != NULL;
			bResult = Construct(&sPlace, NULL, pcInit->mpcInitializer);
			mbListLocal = false;
			if (!bResult)
			{
				return false;
			}
			AddLiving(&sPlace);
		}
		else if (pcInit->mpcInitializer && !LocalInitialiser(&sPlace, pcInit->mpcInitializer, true))
		{
			return false;
		}

		//Each initialiser is a full expression.  A temporary a reference is bound to through another (const S& r =
		//S(1).Self()) would be destroyed at its end, as one bound through a field (const int& r = S(1).v) mustn't.
		if (pcVariable->mpsType->IsReference() && (mcCleanups.NumTemporaries() > uiTemporaries))
		{
			return Unsupported(__FILE__, __LINE__);
		}
		if (!DestroyTemporaries(uiTemporaries))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// static S s; or static int v = g(); reached: made the first time, when its
// guard byte is still 0 (not started).  It is 1 while the object is being
// made and 2 once it is: reaching the declaration again while it is 1
// (from its own initialiser, or an interrupt) stops the program at
// R_RecursiveInit.  An exception from its initialiser puts the guard back to
// 0, so it is made again the next time.  The object isn't one of the scope's
// living ones: it outlives the function.  [stmt.dcl] 4
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::GuardedStaticLocal(CCPPVariable* pcVariable, CCPPSyntax* pcInitialiser)
{
	SCPP65IRPlace	sPlace;
	SIRReference	sGuard;
	SIRReference	sTest;
	CChars			szLabel;
	CChars			szFinaliser;
	CChars			szNode;
	CIRBlock*		pcMake;
	CIRBlock*		pcDone;
	CIRBlock*		pcStarted;
	CIRBlock*		pcAgain;
	SIRReference	sState;
	size			uiTemporaries;
	size			uiMark;

	if (!VariablePlace(pcVariable, &sPlace))
	{
		return false;
	}
	szLabel.Init();
	mpcUnit->GetMangler()->AppendGuardLabel(&szLabel, pcVariable);
	sGuard = mcModule.Symbol(szLabel.Text());
	if (!pcVariable->mpcSharedIn)
	{
		mcModule.SetLocal(sGuard.Index());
	}
	szLabel.Kill();

	pcMake = NewBlock();
	pcDone = NewBlock();
	pcStarted = NewBlock();
	pcAgain = NewBlock();
	sState = mpcFunction->Load(mpcBlock, IRType(IRTK_I8), sGuard);
	sTest = mpcFunction->Add(mpcBlock, IRO_Eq, IRType(IRTK_I8), sState, mpcFunction->AddInteger(IRType(IRTK_I8), 0));
	mpcBlock->Branch(sTest, pcMake, pcStarted);

	//Started: done, or being made now.
	StartBlock(pcStarted);
	sTest = mpcFunction->Add(mpcBlock, IRO_Eq, IRType(IRTK_I8), sState, mpcFunction->AddInteger(IRType(IRTK_I8), 1));
	mpcBlock->Branch(sTest, pcAgain, pcDone);
	StartBlock(pcAgain);
	mpcUnit->UseRecursiveInit();
	mpcFunction->Call(mpcBlock, IRType(IRTK_None), mcModule.Symbol("R_RecursiveInit"));
	JumpTo(pcDone);

	StartBlock(pcMake);
	mpcFunction->Store(mpcBlock, IRType(IRTK_I8), mpcFunction->AddInteger(IRType(IRTK_I8), 1), sGuard);
	uiMark = mcCleanups.NumEntries();
	PushAddressPart(CPP65IRP_Guard, sGuard);
	uiTemporaries = mcCleanups.NumTemporaries();
	if (ElementType(pcVariable->mpsType)->IsClass())
	{
		if (!Construct(&sPlace, NULL, pcInitialiser))
		{
			return false;
		}
	}
	else if (pcInitialiser && !LocalInitialiser(&sPlace, pcInitialiser, false))
	{
		return false;
	}
	if (!DestroyTemporaries(uiTemporaries))
	{
		return false;
	}
	mcCleanups.RemovePartsFrom(uiMark);
	if (pcVariable->mbExitNode)
	{
		//Made: it is destroyed from an exit list, by the R_Finalise the unit writes for it (WriteExitRoutines).
		szLabel.Init();
		mpcUnit->GetMangler()->AppendVariableLabel(&szLabel, pcVariable);
		szFinaliser.Init();
		mpcUnit->GetMangler()->AppendStartUpLabel(&szFinaliser, true, szLabel.Text());
		szNode.Init();
		mpcUnit->GetMangler()->AppendExitNodeLabel(&szNode, szLabel.Text());
		PushExitNode(szNode.Text(), szFinaliser.Text(), pcVariable->mbThreadLocal);
		szNode.Kill();
		szFinaliser.Kill();
		szLabel.Kill();
	}
	mpcFunction->Store(mpcBlock, IRType(IRTK_I8), mpcFunction->AddInteger(IRType(IRTK_I8), 2), sGuard);
	JumpTo(pcDone);
	StartBlock(pcDone);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// = value, (value), {value} or {} for a scalar; a reference takes its
// initialiser's address; an array its list or string.  bInFrame is for a
// local variable: its reference to a value (const int& r = 3, int&& r = 3)
// refers to a temporary in the frame.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::LocalInitialiser(SCPP65IRPlace* psPlace, CCPPSyntax* pcInitialiser, bool bInFrame)
{
	CArraySyntax*	papcElements;
	SCPP65IRValue	sValue;
	SCPP65IRPlace	sReferred;
	bool			bDirect;

	//T x(value) and T x{value} initialise directly: a std::nullptr_t may make a bool so.  [conv.bool]
	bDirect = (pcInitialiser->GetKind() == CPPS_ParenthesisedInitializer) || (pcInitialiser->GetKind() == CPPS_InitializerList);
	if (pcInitialiser->GetKind() == CPPS_ParenthesisedInitializer)
	{
		papcElements = &((CCPPSyntaxParenthesisedInitializer*)pcInitialiser)->macpElements;
		if (papcElements->NumElements() == 0)
		{
			//T(), as new int() has: zero.  [dcl.init] 11
			return ValueInitialise(psPlace);
		}
		if (papcElements->NumElements() != 1)
		{
			//[dcl.init] 17.8
			return mpcDiagnostics->ReportOnce(pcInitialiser, "A number or a pointer is initialised by one value");
		}
		pcInitialiser = papcElements->GetPtr(0);
	}

	if (!IsSupportedType(psPlace->psType))
	{
		//A class (or array of them) is initialised by the generator.
		return Unsupported(__FILE__, __LINE__);
	}
	if (psPlace->psType->IsArray())
	{
		return ArrayInitialiser(psPlace, pcInitialiser);
	}
	if ((pcInitialiser->GetKind() != CPPS_InitializerList) && !mpcUnit->CheckInitialiser(pcInitialiser, psPlace->psType))
	{
		return false;
	}
	if ((pcInitialiser->GetKind() == CPPS_InitializerList) && !psPlace->psType->IsReference())
	{
		//A reference is bound to what the list makes (BindsToTemporary).
		papcElements = mpcUnit->ListElements(pcInitialiser);
		if (papcElements->NumElements() == 0)
		{
			sValue.sValue = mpcFunction->AddConstant(IRTypeOf(psPlace->psType), 0);
			sValue.psType = psPlace->psType;
			Store(psPlace, &sValue);
			return true;
		}
		if (papcElements->NumElements() != 1)
		{
			//[dcl.init.list] 3
			return mpcDiagnostics->ReportOnce(pcInitialiser, "A number or a pointer is initialised by one value");
		}
		pcInitialiser = papcElements->GetPtr(0);
		if (!mpcUnit->CheckBracedValue(pcInitialiser, psPlace->psType))
		{
			return false;
		}
	}

	if (psPlace->psType->IsReference())
	{
		pcInitialiser = mpcUnit->GetBinder()->BoundInitialiser(psPlace->psType, pcInitialiser);
		if (mpcUnit->GetBinder()->BindsToTemporary(psPlace->psType, pcInitialiser))
		{
			//const int& r = 3, int&& r = 3 and const S& r = 3: a temporary in the function's frame, which lasts as
			//long as a local reference does.  A global's, or a field's, would outlast it.
			if (!bInFrame)
			{
				return Unsupported(__FILE__, __LINE__);
			}
			if (!ValueTemporary(pcInitialiser, psPlace->psType->psBase, &sReferred))
			{
				return false;
			}
			sValue.sValue = sReferred.sAddress;
		}
		else if (!ReferredPlace(pcInitialiser, psPlace->psType, &sReferred) || !ReferenceAddress(&sReferred, psPlace->psType, &sValue.sValue))
		{
			return false;
		}
		if (bInFrame)
		{
			ExtendTemporary(sReferred.sAddress);
		}
		sValue.psType = psPlace->psType;
		mpcFunction->Store(mpcBlock, IRType(IRTK_Pointer), sValue.sValue, psPlace->sAddress);
		return true;
	}
	mpcUnit->GetWarner()->ConversionOverflow(pcInitialiser, psPlace->psType);
	if (!ValueFor(pcInitialiser, psPlace->psType, &sValue))
	{
		return false;
	}
	if (bDirect && (psPlace->psType->eKind == CPP65TK_Bool) && sValue.psType->WithoutReference()->IsNullPointer())
	{
		if (!Convert(&sValue, psPlace->psType))
		{
			return false;
		}
	}
	else if (!ConvertImplicit(&sValue, psPlace->psType, pcInitialiser))
	{
		return false;
	}
	Store(psPlace, &sValue);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// An array from a string or a list of initialisers; elements without one are
// zero.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ArrayInitialiser(SCPP65IRPlace* psPlace, CCPPSyntax* pcInitialiser)
{
	SCPP65Type*		psElement;
	CArrayUint8		auiBytes;
	CArraySyntax*	papcElements;
	size			ui;
	size			uiNext;
	SCPP65IRPlace	sPlace;
	SCPP65IRValue	sValue;

	psElement = psPlace->psType->psBase;
	pcInitialiser = mpcUnit->StringInitialiser(pcInitialiser, psElement);
	if (mpcUnit->IsStringLiteral(pcInitialiser) && psElement->IsScalar() && (psElement->iBytes == mpcUnit->GetStrings()->StringElementBytes(pcInitialiser)))
	{
		auiBytes.Init();
		if (!mpcUnit->GetStrings()->DecodeString(pcInitialiser, &auiBytes))
		{
			auiBytes.Kill();
			return Unsupported(__FILE__, __LINE__);
		}
		if (auiBytes.NumElements() / psElement->iBytes > (size)psPlace->psType->iCount)
		{
			//[dcl.init.string] 2
			auiBytes.Kill();
			return mpcDiagnostics->ReportError(pcInitialiser, "The string has more characters, with its end, than the array has elements");
		}
		for (ui = 0; ui < auiBytes.NumElements() / psElement->iBytes; ui++)
		{
			sPlace.sAddress = AddressAt(psPlace->sAddress, (int)ui * psElement->iBytes);
			sPlace.psType = psElement;
			sValue.sValue = Integer(psElement, mpcUnit->GetStrings()->StringElement(&auiBytes, psElement->iBytes, ui));
			sValue.psType = psElement;
			Store(&sPlace, &sValue);
		}
		ZeroElements(psPlace->sAddress, (int)ui * psElement->iBytes, psElement, psPlace->psType->iCount - (int)ui);
		auiBytes.Kill();
		return true;
	}
	if (pcInitialiser->GetKind() != CPPS_InitializerList)
	{
		if (mpcUnit->IsStringLiteral(pcInitialiser))
		{
			//[dcl.init.string] 1
			return mpcDiagnostics->ReportError(pcInitialiser, mpcUnit->GetCheck()->StringArrayError(pcInitialiser, psElement));
		}
		return Unsupported(__FILE__, __LINE__);
	}
	papcElements = mpcUnit->ListElements(pcInitialiser);
	uiNext = 0;
	if (!ArrayElements(psPlace, papcElements, &uiNext))
	{
		return false;
	}
	//[dcl.init.aggr] 7
	return (uiNext >= papcElements->NumElements()) || mpcDiagnostics->ReportOnce(pcInitialiser, "More initialisers than the array's elements take");
}


//////////////////////////////////////////////////////////////////////////
//
// The array at psPlace from papcElements, starting at *puiNext (moved past
// the ones used).  Arrays of arrays take a list or string of their own, or
// (with the braces elided) the next of the enclosing list's.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ArrayElements(SCPP65IRPlace* psPlace, CArraySyntax* papcElements, size* puiNext)
{
	SCPP65Type*		psElement;
	SCPP65IRPlace	sPlace;
	SCPP65IRValue	sValue;
	CCPPSyntax*		pcElement;
	CArraySyntax*	papcOne;
	int				i;

	psElement = psPlace->psType->psBase;
	if (!psElement->IsScalar() && !psElement->IsArray())
	{
		return Unsupported(__FILE__, __LINE__);
	}
	for (i = 0; i < psPlace->psType->iCount; i++)
	{
		sPlace.sAddress = AddressAt(psPlace->sAddress, i * psElement->iBytes);
		sPlace.psType = psElement;
		if (*puiNext >= papcElements->NumElements())
		{
			ZeroElements(psPlace->sAddress, i * psElement->iBytes, psElement, psPlace->psType->iCount - i);
			return true;
		}
		pcElement = papcElements->GetPtr(*puiNext);
		if (psElement->IsArray())
		{
			if ((pcElement->GetKind() == CPPS_InitializerList) || mpcUnit->IsStringLiteral(pcElement))
			{
				(*puiNext)++;
				if (!ArrayInitialiser(&sPlace, pcElement))
				{
					return false;
				}
			}
			else if (!ArrayElements(&sPlace, papcElements, puiNext))
			{
				return false;
			}
			continue;
		}
		(*puiNext)++;
		if (pcElement->GetKind() == CPPS_InitializerList)
		{
			papcOne = mpcUnit->ListElements(pcElement);
			if (papcOne->NumElements() != 1)
			{
				return Unsupported(__FILE__, __LINE__);
			}
			pcElement = papcOne->GetPtr(0);
		}
		if ((psElement->IsInteger() || psElement->IsFloat()) && !mpcUnit->CheckBracedValue(pcElement, psElement))
		{
			//A list's value isn't narrowed to its element.  [dcl.init.aggr] 12, [dcl.init.list] 7
			return false;
		}
		mpcUnit->GetWarner()->ConversionOverflow(pcElement, psElement);
		if (!ValueFor(pcElement, psElement, &sValue) || !ConvertImplicit(&sValue, psElement, pcElement))
		{
			return false;
		}
		Store(&sPlace, &sValue);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Zeroes iCount elements (scalars or arrays of them) from iOffset bytes into
// the array at sAddress: all at once when there are many (ZeroBytes), unless
// they are volatile, which must each be written as themselves.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::ZeroElements(SIRReference sAddress, int iOffset, SCPP65Type* psElement, int iCount)
{
	SCPP65Type*		psScalar;
	int				iScalars;
	int				i;
	SCPP65IRPlace	sPlace;
	SCPP65IRValue	sZero;

	psScalar = psElement;
	iScalars = iCount;
	while (psScalar->IsArray())
	{
		iScalars *= psScalar->iCount;
		psScalar = psScalar->psBase;
	}
	if (!psScalar->bVolatile && (iScalars * psScalar->iBytes > IR65_ZERO_ELEMENT_BYTES))
	{
		ZeroBytes(AddressAt(sAddress, iOffset), iScalars * psScalar->iBytes);
		return;
	}
	sZero.sValue = mpcFunction->AddConstant(IRTypeOf(psScalar), 0);
	sZero.psType = psScalar;
	for (i = 0; i < iScalars; i++)
	{
		sPlace.sAddress = AddressAt(sAddress, iOffset + i * psScalar->iBytes);
		sPlace.psType = psScalar;
		Store(&sPlace, &sZero);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::IfStatement(CCPPSyntaxIfStatement* pcIf)
{
	CIRBlock*	pcThen;
	CIRBlock*	pcElse;
	CIRBlock*	pcEnd;
	size		uiLiving;

	//What the condition declares lives to the end of the if.  [stmt.select] 3
	uiLiving = mcCleanups.NumLocals();
	pcThen = NewBlock();
	pcEnd = NewBlock();
	pcElse = pcIf->mpcElse ? NewBlock() : pcEnd;
	if (!FullCondition(pcIf->mpcCondition, pcThen, pcElse))
	{
		return false;
	}
	StartBlock(pcThen);
	if (!ScopedStatement(pcIf->mpcThen))
	{
		return false;
	}
	JumpTo(pcEnd);
	if (pcIf->mpcElse)
	{
		StartBlock(pcElse);
		if (!ScopedStatement(pcIf->mpcElse))
		{
			return false;
		}
		JumpTo(pcEnd);
	}
	StartBlock(pcEnd);
	return LeaveScope(uiLiving);
}


//////////////////////////////////////////////////////////////////////////
//
// What the condition declares is made each time round and destroyed at the
// end of the body, by a break and a continue too, and when the condition
// is false.  [stmt.while] 2
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::WhileStatement(CCPPSyntaxWhileStatement* pcWhile)
{
	CIRBlock*		pcTest;
	CIRBlock*		pcBody;
	CIRBlock*		pcFalse;
	CIRBlock*		pcEnd;
	bool			bResult;
	size			uiLiving;

	pcTest = NewBlock();
	pcBody = NewBlock();
	pcFalse = NewBlock();
	pcEnd = NewBlock();
	JumpTo(pcTest);
	StartBlock(pcTest);
	uiLiving = mcCleanups.NumLocals();
	if (!FullCondition(pcWhile->mpcCondition, pcBody, pcFalse))
	{
		return false;
	}
	StartBlock(pcBody);
	bResult = JumpStatement(pcWhile->mpcBody, pcEnd, pcTest, uiLiving, uiLiving) && DestroyLiving(uiLiving);
	JumpTo(pcTest);
	StartBlock(pcFalse);
	bResult = bResult && DestroyLiving(uiLiving);
	mcCleanups.CutLocals(uiLiving);
	JumpTo(pcEnd);
	StartBlock(pcEnd);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::DoStatement(CCPPSyntaxDoStatement* pcDo)
{
	CIRBlock*		pcBody;
	CIRBlock*		pcTest;
	CIRBlock*		pcEnd;
	bool			bResult;

	pcBody = NewBlock();
	pcTest = NewBlock();
	pcEnd = NewBlock();
	JumpTo(pcBody);
	StartBlock(pcBody);
	bResult = JumpStatement(pcDo->mpcBody, pcEnd, pcTest);
	if (!bResult)
	{
		return false;
	}
	JumpTo(pcTest);
	StartBlock(pcTest);
	if (!FullCondition(pcDo->mpcCondition, pcBody, pcEnd))
	{
		return false;
	}
	StartBlock(pcEnd);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ForStatement(CCPPSyntaxForStatement* pcFor)
{
	CIRBlock*		pcTest;
	CIRBlock*		pcBody;
	CIRBlock*		pcNext;
	CIRBlock*		pcFalse;
	CIRBlock*		pcEnd;
	SCPP65IRValue	sValue;
	bool			bResult;
	size			uiLiving;
	size			uiCondition;

	uiLiving = mcCleanups.NumLocals();
	if (!Statement(pcFor->mpcInitialiser))
	{
		return false;
	}
	mpcUnit->GetWarner()->LoopCounterWidth(pcFor);
	pcTest = NewBlock();
	pcBody = NewBlock();
	pcNext = NewBlock();
	pcFalse = NewBlock();
	pcEnd = NewBlock();
	JumpTo(pcTest);
	StartBlock(pcTest);

	//What the condition declares is made each time round and destroyed at the end of the body, by a break and a
	//continue too, before the step (as g++ does), and when the condition is false.  [stmt.for] 1
	uiCondition = mcCleanups.NumLocals();
	if (pcFor->mpcCondition)
	{
		if (!FullCondition(pcFor->mpcCondition, pcBody, pcFalse))
		{
			return false;
		}
	}
	else
	{
		JumpTo(pcBody);
	}
	StartBlock(pcBody);
	bResult = JumpStatement(pcFor->mpcBody, pcEnd, pcNext, uiCondition, uiCondition) && DestroyLiving(uiCondition);
	if (!bResult)
	{
		return false;
	}
	JumpTo(pcNext);
	StartBlock(pcNext);
	if (pcFor->mpcIncrement && !FullExpression(pcFor->mpcIncrement, &sValue))
	{
		return false;
	}
	JumpTo(pcTest);
	StartBlock(pcFalse);
	if (!DestroyLiving(uiCondition))
	{
		return false;
	}
	mcCleanups.CutLocals(uiCondition);
	JumpTo(pcEnd);
	StartBlock(pcEnd);
	return LeaveScope(uiLiving);
}


//////////////////////////////////////////////////////////////////////////
//
// for (declaration : range) statement, as what it is made of
// (CCPPUnit::GetRangeFor): the range, the iterators, and the loop on them.
// The variable it declares is made each time round, before the statement
// and in its scope: break and continue leave it as they leave the
// statement.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::RangeForStatement(CCPPSyntaxRangeForStatement* pcFor)
{
	SCPPRangeFor*	psRangeFor;
	SCPP65IRPlace	sRange;
	SCPP65Type*		psBegin;
	SCPP65Type*		psEnd;
	CCPPFunction*	pcBegin;
	CCPPFunction*	pcEnd;
	CIRBlock*		pcTest;
	CIRBlock*		pcBody;
	CIRBlock*		pcNext;
	CIRBlock*		pcAfter;
	SCPP65IRJump*	psJump;
	SCPP65IRValue	sValue;
	size			uiLiving;
	size			uiEach;
	bool			bMember;
	bool			bResult;

	psRangeFor = mpcUnit->GetDesugarings()->GetRangeFor(pcFor);
	uiLiving = mcCleanups.NumLocals();
	if (!VariablePlace(psRangeFor->pcRange, &sRange) || !LocalInitialiser(&sRange, pcFor->mpcRange, true))
	{
		return false;
	}
	if (!mpcUnit->GetDesugarings()->RangeIterators(pcFor, &psBegin, &psEnd, &pcBegin, &pcEnd, &bMember))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (!RangeIterator(psRangeFor->pcBegin, pcBegin, bMember, psRangeFor->pcRangeOfBegin, false) || !RangeIterator(psRangeFor->pcEnd, pcEnd, bMember, psRangeFor->pcRangeOfEnd, true))
	{
		return false;
	}

	pcTest = NewBlock();
	pcBody = NewBlock();
	pcNext = NewBlock();
	pcAfter = NewBlock();
	JumpTo(pcTest);
	StartBlock(pcTest);
	if (!FullCondition(psRangeFor->pcCondition, pcBody, pcAfter))
	{
		return false;
	}
	StartBlock(pcBody);
	uiEach = mcCleanups.NumLocals();
	psJump = masJumps.Add();
	psJump->uiLiving = uiEach;
	psJump->uiContinueLiving = uiEach;
	psJump->pcBreak = pcAfter;
	psJump->pcContinue = pcNext;
	bResult = RangeForVariable(pcFor, psRangeFor->pcElement) && ScopedStatement(pcFor->mpcBody);
	masJumps.Pop();
	if (!bResult || !LeaveScope(uiEach))
	{
		return false;
	}
	JumpTo(pcNext);
	StartBlock(pcNext);
	if (!FullExpression(psRangeFor->pcIncrement, &sValue))
	{
		return false;
	}
	JumpTo(pcTest);
	StartBlock(pcAfter);
	return LeaveScope(uiLiving);
}


//////////////////////////////////////////////////////////////////////////
//
// Where a local variable is, from its symbol.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::VariablePlace(CCPPSymbol* pcSymbol, SCPP65IRPlace* psPlace)
{
	CCPPVariable*	pcVariable;
	SIRReference*	psAddress;

	pcVariable = mpcUnit->GetVariable(pcSymbol);
	psAddress = pcVariable ? mcAddresses.Get((int64)(size)pcVariable) : NULL;
	if (!psAddress)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	psPlace->sAddress = *psAddress;
	psPlace->psType = pcVariable->mpsType;
	psPlace->psBitField = NULL;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Gives a range-for's __begin or __end (pcSymbol) its value: for an array
// its first element's address or the one after its last (bEnd), and for
// an object what pcFunction returns, called on the range (pcRange) or
// with it.  An iterator that is an object is made where the variable is.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::RangeIterator(CCPPSymbol* pcSymbol, CCPPFunction* pcFunction, bool bMember, CCPPSyntax* pcRange, bool bEnd)
{
	SCPP65IRPlace	sPlace;
	SCPP65IRPlace	sArray;
	SCPP65IRValue	sValue;
	CArraySyntax	apcArguments;
	bool			bResult;

	if (!VariablePlace(pcSymbol, &sPlace))
	{
		return false;
	}
	if (!pcFunction)
	{
		if (!Place(pcRange, &sArray) || !sArray.psType->IsArray())
		{
			return Unsupported(__FILE__, __LINE__);
		}
		sValue.sValue = bEnd ? AddressAt(sArray.sAddress, sArray.psType->iCount * sArray.psType->psBase->iBytes) : sArray.sAddress;
		sValue.psType = sPlace.psType;
		Store(&sPlace, &sValue);
		return true;
	}
	if (pcFunction->mpsReturn->IsReference() || pcFunction->mbStatic)
	{
		return Unsupported(__FILE__, __LINE__);
	}

	apcArguments.Init();
	if (!bMember)
	{
		apcArguments.Add(pcRange);
	}
	if (sPlace.psType->IsClass())
	{
		msElide = sPlace.sAddress;
	}
	bResult = Call(pcFunction, FunctionSymbol(pcFunction), NULL, &apcArguments, &sValue, bMember ? pcRange : NULL, false, bMember ? pcFunction->mpcClass : NULL, bMember && pcFunction->mbVirtual);
	msElide = IRNone();
	apcArguments.Kill();
	if (!bResult)
	{
		return false;
	}
	if (sPlace.psType->IsClass())
	{
		if (!sValue.psType->IsClass() || !sValue.sValue.Equals(sPlace.sAddress))
		{
			return Unsupported(__FILE__, __LINE__);
		}
		AddLiving(&sPlace);
		return true;
	}
	if (!ConvertImplicit(&sValue, sPlace.psType, pcRange))
	{
		return false;
	}
	Store(&sPlace, &sValue);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The variable a range-for declares, made from pcElement (*__begin) as a
// local with that initialiser is.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::RangeForVariable(CCPPSyntaxRangeForStatement* pcFor, CCPPSyntax* pcElement)
{
	SCPP65IRPlace	sPlace;
	size			uiTemporaries;

	if (!VariablePlace(pcFor->mpcDeclarator->GetName()->mpcSymbol, &sPlace))
	{
		return false;
	}
	uiTemporaries = mcCleanups.NumTemporaries();
	if (ElementType(sPlace.psType)->IsClass())
	{
		if (!Construct(&sPlace, NULL, pcElement))
		{
			return false;
		}
		AddLiving(&sPlace);
	}
	else if (!LocalInitialiser(&sPlace, pcElement, true))
	{
		return false;
	}

	//A temporary a reference is bound to through another, or through a field, is not supported (see Declaration).
	if (sPlace.psType->IsReference() && (mcCleanups.NumTemporaries() > uiTemporaries))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	return DestroyTemporaries(uiTemporaries);
}


//////////////////////////////////////////////////////////////////////////
//
// The value is compared with each case in turn, as an int.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::SwitchStatement(CCPPSyntaxSwitchStatement* pcSwitch)
{
	SCPP65IRValue	sValue;
	SCPP65IRCase*	psCase;
	CIRBlock*		pcDefault;
	CIRBlock*		pcEnd;
	CIRBlock*		pcNext;
	SIRReference	sEqual;
	size			uiFirst;
	size			ui;
	int64			lliValue;
	bool			bResult;
	size			uiTemporaries;
	CCPPVariable*	pcVariable;
	SCPP65IRPlace	sPlace;
	CCPPFunction*	pcConversion;

	if (!pcSwitch->mpcCondition)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	uiTemporaries = mcCleanups.NumTemporaries();
	if (pcSwitch->mpcCondition->GetKind() == CPPS_SimpleDeclaration)
	{
		//switch (int x = value): the variable, in scope in the body, is what the cases are compared with.  [stmt.select] 4
		pcVariable = NULL;
		if (!Declaration((CCPPSyntaxSimpleDeclaration*)pcSwitch->mpcCondition, &pcVariable))
		{
			return false;
		}
		if (!pcVariable || !VariablePlace(pcVariable, &sPlace))
		{
			return Unsupported(__FILE__, __LINE__);
		}
		if (pcVariable->mpsType->IsClass())
		{
			//An object, converted by its one conversion function to an integer or an enum.  [stmt.switch] 2
			pcConversion = mpcUnit->GetBinder()->FindIntegerConversion(pcVariable->mpsType->pcClass, pcSwitch->mpcCondition);
			if (!pcConversion)
			{
				return mpcDiagnostics->ReportError(pcSwitch->mpcCondition, "A switch's condition is an integer or an enum, or an object with one conversion function to one");
			}
			sValue.psType = pcVariable->mpsType;
			sValue.sValue = sPlace.sAddress;
			if (!ConvertObject(&sValue, pcConversion->mpsReturn->WithoutReference()->Unqualified(), false))
			{
				return false;
			}
		}
		else if (!pcVariable->mpsType->IsScalar() || !Load(&sPlace, &sValue))
		{
			return Unsupported(__FILE__, __LINE__);
		}
	}
	else if (!ContextualValue(pcSwitch->mpcCondition, CPPXT_Integer, &sValue))
	{
		return false;
	}
	if (!sValue.psType->IsInteger())
	{
		if (sValue.psType->IsFloat() || sValue.psType->IsPointer())
		{
			//[stmt.switch] 2
			return mpcDiagnostics->ReportError(pcSwitch->mpcCondition, "A switch's condition is an integer or an enum");
		}
		return Unsupported(__FILE__, __LINE__);
	}
	//The condition is a full expression: its temporaries go before the cases are compared.
	if (!Convert(&sValue, mpcTypes->Promote(sValue.psType)) || !DestroyTemporaries(uiTemporaries))
	{
		return false;
	}

	if (!mpcUnit->GetCheck()->CheckSwitch(pcSwitch, sValue.psType))
	{
		return false;
	}
	pcEnd = NewBlock();
	pcDefault = NULL;
	uiFirst = masCases.NumElements();
	if (!FindCases(pcSwitch->mpcBody, &pcDefault))
	{
		return false;
	}
	for (ui = uiFirst; ui < masCases.NumElements(); ui++)
	{
		psCase = masCases.Get(ui);
		if (!psCase->pcCase->mpcValue)
		{
			continue;
		}
		if (!mpcUnit->GetFolder()->Constant(psCase->pcCase->mpcValue, &lliValue))
		{
			return Unsupported(__FILE__, __LINE__);
		}
		if (!mpcUnit->GetTypeChecker()->ConstantNotNarrowed(psCase->pcCase->mpcValue, sValue.psType))
		{
			return false;
		}
		pcNext = NewBlock();
		sEqual = mpcFunction->Add(mpcBlock, IRO_Eq, IRTypeOf(sValue.psType), sValue.sValue, Integer(sValue.psType, lliValue));
		mpcBlock->Branch(sEqual, masCases.Get(ui)->pcBlock, pcNext);
		StartBlock(pcNext);
	}
	JumpTo(pcDefault ? pcDefault : pcEnd);
	Terminated();

	bResult = JumpStatement(pcSwitch->mpcBody, pcEnd, NULL);
	while (masCases.NumElements() > uiFirst)
	{
		masCases.Pop();
	}
	JumpTo(pcEnd);
	StartBlock(pcEnd);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The case and default labels of a switch (not of switches inside it) get
// blocks.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::FindCases(CCPPSyntax* pcNode, CIRBlock** ppcDefault)
{
	SCPP65IRCase*	psCase;
	size			ui;

	if (!pcNode || (pcNode->GetKind() == CPPS_SwitchStatement) || (pcNode->GetKind() == CPPS_LambdaExpression))
	{
		return true;
	}
	if (pcNode->GetKind() == CPPS_CaseStatement)
	{
		psCase = masCases.Add();
		psCase->pcCase = (CCPPSyntaxCaseStatement*)pcNode;
		psCase->pcBlock = NewBlock();
		if (!psCase->pcCase->mpcValue)
		{
			if (*ppcDefault)
			{
				return Unsupported(__FILE__, __LINE__);
			}
			*ppcDefault = psCase->pcBlock;
		}
	}
	for (ui = 0; ui < pcNode->NumChildren(); ui++)
	{
		if (!FindCases(pcNode->GetChild(ui), ppcDefault))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The code before a case falls through into it.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CaseStatement(CCPPSyntaxCaseStatement* pcCase)
{
	size	ui;

	for (ui = 0; ui < masCases.NumElements(); ui++)
	{
		if (masCases.Get(ui)->pcCase == pcCase)
		{
			JumpTo(masCases.Get(ui)->pcBlock);
			StartBlock(masCases.Get(ui)->pcBlock);
			return Statement(pcCase->mpcStatement);
		}
	}
	return !mpcUnit->GetCheck()->CheckCaseInSwitch(pcCase) && Unsupported(__FILE__, __LINE__);
}


//////////////////////////////////////////////////////////////////////////
//
// return value: worked out (an object made at the result address, which
// is returned; a reference's place), then the living objects are destroyed
// (a destructor's returns go to its last block).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ReturnStatement(CCPPSyntaxReturnStatement* pcReturn)
{
	SCPP65Type*		psReturn;
	SCPP65IRValue	sValue;
	SCPP65IRPlace	sPlace;
	SIRType			sType;
	size			uiTemporaries;

	if (mbConstructorHandler)
	{
		mpcDiagnostics->ReportError(pcReturn, "A handler of a constructor's function-try-block can't return: it ends by rethrowing");
		return Unsupported(__FILE__, __LINE__);
	}
	if (!mpcUnit->GetCheck()->CheckReturn(mpcCPPFunction, pcReturn))
	{
		return false;
	}
	psReturn = mpcCPPFunction->mpsReturn;
	sType = mpcFunction->msReturnType;
	uiTemporaries = mcCleanups.NumTemporaries();
	//The value is worked out, then the living objects are destroyed.  An object is made at the result address, which
	//is returned.
	if (psReturn->IsClass())
	{
		if (!pcReturn->mpcValue)
		{
			return Unsupported(__FILE__, __LINE__);
		}
		sPlace.sAddress = msResult;
		sPlace.psType = psReturn;
		if (!Construct(&sPlace, NULL, pcReturn->mpcValue) || !DestroyTemporaries(uiTemporaries) || !DestroyLiving(0))
		{
			return false;
		}
		mpcBlock->Return(sType, msResult);
		return Terminated();
	}
	if (psReturn->IsVoid())
	{
		if (pcReturn->mpcValue && !Expression(pcReturn->mpcValue, &sValue))
		{
			return false;
		}
		if (!DestroyTemporaries(uiTemporaries) || !DestroyLiving(0))
		{
			return false;
		}
		if (mpcExit)
		{
			JumpTo(mpcExit);
		}
		else
		{
			mpcBlock->Return();
		}
		return Terminated();
	}
	if (!pcReturn->mpcValue)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (!mpcUnit->CheckInitialiser(pcReturn->mpcValue, psReturn))
	{
		return false;
	}
	if (psReturn->IsReference())
	{
		if (!ReferredPlace(pcReturn->mpcValue, psReturn, &sPlace) || !ReferenceAddress(&sPlace, psReturn, &sValue.sValue))
		{
			return false;
		}
	}
	else
	{
		mpcUnit->GetWarner()->ConversionOverflow(pcReturn->mpcValue, psReturn);
		if (!ValueFor(pcReturn->mpcValue, psReturn, &sValue) || !ConvertImplicit(&sValue, psReturn, pcReturn->mpcValue))
		{
			return false;
		}
		sValue.sValue = ConvertIR(sValue.sValue, IRTypeOf(psReturn), sType, psReturn->bSigned && (psReturn->eKind == CPP65TK_Integer));
	}
	if (!DestroyTemporaries(uiTemporaries) || !DestroyLiving(0))
	{
		return false;
	}
	mpcBlock->Return(sType, sValue.sValue);
	return Terminated();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::BreakStatement(CCPPSyntax* pcStatement, bool bContinue)
{
	size		ui;
	CIRBlock*	pcTarget;

	for (ui = masJumps.NumElements(); ui > 0; ui--)
	{
		pcTarget = bContinue ? masJumps.Get(ui - 1)->pcContinue : masJumps.Get(ui - 1)->pcBreak;
		if (pcTarget)
		{
			if (!DestroyLiving(bContinue ? masJumps.Get(ui - 1)->uiContinueLiving : masJumps.Get(ui - 1)->uiLiving))
			{
				return false;
			}
			JumpTo(pcTarget);
			return Terminated();
		}
	}
	mpcDiagnostics->ReportError(pcStatement, bContinue ? "continue outside a loop" : "break outside a loop or switch");
	return Unsupported(__FILE__, __LINE__);
}


//////////////////////////////////////////////////////////////////////////
//
// asm("text"); in a function: a call of a routine that is the text
// (CCPPUnit::AsmRoutine), which leaves what a call leaves.  Only the text:
// GNU's operands after a colon aren't taken.  A statement skipped for
// another reason is one the front end doesn't write.  [dcl.asm]
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::AsmStatement(CCPPSyntax* pcStatement)
{
	CCPPTokeniser*	pcTokeniser;
	CCPPToken*		pcToken;
	CArrayUint8		auiBytes;
	CChars			szLabel;
	int				iToken;
	ECppTokens		eToken;

	pcTokeniser = mpcUnit->GetTokeniser();
	iToken = pcStatement->miFirstToken;
	pcToken = pcTokeniser->Get(iToken);
	if (!pcToken || !pcToken->IsReservedWord() || ((((CCTReservedWord*)pcToken)->GetToken() != CT_asm) && (((CCTReservedWord*)pcToken)->GetToken() != CT___asm__)))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	for (iToken++; (pcToken = pcTokeniser->Get(iToken)) && pcToken->IsReservedWord(); iToken++)
	{
		eToken = ((CCTReservedWord*)pcToken)->GetToken();
		if ((eToken != CT_volatile) && (eToken != CT___volatile__))
		{
			break;
		}
	}
	if (!pcToken || !pcToken->IsReservedWord() || (((CCTReservedWord*)pcToken)->GetToken() != CT_OpenRoundBracket))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	auiBytes.Init();
	for (iToken++; (pcToken = pcTokeniser->Get(iToken)) && pcToken->IsString(); iToken++)
	{
		if (((CCTString*)pcToken)->meEncoding != CTE_Narrow)
		{
			break;
		}
		if (!mpcUnit->GetStrings()->DecodeStringToken(pcStatement, (CCTString*)pcToken, 1, &auiBytes))
		{
			auiBytes.Kill();
			return false;
		}
	}
	if ((auiBytes.NumElements() == 0) || !pcToken || !pcToken->IsReservedWord() || (((CCTReservedWord*)pcToken)->GetToken() != CT_CloseRoundBracket))
	{
		auiBytes.Kill();
		return Unsupported(__FILE__, __LINE__);
	}
	auiBytes.GrowToAtLeastNumElements(auiBytes.NumElements() + 1, true, 0);
	szLabel.Init();
	mpcUnit->GetClassData()->AsmRoutine((char*)auiBytes.GetData(), &szLabel);
	auiBytes.Kill();
	CallNoThrow(szLabel.Text(), IRType(IRTK_None), NULL, NULL, 0);
	szLabel.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::GotoStatement(CCPPSyntaxGotoStatement* pcGoto)
{
	SCPP65IRLabel*			psLabel;
	SCPP65IRPendingGoto*	psPending;
	size					ui;

	//A goto destroys the objects whose scopes it leaves, the last made first: those living here and not at the label.
	//[stmt.jump] 2
	psLabel = GotoLabel(pcGoto->miLabel);
	if (!psLabel)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (psLabel->bPlaced)
	{
		if (!GotoKeeps(NULL, mcCleanups.NumLocals(), pcGoto) || !DestroyLiving(psLabel->uiLiving))
		{
			return false;
		}
		JumpTo(psLabel->pcBlock);
		return Terminated();
	}

	//The label is further on: what is living there is known when it is reached.
	psPending = masPendingGotos.Add();
	psPending->lliName = psLabel->lliName;
	psPending->pcBlock = NewBlock();
	psPending->uiFirst = masGotoLocals.NumElements();
	psPending->uiLiving = mcCleanups.NumLocals();
	psPending->pcGoto = pcGoto;
	for (ui = 0; ui < psPending->uiLiving; ui++)
	{
		masGotoLocals.Add(mcCleanups.GetLocal(ui));
	}
	JumpTo(psPending->pcBlock);
	return Terminated();
}


//////////////////////////////////////////////////////////////////////////
//
// The gotos written before a label that is reached now: each's block
// destroys what is living at the goto and not here, and goes to the label.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::PlaceGotos(SCPP65IRLabel* psLabel)
{
	SCPP65IRPendingGoto	sPending;
	SCPP65IRCleanup		sEntry;
	CIRBlock*			pcHere;
	size				ui;
	size				uiLocal;

	pcHere = mpcBlock;
	for (ui = 0; ui < masPendingGotos.NumElements(); ui++)
	{
		sPending = *masPendingGotos.Get(ui);
		if (sPending.lliName != psLabel->lliName)
		{
			continue;
		}
		StartBlock(sPending.pcBlock);
		if (!GotoKeeps(masGotoLocals.Get(sPending.uiFirst), sPending.uiLiving, sPending.pcGoto))
		{
			return false;
		}
		for (uiLocal = sPending.uiLiving; uiLocal > psLabel->uiLiving; uiLocal--)
		{
			//What a destructor here throws undoes what is living at the label (an exception from one of these is no
			//longer undone by the others: a destructor is noexcept unless it says otherwise).
			sEntry = *masGotoLocals.Get(sPending.uiFirst + uiLocal - 1);
			if (!UndoEntryBelow(&sEntry, mcCleanups.NumEntries()))
			{
				return false;
			}
		}
		JumpTo(psLabel->pcBlock);
	}
	StartBlock(pcHere);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a goto whose living locals are pasLocals (uiLiving of them, the
// cleanup stack's own for NULL) to a label whose locals are those living
// now keeps them all: every one living at the label is living at the goto.
// Otherwise the goto jumps into the scope of an object past its making,
// which is an error.  [stmt.dcl] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::GotoKeeps(SCPP65IRCleanup* pasLocals, size uiLiving, CCPPSyntax* pcGoto)
{
	SCPP65IRLabel*	psLabel;
	size			uiAtLabel;
	size			ui;

	psLabel = GotoLabel(((CCPPSyntaxGotoStatement*)pcGoto)->miLabel);
	uiAtLabel = (psLabel && psLabel->bPlaced) ? psLabel->uiLiving : mcCleanups.NumLocals();
	if (pasLocals)
	{
		//The label is being reached: the stack is its own.
		for (ui = 0; (ui < uiAtLabel) && (ui < uiLiving); ui++)
		{
			if ((pasLocals[ui].eKind != mcCleanups.GetLocal(ui)->eKind) || !pasLocals[ui].sPlace.sAddress.Equals(mcCleanups.GetLocal(ui)->sPlace.sAddress))
			{
				break;
			}
		}
	}
	else
	{
		//The label was reached before: what was living there was the same objects as the first of those living here,
		//or it is in a scope this isn't in.
		ui = (uiAtLabel <= uiLiving) ? uiAtLabel : uiLiving;
		if (psLabel && psLabel->bPlaced && (ui == uiAtLabel))
		{
			for (ui = 0; ui < uiAtLabel; ui++)
			{
				if (!masGotoLocals.Get(psLabel->uiFirstLocal + ui)->sPlace.sAddress.Equals(mcCleanups.GetLocal(ui)->sPlace.sAddress))
				{
					break;
				}
			}
		}
	}
	if (ui < uiAtLabel)
	{
		return mpcDiagnostics->ReportError(pcGoto, "The goto jumps into the scope of an object it hasn't made: past its declaration, or into its block");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::LabelStatement(CCPPSyntaxLabelStatement* pcLabel)
{
	SCPP65IRLabel*	psLabel;
	size			ui;

	psLabel = GotoLabel(pcLabel->miName);
	if (!psLabel)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	JumpTo(psLabel->pcBlock);
	StartBlock(psLabel->pcBlock);
	psLabel->bPlaced = true;
	psLabel->uiLiving = mcCleanups.NumLocals();
	psLabel->uiFirstLocal = masGotoLocals.NumElements();
	for (ui = 0; ui < psLabel->uiLiving; ui++)
	{
		masGotoLocals.Add(mcCleanups.GetLocal(ui));
	}
	return PlaceGotos(psLabel) && Statement(pcLabel->mpcStatement);
}


//////////////////////////////////////////////////////////////////////////
//
// The goto label named by the token, made the first time it is seen with
// its block; NULL if the token is not a name.  Valid until the next label is
// added.
//
//////////////////////////////////////////////////////////////////////////
SCPP65IRLabel* CCPP65IRFrontEnd::GotoLabel(int iToken)
{
	CCPPToken*		pcToken;
	SCPP65IRLabel*	psLabel;
	int64			lliName;
	size			ui;

	pcToken = mpcUnit->GetTokeniser()->Get(iToken);
	if (!pcToken || !pcToken->IsIdentifier())
	{
		return NULL;
	}
	lliName = ((CCTIdentifier*)pcToken)->miIdentifier;
	for (ui = 0; ui < masLabels.NumElements(); ui++)
	{
		if (masLabels.Get(ui)->lliName == lliName)
		{
			return masLabels.Get(ui);
		}
	}
	psLabel = masLabels.Add();
	psLabel->lliName = lliName;
	psLabel->pcBlock = NewBlock();
	psLabel->bPlaced = false;
	psLabel->uiLiving = 0;
	psLabel->uiFirstLocal = 0;
	return psLabel;
}
