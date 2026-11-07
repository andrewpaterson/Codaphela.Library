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
#include "CPP65Generator.h"
#include "CPP65IRFrontEnd.h"


//The IR front end's objects: constructors and destructors (with their vtables), new and delete, temporaries,
//objects as values and conditions, and copying them.


//////////////////////////////////////////////////////////////////////////
//
// c ? x : y of a class.  Two objects that are where they are (lvalues)
// give the address of one of them (a phi); otherwise the one worked out is
// made at psInto (the object being initialised) or a temporary, which both
// make.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ObjectConditional(CCPPSyntaxConditionalExpression* pcConditional, SCPP65IRPlace* psInto, SCPP65IRValue* psValue)
{
	SCPP65Type*		psTrue;
	SCPP65Type*		psFalse;
	CIRBlock*		pcTrue;
	CIRBlock*		pcFalse;
	CIRBlock*		pcEnd;
	CIRBlock*		pcTrueEnd;
	SCPP65IRPlace	sTrue;
	SCPP65IRPlace	sFalse;
	SCPP65IRPlace	sPlace;
	bool			bLvalue;

	psTrue = mpcGenerator->TypeOf(pcConditional->mpcTrue);
	psFalse = mpcGenerator->TypeOf(pcConditional->mpcFalse);
	if (psTrue && psTrue->IsReference())
	{
		psTrue = psTrue->psBase;
	}
	if (psFalse && psFalse->IsReference())
	{
		psFalse = psFalse->psBase;
	}
	if (!psTrue || (psTrue != psFalse) || !psTrue->IsClass())
	{
		return Unsupported(__LINE__);
	}

	bLvalue = !psInto && IsObjectLvalue(pcConditional->mpcTrue) && IsObjectLvalue(pcConditional->mpcFalse);
	if (!bLvalue)
	{
		if (psInto)
		{
			sPlace = *psInto;
		}
		else if (!Temporary(psTrue, &sPlace, true))
		{
			return false;
		}
	}

	pcTrue = NewBlock();
	pcFalse = NewBlock();
	pcEnd = NewBlock();
	if (!Condition(pcConditional->mpcCondition, pcTrue, pcFalse))
	{
		return false;
	}
	miConditional++;
	StartBlock(pcTrue);
	if (bLvalue ? !Place(pcConditional->mpcTrue, &sTrue) : !Construct(&sPlace, NULL, pcConditional->mpcTrue))
	{
		return false;
	}
	pcTrueEnd = mpcBlock;
	JumpTo(pcEnd);
	StartBlock(pcFalse);
	if (bLvalue ? !Place(pcConditional->mpcFalse, &sFalse) : !Construct(&sPlace, NULL, pcConditional->mpcFalse))
	{
		return false;
	}
	miConditional--;
	JumpTo(pcEnd);
	psValue->psType = psTrue;
	if (bLvalue)
	{
		psValue->sValue = Phi(pcEnd, IRType(IRTK_Pointer), pcTrueEnd, sTrue.sAddress, mpcBlock, sFalse.sAddress);
	}
	else
	{
		psValue->sValue = sPlace.sAddress;
	}
	StartBlock(pcEnd);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// An expression of a class type that is an object where it is (named,
// pointed at, indexed, or a reference to one), not one to be made.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::IsObjectLvalue(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	pcExpression = mpcGenerator->SkipParentheses(pcExpression);
	psType = mpcGenerator->TypeOf(pcExpression);
	if (psType && psType->IsReference())
	{
		return true;
	}
	switch (pcExpression->GetKind())
	{
	case CPPS_IdExpression:
	case CPPS_MemberExpression:
	case CPPS_SubscriptExpression:
		return true;
	case CPPS_UnaryExpression:
		return ((CCPPSyntaxUnaryExpression*)pcExpression)->meOperator == CT_Multiply;
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// An expression of a class type, or a reference to one.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::IsObjectExpression(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	psType = mpcGenerator->TypeOf(pcExpression);
	if (psType && psType->IsReference())
	{
		psType = psType->psBase;
	}
	return psType && psType->IsClass();
}


//////////////////////////////////////////////////////////////////////////
//
// A call, or an operator on an object (its operator function's call), that
// may make the object it returns where msElide says.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::IsObjectCall(CCPPSyntax* pcExpression)
{
	CCPPSyntaxBinaryExpression*	pcBinary;
	ECppTokens					eOperator;

	pcExpression = mpcGenerator->SkipParentheses(pcExpression);
	switch (pcExpression->GetKind())
	{
	case CPPS_CallExpression:
		return true;
	case CPPS_BinaryExpression:
		pcBinary = (CCPPSyntaxBinaryExpression*)pcExpression;
		return !mpcGenerator->IsAssignmentOperator(pcBinary->meOperator) && !mpcGenerator->IsMemberPointerOperator(pcExpression) && (pcBinary->meOperator != CT_Comma);
	case CPPS_UnaryExpression:
		eOperator = ((CCPPSyntaxUnaryExpression*)pcExpression)->meOperator;
		return (eOperator == CT_Subtract) || (eOperator == CT_Add) || (eOperator == CT_BitwiseNot);
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a value is an object (its address), or a reference to one.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::IsObjectValue(SCPP65IRValue* psValue)
{
	SCPP65Type*	psType;

	psType = psValue->psType->IsReference() ? psValue->psType->psBase : psValue->psType;
	return psType->IsClass();
}


//////////////////////////////////////////////////////////////////////////
//
// An object converted to a scalar by its class's conversion function (the
// generator's FindConversion: operator T, its own or a base's), then the
// result to psTo.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ConvertObject(SCPP65IRValue* psValue, SCPP65Type* psTo)
{
	SCPP65Class*	psClass;
	SCPP65Function*	psFunction;
	SCPP65IRValue	sResult;

	psClass = (psValue->psType->IsReference() ? psValue->psType->psBase : psValue->psType)->psClass;
	psFunction = mpcGenerator->FindConversion(psClass, psTo, true, NULL);
	if (!psFunction)
	{
		if (mpcValue)
		{
			mpcGenerator->ReportError(mpcValue, "The object has no conversion to the type it is used as");
		}
		return Unsupported(__LINE__);
	}
	if (!Call(psFunction, FunctionSymbol(psFunction), NULL, &mapcNone, &sResult, NULL, false, psClass, true, psValue->sValue))
	{
		return false;
	}
	*psValue = sResult;
	return Convert(psValue, psTo);
}


//////////////////////////////////////////////////////////////////////////
//
// The address of a member call's or member access's object: pointer->,
// object-> (its operator->'s result) and object. (an lvalue).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ObjectAddress(CCPPSyntax* pcObject, bool bArrow, SIRReference* psAddress)
{
	SCPP65IRValue	sPointer;
	SCPP65IRPlace	sPlace;
	SCPP65Type*		psType;

	if (bArrow)
	{
		//An object's operator-> gives the pointer.
		psType = mpcGenerator->TypeOf(pcObject);
		if (psType && psType->IsReference())
		{
			psType = psType->psBase;
		}
		if (psType && psType->IsClass())
		{
			if (!OperatorCall(pcObject, CT_MemberByPointer, NULL, &sPointer))
			{
				return false;
			}
		}
		else if (!Value(pcObject, &sPointer))
		{
			return false;
		}
		if (!sPointer.psType->IsPointer() || !sPointer.psType->psBase->IsClass())
		{
			return Unsupported(__LINE__);
		}
		*psAddress = sPointer.sValue;
		return true;
	}
	if (!Place(pcObject, &sPlace))
	{
		return false;
	}
	if (!sPlace.psType->IsClass())
	{
		return Unsupported(__LINE__);
	}
	*psAddress = sPlace.sAddress;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// memcpy(Dest, Source, Count) with a small constant Count, as the generator
// writes it in line: Source then Dest are worked out and the bytes copied
// 4, 2 and 1 at a time.  The value is Dest.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::InlineMemcpy(CArraySyntax* papcArguments, SCPP65Type* psResult, SCPP65IRValue* psValue)
{
	SCPP65IRValue	sDest;
	SCPP65IRValue	sSource;
	int64			lliBytes;

	if (!psResult || !mpcGenerator->Constant(papcArguments->GetPtr(2), &lliBytes))
	{
		return Unsupported(__LINE__);
	}
	if (!Value(papcArguments->GetPtr(1), &sSource) || !Value(papcArguments->GetPtr(0), &sDest))
	{
		return false;
	}
	if (!sSource.psType->IsPointer() || !sDest.psType->IsPointer())
	{
		return Unsupported(__LINE__);
	}
	CopyPieces(sDest.sValue, sSource.sValue, (int)lliBytes);
	psValue->sValue = sDest.sValue;
	psValue->psType = psResult;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The start of a constructor, as the generator's ConstructorBody: the bases
// (from the member initialiser list or their default constructors), the
// vtable pointers, then the fields from the member initialisers, the
// default member initialisers or their own constructors.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ConstructorStart(CCPPSyntaxFunctionDefinition* pcFunction)
{
	SCPP65Class*					psClass;
	CArraySyntax*					papcInitialisers;
	CCPPSyntaxMemberInitializer*	pcMember;
	CCPPSyntaxMemberInitializer*	pcFound;
	CArraySyntax*					papcArguments;
	SCPP65Field*					psField;
	SCPP65Base*						psBase;
	SCPP65IRPlace					sPlace;
	CCPPSyntax*						pcInitialiser;
	SCPP65Class*					psNamed;
	size							ui;
	size							uiMember;
	int								iOffset;

	//An implicit constructor has no definition, so no member initialisers.
	papcInitialisers = pcFunction ? &pcFunction->macpMemberInitializers : &mapcNone;
	psClass = mpsFunction->psClass;
	if (msThis.IsNone() || !mpcGenerator->CompleteClass(psClass, NULL))
	{
		return Unsupported(__LINE__);
	}

	//The bases, in order.
	for (ui = 0; ui < psClass->macBases.NumElements(); ui++)
	{
		psBase = psClass->macBases.Get(ui);
		pcFound = NULL;
		for (uiMember = 0; uiMember < papcInitialisers->NumElements(); uiMember++)
		{
			pcMember = (CCPPSyntaxMemberInitializer*)papcInitialisers->GetPtr(uiMember);
			psNamed = (pcMember->mpcName->mpcSymbol && (pcMember->mpcName->mpcSymbol->meKind == CPPSK_Field)) ? NULL : mpcGenerator->GetNameClass(pcMember->mpcName, pcMember);
			if (psNamed == psBase->psClass)
			{
				pcFound = pcMember;
			}
		}
		papcArguments = &mapcNone;
		if (pcFound && pcFound->mpcInitializer)
		{
			if (pcFound->mpcInitializer->GetKind() == CPPS_ParenthesisedInitializer)
			{
				papcArguments = &((CCPPSyntaxParenthesisedInitializer*)pcFound->mpcInitializer)->macpElements;
			}
			else if (pcFound->mpcInitializer->GetKind() == CPPS_InitializerList)
			{
				papcArguments = &((CCPPSyntaxInitializerList*)pcFound->mpcInitializer)->macpElements;
			}
		}
		sPlace.sAddress = AddressAt(msThis, psBase->iOffset);
		sPlace.psType = psBase->psClass->psType;
		if (mpsFunction->psInherited && (mpsFunction->psInherited->psClass == psBase->psClass))
		{
			//An inherited constructor (using Base::Base) passes its parameters to the base's.
			if (!InheritedConstructorCall(&sPlace))
			{
				return false;
			}
		}
		else if (!ConstructClass(&sPlace, psBase->psClass, papcArguments))
		{
			return false;
		}
	}

	if (!StoreVtables(msThis, psClass))
	{
		return false;
	}

	//The fields, in order.
	for (ui = 0; ui < psClass->macFields.NumElements(); ui++)
	{
		psField = psClass->macFields.Get(ui);
		pcFound = NULL;
		for (uiMember = 0; uiMember < papcInitialisers->NumElements(); uiMember++)
		{
			pcMember = (CCPPSyntaxMemberInitializer*)papcInitialisers->GetPtr(uiMember);
			if ((pcMember->mpcName->mpcSymbol == psField->pcSymbol) || (!pcMember->mpcName->mpcSymbol && (mpcGenerator->GetComponentName(pcMember->mpcName->GetLast()) == psField->lliName)))
			{
				pcFound = pcMember;
			}
		}
		pcInitialiser = pcFound ? pcFound->mpcInitializer : psField->pcInitialiser;
		if (pcInitialiser || psField->psType->IsClass() || psField->psType->IsArray())
		{
			sPlace.sAddress = AddressAt(msThis, psField->iOffset);
			sPlace.psType = psField->psType;
			if (!Construct(&sPlace, NULL, pcInitialiser))
			{
				return false;
			}
		}
	}

	//Member initialisers that name nothing are the generator's to report.
	for (uiMember = 0; uiMember < papcInitialisers->NumElements(); uiMember++)
	{
		pcMember = (CCPPSyntaxMemberInitializer*)papcInitialisers->GetPtr(uiMember);
		psNamed = (pcMember->mpcName->mpcSymbol && (pcMember->mpcName->mpcSymbol->meKind == CPPSK_Field)) ? NULL : mpcGenerator->GetNameClass(pcMember->mpcName, pcMember);
		if (!(psNamed && mpcGenerator->IsDerived(psClass, psNamed, NULL)) && !mpcGenerator->FindField(psClass, mpcGenerator->GetComponentName(pcMember->mpcName->GetLast()), &iOffset))
		{
			return Unsupported(__LINE__);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The end of a destructor, after its body: the fields in reverse, then the
// bases in reverse.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::DestructorEnd(void)
{
	SCPP65Class*	psClass;
	SCPP65Field*	psField;
	SCPP65Base*		psBase;
	size			ui;

	psClass = mpsFunction->psClass;
	for (ui = psClass->macFields.NumElements(); ui > 0; ui--)
	{
		psField = psClass->macFields.Get(ui - 1);
		if (!Destroy(AddressAt(msThis, psField->iOffset), psField->psType))
		{
			return false;
		}
	}
	for (ui = psClass->macBases.NumElements(); ui > 0; ui--)
	{
		psBase = psClass->macBases.Get(ui - 1);
		if (!Destroy(AddressAt(msThis, psBase->iOffset), psBase->psClass->psType))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A polymorphic class's vtable pointers in the object at sObject, as the
// generator's StoreVtables: its own, and each base vtable it extends.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::StoreVtables(SIRReference sObject, SCPP65Class* psClass)
{
	SCPP65Vtable*	psVtable;
	CChars			szLabel;
	SIRReference	sLabel;
	size			ui;
	int				iEntry;

	if (!mpcGenerator->IsPolymorphic(psClass))
	{
		return true;
	}
	if (sObject.IsNone())
	{
		return Unsupported(__LINE__);
	}
	mpcGenerator->EnsureVtable(psClass);
	szLabel.Init();
	mpcGenerator->AppendClassLabel(&szLabel, psClass, "2");
	sLabel = mcModule.Symbol(szLabel.Text());
	szLabel.Kill();
	if (psClass->lliName < 0)
	{
		mcModule.SetLocal(sLabel.Index());
	}
	DescribeVtable(psClass, sLabel.Index());
	if (psClass->iVtableOffset != -1)
	{
		mpcFunction->Store(mpcBlock, IRType(IRTK_Pointer), sLabel, AddressAt(sObject, psClass->iVtableOffset));
	}
	iEntry = (int)psClass->mapsVirtuals.NumElements();
	for (ui = 0; ui < psClass->macVtables.NumElements(); ui++)
	{
		psVtable = psClass->macVtables.Get(ui);
		iEntry += 2;  //The secondary vtable's header (CCPP65Generator::AppendVtableHeader).
		mpcFunction->Store(mpcBlock, IRType(IRTK_Pointer), AddressAt(sLabel, 4 * iEntry), AddressAt(sObject, psVtable->iOffset + psVtable->psOwner->iVtableOffset));
		iEntry += (int)psVtable->mapsFunctions.NumElements();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The module's known data for a class's vtable, as the generator's
// EnsureVtable writes it, so a virtual call through a vtable pointer the
// optimiser can see stored becomes a call of the function (and may be
// inlined).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::DescribeVtable(SCPP65Class* psClass, uint32 uiSymbol)
{
	CIRGlobal*		pcVtable;
	SCPP65Vtable*	psVtable;
	size			i;
	size			j;

	if (mcModule.FindConstantData(uiSymbol))
	{
		return;
	}
	pcVtable = mcModule.AddKnownData(mcModule.GetSymbolName(uiSymbol), 4);
	for (i = 0; i < psClass->mapsVirtuals.NumElements(); i++)
	{
		AddVtableEntry(pcVtable, psClass->mapsVirtuals.GetPtr(i), 0);
	}
	for (i = 0; i < psClass->macVtables.NumElements(); i++)
	{
		psVtable = psClass->macVtables.Get(i);
		AddVtableHeader(pcVtable, psClass, psVtable->iOffset + psVtable->psOwner->iVtableOffset);
		for (j = 0; j < psVtable->mapsFunctions.NumElements(); j++)
		{
			AddVtableEntry(pcVtable, psVtable->mapsFunctions.GetPtr(j), psVtable->maiAdjust.GetValue(j));
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A secondary vtable's header, as CCPP65Generator::AppendVtableHeader writes it.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::AddVtableHeader(CIRGlobal* pcVtable, SCPP65Class* psClass, int iVtablePointer)
{
	CChars	szLabel;

	pcVtable->AddValue(IRIntegerType(4), (uint64)(int64)(-iVtablePointer));
	szLabel.Init();
	mpcGenerator->AppendClassLabel(&szLabel, psClass, "3");
	pcVtable->AddSymbol(IRType(IRTK_Pointer), mcModule.AddSymbol(szLabel.Text()), 0);
	szLabel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A vtable entry, as the generator's AppendVtableEntry: the function, the
// thunk that adjusts this by iAdjust, or R_Pure.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::AddVtableEntry(CIRGlobal* pcVtable, SCPP65Function* psFunction, int iAdjust)
{
	CChars	szLabel;

	szLabel.Init();
	if (psFunction->bPure)
	{
		szLabel.Append("R_Pure");
	}
	else
	{
		mpcGenerator->AppendFunctionLabel(&szLabel, psFunction);
		if (iAdjust != 0)
		{
			szLabel.Append("__t");
			szLabel.Append(-iAdjust);
		}
	}
	pcVtable->AddSymbol(IRType(IRTK_Pointer), mcModule.AddSymbol(szLabel.Text()), 0);
	szLabel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Initialises the object at a place, as the generator's Construct: from
// nothing, (arguments), {arguments}, = value or = Class(arguments).  Scalars
// and references take their initialiser as a local's do (or are zero),
// arrays are initialised element by element, objects are constructed (or
// copied, or made in place by a call that returns one).  papcArguments is
// used when pcInitialiser is NULL.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Construct(SCPP65IRPlace* psPlace, CArraySyntax* papcArguments, CCPPSyntax* pcInitialiser)
{
	SCPP65Type*						psType;
	SCPP65Type*						psSource;
	SCPP65IRPlace					sElement;
	SCPP65IRValue					sZero;
	CArraySyntax					apcArguments;
	CCPPSyntaxConstructExpression*	pcConstruct;
	SCPP65Class*					psClass;
	SCPP65IRValue					sMade;
	int								i;
	bool							bResult;
	bool							bValueInitialise;

	psType = psPlace->psType;
	if (!papcArguments)
	{
		papcArguments = &mapcNone;
	}

	if (psType->IsReference() || !ElementType(psType)->IsClass())
	{
		if (!pcInitialiser && (papcArguments->NumElements() > 1))
		{
			return Unsupported(__LINE__);
		}
		if (!pcInitialiser && (papcArguments->NumElements() == 1))
		{
			pcInitialiser = papcArguments->GetPtr(0);
		}
		if (pcInitialiser)
		{
			return LocalInitialiser(psPlace, pcInitialiser);
		}
		if (psType->IsReference())
		{
			return Unsupported(__LINE__);
		}
		if (psType->IsArray())
		{
			//An array of scalars is zeroed.
			ZeroBytes(psPlace->sAddress, psType->iBytes);
			return true;
		}
		sZero.sValue = mpcFunction->AddConstant(IRTypeOf(psType), 0);
		sZero.psType = psType;
		Store(psPlace, &sZero);
		return true;
	}

	bValueInitialise = false;
	if (pcInitialiser)
	{
		if (pcInitialiser->GetKind() == CPPS_ParenthesisedInitializer)
		{
			papcArguments = &((CCPPSyntaxParenthesisedInitializer*)pcInitialiser)->macpElements;
			pcInitialiser = NULL;
			bValueInitialise = papcArguments->NumElements() == 0;
		}
		else if (pcInitialiser->GetKind() == CPPS_InitializerList)
		{
			papcArguments = &((CCPPSyntaxInitializerList*)pcInitialiser)->macpElements;
			pcInitialiser = NULL;
			bValueInitialise = papcArguments->NumElements() == 0;
		}
	}

	//() and {} value initialise: an object without a constructor of its own is zeroed first.
	if (bValueInitialise && !ElementType(psType)->psClass->bConstructors)
	{
		ZeroBytes(psPlace->sAddress, psType->iBytes);
	}

	if (psType->IsArray())
	{
		if (pcInitialiser || (papcArguments->NumElements() > (size)psType->iCount))
		{
			return Unsupported(__LINE__);
		}
		sElement.psType = psType->psBase;
		for (i = 0; i < psType->iCount; i++)
		{
			sElement.sAddress = AddressAt(psPlace->sAddress, i * psType->psBase->iBytes);
			if (!Construct(&sElement, NULL, ((size)i < papcArguments->NumElements()) ? papcArguments->GetPtr(i) : NULL))
			{
				return false;
			}
		}
		return true;
	}

	psClass = psType->psClass;
	if (!mpcGenerator->CompleteClass(psClass, NULL))
	{
		return Unsupported(__LINE__);
	}

	//(object) and = object copy an object.
	if (!pcInitialiser && (papcArguments->NumElements() == 1) && !psClass->bConstructors)
	{
		psSource = mpcGenerator->TypeOf(papcArguments->GetPtr(0));
		if (psSource && psSource->IsReference())
		{
			psSource = psSource->psBase;
		}
		if (psSource && psSource->IsClass() && mpcGenerator->IsDerived(psSource->psClass, psClass, NULL))
		{
			return CopyConstruct(psPlace, papcArguments->GetPtr(0));
		}
	}
	if (pcInitialiser)
	{
		//= Class(arguments) constructs in place.
		if (pcInitialiser->GetKind() == CPPS_ConstructExpression)
		{
			pcConstruct = (CCPPSyntaxConstructExpression*)pcInitialiser;
			if ((mpcGenerator->GetTypeIdType(pcConstruct->mpcType) == psType) && pcConstruct->mpcInitializer && (pcConstruct->mpcInitializer->GetKind() == CPPS_ParenthesisedInitializer))
			{
				return ConstructClass(psPlace, psClass, &((CCPPSyntaxParenthesisedInitializer*)pcConstruct->mpcInitializer)->macpElements);
			}
		}
		psSource = mpcGenerator->TypeOf(pcInitialiser);
		if (psSource && (psSource == psType) && (mpcGenerator->SkipParentheses(pcInitialiser)->GetKind() == CPPS_ConditionalExpression))
		{
			//= c ? x : y makes the object here from whichever is worked out.
			return ObjectConditional((CCPPSyntaxConditionalExpression*)mpcGenerator->SkipParentheses(pcInitialiser), psPlace, &sMade);
		}
		if (psSource && psSource->IsClass() && (psSource->psClass == psClass) && IsObjectCall(pcInitialiser))
		{
			//A call (or an operator's) that makes an object of the class makes it here (as C++17 requires).
			msElide = psPlace->sAddress;
			if (!Value(pcInitialiser, &sMade))
			{
				return false;
			}
			msElide = IRNone();
			if (sMade.psType->IsClass() && sMade.sValue.Equals(psPlace->sAddress))
			{
				return true;
			}
			//It returned a reference: the object it refers to is copied.
			if (!mpcGenerator->IsTriviallyCopyable(psClass))
			{
				return Unsupported(__LINE__);
			}
			return CopyBytes(psPlace, &sMade);
		}
		if (psSource && psSource->IsReference())
		{
			psSource = psSource->psBase;
		}
		if (psSource && psSource->IsClass() && mpcGenerator->IsDerived(psSource->psClass, psClass, NULL))
		{
			return CopyConstruct(psPlace, pcInitialiser);
		}
		apcArguments.Init();
		apcArguments.Add(pcInitialiser);
		bResult = ConstructClass(psPlace, psClass, &apcArguments);
		apcArguments.Kill();
		return bResult;
	}
	return ConstructClass(psPlace, psClass, papcArguments);
}


//////////////////////////////////////////////////////////////////////////
//
// Calls the constructor of psClass that matches the arguments for the
// object at a place.  A class without constructors takes its fields from
// {arguments}, in order.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ConstructClass(SCPP65IRPlace* psPlace, SCPP65Class* psClass, CArraySyntax* papcArguments)
{
	SCPP65Function*	psConstructor;
	SCPP65Field*	psField;
	SCPP65IRPlace	sField;
	size			ui;

	if (!mpcGenerator->CompleteClass(psClass, NULL))
	{
		return Unsupported(__LINE__);
	}
	if (psClass->bConstructors || (papcArguments->NumElements() == 0))
	{
		psConstructor = mpcGenerator->FindFunction(psClass, psClass->lliName, CPP65FK_Constructor, papcArguments, false, mpsFunction->pcDefinition);
		if (psConstructor)
		{
			return CallObject(psConstructor, psPlace->sAddress, psClass, papcArguments, false);
		}
		return !psClass->bConstructors || Unsupported(__LINE__);
	}

	//An aggregate: the fields in order, the rest zero.
	if ((psClass->macBases.NumElements() > 0) || mpcGenerator->IsPolymorphic(psClass) || (papcArguments->NumElements() > psClass->macFields.NumElements()))
	{
		return Unsupported(__LINE__);
	}
	for (ui = 0; ui < psClass->macFields.NumElements(); ui++)
	{
		psField = psClass->macFields.Get(ui);
		sField.sAddress = AddressAt(psPlace->sAddress, psField->iOffset);
		sField.psType = psField->psType;
		if (!Construct(&sField, NULL, (ui < papcArguments->NumElements()) ? papcArguments->GetPtr(ui) : NULL))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Calls the destructor of the object (or each object of an array, the last
// first) at sAddress.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Destroy(SIRReference sAddress, SCPP65Type* psType)
{
	int		i;

	if (psType->IsArray())
	{
		if (!ElementType(psType)->IsClass())
		{
			return true;
		}
		for (i = psType->iCount; i > 0; i--)
		{
			if (!Destroy(AddressAt(sAddress, (i - 1) * psType->psBase->iBytes), psType->psBase))
			{
				return false;
			}
		}
		return true;
	}
	if (!psType->IsClass() || !psType->psClass->psDestructor)
	{
		return true;
	}
	return CallObject(psType->psClass->psDestructor, sAddress, psType->psClass, NULL, false);
}


//////////////////////////////////////////////////////////////////////////
//
// new Type, new Type(arguments) and new Type{arguments}: operator new's
// memory (or new (address) Type's address), constructed.  new Type[count]
// is NewArray's.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::NewExpression(CCPPSyntaxNewExpression* pcNew, SCPP65IRValue* psValue)
{
	SCPP65Type*		psType;
	SCPP65IRPlace	sPlace;
	SCPP65IRValue	sAddress;

	if (pcNew->mbPlacement && ((pcNew->macpPlacement.NumElements() != 1) || (pcNew->macpArraySizes.NumElements() > 0)))
	{
		return Unsupported(__LINE__);
	}
	psType = mpcGenerator->GetTypeIdType(pcNew->mpcType);
	if (!psType || !IsLocalType(psType) || psType->IsVoid() || psType->IsArray() || psType->IsReference())
	{
		return Unsupported(__LINE__);
	}
	if (pcNew->macpArraySizes.NumElements() > 0)
	{
		return NewArray(pcNew, psType, psValue);
	}
	if (pcNew->mbPlacement)
	{
		//new (address) Type: the object is made at the address.
		if (!Value(pcNew->macpPlacement.GetPtr(0), &sAddress))
		{
			return false;
		}
		if (!sAddress.psType->IsPointer())
		{
			return Unsupported(__LINE__);
		}
		sPlace.sAddress = sAddress.sValue;
	}
	else
	{
		sPlace.sAddress = CallRuntime("opNew", IRType(IRTK_Pointer), IRType(IRTK_I32), mpcFunction->AddInteger(IRType(IRTK_I32), psType->iBytes));
	}
	sPlace.psType = psType;
	if ((psType->IsClass() || pcNew->mpcInitializer) && !Construct(&sPlace, NULL, pcNew->mpcInitializer))
	{
		return false;
	}
	psValue->sValue = sPlace.sAddress;
	psValue->psType = mpcTypes->Pointer(psType);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// new Type[count], as the generator's NewArray: count elements from operator
// new[], constructed by default first to last.  A class that needs
// destroying has the count (32 bits) in front of the elements.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::NewArray(CCPPSyntaxNewExpression* pcNew, SCPP65Type* psType, SCPP65IRValue* psValue)
{
	SCPP65IRValue	sCount;
	SCPP65IRPlace	sElement;
	SIRType			sSize;
	SIRReference	sBytes;
	SIRReference	sMemory;
	SIRReference	sIndex;
	SIRReference	sNext;
	CIRBlock*		pcEntry;
	CIRBlock*		pcLoop;
	CIRBlock*		pcBody;
	CIRBlock*		pcLatch;
	CIRBlock*		pcDone;
	size			uiPhi;
	bool			bCookie;

	//Only new Type[count] and new Class[count]().
	if ((pcNew->macpArraySizes.NumElements() != 1) || (pcNew->mpcInitializer &&
		((pcNew->mpcInitializer->GetKind() != CPPS_ParenthesisedInitializer) || !psType->IsClass() ||
		(((CCPPSyntaxParenthesisedInitializer*)pcNew->mpcInitializer)->macpElements.NumElements() != 0))))
	{
		return Unsupported(__LINE__);
	}
	if (psType->IsClass() && !mpcGenerator->CompleteClass(psType->psClass, NULL))
	{
		return Unsupported(__LINE__);
	}
	if (!Value(pcNew->macpArraySizes.GetPtr(0), &sCount) || !Convert(&sCount, mpcTypes->Integer(mcModule.GetPointerSize(0), false)))
	{
		return false;
	}
	sSize = IRTypeOf(sCount.psType);
	bCookie = psType->IsClass() && psType->psClass->psDestructor;

	sBytes = mpcFunction->Add(mpcBlock, IRO_Mul, sSize, sCount.sValue, mpcFunction->AddInteger(sSize, psType->iBytes));
	if (bCookie)
	{
		sBytes = mpcFunction->Add(mpcBlock, IRO_Add, sSize, sBytes, mpcFunction->AddInteger(sSize, 4));
	}
	sMemory = CallRuntime("opNewArray", IRType(IRTK_Pointer), sSize, sBytes);
	if (bCookie)
	{
		mpcFunction->Store(mpcBlock, sSize, sCount.sValue, sMemory);
		sMemory = AddressAt(sMemory, 4);
	}

	if (psType->IsClass() && mpcGenerator->NeedsConstructor(psType->psClass))
	{
		//loop: the index counts up to the count.
		pcEntry = mpcBlock;
		pcLoop = NewBlock();
		pcBody = NewBlock();
		pcDone = NewBlock();
		JumpTo(pcLoop);
		StartBlock(pcLoop);
		uiPhi = pcLoop->masPhis.NumElements();
		sIndex = pcLoop->AddPhi(sSize, mpcFunction->AddTemporary(sSize))->sResult;
		mpcBlock->Branch(mpcFunction->Add(mpcBlock, IRO_ULt, sSize, sIndex, sCount.sValue), pcBody, pcDone);
		StartBlock(pcBody);
		sElement.psType = psType;
		sElement.sAddress = mpcFunction->Add(mpcBlock, IRO_PtrAdd, IRType(IRTK_Pointer), sMemory, mpcFunction->Add(mpcBlock, IRO_Mul, sSize, sIndex, mpcFunction->AddInteger(sSize, psType->iBytes)));
		if (!Construct(&sElement, NULL, NULL))
		{
			return false;
		}
		sNext = mpcFunction->Add(mpcBlock, IRO_Add, sSize, sIndex, mpcFunction->AddInteger(sSize, 1));
		pcLatch = mpcBlock;
		JumpTo(pcLoop);
		pcLoop->AddPhiArgument(pcLoop->masPhis.Get(uiPhi), pcEntry, mpcFunction->AddInteger(sSize, 0));
		pcLoop->AddPhiArgument(pcLoop->masPhis.Get(uiPhi), pcLatch, sNext);
		StartBlock(pcDone);
	}
	psValue->sValue = sMemory;
	psValue->psType = mpcTypes->Pointer(psType);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// delete[] of objects with a destructor, as the generator's: the count is
// in front of the elements, which are destroyed last first, and operator
// delete[] is given the memory from the count on.  psPointer isn't null.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::DeleteArray(SCPP65IRValue* psPointer, SCPP65Class* psClass)
{
	SIRType			sSize;
	SIRReference	sBase;
	SIRReference	sCount;
	SIRReference	sIndex;
	SIRReference	sNext;
	SIRReference	sElement;
	CIRBlock*		pcEntry;
	CIRBlock*		pcLoop;
	CIRBlock*		pcBody;
	CIRBlock*		pcLatch;
	CIRBlock*		pcDone;
	size			uiPhi;

	sSize = IRIntegerType(mcModule.GetPointerSize(0));
	sBase = AddressAt(psPointer->sValue, -4);
	sCount = mpcFunction->Load(mpcBlock, sSize, sBase);

	//loop: the index counts down from the count.
	pcEntry = mpcBlock;
	pcLoop = NewBlock();
	pcBody = NewBlock();
	pcDone = NewBlock();
	JumpTo(pcLoop);
	StartBlock(pcLoop);
	uiPhi = pcLoop->masPhis.NumElements();
	sIndex = pcLoop->AddPhi(sSize, mpcFunction->AddTemporary(sSize))->sResult;
	mpcBlock->Branch(mpcFunction->Add(mpcBlock, IRO_Ne, sSize, sIndex, mpcFunction->AddInteger(sSize, 0)), pcBody, pcDone);
	StartBlock(pcBody);
	sNext = mpcFunction->Add(mpcBlock, IRO_Sub, sSize, sIndex, mpcFunction->AddInteger(sSize, 1));
	sElement = mpcFunction->Add(mpcBlock, IRO_PtrAdd, IRType(IRTK_Pointer), psPointer->sValue, mpcFunction->Add(mpcBlock, IRO_Mul, sSize, sNext, mpcFunction->AddInteger(sSize, psClass->psType->iBytes)));
	if (!CallObject(psClass->psDestructor, sElement, psClass, NULL, false))
	{
		return false;
	}
	pcLatch = mpcBlock;
	JumpTo(pcLoop);
	pcLoop->AddPhiArgument(pcLoop->masPhis.Get(uiPhi), pcEntry, sCount);
	pcLoop->AddPhiArgument(pcLoop->masPhis.Get(uiPhi), pcLatch, sNext);
	StartBlock(pcDone);
	CallRuntime("opDeleteArray", IRType(IRTK_None), IRType(IRTK_Pointer), sBase);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// delete pointer: unless it is null, the object's destructor (virtual if
// it is) then operator delete.  delete[] of objects with destructors is
// DeleteArray's.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::DeleteExpression(CCPPSyntaxDeleteExpression* pcDelete, SCPP65IRValue* psValue)
{
	SCPP65IRValue	sPointer;
	SCPP65Type*		psType;
	SIRReference	sCondition;
	CIRBlock*		pcFree;
	CIRBlock*		pcDone;

	if (!Value(pcDelete->mpcOperand, &sPointer))
	{
		return false;
	}
	if (!sPointer.psType->IsPointer())
	{
		return Unsupported(__LINE__);
	}
	psType = sPointer.psType->psBase;
	if (psType->IsClass() && !mpcGenerator->CompleteClass(psType->psClass, NULL))
	{
		return Unsupported(__LINE__);
	}

	pcFree = NewBlock();
	pcDone = NewBlock();
	if (!Truth(&sPointer, &sCondition))
	{
		return false;
	}
	mpcBlock->Branch(sCondition, pcFree, pcDone);
	StartBlock(pcFree);
	if (pcDelete->mbArray && psType->IsClass() && psType->psClass->psDestructor)
	{
		if (!DeleteArray(&sPointer, psType->psClass))
		{
			return false;
		}
	}
	else
	{
		if (psType->IsClass() && psType->psClass->psDestructor && !CallObject(psType->psClass->psDestructor, sPointer.sValue, psType->psClass, NULL, true))
		{
			return false;
		}
		CallRuntime(pcDelete->mbArray ? "opDeleteArray" : "opDelete", IRType(IRTK_None), IRType(IRTK_Pointer), sPointer.sValue);
	}
	JumpTo(pcDone);
	StartBlock(pcDone);
	psValue->sValue = IRNone();
	psValue->psType = &mpcTypes->msVoid;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// An expression of a class type: its address (an object used as a value is
// its address).  Objects named, pointed at or indexed are where they are;
// a call that returns one and Class(arguments) make a temporary (or the
// object being initialised, see msElide); object = object copies and
// object op= value calls the class's operator.  Other operators on objects
// call their operator functions (OperatorExpression).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ObjectValue(CCPPSyntax* pcExpression, SCPP65IRValue* psValue)
{
	SCPP65IRPlace					sPlace;
	CCPPSyntaxConstructExpression*	pcConstruct;
	SCPP65Type*						psType;
	CArraySyntax					apcArguments;
	bool							bResult;
	CCPPSyntaxTypeId*				pcType;
	CCPPSyntax*						pcOperand;

	switch (pcExpression->GetKind())
	{
	case CPPS_IdExpression:
	case CPPS_MemberExpression:
	case CPPS_SubscriptExpression:
	case CPPS_ThisExpression:
		break;
	case CPPS_UnaryExpression:
		switch (((CCPPSyntaxUnaryExpression*)pcExpression)->meOperator)
		{
		case CT_Multiply:
			break;
		case CT_Subtract:
		case CT_Add:
		case CT_BitwiseNot:
			//-object, +object, ~object: the object's operator.
			return OperatorExpression(((CCPPSyntaxUnaryExpression*)pcExpression)->meOperator, ((CCPPSyntaxUnaryExpression*)pcExpression)->mpcOperand, NULL, psValue);
		default:
			return Unsupported(__LINE__);
		}
		break;
	case CPPS_CallExpression:
		return CallExpression((CCPPSyntaxCallExpression*)pcExpression, psValue);
	case CPPS_ConditionalExpression:
		return ObjectConditional((CCPPSyntaxConditionalExpression*)pcExpression, NULL, psValue);
	case CPPS_BinaryExpression:
		if (!mpcGenerator->IsAssignmentOperator(((CCPPSyntaxBinaryExpression*)pcExpression)->meOperator))
		{
			return OperatorExpression(((CCPPSyntaxBinaryExpression*)pcExpression)->meOperator, ((CCPPSyntaxBinaryExpression*)pcExpression)->mpcLeft, ((CCPPSyntaxBinaryExpression*)pcExpression)->mpcRight, psValue);
		}
		return Assignment((CCPPSyntaxBinaryExpression*)pcExpression, psValue);
	case CPPS_ConstructExpression:
		pcConstruct = (CCPPSyntaxConstructExpression*)pcExpression;
		psType = mpcGenerator->GetTypeIdType(pcConstruct->mpcType);
		if (!psType || !psType->IsClass() || !Temporary(psType, &sPlace, true))
		{
			return Unsupported(__LINE__);
		}
		if (!Construct(&sPlace, NULL, pcConstruct->mpcInitializer))
		{
			return false;
		}
		psValue->sValue = sPlace.sAddress;
		psValue->psType = psType;
		return true;
	case CPPS_CastExpression:
	case CPPS_NamedCastExpression:
		//(Class)value and static_cast<Class>(value) are a temporary made from the value, as Class(value).
		if (pcExpression->GetKind() == CPPS_CastExpression)
		{
			pcType = ((CCPPSyntaxCastExpression*)pcExpression)->mpcType;
			pcOperand = ((CCPPSyntaxCastExpression*)pcExpression)->mpcOperand;
		}
		else if (((CCPPSyntaxNamedCastExpression*)pcExpression)->meCast == CT_static_cast)
		{
			pcType = ((CCPPSyntaxNamedCastExpression*)pcExpression)->mpcType;
			pcOperand = ((CCPPSyntaxNamedCastExpression*)pcExpression)->mpcOperand;
		}
		else
		{
			if (((CCPPSyntaxNamedCastExpression*)pcExpression)->meCast == CT_dynamic_cast)
			{
				mpcGenerator->ReportError(pcExpression, "dynamic_cast to a reference is not supported");
			}
			return Unsupported(__LINE__);
		}
		psType = mpcGenerator->GetTypeIdType(pcType);
		if (!psType || !psType->IsClass() || !Temporary(psType, &sPlace, true))
		{
			return Unsupported(__LINE__);
		}
		apcArguments.Init();
		apcArguments.Add(pcOperand);
		bResult = Construct(&sPlace, &apcArguments, NULL);
		apcArguments.Kill();
		if (!bResult)
		{
			return false;
		}
		psValue->sValue = sPlace.sAddress;
		psValue->psType = psType;
		return true;
	default:
		return Unsupported(__LINE__);
	}
	if (!Place(pcExpression, &sPlace))
	{
		return false;
	}
	return Load(&sPlace, psValue);
}


//////////////////////////////////////////////////////////////////////////
//
// A place in the frame for a temporary object.  One that needs destroying
// (and bDestroy: not a copy of an argument's bytes, which the callee
// destroys) is destroyed at the end of the full expression
// (DestroyTemporaries); one that may not be made (miConditional) only if it
// was, so its place and flag are in the entry block (which reaches the
// destroying).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Temporary(SCPP65Type* psType, SCPP65IRPlace* psPlace, bool bDestroy)
{
	bool				bDestructor;
	SCPP65IRTemporary*	psTemporary;
	CIRBlock*			pcEntry;
	SIRType				sFlag;

	if (!mpcGenerator->CompleteClass(psType->psClass, NULL))
	{
		return Unsupported(__LINE__);
	}
	bDestructor = bDestroy && mpcGenerator->NeedsDestructor(psType->psClass);
	psPlace->psType = psType;
	if (!bDestructor || (miConditional == 0))
	{
		psPlace->sAddress = mpcFunction->Slot(mpcBlock, IRType(IRTK_Pointer), (uint32)psType->iBytes, 4);
		if (bDestructor)
		{
			psTemporary = masTemporaries.Add();
			psTemporary->sPlace = *psPlace;
			psTemporary->bConditional = false;
			psTemporary->sMade = IRNone();
		}
		return true;
	}

	pcEntry = mpcFunction->GetBlock(0);
	sFlag = IRTypeOf(&mpcTypes->msBool);
	psPlace->sAddress = mpcFunction->Slot(pcEntry, IRType(IRTK_Pointer), (uint32)psType->iBytes, 4);
	psTemporary = masTemporaries.Add();
	psTemporary->sPlace = *psPlace;
	psTemporary->bConditional = true;
	psTemporary->sMade = mpcFunction->Slot(pcEntry, IRType(IRTK_Pointer), 1, 1);
	mpcFunction->Store(pcEntry, sFlag, mpcFunction->AddConstant(sFlag, 0), psTemporary->sMade);
	mpcFunction->Store(mpcBlock, sFlag, mpcFunction->AddConstant(sFlag, 1), psTemporary->sMade);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Copy constructs the object at psPlace from pcSource (of its class or one
// derived from it), as the generator's CopyConstruct: with the class's copy
// constructor, its implicit one (CopyMembers), or its bytes.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CopyConstruct(SCPP65IRPlace* psPlace, CCPPSyntax* pcSource)
{
	SCPP65Class*	psClass;
	CArraySyntax	apcArguments;
	bool			bResult;

	psClass = psPlace->psType->psClass;
	if (mpcGenerator->HasCopyConstructor(psClass))
	{
		apcArguments.Init();
		apcArguments.Add(pcSource);
		bResult = ConstructClass(psPlace, psClass, &apcArguments);
		apcArguments.Kill();
		return bResult;
	}
	if (!mpcGenerator->IsTriviallyCopyable(psClass))
	{
		return CopyMembers(psPlace, pcSource);
	}
	return CopyObject(psPlace, pcSource);
}


//////////////////////////////////////////////////////////////////////////
//
// Zeroes iBytes from sAddress on: a long at a time (in a loop when there
// are many), then the bytes left.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::ZeroBytes(SIRReference sAddress, int iBytes)
{
	SIRType			sOffset;
	SIRReference	sIndex;
	SIRReference	sNext;
	CIRBlock*		pcEntry;
	CIRBlock*		pcLoop;
	CIRBlock*		pcBody;
	CIRBlock*		pcLatch;
	CIRBlock*		pcDone;
	size			uiPhi;
	int				iLongs;
	int				i;

	iLongs = iBytes / 4;
	if (iLongs > 4)
	{
		//loop: the offset counts up in longs.
		sOffset = IRIntegerType(mcModule.GetPointerSize(0));
		pcEntry = mpcBlock;
		pcLoop = NewBlock();
		pcBody = NewBlock();
		pcDone = NewBlock();
		JumpTo(pcLoop);
		StartBlock(pcLoop);
		uiPhi = pcLoop->masPhis.NumElements();
		sIndex = pcLoop->AddPhi(sOffset, mpcFunction->AddTemporary(sOffset))->sResult;
		mpcBlock->Branch(mpcFunction->Add(mpcBlock, IRO_ULt, sOffset, sIndex, mpcFunction->AddInteger(sOffset, 4 * iLongs)), pcBody, pcDone);
		StartBlock(pcBody);
		mpcFunction->Store(mpcBlock, IRType(IRTK_I32), mpcFunction->AddInteger(IRType(IRTK_I32), 0), mpcFunction->Add(mpcBlock, IRO_PtrAdd, IRType(IRTK_Pointer), sAddress, sIndex));
		sNext = mpcFunction->Add(mpcBlock, IRO_Add, sOffset, sIndex, mpcFunction->AddInteger(sOffset, 4));
		pcLatch = mpcBlock;
		JumpTo(pcLoop);
		pcLoop->AddPhiArgument(pcLoop->masPhis.Get(uiPhi), pcEntry, mpcFunction->AddInteger(sOffset, 0));
		pcLoop->AddPhiArgument(pcLoop->masPhis.Get(uiPhi), pcLatch, sNext);
		StartBlock(pcDone);
	}
	else
	{
		for (i = 0; i < iLongs; i++)
		{
			mpcFunction->Store(mpcBlock, IRType(IRTK_I32), mpcFunction->AddInteger(IRType(IRTK_I32), 0), AddressAt(sAddress, 4 * i));
		}
	}
	for (i = 4 * iLongs; i < iBytes; i++)
	{
		mpcFunction->Store(mpcBlock, IRType(IRTK_I8), mpcFunction->AddInteger(IRType(IRTK_I8), 0), AddressAt(sAddress, i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The implicit copy constructor of a class whose bases or fields have copy
// constructors, as the generator's CopyMembers: the bytes are copied (and
// the vtable pointers stored), then those bases and fields are copied again
// with their copy constructors.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CopyMembers(SCPP65IRPlace* psPlace, CCPPSyntax* pcSource)
{
	SCPP65IRPlace	sSource;
	SIRReference	sAddress;

	if (!Place(pcSource, &sSource))
	{
		return false;
	}
	if (!sSource.psType->IsClass() || !mpcGenerator->IsDerived(sSource.psType->psClass, psPlace->psType->psClass, NULL) ||
		!ReferenceAddress(&sSource, mpcTypes->Pointer(psPlace->psType), &sAddress))
	{
		return Unsupported(__LINE__);
	}
	CopyPieces(psPlace->sAddress, sAddress, psPlace->psType->iBytes);
	if (!StoreVtables(psPlace->sAddress, psPlace->psType->psClass))
	{
		return false;
	}
	return CopyParts(psPlace->psType->psClass, psPlace->sAddress, sAddress);
}


//////////////////////////////////////////////////////////////////////////
//
// Copies the bases and fields of the class (the objects at sDestination and
// sSource) that have copy constructors with them.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CopyParts(SCPP65Class* psClass, SIRReference sDestination, SIRReference sSource)
{
	SCPP65Base*		psBase;
	SCPP65Field*	psField;
	SCPP65Type*		psType;
	size			i;
	int				j;
	int				iCount;
	int				iOffset;

	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		psBase = psClass->macBases.Get(i);
		if (!CopyPart(psBase->psClass, AddressAt(sDestination, psBase->iOffset), AddressAt(sSource, psBase->iOffset)))
		{
			return false;
		}
	}
	for (i = 0; i < psClass->macFields.NumElements(); i++)
	{
		psField = psClass->macFields.Get(i);
		psType = psField->psType;
		iCount = 1;
		if (psType->IsArray())
		{
			iCount = psType->iCount;
			psType = psType->psBase;
		}
		if (!psType->IsClass())
		{
			continue;
		}
		for (j = 0; j < iCount; j++)
		{
			iOffset = psField->iOffset + j * psType->iBytes;
			if (!CopyPart(psType->psClass, AddressAt(sDestination, iOffset), AddressAt(sSource, iOffset)))
			{
				return false;
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// An inherited constructor's call of the base's constructor it stands for,
// with its own parameters (scalars and references only).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::InheritedConstructorCall(SCPP65IRPlace* psBase)
{
	SCPP65Type*		psType;
	SIRReference	sValue;
	SIRReference	sAddress;
	size			ui;
	int32			iIndex;

	//The IR parameters after this and the result's address.
	iIndex = (msThis.IsNone() ? 0 : 1) + (msResult.IsNone() ? 0 : 1);
	mpcFunction->Argument(mpcBlock, IRType(IRTK_Pointer), psBase->sAddress);
	for (ui = 0; ui < mpsFunction->mapsParameters.NumElements(); ui++)
	{
		psType = mpsFunction->mapsParameters.GetPtr(ui);
		if (psType->IsClass() || !IsSupportedType(psType) || psType->IsVoid() || psType->IsArray())
		{
			return Unsupported(__LINE__);
		}
		sAddress = mpcFunction->Add(mpcBlock, IRO_ArgAddress, IRType(IRTK_Pointer), mpcFunction->AddInteger(IRType(IRTK_I32), iIndex));
		sValue = mpcFunction->Load(mpcBlock, IRTypeOf(psType), sAddress);
		mpcFunction->Argument(mpcBlock, IRTypeOf(psType), sValue);
		iIndex++;
	}
	mpcFunction->Call(mpcBlock, IRType(IRTK_None), FunctionSymbol(mpsFunction->psInherited), -1, true);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A part (base or field) of a class: copied with its copy constructor (its
// parameters after the first given their defaults, scalars only), or its
// own parts, or left as its bytes.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CopyPart(SCPP65Class* psClass, SIRReference sDestination, SIRReference sSource)
{
	SCPP65Function*					psConstructor;
	SCPP65Type*						psParameter;
	CArrayTemplate<SCPP65IRValue>	asDefaults;
	SCPP65IRValue*					psDefault;
	size							ui;

	psConstructor = mpcGenerator->GetCopyConstructor(psClass);
	if (!psConstructor)
	{
		return mpcGenerator->IsTriviallyCopyable(psClass) || CopyParts(psClass, sDestination, sSource);
	}

	//The defaults are worked out before the arguments are given: one may be a call.
	asDefaults.Init();
	for (ui = 1; ui < psConstructor->mapsParameters.NumElements(); ui++)
	{
		psParameter = psConstructor->mapsParameters.GetPtr(ui);
		psDefault = asDefaults.Add();
		if (psParameter->IsClass() || psParameter->IsArray() || !IsSupportedType(psParameter))
		{
			asDefaults.Kill();
			return Unsupported(__LINE__);
		}
		if (!Value(psConstructor->mapcDefaults.GetPtr(ui), psDefault) || !Convert(psDefault, psParameter))
		{
			asDefaults.Kill();
			return false;
		}
	}
	mpcFunction->Argument(mpcBlock, IRType(IRTK_Pointer), sDestination);
	mpcFunction->Argument(mpcBlock, IRType(IRTK_Pointer), sSource);
	for (ui = 0; ui < asDefaults.NumElements(); ui++)
	{
		psDefault = asDefaults.Get(ui);
		mpcFunction->Argument(mpcBlock, IRTypeOf(psDefault->psType), psDefault->sValue);
	}
	asDefaults.Kill();
	mpcFunction->Call(mpcBlock, IRType(IRTK_None), FunctionSymbol(psConstructor), -1, true);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// object = source: the bytes (whatever the class), keeping the
// destination's vtable pointers.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CopyObject(SCPP65IRPlace* psPlace, CCPPSyntax* pcSource)
{
	SCPP65IRValue	sSource;

	return Value(pcSource, &sSource) && CopyBytes(psPlace, &sSource);
}


//////////////////////////////////////////////////////////////////////////
//
// Copies the object psSource is (its address, of the place's class or one
// derived from it, or a reference to one) to the place, a piece at a time,
// then stores the place's class's vtable pointers.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CopyBytes(SCPP65IRPlace* psPlace, SCPP65IRValue* psSource)
{
	SCPP65IRPlace	sSource;
	SIRReference	sAddress;

	sSource.sAddress = psSource->sValue;
	sSource.psType = psSource->psType->IsReference() ? psSource->psType->psBase : psSource->psType;
	if (!sSource.psType->IsClass() || !mpcGenerator->IsDerived(sSource.psType->psClass, psPlace->psType->psClass, NULL) ||
		!ReferenceAddress(&sSource, mpcTypes->Pointer(psPlace->psType), &sAddress))
	{
		return Unsupported(__LINE__);
	}
	CopyPieces(psPlace->sAddress, sAddress, psPlace->psType->iBytes);
	return StoreVtables(psPlace->sAddress, psPlace->psType->psClass);
}


//////////////////////////////////////////////////////////////////////////
//
// Copies iBytes from sSource to sDest, 4, 2 and 1 at a time.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::CopyPieces(SIRReference sDest, SIRReference sSource, int iBytes)
{
	SIRType	sPiece;
	int		iOffset;
	int		iPiece;

	for (iOffset = 0; iOffset < iBytes; iOffset += iPiece)
	{
		iPiece = PieceBytes(iBytes - iOffset);
		sPiece = IRIntegerType(iPiece);
		mpcFunction->Store(mpcBlock, sPiece, mpcFunction->Load(mpcBlock, sPiece, AddressAt(sSource, iOffset)), AddressAt(sDest, iOffset));
	}
}
