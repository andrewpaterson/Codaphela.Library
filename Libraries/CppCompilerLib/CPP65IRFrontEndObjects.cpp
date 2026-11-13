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

	psTrue = mpcGenerator->ValueTypeOf(pcConditional->mpcTrue);
	psFalse = mpcGenerator->ValueTypeOf(pcConditional->mpcFalse);
	if (!psTrue || (psTrue != psFalse) || !psTrue->IsClass())
	{
		return Unsupported(__FILE__, __LINE__);
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

	psType = mpcGenerator->ValueTypeOf(pcExpression);
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
		return !mpcGenerator->IsAssignmentOperator(pcBinary->meOperator) && !mpcGenerator->IsMemberPointerOperator(pcExpression) && ((pcBinary->meOperator != CT_Comma) || mpcGenerator->IsOverloadedComma(pcBinary));
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

	psType = psValue->psType->WithoutReference();
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
	CCPPClass*	pcClass;
	CCPPFunction*	pcFunction;
	SCPP65IRValue	sResult;

	pcClass = psValue->psType->WithoutReference()->pcClass;
	pcFunction = mpcGenerator->FindConversion(pcClass, psTo, true, NULL);
	if (!pcFunction)
	{
		if (mpcValue)
		{
			mpcGenerator->ReportError(mpcValue, "The object has no conversion to the type it is used as");
		}
		return Unsupported(__FILE__, __LINE__);
	}
	if (!Call(pcFunction, FunctionSymbol(pcFunction), NULL, &mapcNone, &sResult, NULL, false, pcClass, true, psValue->sValue))
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
		psType = mpcGenerator->ValueTypeOf(pcObject);
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
			return Unsupported(__FILE__, __LINE__);
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
		return Unsupported(__FILE__, __LINE__);
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
		return Unsupported(__FILE__, __LINE__);
	}
	if (!Value(papcArguments->GetPtr(1), &sSource) || !Value(papcArguments->GetPtr(0), &sDest))
	{
		return false;
	}
	if (!sSource.psType->IsPointer() || !sDest.psType->IsPointer())
	{
		return Unsupported(__FILE__, __LINE__);
	}
	CopyPieces(sDest.sValue, sSource.sValue, (int)lliBytes);
	psValue->sValue = sDest.sValue;
	psValue->psType = psResult;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The start of a constructor: the bases
// (from the member initialiser list or their default constructors), the
// vtable pointers, then the fields from the member initialisers, the
// default member initialisers or their own constructors.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ConstructorStart(CCPPSyntaxFunctionDefinition* pcFunction)
{
	CCPPClass*					pcClass;
	CArraySyntax*					papcInitialisers;
	CCPPSyntaxMemberInitializer*	pcMember;
	CCPPSyntaxMemberInitializer*	pcFound;
	CArraySyntax*					papcArguments;
	SCPPField*					psField;
	SCPPBase*						psBase;
	SCPP65IRPlace					sPlace;
	CCPPSyntax*						pcInitialiser;
	CCPPClass*					pcNamed;
	size							ui;
	size							uiMember;
	int								iOffset;

	//An implicit constructor has no definition, so no member initialisers.
	papcInitialisers = pcFunction ? &pcFunction->macpMemberInitializers : &mapcNone;
	pcClass = mpcCPPFunction->mpcClass;
	if (msThis.IsNone() || !mpcGenerator->CompleteClass(pcClass, NULL))
	{
		return Unsupported(__FILE__, __LINE__);
	}

	//A delegating constructor (C() : C(7) {}) has the other constructor make the whole object, the bases, vtable
	//pointers and fields included; its own member initialiser must be its only one ([class.base.init] 6).
	for (uiMember = 0; uiMember < papcInitialisers->NumElements(); uiMember++)
	{
		pcMember = (CCPPSyntaxMemberInitializer*)papcInitialisers->GetPtr(uiMember);
		pcNamed = (pcMember->mpcName->mpcSymbol && (pcMember->mpcName->mpcSymbol->meKind == CPPSK_Field)) ? NULL : mpcGenerator->GetNameClass(pcMember->mpcName, pcMember);
		if (pcNamed == pcClass)
		{
			if (papcInitialisers->NumElements() != 1)
			{
				return Unsupported(__FILE__, __LINE__);
			}
			return DelegatedConstructor(pcClass, pcMember);
		}
	}

	//The bases, in order.
	for (ui = 0; ui < pcClass->macBases.NumElements(); ui++)
	{
		psBase = pcClass->macBases.Get(ui);
		pcFound = NULL;
		for (uiMember = 0; uiMember < papcInitialisers->NumElements(); uiMember++)
		{
			pcMember = (CCPPSyntaxMemberInitializer*)papcInitialisers->GetPtr(uiMember);
			pcNamed = (pcMember->mpcName->mpcSymbol && (pcMember->mpcName->mpcSymbol->meKind == CPPSK_Field)) ? NULL : mpcGenerator->GetNameClass(pcMember->mpcName, pcMember);
			if (pcNamed == psBase->pcClass)
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
		sPlace.psType = psBase->pcClass->mpsType;
		sPlace.psBitField = NULL;
		if (mpcCPPFunction->mpcInherited && (mpcCPPFunction->mpcInherited->mpcClass == psBase->pcClass))
		{
			//An inherited constructor (using Base::Base) passes its parameters to the base's.
			if (!InheritedConstructorCall(&sPlace))
			{
				return false;
			}
		}
		else if (!ConstructClass(&sPlace, psBase->pcClass, papcArguments))
		{
			return false;
		}
	}

	if (!StoreVtables(msThis, pcClass))
	{
		return false;
	}

	//The fields, in order.
	for (ui = 0; ui < pcClass->macFields.NumElements(); ui++)
	{
		psField = pcClass->macFields.Get(ui);
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
			sPlace.psBitField = psField->IsBitField() ? psField : NULL;
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
		pcNamed = (pcMember->mpcName->mpcSymbol && (pcMember->mpcName->mpcSymbol->meKind == CPPSK_Field)) ? NULL : mpcGenerator->GetNameClass(pcMember->mpcName, pcMember);
		if (!(pcNamed && mpcGenerator->IsDerived(pcClass, pcNamed, NULL)) && !mpcGenerator->FindField(pcClass, mpcGenerator->GetComponentName(pcMember->mpcName->GetLast()), &iOffset))
		{
			return Unsupported(__FILE__, __LINE__);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A delegating constructor's start: the constructor its member initialiser
// pcMember chooses from its arguments (C(7)) makes the object at this.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::DelegatedConstructor(CCPPClass* pcClass, CCPPSyntaxMemberInitializer* pcMember)
{
	CArraySyntax*	papcArguments;
	SCPP65IRPlace	sPlace;

	papcArguments = &mapcNone;
	if (pcMember->mpcInitializer)
	{
		if (pcMember->mpcInitializer->GetKind() == CPPS_ParenthesisedInitializer)
		{
			papcArguments = &((CCPPSyntaxParenthesisedInitializer*)pcMember->mpcInitializer)->macpElements;
		}
		else if (pcMember->mpcInitializer->GetKind() == CPPS_InitializerList)
		{
			papcArguments = &((CCPPSyntaxInitializerList*)pcMember->mpcInitializer)->macpElements;
		}
	}
	sPlace.sAddress = msThis;
	sPlace.psType = pcClass->mpsType;
	return ConstructClass(&sPlace, pcClass, papcArguments);
}


//////////////////////////////////////////////////////////////////////////
//
// The end of a destructor, after its body: the fields in reverse, then the
// bases in reverse.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::DestructorEnd(void)
{
	CCPPClass*	pcClass;
	SCPPField*	psField;
	SCPPBase*		psBase;
	size			ui;

	pcClass = mpcCPPFunction->mpcClass;
	for (ui = pcClass->macFields.NumElements(); ui > 0; ui--)
	{
		psField = pcClass->macFields.Get(ui - 1);
		if (!Destroy(AddressAt(msThis, psField->iOffset), psField->psType))
		{
			return false;
		}
	}
	for (ui = pcClass->macBases.NumElements(); ui > 0; ui--)
	{
		psBase = pcClass->macBases.Get(ui - 1);
		if (!Destroy(AddressAt(msThis, psBase->iOffset), psBase->pcClass->mpsType))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A polymorphic class's vtable pointers in the object at sObject: each
// part of its vtable (CCPP65Generator::VtableEntries) is pointed at by the
// vtable pointer its header says.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::StoreVtables(SIRReference sObject, CCPPClass* pcClass)
{
	CArrayCPPVtableEntries	asEntries;
	SCPPVtableEntry*			psEntry;
	CChars						szLabel;
	SIRReference				sLabel;
	size						ui;
	int							iWord;

	if (!mpcGenerator->IsPolymorphic(pcClass))
	{
		return true;
	}
	if (sObject.IsNone())
	{
		return Unsupported(__FILE__, __LINE__);
	}
	mpcGenerator->EnsureVtable(pcClass);
	szLabel.Init();
	mpcGenerator->AppendVtableLabel(&szLabel, pcClass);
	sLabel = mcModule.Symbol(szLabel.Text());
	szLabel.Kill();
	if (pcClass->mlliName < 0)
	{
		mcModule.SetLocal(sLabel.Index());
	}
	asEntries.Init();
	mpcGenerator->VtableEntries(pcClass, &asEntries);
	DescribeVtable(&asEntries, pcClass, sLabel.Index());

	//The words from the label: the first part's header is before it (its pointer only when the class has its own).
	if (pcClass->miVtableOffset != -1)
	{
		mpcFunction->Store(mpcBlock, IRType(IRTK_Pointer), sLabel, AddressAt(sObject, pcClass->miVtableOffset));
	}
	iWord = 0;
	for (ui = 1; ui < asEntries.NumElements(); ui++)
	{
		psEntry = asEntries.Get(ui);
		if (psEntry->eKind == CPPVE_Header)
		{
			iWord += CPP65_VTABLE_HEADER_ENTRIES;
			mpcFunction->Store(mpcBlock, IRType(IRTK_Pointer), AddressAt(sLabel, CPP65_VTABLE_ENTRY_BYTES * iWord), AddressAt(sObject, psEntry->iVtablePointer));
		}
		else
		{
			iWord++;
		}
	}
	asEntries.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The module's known data for a class's vtable from its label, the entries
// the generator writes (pasEntries, CCPP65Generator::VtableEntries), so a
// virtual call through a vtable pointer the optimiser can see stored
// becomes a call of the function (and may be inlined).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::DescribeVtable(CArrayCPPVtableEntries* pasEntries, CCPPClass* pcClass, uint32 uiSymbol)
{
	CIRGlobal*			pcVtable;
	SCPPVtableEntry*	psEntry;
	CChars				szLabel;
	size				ui;

	if (mcModule.FindConstantData(uiSymbol))
	{
		return;
	}
	pcVtable = mcModule.AddKnownData(mcModule.GetSymbolName(uiSymbol), CPP65_VTABLE_ENTRY_BYTES);
	for (ui = 1; ui < pasEntries->NumElements(); ui++)
	{
		psEntry = pasEntries->Get(ui);
		szLabel.Init();
		if (psEntry->eKind == CPPVE_Header)
		{
			pcVtable->AddValue(IRIntegerType(CPP65_VTABLE_ENTRY_BYTES), (uint64)(int64)(-psEntry->iVtablePointer));
			mpcGenerator->AppendTypeInfoLabel(&szLabel, pcClass);
		}
		else
		{
			mpcGenerator->AppendVtableEntryLabel(&szLabel, psEntry->pcFunction, psEntry->iAdjust);
		}
		pcVtable->AddSymbol(IRType(IRTK_Pointer), mcModule.AddSymbol(szLabel.Text()), 0);
		szLabel.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Initialises the object at a place: from
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
	CCPPClass*					pcClass;
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
			return Unsupported(__FILE__, __LINE__);
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
			return Unsupported(__FILE__, __LINE__);
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
	if (bValueInitialise && !ElementType(psType)->pcClass->mbConstructors)
	{
		ZeroBytes(psPlace->sAddress, psType->iBytes);
	}

	if (psType->IsArray())
	{
		if (pcInitialiser || (papcArguments->NumElements() > (size)psType->iCount))
		{
			return Unsupported(__FILE__, __LINE__);
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

	pcClass = psType->pcClass;
	if (!mpcGenerator->CompleteClass(pcClass, NULL))
	{
		return Unsupported(__FILE__, __LINE__);
	}

	//(object) and = object copy an object.
	if (!pcInitialiser && (papcArguments->NumElements() == 1) && !pcClass->mbConstructors)
	{
		psSource = mpcGenerator->ValueTypeOf(papcArguments->GetPtr(0));
		if (psSource && psSource->IsClass() && mpcGenerator->IsDerived(psSource->pcClass, pcClass, NULL))
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
				return ConstructClass(psPlace, pcClass, &((CCPPSyntaxParenthesisedInitializer*)pcConstruct->mpcInitializer)->macpElements);
			}
		}
		psSource = mpcGenerator->TypeOf(pcInitialiser);
		if (psSource && (psSource == psType) && (mpcGenerator->SkipParentheses(pcInitialiser)->GetKind() == CPPS_ConditionalExpression))
		{
			//= c ? x : y makes the object here from whichever is worked out.
			return ObjectConditional((CCPPSyntaxConditionalExpression*)mpcGenerator->SkipParentheses(pcInitialiser), psPlace, &sMade);
		}
		if (psSource && psSource->IsClass() && (psSource->pcClass == pcClass) && IsObjectCall(pcInitialiser))
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
			if (!mpcGenerator->IsTriviallyCopyable(pcClass))
			{
				return Unsupported(__FILE__, __LINE__);
			}
			return CopyBytes(psPlace, &sMade);
		}
		psSource = psSource ? psSource->WithoutReference() : NULL;
		if (psSource && psSource->IsClass() && mpcGenerator->IsDerived(psSource->pcClass, pcClass, NULL))
		{
			return CopyConstruct(psPlace, pcInitialiser);
		}
		apcArguments.Init();
		apcArguments.Add(pcInitialiser);
		bResult = ConstructClass(psPlace, pcClass, &apcArguments);
		apcArguments.Kill();
		return bResult;
	}
	return ConstructClass(psPlace, pcClass, papcArguments);
}


//////////////////////////////////////////////////////////////////////////
//
// Calls the constructor of pcClass that matches the arguments for the
// object at a place.  A class without constructors takes its fields from
// {arguments}, in order.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ConstructClass(SCPP65IRPlace* psPlace, CCPPClass* pcClass, CArraySyntax* papcArguments)
{
	CCPPFunction*	pcConstructor;
	SCPPField*	psField;
	SCPP65IRPlace	sField;
	size			ui;
	size			uiArgument;

	if (!mpcGenerator->CompleteClass(pcClass, NULL))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (pcClass->mbConstructors || (papcArguments->NumElements() == 0))
	{
		pcConstructor = mpcGenerator->FindFunction(pcClass, pcClass->mlliName, CPPFK_Constructor, papcArguments, false, mpcCPPFunction->mpcDefinition);
		if (pcConstructor)
		{
			return CallObject(pcConstructor, psPlace->sAddress, pcClass, papcArguments, false);
		}
		return !pcClass->mbConstructors || Unsupported(__FILE__, __LINE__);
	}

	//An aggregate: the fields in order, the rest zero.
	if ((pcClass->macBases.NumElements() > 0) || mpcGenerator->IsPolymorphic(pcClass) || (papcArguments->NumElements() > mpcGenerator->NumAggregateFields(pcClass)))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	uiArgument = 0;
	for (ui = 0; ui < pcClass->macFields.NumElements(); ui++)
	{
		psField = pcClass->macFields.Get(ui);
		if (psField->IsUnnamedBitField())
		{
			continue;
		}
		sField.sAddress = AddressAt(psPlace->sAddress, psField->iOffset);
		sField.psType = psField->psType;
		sField.psBitField = psField->IsBitField() ? psField : NULL;
		if (!Construct(&sField, NULL, (uiArgument < papcArguments->NumElements()) ? papcArguments->GetPtr(uiArgument) : NULL))
		{
			return false;
		}
		uiArgument++;
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
	if (!psType->IsClass() || !psType->pcClass->mpcDestructor)
	{
		return true;
	}
	return CallObject(psType->pcClass->mpcDestructor, sAddress, psType->pcClass, NULL, false);
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
		return Unsupported(__FILE__, __LINE__);
	}
	psType = mpcGenerator->GetTypeIdType(pcNew->mpcType);
	if (!psType || !IsLocalType(psType) || psType->IsVoid() || psType->IsArray() || psType->IsReference())
	{
		return Unsupported(__FILE__, __LINE__);
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
			return Unsupported(__FILE__, __LINE__);
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
// new Type[count]: count elements from operator
// new[], constructed by default first to last.  A class that needs
// destroying has the count (32 bits) in front of the elements.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::NewArray(CCPPSyntaxNewExpression* pcNew, SCPP65Type* psType, SCPP65IRValue* psValue)
{
	SCPP65IRValue		sCount;
	SCPP65IRPlace		sElement;
	SIRType				sSize;
	SIRReference		sBytes;
	SIRReference		sMemory;
	SIRReference		sIndex;
	SCPP65IRCountedLoop	sLoop;
	bool				bCookie;

	//Only new Type[count] and new Class[count]().
	if ((pcNew->macpArraySizes.NumElements() != 1) || (pcNew->mpcInitializer &&
		((pcNew->mpcInitializer->GetKind() != CPPS_ParenthesisedInitializer) || !psType->IsClass() ||
		(((CCPPSyntaxParenthesisedInitializer*)pcNew->mpcInitializer)->macpElements.NumElements() != 0))))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (psType->IsClass() && !mpcGenerator->CompleteClass(psType->pcClass, NULL))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (!Value(pcNew->macpArraySizes.GetPtr(0), &sCount) || !Convert(&sCount, mpcTypes->Integer(mcModule.GetPointerSize(0), false)))
	{
		return false;
	}
	sSize = IRTypeOf(sCount.psType);
	bCookie = psType->IsClass() && psType->pcClass->mpcDestructor;

	sBytes = mpcFunction->Add(mpcBlock, IRO_Mul, sSize, sCount.sValue, mpcFunction->AddInteger(sSize, psType->iBytes));
	if (bCookie)
	{
		sBytes = mpcFunction->Add(mpcBlock, IRO_Add, sSize, sBytes, mpcFunction->AddInteger(sSize, mcModule.GetPointerSize(0)));
	}
	sMemory = CallRuntime("opNewArray", IRType(IRTK_Pointer), sSize, sBytes);
	if (bCookie)
	{
		mpcFunction->Store(mpcBlock, sSize, sCount.sValue, sMemory);
		sMemory = AddressAt(sMemory, mcModule.GetPointerSize(0));
	}

	if (psType->IsClass() && mpcGenerator->NeedsConstructor(psType->pcClass))
	{
		//loop: the index counts up to the count.
		sIndex = BeginCountedLoop(&sLoop, sSize, mpcFunction->AddInteger(sSize, 0), IRO_ULt, sCount.sValue);
		sElement.psType = psType;
		sElement.sAddress = mpcFunction->Add(mpcBlock, IRO_PtrAdd, IRType(IRTK_Pointer), sMemory, mpcFunction->Add(mpcBlock, IRO_Mul, sSize, sIndex, mpcFunction->AddInteger(sSize, psType->iBytes)));
		if (!Construct(&sElement, NULL, NULL))
		{
			return false;
		}
		EndCountedLoop(&sLoop, mpcFunction->Add(mpcBlock, IRO_Add, sSize, sIndex, mpcFunction->AddInteger(sSize, 1)));
	}
	psValue->sValue = sMemory;
	psValue->psType = mpcTypes->Pointer(psType);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// delete[] of objects with a destructor: the count is
// in front of the elements, which are destroyed last first, and operator
// delete[] is given the memory from the count on.  psPointer isn't null.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::DeleteArray(SCPP65IRValue* psPointer, CCPPClass* pcClass)
{
	SIRType				sSize;
	SIRReference		sBase;
	SIRReference		sCount;
	SIRReference		sIndex;
	SIRReference		sNext;
	SIRReference		sElement;
	SCPP65IRCountedLoop	sLoop;

	sSize = IRIntegerType(mcModule.GetPointerSize(0));
	sBase = AddressAt(psPointer->sValue, -mcModule.GetPointerSize(0));
	sCount = mpcFunction->Load(mpcBlock, sSize, sBase);

	//loop: the index counts down from the count.
	sIndex = BeginCountedLoop(&sLoop, sSize, sCount, IRO_Ne, mpcFunction->AddInteger(sSize, 0));
	sNext = mpcFunction->Add(mpcBlock, IRO_Sub, sSize, sIndex, mpcFunction->AddInteger(sSize, 1));
	sElement = mpcFunction->Add(mpcBlock, IRO_PtrAdd, IRType(IRTK_Pointer), psPointer->sValue, mpcFunction->Add(mpcBlock, IRO_Mul, sSize, sNext, mpcFunction->AddInteger(sSize, pcClass->mpsType->iBytes)));
	if (!CallObject(pcClass->mpcDestructor, sElement, pcClass, NULL, false))
	{
		return false;
	}
	EndCountedLoop(&sLoop, sNext);
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
		return Unsupported(__FILE__, __LINE__);
	}
	psType = sPointer.psType->psBase;
	if (psType->IsClass() && !mpcGenerator->CompleteClass(psType->pcClass, NULL))
	{
		return Unsupported(__FILE__, __LINE__);
	}

	pcFree = NewBlock();
	pcDone = NewBlock();
	if (!Truth(&sPointer, &sCondition))
	{
		return false;
	}
	mpcBlock->Branch(sCondition, pcFree, pcDone);
	StartBlock(pcFree);
	if (pcDelete->mbArray && psType->IsClass() && psType->pcClass->mpcDestructor)
	{
		if (!DeleteArray(&sPointer, psType->pcClass))
		{
			return false;
		}
	}
	else
	{
		if (psType->IsClass() && psType->pcClass->mpcDestructor && !CallObject(psType->pcClass->mpcDestructor, sPointer.sValue, psType->pcClass, NULL, true))
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
			return Unsupported(__FILE__, __LINE__);
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
			return Unsupported(__FILE__, __LINE__);
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
			return Unsupported(__FILE__, __LINE__);
		}
		psType = mpcGenerator->GetTypeIdType(pcType);
		if (!psType || !psType->IsClass() || !Temporary(psType, &sPlace, true))
		{
			return Unsupported(__FILE__, __LINE__);
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
		return Unsupported(__FILE__, __LINE__);
	}
	if (!Place(pcExpression, &sPlace))
	{
		return false;
	}
	return Load(&sPlace, psValue);
}


//////////////////////////////////////////////////////////////////////////
//
// A temporary of psType (not a class) holding pcValue, for a reference to
// bind to.  It is in the function's frame, so it outlasts the call.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ValueTemporary(CCPPSyntax* pcValue, SCPP65Type* psType, SCPP65IRPlace* psPlace)
{
	SCPP65IRValue	sValue;

	psType = psType->Unqualified();
	if (!IsSupportedType(psType))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (!ValueFor(pcValue, psType, &sValue) || !Convert(&sValue, psType))
	{
		return false;
	}
	psPlace->psType = psType;
	psPlace->sAddress = mpcFunction->Slot(mpcBlock, IRType(IRTK_Pointer), (uint32)psType->iBytes, 4);
	Store(psPlace, &sValue);
	return true;
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

	if (!mpcGenerator->CompleteClass(psType->pcClass, NULL))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	bDestructor = bDestroy && mpcGenerator->NeedsDestructor(psType->pcClass);
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
// derived from it): with the class's copy
// constructor, its implicit one (CopyMembers), or its bytes.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CopyConstruct(SCPP65IRPlace* psPlace, CCPPSyntax* pcSource)
{
	CCPPClass*	pcClass;
	CArraySyntax	apcArguments;
	bool			bResult;

	pcClass = psPlace->psType->pcClass;
	if (mpcGenerator->HasCopyConstructor(pcClass))
	{
		apcArguments.Init();
		apcArguments.Add(pcSource);
		bResult = ConstructClass(psPlace, pcClass, &apcArguments);
		apcArguments.Kill();
		return bResult;
	}
	if (!mpcGenerator->IsTriviallyCopyable(pcClass))
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
	SIRType				sOffset;
	SIRReference		sIndex;
	SCPP65IRCountedLoop	sLoop;
	int					iLongs;
	int					i;

	iLongs = iBytes / IR65_ZERO_LONG_BYTES;
	if (iLongs > IR65_ZERO_LONGS)
	{
		//loop: the offset counts up in longs.
		sOffset = IRIntegerType(mcModule.GetPointerSize(0));
		sIndex = BeginCountedLoop(&sLoop, sOffset, mpcFunction->AddInteger(sOffset, 0), IRO_ULt, mpcFunction->AddInteger(sOffset, IR65_ZERO_LONG_BYTES * iLongs));
		mpcFunction->Store(mpcBlock, IRType(IRTK_I32), mpcFunction->AddInteger(IRType(IRTK_I32), 0), mpcFunction->Add(mpcBlock, IRO_PtrAdd, IRType(IRTK_Pointer), sAddress, sIndex));
		EndCountedLoop(&sLoop, mpcFunction->Add(mpcBlock, IRO_Add, sOffset, sIndex, mpcFunction->AddInteger(sOffset, IR65_ZERO_LONG_BYTES)));
	}
	else
	{
		for (i = 0; i < iLongs; i++)
		{
			mpcFunction->Store(mpcBlock, IRType(IRTK_I32), mpcFunction->AddInteger(IRType(IRTK_I32), 0), AddressAt(sAddress, IR65_ZERO_LONG_BYTES * i));
		}
	}
	for (i = IR65_ZERO_LONG_BYTES * iLongs; i < iBytes; i++)
	{
		mpcFunction->Store(mpcBlock, IRType(IRTK_I8), mpcFunction->AddInteger(IRType(IRTK_I8), 0), AddressAt(sAddress, i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The implicit copy constructor of a class whose bases or fields have copy
// constructors: the bytes are copied (and
// the vtable pointers stored), then those bases and fields are copied again
// with their copy constructors.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CopyMembers(SCPP65IRPlace* psPlace, CCPPSyntax* pcSource)
{
	SCPP65IRPlace	sSource;
	SIRReference	sAddress;

	return Place(pcSource, &sSource) && CopyPlace(psPlace, &sSource, &sAddress) && CopyParts(psPlace->psType->pcClass, psPlace->sAddress, sAddress);
}


//////////////////////////////////////////////////////////////////////////
//
// Copies the bases and fields of the class (the objects at sDestination and
// sSource) that have copy constructors with them.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CopyParts(CCPPClass* pcClass, SIRReference sDestination, SIRReference sSource)
{
	SCPPBase*		psBase;
	SCPPField*	psField;
	SCPP65Type*		psType;
	size			i;
	int				j;
	int				iCount;
	int				iOffset;

	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		psBase = pcClass->macBases.Get(i);
		if (!CopyPart(psBase->pcClass, AddressAt(sDestination, psBase->iOffset), AddressAt(sSource, psBase->iOffset)))
		{
			return false;
		}
	}
	for (i = 0; i < pcClass->macFields.NumElements(); i++)
	{
		psField = pcClass->macFields.Get(i);
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
			if (!CopyPart(psType->pcClass, AddressAt(sDestination, iOffset), AddressAt(sSource, iOffset)))
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
	for (ui = 0; ui < mpcCPPFunction->mapsParameters.NumElements(); ui++)
	{
		psType = mpcCPPFunction->mapsParameters.GetPtr(ui);
		if (psType->IsClass() || !IsSupportedType(psType) || psType->IsVoid() || psType->IsArray())
		{
			return Unsupported(__FILE__, __LINE__);
		}
		sAddress = mpcFunction->Add(mpcBlock, IRO_ArgAddress, IRType(IRTK_Pointer), mpcFunction->AddInteger(IRType(IRTK_I32), iIndex));
		sValue = mpcFunction->Load(mpcBlock, IRTypeOf(psType), sAddress);
		mpcFunction->Argument(mpcBlock, IRTypeOf(psType), sValue);
		iIndex++;
	}
	mpcFunction->Call(mpcBlock, IRType(IRTK_None), FunctionSymbol(mpcCPPFunction->mpcInherited), -1, true);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A part (base or field) of a class: copied with its copy constructor (its
// parameters after the first given their defaults, scalars only), or its
// own parts, or left as its bytes.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CopyPart(CCPPClass* pcClass, SIRReference sDestination, SIRReference sSource)
{
	CCPPFunction*					pcConstructor;
	SCPP65Type*						psParameter;
	CArrayTemplate<SCPP65IRValue>	asDefaults;
	SCPP65IRValue*					psDefault;
	size							ui;

	pcConstructor = mpcGenerator->GetCopyConstructor(pcClass);
	if (!pcConstructor)
	{
		return mpcGenerator->IsTriviallyCopyable(pcClass) || CopyParts(pcClass, sDestination, sSource);
	}

	//The defaults are worked out before the arguments are given: one may be a call.
	asDefaults.Init();
	for (ui = 1; ui < pcConstructor->mapsParameters.NumElements(); ui++)
	{
		psParameter = pcConstructor->mapsParameters.GetPtr(ui);
		psDefault = asDefaults.Add();
		if (psParameter->IsClass() || psParameter->IsArray() || !IsSupportedType(psParameter))
		{
			asDefaults.Kill();
			return Unsupported(__FILE__, __LINE__);
		}
		if (!Value(pcConstructor->mapcDefaults.GetPtr(ui), psDefault) || !Convert(psDefault, psParameter))
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
	mpcFunction->Call(mpcBlock, IRType(IRTK_None), FunctionSymbol(pcConstructor), -1, true);
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
// derived from it, or a reference to one) to the place (CopyPlace).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CopyBytes(SCPP65IRPlace* psPlace, SCPP65IRValue* psSource)
{
	SCPP65IRPlace	sSource;
	SIRReference	sAddress;

	sSource.sAddress = psSource->sValue;
	sSource.psType = psSource->psType->WithoutReference();
	return CopyPlace(psPlace, &sSource, &sAddress);
}


//////////////////////////////////////////////////////////////////////////
//
// Copies the object at psSource (of the place's class or one derived from
// it) to the place, a piece at a time, then stores the place's class's
// vtable pointers.  *psAddress is where it was copied from, as the place's
// class (its base part).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CopyPlace(SCPP65IRPlace* psPlace, SCPP65IRPlace* psSource, SIRReference* psAddress)
{
	if (!psSource->psType->IsClass() || !mpcGenerator->IsDerived(psSource->psType->pcClass, psPlace->psType->pcClass, NULL) ||
		!ReferenceAddress(psSource, mpcTypes->Pointer(psPlace->psType), psAddress))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	CopyPieces(psPlace->sAddress, *psAddress, psPlace->psType->iBytes);
	return StoreVtables(psPlace->sAddress, psPlace->psType->pcClass);
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
