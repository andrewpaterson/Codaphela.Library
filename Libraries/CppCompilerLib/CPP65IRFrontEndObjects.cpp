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
#include "CPPUnit.h"
#include "CPP65IRFrontEnd.h"


//The IR front end's objects: constructors and destructors (with their vtables), new and delete, temporaries,
//objects as values and conditions, and copying them.


//////////////////////////////////////////////////////////////////////////
//
// c ? x : y of a class (CCPPUnit::ConditionalClass).  Two objects that are
// where they are (lvalues) give the address of one of them (a phi), a
// derived one's base's; otherwise the one worked out is made at psInto (the
// object being initialised) or a temporary, which both make.
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
	SCPP65Type*		psType;
	CCPPClass*		pcClass;
	bool			bLvalue;

	psTrue = mpcUnit->ValueTypeOf(pcConditional->mpcTrue);
	psFalse = mpcUnit->ValueTypeOf(pcConditional->mpcFalse);
	pcClass = (psTrue && psFalse) ? mpcUnit->ConditionalClass(psTrue, psFalse) : NULL;
	if (!pcClass)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	psType = (psTrue->pcClass == pcClass) ? psTrue : psFalse;

	bLvalue = !psInto && mpcUnit->IsObjectLvalue(pcConditional->mpcTrue) && mpcUnit->IsObjectLvalue(pcConditional->mpcFalse);
	if (!bLvalue)
	{
		if (psInto)
		{
			sPlace = *psInto;
		}
		else if (!Temporary(psType, &sPlace, true))
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
	if (bLvalue)
	{
		sTrue.sAddress = AddressAt(sTrue.sAddress, BaseOffset(psTrue->pcClass, pcClass));
	}
	pcTrueEnd = mpcBlock;
	JumpTo(pcEnd);
	StartBlock(pcFalse);
	if (bLvalue ? !Place(pcConditional->mpcFalse, &sFalse) : !Construct(&sPlace, NULL, pcConditional->mpcFalse))
	{
		return false;
	}
	if (bLvalue)
	{
		sFalse.sAddress = AddressAt(sFalse.sAddress, BaseOffset(psFalse->pcClass, pcClass));
	}
	miConditional--;
	JumpTo(pcEnd);
	psValue->psType = psType;
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
// An expression of pcClass, a class derived from it, or a reference to
// one: an object that is one of pcClass's as it is.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::IsObjectOf(CCPPSyntax* pcExpression, CCPPClass* pcClass)
{
	SCPP65Type*	psType;

	psType = mpcUnit->ValueTypeOf(pcExpression);
	return psType && psType->IsClass() && mpcUnit->IsDerived(psType->pcClass, pcClass, NULL);
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

	pcExpression = mpcUnit->SkipParentheses(pcExpression);
	switch (pcExpression->GetKind())
	{
	case CPPS_CallExpression:
	case CPPS_LambdaExpression:
		return true;
	case CPPS_LiteralExpression:
	case CPPS_StringExpression:
		return mpcUnit->UserLiteralToken(pcExpression) != NULL;
	case CPPS_BinaryExpression:
		pcBinary = (CCPPSyntaxBinaryExpression*)pcExpression;
		return !mpcUnit->IsAssignmentOperator(pcBinary->meOperator) && !mpcUnit->IsMemberPointerOperator(pcExpression) && ((pcBinary->meOperator != CT_Comma) || mpcUnit->IsOverloadedComma(pcBinary));
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
	CCPPBoundConversion	cBound;

	mpcUnit->GetBinder()->BindObjectConversion(psValue->psType->WithoutReference()->pcClass, psTo, true, NULL, &cBound);
	if (cBound.meHow != CPPBV_Function)
	{
		if (mpcValue)
		{
			mpcUnit->ReportError(mpcValue, "The object has no conversion to the type it is used as");
		}
		return Unsupported(__FILE__, __LINE__);
	}
	return ConvertObjectBy(psValue, &cBound, psTo);
}


//////////////////////////////////////////////////////////////////////////
//
// The object at psValue converted by the conversion function bound for it, then the
// result to psTo.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ConvertObjectBy(SCPP65IRValue* psValue, CCPPBoundConversion* pcBound, SCPP65Type* psTo)
{
	SCPP65IRValue	sResult;

	if (!Call(pcBound->mpcFunction, FunctionSymbol(pcBound->mpcFunction), NULL, &mapcNone, &sResult, NULL, false, pcBound->mpcClass, true, psValue->sValue))
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
	CCPPClass*		pcClass;
	CCPPFunction*	pcFunction;
	int				i;

	if (bArrow)
	{
		//An object's operator-> gives the pointer.
		psType = mpcUnit->ValueTypeOf(pcObject);
		if (psType && psType->IsClass())
		{
			if (!OperatorCall(pcObject, CT_MemberByPointer, NULL, &sPointer))
			{
				return false;
			}
			for (i = 0; sPointer.psType->IsClass() && (i < CPP_MAX_ARROWS); i++)
			{
				//An operator-> that returns an object (its address): its operator->, on it.  [over.ref] 1
				pcClass = sPointer.psType->pcClass;
				pcFunction = mpcUnit->GetBinder()->FindArrowOperator(pcClass, NULL, pcObject);
				if (!pcFunction || !Call(pcFunction, FunctionSymbol(pcFunction), NULL, &mapcNone, &sPointer, NULL, false, pcClass, false, sPointer.sValue))
				{
					return Unsupported(__FILE__, __LINE__);
				}
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

	if (!psResult || !mpcUnit->Constant(papcArguments->GetPtr(2), &lliBytes))
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
	papcInitialisers = pcFunction ? mpcUnit->ExpandArguments(&pcFunction->macpMemberInitializers) : &mapcNone;
	if (!papcInitialisers)
	{
		return false;
	}
	pcClass = mpcCPPFunction->mpcClass;
	if (msThis.IsNone() || !mpcUnit->CompleteClass(pcClass, NULL))
	{
		return Unsupported(__FILE__, __LINE__);
	}

	//A delegating constructor (C() : C(7) {}) has the other constructor make the whole object, the bases, vtable
	//pointers and fields included; its own member initialiser must be its only one ([class.base.init] 6).
	for (uiMember = 0; uiMember < papcInitialisers->NumElements(); uiMember++)
	{
		pcMember = (CCPPSyntaxMemberInitializer*)papcInitialisers->GetPtr(uiMember);
		pcNamed = (pcMember->mpcName->mpcSymbol && (pcMember->mpcName->mpcSymbol->meKind == CPPSK_Field)) ? NULL : mpcUnit->GetNameClass(pcMember->mpcName, pcMember);
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
			pcNamed = (pcMember->mpcName->mpcSymbol && (pcMember->mpcName->mpcSymbol->meKind == CPPSK_Field)) ? NULL : mpcUnit->GetNameClass(pcMember->mpcName, pcMember);
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
				papcArguments = mpcUnit->ListElements(pcFound->mpcInitializer);
			}
			else if (pcFound->mpcInitializer->GetKind() == CPPS_InitializerList)
			{
				papcArguments = mpcUnit->ListElements(pcFound->mpcInitializer);
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
			if ((pcMember->mpcName->mpcSymbol == psField->pcSymbol) || (!pcMember->mpcName->mpcSymbol && (mpcUnit->GetComponentName(pcMember->mpcName->GetLast()) == psField->lliName)))
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
		pcNamed = (pcMember->mpcName->mpcSymbol && (pcMember->mpcName->mpcSymbol->meKind == CPPSK_Field)) ? NULL : mpcUnit->GetNameClass(pcMember->mpcName, pcMember);
		if (!(pcNamed && mpcUnit->IsDerived(pcClass, pcNamed, NULL)) && !pcClass->FindField(mpcUnit->GetComponentName(pcMember->mpcName->GetLast()), &iOffset))
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
			papcArguments = mpcUnit->ListElements(pcMember->mpcInitializer);
		}
		else if (pcMember->mpcInitializer->GetKind() == CPPS_InitializerList)
		{
			papcArguments = mpcUnit->ListElements(pcMember->mpcInitializer);
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
// part of its vtable (CCPPVtable::GetEntries) is pointed at by the
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

	if (!pcClass->IsPolymorphic())
	{
		return true;
	}
	if (sObject.IsNone())
	{
		return Unsupported(__FILE__, __LINE__);
	}
	mpcUnit->EnsureVtable(pcClass);
	szLabel.Init();
	mpcUnit->GetMangler()->AppendVtableLabel(&szLabel, pcClass);
	sLabel = mcModule.Symbol(szLabel.Text());
	szLabel.Kill();
	if (pcClass->IsUnnamed())
	{
		mcModule.SetLocal(sLabel.Index());
	}
	asEntries.Init();
	pcClass->mcVtable.GetEntries(&asEntries);
	DescribeVtable(&asEntries, pcClass, sLabel.Index());

	//The words from the label: the first part's header is before it (its pointer only when the class has its own).
	if (pcClass->mcVtable.miPointerOffset != -1)
	{
		mpcFunction->Store(mpcBlock, IRType(IRTK_Pointer), sLabel, AddressAt(sObject, pcClass->mcVtable.miPointerOffset));
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
// the generator writes (pasEntries, CCPPVtable::GetEntries), so a
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
			mpcUnit->GetMangler()->AppendTypeInfoLabel(&szLabel, pcClass);
		}
		else
		{
			mpcUnit->GetMangler()->AppendVtableEntryLabel(&szLabel, psEntry->pcFunction, psEntry->iAdjust);
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
	CCPPClass*						pcClass;
	CCPPFunction*					pcConversion;
	CCPPFunction*					pcConstructor;
	CCPPSyntax*						pcList;
	SCPP65Type*						psElement;
	SCPP65IRValue					sMade;
	int								i;
	bool							bResult;
	bool							bValueInitialise;

	pcList = NULL;
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
			papcArguments = mpcUnit->ListElements(pcInitialiser);
			pcInitialiser = NULL;
			bValueInitialise = papcArguments->NumElements() == 0;
		}
		else if (pcInitialiser->GetKind() == CPPS_InitializerList)
		{
			papcArguments = mpcUnit->ListElements(pcInitialiser);
			pcList = pcInitialiser;
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
	if (!mpcUnit->CompleteClass(pcClass, NULL))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (pcList)
	{
		//A std::initializer_list is made from the list's elements; a class with an initializer-list constructor
		//that takes the list, by it.  [dcl.init.list] 3 and 5
		psElement = mpcUnit->InitializerListElement(psType);
		if (psElement)
		{
			return InitializerList(psPlace, psElement, papcArguments);
		}
		pcConstructor = mpcUnit->GetBinder()->InitializerListConstructor(pcClass, pcList);
		if (pcConstructor)
		{
			apcArguments.Init();
			apcArguments.Add(pcList);
			bResult = CallObject(pcConstructor, psPlace->sAddress, pcClass, &apcArguments, false);
			apcArguments.Kill();
			return bResult;
		}
	}

	//(object) and = object copy an object, when no copy constructor of the class's own is to be called.
	if (!pcInitialiser && (papcArguments->NumElements() == 1) && !pcClass->HasCopyConstructor())
	{
		psSource = mpcUnit->ValueTypeOf(papcArguments->GetPtr(0));
		if (psSource && psSource->IsClass() && mpcUnit->IsDerived(psSource->pcClass, pcClass, NULL))
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
			if ((mpcUnit->GetDeclarators()->GetTypeIdType(pcConstruct->mpcType) == psType) && pcConstruct->mpcInitializer && (pcConstruct->mpcInitializer->GetKind() == CPPS_ParenthesisedInitializer))
			{
				return ConstructClass(psPlace, pcClass, &((CCPPSyntaxParenthesisedInitializer*)pcConstruct->mpcInitializer)->macpElements);
			}
		}
		psSource = mpcUnit->TypeOf(pcInitialiser);
		if (psSource && (psSource == psType) && (mpcUnit->SkipParentheses(pcInitialiser)->GetKind() == CPPS_ConditionalExpression))
		{
			//= c ? x : y makes the object here from whichever is worked out.
			return ObjectConditional((CCPPSyntaxConditionalExpression*)mpcUnit->SkipParentheses(pcInitialiser), psPlace, &sMade);
		}
		if (psSource && psSource->IsClass() && (psSource->pcClass == pcClass) && IsObjectCall(pcInitialiser) && !(mpcUnit->DeclaredTypeOf(pcInitialiser)->IsReference() && pcClass->HasCopyConstructor()))
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
			if (!pcClass->IsTriviallyCopyable())
			{
				return Unsupported(__FILE__, __LINE__);
			}
			return CopyBytes(psPlace, &sMade);
		}
		psSource = psSource ? psSource->WithoutReference() : NULL;
		if (psSource && psSource->IsClass() && mpcUnit->IsDerived(psSource->pcClass, pcClass, NULL))
		{
			return CopyConstruct(psPlace, pcInitialiser);
		}
		pcConversion = mpcUnit->GetBinder()->ConversionToClass(pcClass, pcInitialiser, pcInitialiser);
		if (pcConversion)
		{
			return ConvertToObject(psPlace, pcConversion, pcInitialiser);
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
// = object of another class: its conversion function's result made at the
// place (as a call's that returns the class is), or the object a
// reference it returns refers to copied there.  [over.match.copy]
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ConvertToObject(SCPP65IRPlace* psPlace, CCPPFunction* pcConversion, CCPPSyntax* pcSource)
{
	SCPP65IRValue	sMade;
	CCPPClass*		pcClass;
	bool			bResult;

	pcClass = psPlace->psType->pcClass;
	msElide = psPlace->sAddress;
	bResult = Call(pcConversion, FunctionSymbol(pcConversion), NULL, &mapcNone, &sMade, pcSource, false, pcConversion->mpcClass, true, IRNone());
	msElide = IRNone();
	if (!bResult)
	{
		return false;
	}
	if (sMade.psType->IsClass() && sMade.sValue.Equals(psPlace->sAddress))
	{
		return true;
	}
	if (!pcClass->IsTriviallyCopyable())
	{
		return Unsupported(__FILE__, __LINE__);
	}
	return CopyBytes(psPlace, &sMade);
}


//////////////////////////////////////////////////////////////////////////
//
// A std::initializer_list<psElement> at psPlace from the elements of a list
// in braces: they are an array in the frame, which it points to with their
// count (or end).  [dcl.init.list] 5
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::InitializerList(SCPP65IRPlace* psPlace, SCPP65Type* psElement, CArraySyntax* papcElements)
{
	SCPP65IRPlace	sElement;
	SCPP65IRPlace	sField;
	SCPP65IRValue	sValue;
	SIRReference	sArray;
	SCPPField*		psFirst;
	SCPPField*		psSecond;
	size			uiCount;
	size			i;
	bool			bEnd;

	if (!mpcUnit->InitializerListLayout(psPlace->psType->pcClass, &bEnd))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	uiCount = papcElements->NumElements();
	sArray = mpcFunction->Slot(mpcBlock, IRType(IRTK_Pointer), (uint32)((uiCount > 0) ? uiCount * psElement->iBytes : 1), 4);
	sElement.psType = psElement->Unqualified();
	sElement.psBitField = NULL;
	for (i = 0; i < uiCount; i++)
	{
		sElement.sAddress = AddressAt(sArray, (int)(i * psElement->iBytes));
		if (sElement.psType->IsClass() ? !Construct(&sElement, NULL, papcElements->GetPtr(i)) : !LocalInitialiser(&sElement, papcElements->GetPtr(i), false))
		{
			return false;
		}
	}

	psFirst = psPlace->psType->pcClass->macFields.Get(0);
	psSecond = psPlace->psType->pcClass->macFields.Get(1);
	sField.psBitField = NULL;
	sField.sAddress = AddressAt(psPlace->sAddress, psFirst->iOffset);
	sField.psType = psFirst->psType;
	sValue.sValue = sArray;
	sValue.psType = psFirst->psType;
	Store(&sField, &sValue);
	sField.sAddress = AddressAt(psPlace->sAddress, psSecond->iOffset);
	sField.psType = psSecond->psType;
	sValue.sValue = bEnd ? AddressAt(sArray, (int)(uiCount * psElement->iBytes)) : Integer(psSecond->psType, (int64)uiCount);
	sValue.psType = psSecond->psType;
	Store(&sField, &sValue);
	return true;
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
	CCPPBoundConstruction	cBound;
	SCPPField*				psField;
	SCPP65IRPlace			sField;
	size					ui;
	size					uiArgument;

	papcArguments = mpcUnit->ExpandArguments(papcArguments);
	if (!papcArguments || !mpcUnit->GetBinder()->BindConstruction(pcClass, papcArguments, mpcCPPFunction->mpcDefinition, &cBound, false))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (cBound.meHow == CPPBO_Constructor)
	{
		return CallObject(cBound.mpcConstructor, psPlace->sAddress, pcClass, papcArguments, false);
	}
	if (cBound.meHow == CPPBO_Nothing)
	{
		return true;
	}

	//An aggregate: the fields in order, the rest by their default member initialisers, or zero.
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
		if (!Construct(&sField, NULL, (uiArgument < papcArguments->NumElements()) ? papcArguments->GetPtr(uiArgument) : psField->pcInitialiser))
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
// new Type, new Type(arguments), new Type{arguments} and new (placement...)
// Type: the allocation function's memory (Allocate), constructed.  new
// Type[count]... is NewArray's.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::NewExpression(CCPPSyntaxNewExpression* pcNew, SCPP65IRValue* psValue)
{
	SCPP65Type*		psType;
	SCPP65IRPlace	sPlace;
	SCPP65IRValue	sBytes;

	psType = mpcUnit->NewElementType(pcNew);
	if (!psType || !IsLocalType(psType) || psType->IsVoid() || psType->IsReference() || (psType->IsArray() && (pcNew->macpArraySizes.NumElements() == 0)))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (pcNew->macpArraySizes.NumElements() > 0)
	{
		return NewArray(pcNew, psType, psValue);
	}
	sBytes.psType = mpcTypes->Integer(4, false);
	sBytes.sValue = mpcFunction->AddInteger(IRType(IRTK_I32), psType->iBytes);
	if (!Allocate(pcNew, psType, &sBytes, &sPlace.sAddress))
	{
		return false;
	}
	sPlace.psType = psType;
	sPlace.psBitField = NULL;
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
// The memory a new expression makes its object in, psBytes long: from the
// allocation function its type and placement arguments choose
// (CCPPBinder::FindAllocation), the runtime's operator new or new[], or
// new (address) Type's address.  [expr.new] 9
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Allocate(CCPPSyntaxNewExpression* pcNew, SCPP65Type* psType, SCPP65IRValue* psBytes, SIRReference* psMemory)
{
	CArraySyntax	apcArguments;
	CCPPFunction*	pcAllocation;
	SCPP65IRValue	sResult;
	bool			bArray;
	bool			bFailed;
	bool			bResult;

	bArray = pcNew->macpArraySizes.NumElements() > 0;
	apcArguments.Init();
	mpcUnit->NewArguments(pcNew, &apcArguments);
	pcAllocation = mpcUnit->GetBinder()->FindAllocation(psType, bArray, &apcArguments, pcNew, &bFailed);
	bResult = !bFailed;
	if (bResult && pcAllocation)
	{
		bResult = CallWithValue(pcAllocation, &apcArguments, apcArguments.GetPtr(0), psBytes, &sResult);
		*psMemory = sResult.sValue;
	}
	else if (bResult && pcNew->mbPlacement)
	{
		//new (address) Type: the object is made at the address.
		bResult = Value(pcNew->macpPlacement.GetPtr(0), &sResult) && (sResult.psType->IsPointer() || Unsupported(__FILE__, __LINE__));
		*psMemory = sResult.sValue;
	}
	else if (bResult)
	{
		*psMemory = CallRuntime(bArray ? "opNewArray" : "opNew", IRType(IRTK_Pointer), IRTypeOf(psBytes->psType), psBytes->sValue);
	}
	apcArguments.Kill();
	return bResult || Unsupported(__FILE__, __LINE__);
}


//////////////////////////////////////////////////////////////////////////
//
// Calls a function that isn't a member's object's with papcArguments, the
// implicit one (pcImplicit, CCPPUnit::ImplicitValue) given the value
// psImplicit.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CallWithValue(CCPPFunction* pcFunction, CArraySyntax* papcArguments, CCPPSyntax* pcImplicit, SCPP65IRValue* psImplicit, SCPP65IRValue* psResult)
{
	bool	bResult;

	mcImplicitValues.Put((int64)(size)pcImplicit, psImplicit);
	bResult = Call(pcFunction, FunctionSymbol(pcFunction), NULL, papcArguments, psResult, NULL, false, NULL, false, IRNone());
	mcImplicitValues.Remove((int64)(size)pcImplicit);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// new Type[count]...: count elements from the allocation function, a class
// that needs destroying with the count (pointer size) in front of them.
// Each is default initialised, or value initialised by (), or from a list
// in braces the first of them and the rest value initialised.
// [expr.new] 17
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::NewArray(CCPPSyntaxNewExpression* pcNew, SCPP65Type* psType, SCPP65IRValue* psValue)
{
	SCPP65IRValue		sCount;
	SCPP65IRValue		sBytes;
	SCPP65IRPlace		sElement;
	CArraySyntax*		papcElements;
	SCPP65Type*			psElement;
	SIRType				sSize;
	SIRReference		sMemory;
	SIRReference		sIndex;
	SCPP65IRCountedLoop	sLoop;
	size				uiFirst;
	size				i;
	bool				bCookie;
	bool				bValue;

	for (psElement = psType; psElement->IsArray(); psElement = psElement->psBase)
	{
	}
	if (psElement->IsClass() && !mpcUnit->CompleteClass(psElement->pcClass, NULL))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (!Value(pcNew->macpArraySizes.GetPtr(0), &sCount) || !Convert(&sCount, mpcTypes->Integer(mcModule.GetPointerSize(0), false)))
	{
		return false;
	}
	sSize = IRTypeOf(sCount.psType);
	bCookie = psType->IsClass() && psType->pcClass->mpcDestructor;

	sBytes.psType = sCount.psType;
	sBytes.sValue = mpcFunction->Add(mpcBlock, IRO_Mul, sSize, sCount.sValue, mpcFunction->AddInteger(sSize, psType->iBytes));
	if (bCookie)
	{
		sBytes.sValue = mpcFunction->Add(mpcBlock, IRO_Add, sSize, sBytes.sValue, mpcFunction->AddInteger(sSize, mcModule.GetPointerSize(0)));
	}
	if (!Allocate(pcNew, psType, &sBytes, &sMemory))
	{
		return false;
	}
	if (bCookie)
	{
		mpcFunction->Store(mpcBlock, sSize, sCount.sValue, sMemory);
		sMemory = AddressAt(sMemory, mcModule.GetPointerSize(0));
	}

	//The elements a list in braces gives, then the rest.
	papcElements = pcNew->mpcInitializer ? mpcUnit->ListElements(pcNew->mpcInitializer) : NULL;
	bValue = pcNew->mpcInitializer != NULL;
	uiFirst = papcElements ? papcElements->NumElements() : 0;
	sElement.psType = psType;
	sElement.psBitField = NULL;
	for (i = 0; i < uiFirst; i++)
	{
		sElement.sAddress = AddressAt(sMemory, (int)(i * psType->iBytes));
		if (psType->IsClass() ? !Construct(&sElement, NULL, papcElements->GetPtr(i)) : !LocalInitialiser(&sElement, papcElements->GetPtr(i), false))
		{
			return false;
		}
	}
	if ((psType->IsClass() && psType->pcClass->NeedsConstructor()) || (bValue && !psType->IsClass()))
	{
		//loop: the index counts up to the count.
		sIndex = BeginCountedLoop(&sLoop, sSize, mpcFunction->AddInteger(sSize, uiFirst), IRO_ULt, sCount.sValue);
		sElement.sAddress = mpcFunction->Add(mpcBlock, IRO_PtrAdd, IRType(IRTK_Pointer), sMemory, mpcFunction->Add(mpcBlock, IRO_Mul, sSize, sIndex, mpcFunction->AddInteger(sSize, psType->iBytes)));
		if (psType->IsClass() ? !Construct(&sElement, NULL, NULL) : !ValueInitialise(&sElement))
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
// A scalar, or an array of them, value initialised: zero.  [dcl.init] 8
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ValueInitialise(SCPP65IRPlace* psPlace)
{
	SCPP65IRPlace	sElement;
	SCPP65IRValue	sZero;
	int				i;

	if (psPlace->psType->IsArray())
	{
		sElement.psType = psPlace->psType->psBase;
		sElement.psBitField = NULL;
		for (i = 0; i < psPlace->psType->iCount; i++)
		{
			sElement.sAddress = AddressAt(psPlace->sAddress, i * psPlace->psType->psBase->iBytes);
			if (!ValueInitialise(&sElement))
			{
				return false;
			}
		}
		return true;
	}
	if (!psPlace->psType->IsScalar())
	{
		return Unsupported(__FILE__, __LINE__);
	}
	sZero.psType = &mpcTypes->msInt;
	sZero.sValue = mpcFunction->AddInteger(IRTypeOf(&mpcTypes->msInt), 0);
	if (!Convert(&sZero, psPlace->psType))
	{
		return false;
	}
	Store(psPlace, &sZero);
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
// it is) then operator delete, the class's own if it has one.  delete[]
// of objects with destructors is DeleteArray's.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::DeleteExpression(CCPPSyntaxDeleteExpression* pcDelete, SCPP65IRValue* psValue)
{
	SCPP65IRValue	sPointer;
	SCPP65IRValue	sAddress;
	SCPP65IRValue	sResult;
	CCPPFunction*	pcDeallocation;
	CArraySyntax	apcArguments;
	bool			bCalled;
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
	if (psType->IsClass() && !mpcUnit->CompleteClass(psType->pcClass, NULL))
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
		pcDeallocation = psType->IsClass() ? mpcUnit->GetBinder()->FindDeallocation(psType->pcClass, pcDelete->mbArray) : NULL;
		if (pcDeallocation)
		{
			//The class's own operator delete.  [expr.delete] 9
			apcArguments.Init();
			apcArguments.Add(mpcUnit->ImplicitValue(pcDelete, mpcTypes->Pointer(&mpcTypes->msVoid)));
			sAddress.psType = mpcTypes->Pointer(&mpcTypes->msVoid);
			sAddress.sValue = sPointer.sValue;
			bCalled = CallWithValue(pcDeallocation, &apcArguments, apcArguments.GetPtr(0), &sAddress, &sResult);
			apcArguments.Kill();
			if (!bCalled)
			{
				return false;
			}
		}
		else
		{
			CallRuntime(pcDelete->mbArray ? "opDeleteArray" : "opDelete", IRType(IRTK_None), IRType(IRTK_Pointer), sPointer.sValue);
		}
	}
	JumpTo(pcDone);
	StartBlock(pcDone);
	psValue->sValue = IRNone();
	psValue->psType = &mpcTypes->msVoid;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Makes a lambda's closure at psPlace: each field from what it captures,
// where the lambda is: a copy (constructed as a variable is from its
// initialiser), the address of what it refers to, or this.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::LambdaObject(SCPP65IRPlace* psPlace, CCPPSyntaxLambdaExpression* pcSyntax)
{
	SCPPLambda*			psLambda;
	SCPPLambdaCapture*	psCapture;
	SCPPField*			psField;
	SCPP65IRPlace		sField;
	SCPP65IRPlace		sReferred;
	SIRReference		sAddress;
	size				i;

	psLambda = mpcUnit->GetLambda(pcSyntax);
	if (!psLambda)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	for (i = 0; i < psLambda->asCaptures.NumElements(); i++)
	{
		psCapture = psLambda->asCaptures.Get(i);
		psField = psLambda->pcClass->macFields.Get(psCapture->uiField);
		sField.sAddress = AddressAt(psPlace->sAddress, psField->iOffset);
		sField.psType = psField->psType;
		sField.psBitField = NULL;
		if (psCapture->bThis)
		{
			if (msThis.IsNone())
			{
				return Unsupported(__FILE__, __LINE__);
			}
			mpcFunction->Store(mpcBlock, IRType(IRTK_Pointer), msThis, sField.sAddress);
		}
		else if (psCapture->bReference)
		{
			if (!Place(psCapture->pcInitialiser, &sReferred) || !ReferenceAddress(&sReferred, psField->psType, &sAddress))
			{
				return false;
			}
			mpcFunction->Store(mpcBlock, IRType(IRTK_Pointer), sAddress, sField.sAddress);
		}
		else if (!Construct(&sField, NULL, psCapture->pcInitialiser))
		{
			return false;
		}
	}
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
	case CPPS_LambdaExpression:
		//Its closure, made where the object it initialises is (msElide), or as a temporary.
		psType = mpcUnit->TypeOf(pcExpression);
		if (!psType || !psType->IsClass())
		{
			return Unsupported(__FILE__, __LINE__);
		}
		if (!msElide.IsNone())
		{
			sPlace.sAddress = msElide;
			sPlace.psType = psType;
			msElide = IRNone();
		}
		else if (!Temporary(psType, &sPlace, true))
		{
			return false;
		}
		if (!LambdaObject(&sPlace, (CCPPSyntaxLambdaExpression*)pcExpression))
		{
			return false;
		}
		psValue->sValue = sPlace.sAddress;
		psValue->psType = psType;
		return true;
	case CPPS_IdExpression:
	case CPPS_MemberExpression:
	case CPPS_SubscriptExpression:
	case CPPS_ThisExpression:
	case CPPS_TypeidExpression:
		break;
	case CPPS_UnaryExpression:
		switch (((CCPPSyntaxUnaryExpression*)pcExpression)->meOperator)
		{
		case CT_Multiply:
			break;
		case CT_Increment:
		case CT_Decrement:
			if (((CCPPSyntaxUnaryExpression*)pcExpression)->mbPostfix)
			{
				//object++ and object--: the object's operator with an int 0.  [over.inc] 1
				return OperatorExpression(((CCPPSyntaxUnaryExpression*)pcExpression)->meOperator, ((CCPPSyntaxUnaryExpression*)pcExpression)->mpcOperand, mpcUnit->ImplicitZero(), psValue);
			}
		case CT_Subtract:
		case CT_Add:
		case CT_BitwiseNot:
			//-object, +object, ~object, ++object, --object: the object's operator.
			return OperatorExpression(((CCPPSyntaxUnaryExpression*)pcExpression)->meOperator, ((CCPPSyntaxUnaryExpression*)pcExpression)->mpcOperand, NULL, psValue);
		default:
			return Unsupported(__FILE__, __LINE__);
		}
		break;
	case CPPS_CallExpression:
		return CallExpression((CCPPSyntaxCallExpression*)pcExpression, psValue);
	case CPPS_LiteralExpression:
	case CPPS_StringExpression:
		//A user-defined literal that makes an object: its literal operator's call.
		return LiteralCall(pcExpression, psValue);
	case CPPS_ConditionalExpression:
		return ObjectConditional((CCPPSyntaxConditionalExpression*)pcExpression, NULL, psValue);
	case CPPS_BinaryExpression:
		if (!mpcUnit->IsAssignmentOperator(((CCPPSyntaxBinaryExpression*)pcExpression)->meOperator))
		{
			return OperatorExpression(((CCPPSyntaxBinaryExpression*)pcExpression)->meOperator, ((CCPPSyntaxBinaryExpression*)pcExpression)->mpcLeft, ((CCPPSyntaxBinaryExpression*)pcExpression)->mpcRight, psValue);
		}
		return Assignment((CCPPSyntaxBinaryExpression*)pcExpression, psValue);
	case CPPS_ConstructExpression:
		pcConstruct = (CCPPSyntaxConstructExpression*)pcExpression;
		psType = mpcUnit->GetDeclarators()->GetTypeIdType(pcConstruct->mpcType);
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
		if (IsReferenceCast(pcExpression))
		{
			//static_cast<Class&&>(object) is the object.
			break;
		}

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
			return Unsupported(__FILE__, __LINE__);
		}
		psType = mpcUnit->GetDeclarators()->GetTypeIdType(pcType);
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
// A temporary of psType holding pcValue, for a reference to bind to.  It
// is in the function's frame, so it outlasts the call; an object, made
// from the value by its class's constructor, is destroyed at the end of
// the full expression (or a local reference's scope, ExtendTemporary).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ValueTemporary(CCPPSyntax* pcValue, SCPP65Type* psType, SCPP65IRPlace* psPlace)
{
	SCPP65IRValue	sValue;

	psType = psType->Unqualified();
	if (psType->IsClass())
	{
		//Made as a variable is from its initialiser (const S& s = 3 and const S& s = {1, 2}).
		if (!Temporary(psType, psPlace, true))
		{
			return Unsupported(__FILE__, __LINE__);
		}
		return Construct(psPlace, NULL, pcValue);
	}
	if (!IsSupportedType(psType))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (!ValueFor(pcValue, psType, &sValue) || !ConvertImplicit(&sValue, psType, pcValue))
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

	if (!mpcUnit->CompleteClass(psType->pcClass, NULL))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	bDestructor = bDestroy && psType->pcClass->NeedsDestructor();
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
// derived from it): with a constructor the class declares for that (the
// one chosen for the source, so a move for what is going), or as the
// compiler's own does, its parts one by one (CopyMembers) or its bytes.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CopyConstruct(SCPP65IRPlace* psPlace, CCPPSyntax* pcSource)
{
	CCPPClass*	pcClass;
	CArraySyntax	apcArguments;
	bool			bResult;

	pcClass = psPlace->psType->pcClass;
	if (pcClass->HasCopyConstructor())
	{
		apcArguments.Init();
		apcArguments.Add(pcSource);
		bResult = ConstructClass(psPlace, pcClass, &apcArguments);
		apcArguments.Kill();
		return bResult;
	}
	if (pcClass->IsCopyDeleted())
	{
		return CopyDeleted(pcSource);
	}
	if (!pcClass->IsTriviallyCopyable())
	{
		return CopyMembers(psPlace, pcSource);
	}
	return CopyObject(psPlace, pcSource);
}


//////////////////////////////////////////////////////////////////////////
//
// The error that an object is copied whose class's copy the compiler
// may not write.  Always false.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CopyDeleted(CCPPSyntax* pcNode)
{
	mpcUnit->ReportError(pcNode, "The object cannot be copied: its class declares a move and no copy");
	return Unsupported(__FILE__, __LINE__);
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

	return Place(pcSource, &sSource) && CopyPlace(psPlace, &sSource, &sAddress) && CopyParts(psPlace->psType->pcClass, psPlace->sAddress, sAddress, mpcUnit->IsRvalue(pcSource));
}


//////////////////////////////////////////////////////////////////////////
//
// Copies the bases and fields of the class (the objects at sDestination and
// sSource) that have copy constructors with them.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CopyParts(CCPPClass* pcClass, SIRReference sDestination, SIRReference sSource, bool bMove)
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
		if (!CopyPart(psBase->pcClass, AddressAt(sDestination, psBase->iOffset), AddressAt(sSource, psBase->iOffset), bMove))
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
			if (!CopyPart(psType->pcClass, AddressAt(sDestination, iOffset), AddressAt(sSource, iOffset), bMove))
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
bool CCPP65IRFrontEnd::CopyPart(CCPPClass* pcClass, SIRReference sDestination, SIRReference sSource, bool bMove)
{
	CCPPFunction*					pcConstructor;
	SCPP65Type*						psParameter;
	CArrayTemplate<SCPP65IRValue>	asDefaults;
	SCPP65IRValue*					psDefault;
	size							ui;

	pcConstructor = pcClass->GetPartCopier(false, bMove);
	if (!pcConstructor)
	{
		if (pcClass->IsCopyDeleted())
		{
			return CopyDeleted(mpcValue);
		}
		return pcClass->IsTriviallyCopyable() || CopyParts(pcClass, sDestination, sSource, bMove);
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
// object = pcSource for a class with no operator= of its own whose parts
// have: as the compiler's own operator= does, each base and field is
// assigned in turn, with its class's operator= or its bytes.  What is
// going (bMove of AssignParts) is moved from where a part's class has a
// move assignment.  [class.copy] 28
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::AssignMembers(SCPP65IRPlace* psPlace, SCPP65IRValue* psSource, CCPPSyntax* pcSource)
{
	SCPP65IRPlace	sSource;
	SIRReference	sAddress;

	sSource.sAddress = psSource->sValue;
	sSource.psType = psSource->psType->WithoutReference();
	if (!sSource.psType->IsClass() || !mpcUnit->IsDerived(sSource.psType->pcClass, psPlace->psType->pcClass, NULL) || !ReferenceAddress(&sSource, mpcTypes->Pointer(psPlace->psType), &sAddress))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	return AssignParts(psPlace->psType->pcClass, psPlace->sAddress, sAddress, mpcUnit->IsRvalue(pcSource)) && StoreVtables(psPlace->sAddress, psPlace->psType->pcClass);
}


//////////////////////////////////////////////////////////////////////////
//
// Assigns the bases and the fields of the pcClass at sSource to those of
// the one at sDestination, in the order they are declared.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::AssignParts(CCPPClass* pcClass, SIRReference sDestination, SIRReference sSource, bool bMove)
{
	SCPPBase*	psBase;
	SCPPField*	psField;
	SCPP65Type*	psType;
	size		i;
	int			j;
	int			iOffset;

	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		psBase = pcClass->macBases.Get(i);
		if (!AssignPart(psBase->pcClass, AddressAt(sDestination, psBase->iOffset), AddressAt(sSource, psBase->iOffset), bMove))
		{
			return false;
		}
	}
	for (i = 0; i < pcClass->macFields.NumElements(); i++)
	{
		psField = pcClass->macFields.Get(i);
		psType = psField->psType->IsArray() ? psField->psType->psBase : psField->psType;
		if (!psType->IsClass())
		{
			CopyPieces(AddressAt(sDestination, psField->iOffset), AddressAt(sSource, psField->iOffset), psField->psType->iBytes);
			continue;
		}
		for (j = 0; j < (psField->psType->IsArray() ? psField->psType->iCount : 1); j++)
		{
			iOffset = psField->iOffset + j * psType->iBytes;
			if (!AssignPart(psType->pcClass, AddressAt(sDestination, iOffset), AddressAt(sSource, iOffset), bMove))
			{
				return false;
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Assigns the pcClass at sSource to the one at sDestination, a part of an
// object being assigned: with the class's operator= for its own objects,
// or as AssignParts, or its bytes.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::AssignPart(CCPPClass* pcClass, SIRReference sDestination, SIRReference sSource, bool bMove)
{
	CCPPFunction*	pcOperator;
	SCPP65Type*		psReturn;

	pcOperator = pcClass->GetPartCopier(true, bMove);
	if (!pcOperator)
	{
		if (pcClass->IsCopyDeleted())
		{
			return CopyDeleted(mpcValue);
		}
		if (!pcClass->IsTriviallyAssignable())
		{
			return AssignParts(pcClass, sDestination, sSource, bMove);
		}
		CopyPieces(sDestination, sSource, pcClass->mpsType->iBytes);
		return true;
	}

	psReturn = pcOperator->mpsReturn;
	if ((pcOperator->mapsParameters.NumElements() != 1) || psReturn->IsClass() || (!psReturn->IsVoid() && !psReturn->IsReference() && !IsSupportedType(psReturn)))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	mpcFunction->Argument(mpcBlock, IRType(IRTK_Pointer), sDestination);
	mpcFunction->Argument(mpcBlock, IRType(IRTK_Pointer), sSource);
	mpcFunction->Call(mpcBlock, psReturn->IsVoid() ? IRType(IRTK_None) : (psReturn->IsReference() ? IRType(IRTK_Pointer) : IRTypeOf(psReturn)), FunctionSymbol(pcOperator), -1, true);
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
	if (!psSource->psType->IsClass() || !mpcUnit->IsDerived(psSource->psType->pcClass, psPlace->psType->pcClass, NULL) ||
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
