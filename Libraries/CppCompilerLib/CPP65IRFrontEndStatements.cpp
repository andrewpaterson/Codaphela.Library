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
#include "CPP65Generator.h"
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
	uiTemporaries = masTemporaries.NumElements();
	bResult = StatementOf(pcStatement) && ((masTemporaries.NumElements() <= uiTemporaries) || Unsupported(__FILE__, __LINE__));
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
	case CPPS_AttributedStatement:
		//[[fallthrough]], [[likely]] and the like change nothing.
		return StatementOf(((CCPPSyntaxAttributedStatement*)pcStatement)->mpcStatement);
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

	uiLiving = masLiving.NumElements();
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
bool CCPP65IRFrontEnd::JumpStatement(CCPPSyntax* pcStatement, CIRBlock* pcBreak, CIRBlock* pcContinue)
{
	SCPP65IRJump*	psJump;
	bool			bResult;

	psJump = masJumps.Add();
	psJump->uiLiving = masLiving.NumElements();
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

	uiLiving = masLiving.NumElements();
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
		masLiving.Add(psPlace);
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
	masLiving.FakeSetUsedElements(uiLiving);
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
	return DestroyPlaces(&masLiving, uiLiving);
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

	uiTemporaries = masTemporaries.NumElements();
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
	masTemporaries.FakeSetUsedElements(uiTemporaries);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Destroys the objects in pasPlaces from uiFrom, last first.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::DestroyPlaces(CArrayCPP65IRPlaces* pasPlaces, size uiFrom)
{
	SCPP65IRPlace*	psPlace;
	size			ui;

	for (ui = pasPlaces->NumElements(); ui > uiFrom; ui--)
	{
		psPlace = pasPlaces->Get(ui - 1);
		if (!Destroy(psPlace->sAddress, psPlace->psType))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Destroys the temporaries made since uiFrom, last made first; one that may
// not have been made only if its flag says it was (the flag is cleared for
// the next time round a loop).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::DestroyTemporaryPlaces(size uiFrom)
{
	SCPP65IRTemporary*	psTemporary;
	size				ui;
	CIRBlock*			pcDestroy;
	CIRBlock*			pcNext;
	SCPP65IRValue		sMade;
	SIRReference		sCondition;
	SIRType				sFlag;

	sFlag = IRTypeOf(&mpcTypes->msBool);
	for (ui = masTemporaries.NumElements(); ui > uiFrom; ui--)
	{
		psTemporary = masTemporaries.Get(ui - 1);
		if (!psTemporary->bConditional)
		{
			if (!Destroy(psTemporary->sPlace.sAddress, psTemporary->sPlace.psType))
			{
				return false;
			}
			continue;
		}

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
	}
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
	if (pcStatement->mpcDeclaration && (pcStatement->mpcDeclaration->GetKind() == CPPS_StaticAssert))
	{
		//Checked when it is compiled; nothing is written.
		return mpcGenerator->StaticAssert((CCPPSyntaxStaticAssert*)pcStatement->mpcDeclaration) || Unsupported(__FILE__, __LINE__);
	}
	if (pcStatement->mpcDeclaration && mpcGenerator->IsScopeDeclaration(pcStatement->mpcDeclaration))
	{
		return true;
	}
	if (!pcStatement->mpcDeclaration || (pcStatement->mpcDeclaration->GetKind() != CPPS_SimpleDeclaration))
	{
		return Unsupported(__FILE__, __LINE__);
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

	if (pcDeclaration->mpcSpecifiers && (pcDeclaration->mpcSpecifiers->muiFlags & CPPDS_TYPEDEF))
	{
		return true;
	}
	for (ui = 0; ui < pcDeclaration->macpDeclarators.NumElements(); ui++)
	{
		pcInit = (CCPPSyntaxInitDeclarator*)pcDeclaration->macpDeclarators.GetPtr(ui);
		if (pcInit->mpcDeclarator->IsFunction())
		{
			mpcGenerator->DeclareBlockFunction(pcDeclaration->mpcSpecifiers, pcInit->mpcDeclarator);
			continue;
		}
		pcVariable = pcInit->mpcSymbol ? mpcGenerator->GetVariable(pcInit->mpcSymbol) : NULL;
		if (!pcVariable)
		{
			return Unsupported(__FILE__, __LINE__);
		}
		if (pcVariable->meStorage == CPPVS_Static)
		{
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
		sPlace.sAddress = *mcAddresses.Get((int64)(size)pcVariable);
		sPlace.psType = pcVariable->mpsType;
		uiTemporaries = masTemporaries.NumElements();
		if (ElementType(pcVariable->mpsType)->IsClass())
		{
			//Objects are constructed (by default without an initialiser) and destroyed when their scope is left.
			if (!Construct(&sPlace, NULL, pcInit->mpcInitializer))
			{
				return false;
			}
			AddLiving(&sPlace);
		}
		else if (pcInit->mpcInitializer && !LocalInitialiser(&sPlace, pcInit->mpcInitializer))
		{
			return false;
		}

		//Each initialiser is a full expression.  A reference to a temporary would keep it alive (not supported).
		if (pcVariable->mpsType->IsReference() && (masTemporaries.NumElements() > uiTemporaries))
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
// = value, (value), {value} or {} for a scalar; a reference takes its
// initialiser's address; an array its list or string.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::LocalInitialiser(SCPP65IRPlace* psPlace, CCPPSyntax* pcInitialiser)
{
	CArraySyntax*	papcElements;
	SCPP65IRValue	sValue;
	SCPP65IRPlace	sReferred;

	if (pcInitialiser->GetKind() == CPPS_ParenthesisedInitializer)
	{
		papcElements = &((CCPPSyntaxParenthesisedInitializer*)pcInitialiser)->macpElements;
		if (papcElements->NumElements() != 1)
		{
			return Unsupported(__FILE__, __LINE__);
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
	if ((pcInitialiser->GetKind() != CPPS_InitializerList) && !mpcGenerator->CheckInitialiser(pcInitialiser, psPlace->psType))
	{
		return false;
	}
	if (pcInitialiser->GetKind() == CPPS_InitializerList)
	{
		papcElements = &((CCPPSyntaxInitializerList*)pcInitialiser)->macpElements;
		if (papcElements->NumElements() == 0)
		{
			sValue.sValue = mpcFunction->AddConstant(IRTypeOf(psPlace->psType), 0);
			sValue.psType = psPlace->psType;
			Store(psPlace, &sValue);
			return true;
		}
		if (papcElements->NumElements() != 1)
		{
			return Unsupported(__FILE__, __LINE__);
		}
		pcInitialiser = papcElements->GetPtr(0);
	}

	if (psPlace->psType->IsReference())
	{
		if (!Place(pcInitialiser, &sReferred) || !ReferenceAddress(&sReferred, psPlace->psType, &sValue.sValue))
		{
			return false;
		}
		sValue.psType = psPlace->psType;
		mpcFunction->Store(mpcBlock, IRType(IRTK_Pointer), sValue.sValue, psPlace->sAddress);
		return true;
	}
	mpcGenerator->ConversionOverflow(pcInitialiser, psPlace->psType);
	if (!ValueFor(pcInitialiser, psPlace->psType, &sValue) || !Convert(&sValue, psPlace->psType))
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
	pcInitialiser = mpcGenerator->StringInitialiser(pcInitialiser, psElement);
	if ((pcInitialiser->GetKind() == CPPS_StringExpression) && psElement->IsScalar() && (psElement->iBytes == mpcGenerator->StringElementBytes(pcInitialiser)))
	{
		auiBytes.Init();
		if (!mpcGenerator->DecodeString(pcInitialiser, &auiBytes) || (auiBytes.NumElements() / psElement->iBytes > (size)psPlace->psType->iCount))
		{
			auiBytes.Kill();
			return Unsupported(__FILE__, __LINE__);
		}
		for (ui = 0; ui < auiBytes.NumElements() / psElement->iBytes; ui++)
		{
			sPlace.sAddress = AddressAt(psPlace->sAddress, (int)ui * psElement->iBytes);
			sPlace.psType = psElement;
			sValue.sValue = Integer(psElement, mpcGenerator->StringElement(&auiBytes, psElement->iBytes, ui));
			sValue.psType = psElement;
			Store(&sPlace, &sValue);
		}
		ZeroElements(psPlace->sAddress, (int)ui * psElement->iBytes, psElement, psPlace->psType->iCount - (int)ui);
		auiBytes.Kill();
		return true;
	}
	if (pcInitialiser->GetKind() != CPPS_InitializerList)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	papcElements = &((CCPPSyntaxInitializerList*)pcInitialiser)->macpElements;
	uiNext = 0;
	if (!ArrayElements(psPlace, papcElements, &uiNext))
	{
		return false;
	}
	return (uiNext >= papcElements->NumElements()) || Unsupported(__FILE__, __LINE__);
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
			if ((pcElement->GetKind() == CPPS_InitializerList) || (pcElement->GetKind() == CPPS_StringExpression))
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
			papcOne = &((CCPPSyntaxInitializerList*)pcElement)->macpElements;
			if (papcOne->NumElements() != 1)
			{
				return Unsupported(__FILE__, __LINE__);
			}
			pcElement = papcOne->GetPtr(0);
		}
		mpcGenerator->ConversionOverflow(pcElement, psElement);
		if (!ValueFor(pcElement, psElement, &sValue) || !Convert(&sValue, psElement))
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
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::WhileStatement(CCPPSyntaxWhileStatement* pcWhile)
{
	CIRBlock*		pcTest;
	CIRBlock*		pcBody;
	CIRBlock*		pcEnd;
	bool			bResult;

	pcTest = NewBlock();
	pcBody = NewBlock();
	pcEnd = NewBlock();
	JumpTo(pcTest);
	StartBlock(pcTest);
	if (!FullCondition(pcWhile->mpcCondition, pcBody, pcEnd))
	{
		return false;
	}
	StartBlock(pcBody);
	bResult = JumpStatement(pcWhile->mpcBody, pcEnd, pcTest);
	JumpTo(pcTest);
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
	CIRBlock*		pcEnd;
	SCPP65IRValue	sValue;
	bool			bResult;
	size			uiLiving;

	uiLiving = masLiving.NumElements();
	if (!Statement(pcFor->mpcInitialiser))
	{
		return false;
	}
	mpcGenerator->LoopCounterWidth(pcFor);
	pcTest = NewBlock();
	pcBody = NewBlock();
	pcNext = NewBlock();
	pcEnd = NewBlock();
	JumpTo(pcTest);
	StartBlock(pcTest);
	if (pcFor->mpcCondition)
	{
		if (!FullCondition(pcFor->mpcCondition, pcBody, pcEnd))
		{
			return false;
		}
	}
	else
	{
		JumpTo(pcBody);
	}
	StartBlock(pcBody);
	bResult = JumpStatement(pcFor->mpcBody, pcEnd, pcNext);
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
	StartBlock(pcEnd);
	return LeaveScope(uiLiving);
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

	if (!pcSwitch->mpcCondition || (pcSwitch->mpcCondition->GetKind() == CPPS_SimpleDeclaration))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	uiTemporaries = masTemporaries.NumElements();
	if (!Value(pcSwitch->mpcCondition, &sValue))
	{
		return false;
	}
	if (!sValue.psType->IsInteger())
	{
		return Unsupported(__FILE__, __LINE__);
	}
	//The condition is a full expression: its temporaries go before the cases are compared.
	if (!Convert(&sValue, mpcTypes->Promote(sValue.psType)) || !DestroyTemporaries(uiTemporaries))
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
		if (!mpcGenerator->Constant(psCase->pcCase->mpcValue, &lliValue))
		{
			return Unsupported(__FILE__, __LINE__);
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

	if (!pcNode || (pcNode->GetKind() == CPPS_SwitchStatement))
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
	return Unsupported(__FILE__, __LINE__);
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

	psReturn = mpcCPPFunction->mpsReturn;
	sType = mpcFunction->msReturnType;
	uiTemporaries = masTemporaries.NumElements();
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
	if (!mpcGenerator->CheckInitialiser(pcReturn->mpcValue, psReturn))
	{
		return false;
	}
	if (psReturn->IsReference())
	{
		if (!Place(pcReturn->mpcValue, &sPlace) || !ReferenceAddress(&sPlace, psReturn, &sValue.sValue))
		{
			return false;
		}
	}
	else
	{
		mpcGenerator->ConversionOverflow(pcReturn->mpcValue, psReturn);
		if (!ValueFor(pcReturn->mpcValue, psReturn, &sValue) || !Convert(&sValue, psReturn))
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
			if (!DestroyLiving(masJumps.Get(ui - 1)->uiLiving))
			{
				return false;
			}
			JumpTo(pcTarget);
			return Terminated();
		}
	}
	mpcGenerator->ReportError(pcStatement, bContinue ? "continue outside a loop" : "break outside a loop or switch");
	return Unsupported(__FILE__, __LINE__);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::GotoStatement(CCPPSyntaxGotoStatement* pcGoto)
{
	CIRBlock*	pcBlock;

	pcBlock = GotoBlock(pcGoto->miLabel);
	if (!pcBlock || (masLiving.NumElements() > 0))
	{
		//Which objects a goto leaves the scope of isn't worked out.
		return Unsupported(__FILE__, __LINE__);
	}
	JumpTo(pcBlock);
	return Terminated();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::LabelStatement(CCPPSyntaxLabelStatement* pcLabel)
{
	CIRBlock*	pcBlock;

	pcBlock = GotoBlock(pcLabel->miName);
	if (!pcBlock)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	JumpTo(pcBlock);
	StartBlock(pcBlock);
	return Statement(pcLabel->mpcStatement);
}


//////////////////////////////////////////////////////////////////////////
//
// The block for the goto label named by the token, made the first time it
// is seen.  NULL if the token is not a name.
//
//////////////////////////////////////////////////////////////////////////
CIRBlock* CCPP65IRFrontEnd::GotoBlock(int iToken)
{
	CCPPToken*		pcToken;
	SCPP65IRLabel*	psLabel;
	int64			lliName;
	size			ui;

	pcToken = mpcGenerator->GetTokeniser()->Get(iToken);
	if (!pcToken || !pcToken->IsIdentifier())
	{
		return NULL;
	}
	lliName = ((CCTIdentifier*)pcToken)->miIdentifier;
	for (ui = 0; ui < masLabels.NumElements(); ui++)
	{
		if (masLabels.Get(ui)->lliName == lliName)
		{
			return masLabels.Get(ui)->pcBlock;
		}
	}
	psLabel = masLabels.Add();
	psLabel->lliName = lliName;
	psLabel->pcBlock = NewBlock();
	return psLabel->pcBlock;
}
