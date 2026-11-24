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
// c ? x : y that is an object, of the class the type checker gives it.  Two
// objects that are where they are (lvalues) give the address of one of them
// (a phi), a derived one's base's; otherwise the one worked out is made at
// psInto (the object being initialised) or a temporary, which both make,
// from an object of its class or from what the class is made of.
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
	SCPP65IRMade	sTemporary;
	SCPP65Type*		psType;
	CCPPClass*		pcClass;
	bool			bLvalue;

	psTrue = mpcUnit->GetTypeChecker()->ValueTypeOf(pcConditional->mpcTrue);
	psFalse = mpcUnit->GetTypeChecker()->ValueTypeOf(pcConditional->mpcFalse);
	psType = mpcUnit->GetTypeChecker()->ValueTypeOf(pcConditional);
	if (!psTrue || !psFalse || !psType || !psType->IsClass())
	{
		return Unsupported(__FILE__, __LINE__);
	}
	pcClass = psType->pcClass;

	bLvalue = !psInto && mpcUnit->GetTypeChecker()->ConditionalClass(psTrue, psFalse) && mpcUnit->GetTypeChecker()->IsObjectLvalue(pcConditional->mpcTrue) && mpcUnit->GetTypeChecker()->IsObjectLvalue(pcConditional->mpcFalse);
	if (!bLvalue)
	{
		if (psInto)
		{
			sPlace = *psInto;
		}
		else if (!Temporary(psType, &sPlace, &sTemporary))
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
		sTrue.sAddress = BaseAddress(sTrue.sAddress, psTrue->pcClass, pcClass, false);
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
		sFalse.sAddress = BaseAddress(sFalse.sAddress, psFalse->pcClass, pcClass, false);
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
	if (!bLvalue && !psInto)
	{
		//Made on whichever way was taken.
		TemporaryMade(&sTemporary);
	}
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

	psType = mpcUnit->GetTypeChecker()->ValueTypeOf(pcExpression);
	return psType && psType->IsClass() && mpcUnit->GetClasses()->IsDerived(psType->pcClass, pcClass, NULL);
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

	pcExpression = CPPSkipParentheses(pcExpression);
	switch (pcExpression->GetKind())
	{
	case CPPS_CallExpression:
	case CPPS_LambdaExpression:
		return true;
	case CPPS_LiteralExpression:
	case CPPS_StringExpression:
		return mpcUnit->GetDesugarings()->UserLiteralToken(pcExpression) != NULL;
	case CPPS_BinaryExpression:
		pcBinary = (CCPPSyntaxBinaryExpression*)pcExpression;
		return !IsCPPAssignment(pcBinary->meOperator) && !pcExpression->IsMemberPointerOperator() && ((pcBinary->meOperator != CT_Comma) || mpcUnit->GetBinder()->IsOverloadedComma(pcBinary));
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
// result to psTo.  An explicit one is used only with bExplicit: by a cast
// or a condition, not where no cast asks (ConvertImplicit).
// [class.conv.fct] 2
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ConvertObject(SCPP65IRValue* psValue, SCPP65Type* psTo, bool bExplicit)
{
	CCPPBoundConversion	cBound;
	CCPPBoundConversion	cExplicit;
	CCPPClass*			pcClass;

	pcClass = psValue->psType->WithoutReference()->pcClass;
	mpcUnit->GetBinder()->BindObjectConversion(pcClass, psTo, bExplicit, NULL, &cBound);
	if (cBound.meHow != CPPBV_Function)
	{
		mpcUnit->GetBinder()->BindObjectConversion(pcClass, psTo, true, NULL, &cExplicit);
		if (mpcValue)
		{
			mpcDiagnostics->ReportError(mpcValue, (cExplicit.meHow == CPPBV_Function) ? "The object's conversion to the type it is used as is explicit: it needs a cast" : "The object has no conversion to the type it is used as");
		}
		return Unsupported(__FILE__, __LINE__);
	}
	return ConvertObjectBy(psValue, &cBound, psTo);
}


//////////////////////////////////////////////////////////////////////////
//
// The object at psValue converted by the conversion function bound for it, then the
// result (what it refers to, if it returns a reference) to psTo.
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
	return ReadReferred(psValue) && Convert(psValue, psTo);
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
		psType = mpcUnit->GetTypeChecker()->ValueTypeOf(pcObject);
		if (psType && psType->IsClass())
		{
			if (!OperatorCall(pcObject, CT_MemberByPointer, NULL, &sPointer))
			{
				return false;
			}
			ArrowResult(&sPointer);
			for (i = 0; sPointer.psType->IsClass() && (i < CPP_MAX_ARROWS); i++)
			{
				//An operator-> that returns an object (its address) or a reference to one: its operator->, on it.  [over.ref] 1
				pcClass = sPointer.psType->pcClass;
				pcFunction = mpcUnit->GetBinder()->FindArrowOperator(pcClass, NULL, pcObject);
				if (!pcFunction || !Call(pcFunction, FunctionSymbol(pcFunction), NULL, &mapcNone, &sPointer, NULL, false, pcClass, false, sPointer.sValue))
				{
					return Unsupported(__FILE__, __LINE__);
				}
				ArrowResult(&sPointer);
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
// What an operator-> returned: a reference to an object (its address) is
// that object, whose operator-> is applied next.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::ArrowResult(SCPP65IRValue* psResult)
{
	if (psResult->psType->IsReference() && psResult->psType->psBase->IsClass())
	{
		psResult->psType = psResult->psType->psBase;
	}
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

	if (!psResult || !mpcUnit->GetFolder()->Constant(papcArguments->GetPtr(2), &lliBytes))
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
// default member initialisers or their own constructors.  With exceptions
// each is a part once it is made, so that an exception from what comes
// after it, the body included, destroys it ([except.ctor] 2).  The parts
// stay for the rest of the function: no normal way out sees them.  Each
// member initialiser is a full expression, whose temporaries are destroyed
// at its end ([intro.execution] 10).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ConstructorStart(CCPPSyntaxFunctionDefinition* pcFunction)
{
	CCPPClass*					pcClass;
	CArraySyntax*					papcInitialisers;
	CCPPSyntaxMemberInitializer*	pcMember;
	CCPPSyntaxMemberInitializer*	pcFound;
	SCPPField*						psField;
	SCPPBase*						psBase;
	SCPPVirtualBase*				psVirtual;
	SCPP65IRPlace					sPlace;
	CCPPSyntax*						pcInitialiser;
	CCPPClass*					pcNamed;
	size							ui;
	size							uiMember;
	size							uiAt;
	size							uiTemporaries;
	int								iOffset;

	//An implicit constructor has no definition, so no member initialisers.
	papcInitialisers = pcFunction ? mpcUnit->GetInstantiator()->ExpandArguments(&pcFunction->macpMemberInitializers) : &mapcNone;
	if (!papcInitialisers)
	{
		return false;
	}
	pcClass = mpcCPPFunction->mpcClass;
	if (msThis.IsNone() || !mpcUnit->GetClasses()->CompleteClass(pcClass, NULL))
	{
		return Unsupported(__FILE__, __LINE__);
	}

	//A delegating constructor (C() : C(7) {}) has the other constructor make the whole object, the bases, vtable
	//pointers and fields included; its own member initialiser must be its only one ([class.base.init] 6).
	for (uiMember = 0; uiMember < papcInitialisers->NumElements(); uiMember++)
	{
		pcMember = (CCPPSyntaxMemberInitializer*)papcInitialisers->GetPtr(uiMember);
		pcNamed = (pcMember->mpcName->mpcSymbol && (pcMember->mpcName->mpcSymbol->meKind == CPPSK_Field)) ? NULL : mpcUnit->GetNames()->GetNameClass(pcMember->mpcName, pcMember);
		if (pcNamed == pcClass)
		{
			if (papcInitialisers->NumElements() != 1)
			{
				return Unsupported(__FILE__, __LINE__);
			}
			uiAt = mcCleanups.NumEntries();
			uiTemporaries = mcCleanups.NumTemporaries();
			if (!DelegatedConstructor(pcClass, pcMember))
			{
				return false;
			}

			//The object is made: an exception from the body destroys it.  [except.ctor] 2
			return MemberInitialised(uiAt, uiTemporaries, msThis, pcClass->mpsType, mbBaseVersion ? pcClass : NULL);
		}
	}

	//The virtual bases, direct or not, in their order, made by the complete object's constructor (not a base version)
	//from the member initialiser that names them, or by default.  [class.base.init] 10 and 13
	for (ui = 0; !mbBaseVersion && (ui < pcClass->macVirtualBases.NumElements()); ui++)
	{
		psVirtual = pcClass->macVirtualBases.Get(ui);
		uiAt = mcCleanups.NumEntries();
		uiTemporaries = mcCleanups.NumTemporaries();
		if (!ConstructBase(papcInitialisers, psVirtual->pcClass, AddressAt(msThis, psVirtual->iOffset)) ||
			!MemberInitialised(uiAt, uiTemporaries, AddressAt(msThis, psVirtual->iOffset), psVirtual->pcClass->mpsType, psVirtual->pcClass))
		{
			return false;
		}
	}

	//The non-virtual bases, in order.
	for (ui = 0; ui < pcClass->macBases.NumElements(); ui++)
	{
		psBase = pcClass->macBases.Get(ui);
		if (psBase->bVirtual)
		{
			continue;
		}
		uiAt = mcCleanups.NumEntries();
		uiTemporaries = mcCleanups.NumTemporaries();
		if (!ConstructBase(papcInitialisers, psBase->pcClass, AddressAt(msThis, psBase->iOffset)) ||
			!MemberInitialised(uiAt, uiTemporaries, AddressAt(msThis, psBase->iOffset), psBase->pcClass->mpsType, psBase->pcClass))
		{
			return false;
		}
	}

	if (!StoreVtables(msThis, pcClass, mbBaseVersion))
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
		if (pcInitialiser || ((psField->psType->IsClass() || psField->psType->IsArray()) && !pcClass->mbUnion))
		{
			//A union's constructor makes only the member it names, or the one with a default initialiser.
			//[class.base.init] 8
			sPlace.sAddress = AddressAt(msThis, psField->iOffset);
			sPlace.psType = psField->psType;
			sPlace.psBitField = psField->IsBitField() ? psField : NULL;
			uiAt = mcCleanups.NumEntries();
			uiTemporaries = mcCleanups.NumTemporaries();
			if (!Construct(&sPlace, NULL, pcInitialiser) || !MemberInitialised(uiAt, uiTemporaries, sPlace.sAddress, psField->psType, NULL))
			{
				return false;
			}
		}
	}

	//Member initialisers that name nothing are the generator's to report.
	for (uiMember = 0; uiMember < papcInitialisers->NumElements(); uiMember++)
	{
		pcMember = (CCPPSyntaxMemberInitializer*)papcInitialisers->GetPtr(uiMember);
		pcNamed = (pcMember->mpcName->mpcSymbol && (pcMember->mpcName->mpcSymbol->meKind == CPPSK_Field)) ? NULL : mpcUnit->GetNames()->GetNameClass(pcMember->mpcName, pcMember);
		if (!(pcNamed && mpcUnit->GetClasses()->IsDerived(pcClass, pcNamed, NULL)) && !pcClass->FindField(mpcUnit->GetComponentName(pcMember->mpcName->GetLast()), &iOffset))
		{
			return Unsupported(__FILE__, __LINE__);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A base of the class whose constructor is being written, at sAddress: from
// the member initialiser that names it (with an inherited constructor's
// parameters, for the base it inherits from), or by default.  One with
// virtual bases is made by its base version (mpcBaseObject).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ConstructBase(CArraySyntax* papcInitialisers, CCPPClass* pcBase, SIRReference sAddress)
{
	CCPPSyntaxMemberInitializer*	pcMember;
	CCPPSyntaxMemberInitializer*	pcFound;
	CArraySyntax*					papcArguments;
	SCPP65IRPlace					sPlace;
	CCPPClass*						pcNamed;
	size							uiMember;
	bool							bResult;

	pcFound = NULL;
	for (uiMember = 0; uiMember < papcInitialisers->NumElements(); uiMember++)
	{
		pcMember = (CCPPSyntaxMemberInitializer*)papcInitialisers->GetPtr(uiMember);
		pcNamed = (pcMember->mpcName->mpcSymbol && (pcMember->mpcName->mpcSymbol->meKind == CPPSK_Field)) ? NULL : mpcUnit->GetNames()->GetNameClass(pcMember->mpcName, pcMember);
		if (pcNamed == pcBase)
		{
			pcFound = pcMember;
		}
	}
	papcArguments = &mapcNone;
	if (pcFound && pcFound->mpcInitializer && ((pcFound->mpcInitializer->GetKind() == CPPS_ParenthesisedInitializer) || (pcFound->mpcInitializer->GetKind() == CPPS_InitializerList)))
	{
		papcArguments = mpcUnit->GetInstantiator()->ListElements(pcFound->mpcInitializer);
	}
	sPlace.sAddress = sAddress;
	sPlace.psType = pcBase->mpsType;
	sPlace.psBitField = NULL;
	mpcBaseObject = pcBase;
	if (mpcCPPFunction->mpcInherited && (mpcCPPFunction->mpcInherited->mpcClass == pcBase))
	{
		//An inherited constructor (using Base::Base) passes its parameters to the base's.
		bResult = InheritedConstructorCall(&sPlace);
	}
	else if ((papcArguments->NumElements() == 1) && !pcBase->HasVirtualBases())
	{
		//(B(b) of a B without a copy constructor of its own copies it, as Construct does.)
		bResult = Construct(&sPlace, papcArguments, NULL);
	}
	else
	{
		bResult = ConstructClass(&sPlace, pcBase, papcArguments);
	}
	mpcBaseObject = NULL;
	return bResult;
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
	bool			bResult;

	papcArguments = &mapcNone;
	if (pcMember->mpcInitializer)
	{
		if (pcMember->mpcInitializer->GetKind() == CPPS_ParenthesisedInitializer)
		{
			papcArguments = mpcUnit->GetInstantiator()->ListElements(pcMember->mpcInitializer);
		}
		else if (pcMember->mpcInitializer->GetKind() == CPPS_InitializerList)
		{
			papcArguments = mpcUnit->GetInstantiator()->ListElements(pcMember->mpcInitializer);
		}
	}
	sPlace.sAddress = msThis;
	sPlace.psType = pcClass->mpsType;
	mpcBaseObject = mbBaseVersion ? pcClass : NULL;
	bResult = ConstructClass(&sPlace, pcClass, papcArguments);
	mpcBaseObject = NULL;
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The end of a destructor, after its body: the fields in reverse, then the
// non-virtual bases in reverse, then (but in a base version) the virtual
// bases in reverse.  Each one's part (DestructorStart) is taken out before
// it is destroyed: an exception from its destructor destroys the rest.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::DestructorEnd(void)
{
	CCPPClass*			pcClass;
	SCPPField*			psField;
	SCPPBase*			psBase;
	SCPPVirtualBase*	psVirtual;
	size				ui;

	pcClass = mpcCPPFunction->mpcClass;

	//A union's destructor destroys none of its members: which one lives is the program's to know.  [class.union] 4
	for (ui = pcClass->mbUnion ? 0 : pcClass->macFields.NumElements(); ui > 0; ui--)
	{
		psField = pcClass->macFields.Get(ui - 1);
		TakeDestroyPart(psField->psType);
		if (!Destroy(AddressAt(msThis, psField->iOffset), psField->psType))
		{
			return false;
		}
	}
	for (ui = pcClass->macBases.NumElements(); ui > 0; ui--)
	{
		psBase = pcClass->macBases.Get(ui - 1);
		if (psBase->bVirtual)
		{
			continue;
		}
		TakeDestroyPart(psBase->pcClass->mpsType);
		if (!DestroyBase(AddressAt(msThis, psBase->iOffset), psBase->pcClass))
		{
			return false;
		}
	}
	for (ui = mbBaseVersion ? 0 : pcClass->macVirtualBases.NumElements(); ui > 0; ui--)
	{
		psVirtual = pcClass->macVirtualBases.Get(ui - 1);
		TakeDestroyPart(psVirtual->pcClass->mpsType);
		if (!DestroyBase(AddressAt(msThis, psVirtual->iOffset), psVirtual->pcClass))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Destroys a base subobject of the object whose destructor is being written,
// with its destructor's base version if it has one (mpcBaseObject).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::DestroyBase(SIRReference sAddress, CCPPClass* pcBase)
{
	bool	bResult;

	mpcBaseObject = pcBase;
	bResult = Destroy(sAddress, pcBase->mpsType);
	mpcBaseObject = NULL;
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A polymorphic class's vtable pointers in the object at sObject: each
// part of its vtable (CCPPVtable::GetEntries) is pointed at by the
// vtable pointer its header says.  bConstruction: the object is a base
// subobject whose constructor's or destructor's base version is being
// written, which stores the construction vtable its VTT entries give.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::StoreVtables(SIRReference sObject, CCPPClass* pcClass, bool bConstruction)
{
	CArrayCPPVtableEntries	asEntries;
	SCPPVtableEntry*			psEntry;
	CChars						szLabel;
	SIRReference				sLabel;
	SIRReference				sPointer;
	size						ui;
	int							iWord;
	bool						bLabelled;

	if (!pcClass->IsPolymorphic())
	{
		return true;
	}
	if (sObject.IsNone())
	{
		return Unsupported(__FILE__, __LINE__);
	}
	asEntries.Init();
	pcClass->mcVtable.GetEntries(&asEntries);
	if (bConstruction)
	{
		//A base version's: the construction vtable its VTT entries start with, of the same parts.
		sLabel = mpcFunction->Load(mpcBlock, IRType(IRTK_Pointer), msVTT);
	}
	else
	{
		mpcUnit->GetClassData()->EnsureVtable(pcClass);
		szLabel.Init();
		mpcUnit->GetMangler()->AppendVtableLabel(&szLabel, pcClass);
		sLabel = mcModule.Symbol(szLabel.Text());
		szLabel.Kill();
		if (pcClass->IsUnnamed())
		{
			mcModule.SetLocal(sLabel.Index());
		}
		DescribeVtable(&asEntries, pcClass, sLabel.Index());
	}

	//The words from the label: the first part's offsets and header are before it (its pointer only when the class has
	//its own, which a part in a virtual base is found through).  In a complete object every part is at a fixed place;
	//in a base subobject one in a virtual base is where the object's first part says the base is.
	if (pcClass->mcVtable.miPointerOffset != -1)
	{
		mpcFunction->Store(mpcBlock, IRType(IRTK_Pointer), sLabel, AddressAt(sObject, pcClass->mcVtable.miPointerOffset));
	}
	iWord = 0;
	bLabelled = false;
	for (ui = 0; ui < asEntries.NumElements(); ui++)
	{
		psEntry = asEntries.Get(ui);
		if (!bLabelled)
		{
			bLabelled = psEntry->eKind == CPPVE_Header;
		}
		else if (psEntry->eKind == CPPVE_Header)
		{
			iWord += CPP65_VTABLE_HEADER_ENTRIES;
			sPointer = (bConstruction && psEntry->pcVirtualBase) ? AddressAt(VirtualBaseAddress(sObject, pcClass, psEntry->pcVirtualBase), psEntry->iInVirtual) : AddressAt(sObject, psEntry->iVtablePointer);
			mpcFunction->Store(mpcBlock, IRType(IRTK_Pointer), AddressAt(sLabel, CPP65_VTABLE_ENTRY_BYTES * iWord), sPointer);
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
// The VTT entries of the class whose constructor or destructor is being
// written: a base version's parameter, or the complete object's VTT
// (CCPPUnit::EnsureVTT).
//
//////////////////////////////////////////////////////////////////////////
SIRReference CCPP65IRFrontEnd::VTTOf(CCPPClass* pcClass)
{
	return mbBaseVersion ? msVTT : CompleteVTT(pcClass);
}


//////////////////////////////////////////////////////////////////////////
//
// The VTT of a complete object of the class (_Class__4, CCPPUnit::
// EnsureVTT).
//
//////////////////////////////////////////////////////////////////////////
SIRReference CCPP65IRFrontEnd::CompleteVTT(CCPPClass* pcClass)
{
	CChars			szLabel;
	SIRReference	sVTT;

	mpcUnit->GetClassData()->EnsureVTT(pcClass);
	szLabel.Init();
	mpcUnit->GetMangler()->AppendVTTLabel(&szLabel, pcClass);
	sVTT = mcModule.Symbol(szLabel.Text());
	szLabel.Kill();
	if (pcClass->IsUnnamed())
	{
		mcModule.SetLocal(sVTT.Index());
	}
	return sVTT;
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
	bool				bLabelled;

	if (mcModule.FindConstantData(uiSymbol))
	{
		return;
	}
	pcVtable = mcModule.AddKnownData(mcModule.GetSymbolName(uiSymbol), CPP65_VTABLE_ENTRY_BYTES);
	bLabelled = false;
	for (ui = 0; ui < pasEntries->NumElements(); ui++)
	{
		psEntry = pasEntries->Get(ui);
		if (!bLabelled)
		{
			//The first part's offsets and header are before the label.
			bLabelled = psEntry->eKind == CPPVE_Header;
			continue;
		}
		if (psEntry->eKind == CPPVE_Offset)
		{
			pcVtable->AddValue(IRIntegerType(CPP65_VTABLE_ENTRY_BYTES), (uint64)(int64)psEntry->iAdjust);
			continue;
		}
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
	size							uiMark;
	bool							bResult;
	bool							bValueInitialise;

	pcList = NULL;
	psType = psPlace->psType;
	pcInitialiser = mpcUnit->GetDesugarings()->BracedFor(psType, pcInitialiser);
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
			papcArguments = mpcUnit->GetInstantiator()->ListElements(pcInitialiser);
			pcInitialiser = NULL;
			bValueInitialise = papcArguments->NumElements() == 0;
		}
		else if (pcInitialiser->GetKind() == CPPS_InitializerList)
		{
			papcArguments = mpcUnit->GetInstantiator()->ListElements(pcInitialiser);
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
		//Each element made is a part until the array is: an exception destroys those made.  [except.ctor] 2
		sElement.psType = psType->psBase;
		uiMark = mcCleanups.NumEntries();
		for (i = 0; i < psType->iCount; i++)
		{
			sElement.sAddress = AddressAt(psPlace->sAddress, i * psType->psBase->iBytes);
			if (!Construct(&sElement, NULL, ((size)i < papcArguments->NumElements()) ? papcArguments->GetPtr(i) : NULL))
			{
				return false;
			}
			PushDestroyPart(sElement.sAddress, sElement.psType, NULL);
		}
		mcCleanups.RemovePartsFrom(uiMark);
		return true;
	}

	pcClass = psType->pcClass;
	if (!mpcUnit->GetClasses()->CompleteClass(pcClass, NULL))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (pcList)
	{
		//A std::initializer_list is made from the list's elements; a class with an initializer-list constructor
		//that takes the list, by it.  [dcl.init.list] 3 and 5
		psElement = mpcUnit->GetDesugarings()->InitializerListElement(psType);
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
	if (!pcInitialiser && (papcArguments->NumElements() == 1) && !mpcUnit->GetBinder()->CopiesByConstructor(pcClass, papcArguments->GetPtr(0)))
	{
		psSource = mpcUnit->GetTypeChecker()->ValueTypeOf(papcArguments->GetPtr(0));
		if (psSource && psSource->IsClass() && mpcUnit->GetClasses()->IsDerived(psSource->pcClass, pcClass, NULL))
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
		psSource = mpcUnit->GetTypeChecker()->TypeOf(pcInitialiser);
		if (psSource && (psSource == psType) && (CPPSkipParentheses(pcInitialiser)->GetKind() == CPPS_ConditionalExpression))
		{
			//= c ? x : y makes the object here from whichever is worked out.
			return ObjectConditional((CCPPSyntaxConditionalExpression*)CPPSkipParentheses(pcInitialiser), psPlace, &sMade);
		}
		if (psSource && psSource->IsClass() && (psSource->pcClass == pcClass) && IsObjectCall(pcInitialiser) && !(mpcUnit->GetTypeChecker()->DeclaredTypeOf(pcInitialiser)->IsReference() && pcClass->HasCopyConstructor()))
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
		if (psSource && psSource->IsClass() && mpcUnit->GetClasses()->IsDerived(psSource->pcClass, pcClass, NULL))
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
		bResult = mpcUnit->GetBinder()->ConstructsWithoutBraces(pcClass, &apcArguments, pcInitialiser) && ConstructClass(psPlace, pcClass, &apcArguments);
		apcArguments.Kill();
		return bResult;
	}
	if (!pcList && !mpcUnit->GetBinder()->ConstructsWithoutBraces(pcClass, papcArguments, mpcValue))
	{
		return false;
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
// count (or end).  [dcl.init.list] 5  An exception from an element
// destroys those made before it.  Objects that need destroying are then
// living ones of the local list being declared (mbListLocal), whose scope
// they end with, or temporaries of the full expression: the array lives as
// long as the list.  [dcl.init.list] 6, [class.temporary] 5
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
	size			uiMark;
	size			i;
	bool			bEnd;
	bool			bListLocal;

	if (!mpcUnit->GetDesugarings()->InitializerListLayout(psPlace->psType->pcClass, &bEnd))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	bListLocal = mbListLocal;
	mbListLocal = false;
	uiCount = papcElements->NumElements();
	if (NeedsDestroying(psElement) && !bListLocal && (miConditional > 0))
	{
		//A temporary made where it may not be would need a flag for each element.
		return Unsupported(__FILE__, __LINE__);
	}
	sArray = mpcFunction->Slot(mpcBlock, IRType(IRTK_Pointer), (uint32)((uiCount > 0) ? uiCount * psElement->iBytes : 1), 4);
	sElement.psType = psElement->Unqualified();
	sElement.psBitField = NULL;
	uiMark = mcCleanups.NumEntries();
	for (i = 0; i < uiCount; i++)
	{
		sElement.sAddress = AddressAt(sArray, (int)(i * psElement->iBytes));
		if (sElement.psType->IsClass() ? !Construct(&sElement, NULL, papcElements->GetPtr(i)) : !LocalInitialiser(&sElement, papcElements->GetPtr(i), false))
		{
			return false;
		}
		PushDestroyPart(sElement.sAddress, sElement.psType, NULL);
	}
	mcCleanups.RemovePartsFrom(uiMark);
	for (i = 0; NeedsDestroying(sElement.psType) && (i < uiCount); i++)
	{
		sElement.sAddress = AddressAt(sArray, (int)(i * psElement->iBytes));
		if (bListLocal)
		{
			AddLiving(&sElement);
		}
		else
		{
			mcCleanups.PushTemporary(&sElement, false, IRNone());
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
	CCPPSyntax*				pcValue;
	SIRReference			sThis;
	size					ui;
	size					uiErrors;
	bool					bResult;

	papcArguments = mpcUnit->GetInstantiator()->ExpandArguments(papcArguments);
	if (!papcArguments || !mpcUnit->GetBinder()->BindConstruction(pcClass, papcArguments, mpcCPPFunction->mpcDefinition, &cBound, false))
	{
		//Why it can't be made is said where the type checker is asked: of an expression, and of a variable's
		//declaration (CCPPUnit::CheckDeclared).  It isn't asked of a base or a member its constructor gives
		//nothing, so that is said here.
		uiErrors = mpcDiagnostics->GetErrors()->NumErrors();
		if (papcArguments)
		{
			mpcDiagnostics->BeginLoud();
			mpcUnit->GetBinder()->BindConstruction(pcClass, papcArguments, mpcCPPFunction->mpcDefinition, &cBound, true);
			mpcDiagnostics->EndLoud();
		}
		return (mpcDiagnostics->GetErrors()->NumErrors() > uiErrors) ? false : Unsupported(__FILE__, __LINE__);
	}
	if (cBound.meHow == CPPBO_Constructor)
	{
		return CallObject(cBound.mpcConstructor, psPlace->sAddress, pcClass, papcArguments, false);
	}
	if (cBound.meHow == CPPBO_Nothing)
	{
		return true;
	}

	//An aggregate: the fields its list fills (CCPPClass::AggregateField), in order, the rest by their default member
	//initialisers, or zero.  [dcl.init.aggr] 7, 16
	for (ui = 0; (psField = pcClass->AggregateField(ui)) != NULL; ui++)
	{
		sField.sAddress = AddressAt(psPlace->sAddress, psField->iOffset);
		sField.psType = psField->psType;
		sField.psBitField = psField->IsBitField() ? psField : NULL;
		pcValue = (ui < papcArguments->NumElements()) ? papcArguments->GetPtr(ui) : NULL;
		if (pcValue && (psField->psType->IsInteger() || psField->psType->IsFloat()) && (pcValue->GetKind() != CPPS_InitializerList) && !mpcUnit->GetTypeChecker()->CheckBracedValue(pcValue, psField->psType))
		{
			//A list's value isn't narrowed to its field.  [dcl.init.aggr] 12, [dcl.init.list] 7
			return false;
		}
		if (pcValue || !psField->pcInitialiser)
		{
			bResult = Construct(&sField, NULL, pcValue);
		}
		else
		{
			//A default member initialiser is written in the class: its this is the object being filled.
			sThis = msThis;
			msThis = psPlace->sAddress;
			mpcUnit->GetContext()->EnterObject(pcClass);
			bResult = Construct(&sField, NULL, psField->pcInitialiser);
			mpcUnit->GetContext()->LeaveObject();
			msThis = sThis;
		}
		if (!bResult)
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
	SCPP65IRCleanup	sFree;
	CCPPFunction*	pcAllocation;
	CIRBlock*		pcNull;
	CIRBlock*		pcMake;
	CIRBlock*		pcEnd;
	size			uiMark;

	psType = mpcUnit->GetTypeChecker()->NewElementType(pcNew);
	if (!psType || !IsLocalType(psType) || psType->IsVoid() || psType->IsReference())
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (mpcUnit->GetTypeChecker()->IsArrayNew(pcNew))
	{
		return NewArray(pcNew, psType, psValue);
	}
	//The size is a std::size_t ([expr.new] 10).
	sBytes = ByteCount(psType->iBytes);
	if (!Allocate(pcNew, psType, &sBytes, &sPlace.sAddress, &sFree, &pcAllocation))
	{
		return false;
	}
	sPlace.psType = psType;
	sPlace.psBitField = NULL;
	pcNull = NULL;
	pcEnd = NULL;
	if (IsNullable(pcAllocation) && (psType->IsClass() || pcNew->mpcInitializer))
	{
		//NULL: nothing is made.  [expr.new] 13
		pcNull = mpcBlock;
		pcMake = NewBlock();
		pcEnd = NewBlock();
		mpcBlock->Branch(mpcFunction->Add(mpcBlock, IRO_Ne, IRType(IRTK_Pointer), sPlace.sAddress, mpcFunction->AddConstant(IRType(IRTK_Pointer), 0)), pcMake, pcEnd);
		StartBlock(pcMake);
	}
	uiMark = mcCleanups.NumEntries();
	if (PartsWanted() && psType->IsClass() && (sFree.pcFunction || sFree.szRuntime))
	{
		sFree.sPlace.sAddress = sPlace.sAddress;
		mcCleanups.PushPart(&sFree);
	}
	if ((psType->IsClass() || pcNew->mpcInitializer) && !Construct(&sPlace, NULL, pcNew->mpcInitializer))
	{
		return false;
	}
	mcCleanups.RemovePartsFrom(uiMark);
	if (pcNull)
	{
		pcMake = mpcBlock;
		JumpTo(pcEnd);
		StartBlock(pcEnd);
		sPlace.sAddress = Phi(pcEnd, IRType(IRTK_Pointer), pcNull, mpcFunction->AddConstant(IRType(IRTK_Pointer), 0), pcMake, sPlace.sAddress);
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
// new (address) Type's address.  [expr.new] 9  psFree (if it is wanted)
// says how the memory is given back if the object's construction throws
// (Deallocation); *ppcAllocation (if it is wanted) is the allocation
// function, NULL for the runtime's or the address.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Allocate(CCPPSyntaxNewExpression* pcNew, SCPP65Type* psType, SCPP65IRValue* psBytes, SIRReference* psMemory, SCPP65IRCleanup* psFree, CCPPFunction** ppcAllocation)
{
	CArraySyntax	apcArguments;
	CCPPFunction*	pcAllocation;
	SCPP65IRValue	sResult;
	bool			bArray;
	bool			bFailed;
	bool			bResult;

	bArray = mpcUnit->GetTypeChecker()->IsArrayNew(pcNew);
	apcArguments.Init();
	mpcUnit->GetTypeChecker()->NewArguments(pcNew, &apcArguments);
	pcAllocation = mpcUnit->GetBinder()->FindAllocation(pcNew, psType, &apcArguments, &bFailed);
	bResult = !bFailed;
	if (ppcAllocation)
	{
		*ppcAllocation = pcAllocation;
	}
	if (bResult && psFree)
	{
		if (PartsWanted())
		{
			bResult = Deallocation(pcNew, pcAllocation, psType, &apcArguments, psBytes, psFree);
		}
		else
		{
			psFree->Clear(CPP65IRC_Part);
		}
	}
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
		//The global operator new or new[] (the program's, or Libc65's, which throws std::bad_alloc).
		*psMemory = CallRuntime(bArray ? "opNewArray" : "opNew", IRType(IRTK_Pointer), IRTypeOf(psBytes->psType), psBytes->sValue);
		TestThrown(NULL, mcModule.Symbol(bArray ? "_opNewArray" : "_opNew"));
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
// The bytes asked for are a std::size_t.  When count elements, and the
// count in front of them, are more than one holds (the W65C816's is 16
// bits and the count 32), no allocation function is called, nothing is
// made and the pointer is null: the bytes cut to a size_t would be given
// less memory than is then used ([expr.new] 7).  With exceptions on that
// is where std::bad_array_new_length is thrown (the user, 2026-11-19:
// null, and "It should throw if exceptions are enabled"; ExceptionsPlan.txt
// stage 8).  An allocation function declared noexcept that gives NULL
// makes nothing, and the result is NULL.  [expr.new] 13
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::NewArray(CCPPSyntaxNewExpression* pcNew, SCPP65Type* psType, SCPP65IRValue* psValue)
{
	SCPP65IRValue		sCount;
	SCPP65IRValue		sBytes;
	SCPP65IRPlace		sElement;
	CArraySyntax*		papcElements;
	CCPPSyntax*			pcString;
	SCPP65Type*			psElement;
	SCPP65Type*			psWhole;
	SIRType				sSize;
	SIRType				sBytesType;
	SIRReference		sMemory;
	SIRReference		sIndex;
	SIRReference		sTooMany;
	SIRReference		sMade;
	SIRReference		sNext;
	SCPP65IRCleanup		sFree;
	SCPP65IRCleanup		sElements;
	SCPP65IRCountedLoop	sLoop;
	CCPPFunction*		pcAllocation;
	CIRBlock*			pcNull;
	CIRBlock*			pcMake;
	CIRBlock*			pcNullEnd;
	CIRBlock*			pcTooMany;
	CIRBlock*			pcStart;
	CIRBlock*			pcAllocate;
	CIRBlock*			pcEnd;
	uint64				ulliMost;
	uint64				ulliCookie;
	size				uiFirst;
	size				uiMark;
	size				i;
	bool				bCookie;
	bool				bValue;

	for (psElement = psType; psElement->IsArray(); psElement = psElement->psBase)
	{
	}
	if (psElement->IsClass() && !mpcUnit->GetClasses()->CompleteClass(psElement->pcClass, NULL))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	psWhole = mpcUnit->GetTypeChecker()->NewTypeIdArray(pcNew);
	if (psWhole)
	{
		//new (Type[N]): the type-id's own bound.
		sCount.psType = mpcTypes->Size();
		sCount.sValue = mpcFunction->AddInteger(IRTypeOf(sCount.psType), psWhole->iCount);
	}
	else if (!ContextualValue(pcNew->macpArraySizes.GetPtr(0), CPPXT_Integer, &sCount))
	{
		return false;
	}
	if (!Convert(&sCount, mpcTypes->Integer(mcModule.GetPointerSize(0), false)))
	{
		return false;
	}
	sSize = IRTypeOf(sCount.psType);
	bCookie = mpcUnit->GetBinder()->KeepsArrayCount(psType);

	//The most elements whose bytes, with the count in front, a size_t holds.  When the count can be more, more than
	//that goes straight to the end with a null pointer.
	sBytes.psType = mpcTypes->Size();
	sBytesType = IRTypeOf(sBytes.psType);
	ulliCookie = bCookie ? (uint64)mcModule.GetPointerSize(0) : 0;
	pcStart = NULL;
	pcEnd = NULL;
	if (psType->iBytes > 0)
	{
		ulliMost = (IRTruncate(sBytesType, ~(uint64)0) - ulliCookie) / (uint64)psType->iBytes;
		if (ulliMost < IRTruncate(sSize, ~(uint64)0))
		{
			pcStart = mpcBlock;
			pcAllocate = NewBlock();
			pcEnd = NewBlock();
			sTooMany = mpcFunction->Add(mpcBlock, IRO_UGt, sSize, sCount.sValue, mpcFunction->AddInteger(sSize, (int64)ulliMost));
			if (mpcUnit->GetOptions()->mbExceptions)
			{
				//Too many: std::bad_array_new_length.  [expr.new] 7
				pcTooMany = NewBlock();
				mpcBlock->Branch(sTooMany, pcTooMany, pcAllocate);
				StartBlock(pcTooMany);
				ThrowRoutine(CPP65_THROW_BAD_LENGTH);
				JumpTo(pcEnd);
				pcStart = mpcBlock;
			}
			else
			{
				mpcBlock->Branch(sTooMany, pcEnd, pcAllocate);
			}
			StartBlock(pcAllocate);
		}
	}

	sBytes.sValue = mpcFunction->Add(mpcBlock, IRO_Mul, sSize, sCount.sValue, mpcFunction->AddInteger(sSize, psType->iBytes));
	if (bCookie)
	{
		sBytes.sValue = mpcFunction->Add(mpcBlock, IRO_Add, sSize, sBytes.sValue, mpcFunction->AddInteger(sSize, (int64)ulliCookie));
	}
	sBytes.sValue = ConvertIR(sBytes.sValue, sSize, sBytesType, false);
	if (!Allocate(pcNew, psType, &sBytes, &sMemory, &sFree, &pcAllocation))
	{
		return false;
	}
	pcNull = NULL;
	pcNullEnd = NULL;
	if (IsNullable(pcAllocation))
	{
		//NULL: nothing is made.  [expr.new] 13
		pcNull = mpcBlock;
		pcMake = NewBlock();
		pcNullEnd = NewBlock();
		mpcBlock->Branch(mpcFunction->Add(mpcBlock, IRO_Ne, IRType(IRTK_Pointer), sMemory, mpcFunction->AddConstant(IRType(IRTK_Pointer), 0)), pcMake, pcNullEnd);
		StartBlock(pcMake);
	}

	//If an element's construction throws, the elements made are destroyed and the memory given back.  [expr.new] 20
	uiMark = mcCleanups.NumEntries();
	if (PartsWanted() && (sFree.pcFunction || sFree.szRuntime) && (psType->IsClass() || pcNew->mpcInitializer))
	{
		sFree.sPlace.sAddress = sMemory;
		mcCleanups.PushPart(&sFree);
	}
	if (bCookie)
	{
		mpcFunction->Store(mpcBlock, sSize, sCount.sValue, sMemory);
		sMemory = AddressAt(sMemory, mcModule.GetPointerSize(0));
	}
	sMade = IRNone();
	if (PartsWanted() && NeedsDestroying(psType))
	{
		sMade = mpcFunction->Slot(mpcBlock, sSize, (uint32)mcModule.GetPointerSize(0), (uint32)mcModule.GetPointerSize(0));
		mpcFunction->Store(mpcBlock, sSize, mpcFunction->AddInteger(sSize, 0), sMade);
		sElements.Clear(CPP65IRC_Part);
		sElements.ePart = CPP65IRP_Elements;
		sElements.sPlace.sAddress = sMemory;
		sElements.sPlace.psType = psType;
		sElements.sCount = sMade;
		mcCleanups.PushPart(&sElements);
	}

	//The elements a list in braces gives (or the characters of the string it holds, as an array's are: new
	//char[n]{"abc"}, [dcl.init.string]), then the rest.
	pcString = mpcUnit->GetTypeChecker()->StringInitialiser(pcNew->mpcInitializer, psType);
	papcElements = pcNew->mpcInitializer ? mpcUnit->GetInstantiator()->ListElements(pcNew->mpcInitializer) : NULL;
	bValue = pcNew->mpcInitializer != NULL;
	uiFirst = papcElements ? papcElements->NumElements() : 0;
	sElement.psBitField = NULL;
	if (mpcUnit->GetTypeChecker()->IsStringLiteral(pcString))
	{
		uiFirst = mpcUnit->GetStrings()->StringArrayType(pcString)->iCount;
		sElement.sAddress = sMemory;
		sElement.psType = mpcTypes->Array(psType, (int)uiFirst);
		if (!ArrayInitialiser(&sElement, pcString))
		{
			return false;
		}
		papcElements = NULL;
	}
	sElement.psType = psType;
	for (i = 0; papcElements && (i < uiFirst); i++)
	{
		sElement.sAddress = AddressAt(sMemory, (int)(i * psType->iBytes));
		if (psType->IsClass() ? !Construct(&sElement, NULL, papcElements->GetPtr(i)) : !LocalInitialiser(&sElement, papcElements->GetPtr(i), false))
		{
			return false;
		}
		if (!sMade.IsNone())
		{
			mpcFunction->Store(mpcBlock, sSize, mpcFunction->AddInteger(sSize, (int64)(i + 1)), sMade);
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
		sNext = mpcFunction->Add(mpcBlock, IRO_Add, sSize, sIndex, mpcFunction->AddInteger(sSize, 1));
		if (!sMade.IsNone())
		{
			mpcFunction->Store(mpcBlock, sSize, sNext, sMade);
		}
		EndCountedLoop(&sLoop, sNext);
	}
	mcCleanups.RemovePartsFrom(uiMark);
	if (pcNull)
	{
		pcMake = mpcBlock;
		JumpTo(pcNullEnd);
		StartBlock(pcNullEnd);
		sMemory = Phi(pcNullEnd, IRType(IRTK_Pointer), pcNull, mpcFunction->AddConstant(IRType(IRTK_Pointer), 0), pcMake, sMemory);
	}
	if (pcEnd)
	{
		//Null when there were too many, else the elements.
		pcAllocate = mpcBlock;
		JumpTo(pcEnd);
		StartBlock(pcEnd);
		sMemory = Phi(pcEnd, IRType(IRTK_Pointer), pcStart, mpcFunction->AddConstant(IRType(IRTK_Pointer), 0), pcAllocate, sMemory);
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
// delete[] of an array whose count is kept (CCPPBinder::KeepsArrayCount):
// the count is in front of the elements, which are destroyed last first
// if they have a destructor, and operator delete[], the class's own if it
// has one, is given the memory from the count on, and its size if it takes
// one.  psPointer isn't null.  [expr.delete] 6, 9, 10
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::DeleteArray(CCPPSyntaxDeleteExpression* pcDelete, SCPP65IRValue* psPointer, CCPPClass* pcClass)
{
	SCPP65IRValue		sBytes;
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

	if (pcClass->mpcDestructor)
	{
		//loop: the index counts down from the count.
		sIndex = BeginCountedLoop(&sLoop, sSize, sCount, IRO_Ne, mpcFunction->AddInteger(sSize, 0));
		sNext = mpcFunction->Add(mpcBlock, IRO_Sub, sSize, sIndex, mpcFunction->AddInteger(sSize, 1));
		sElement = mpcFunction->Add(mpcBlock, IRO_PtrAdd, IRType(IRTK_Pointer), psPointer->sValue, mpcFunction->Add(mpcBlock, IRO_Mul, sSize, sNext, mpcFunction->AddInteger(sSize, pcClass->mpsType->iBytes)));
		if (!CallObject(pcClass->mpcDestructor, sElement, pcClass, NULL, false))
		{
			return false;
		}
		EndCountedLoop(&sLoop, sNext);
	}

	//An operator delete[] that takes a size is given the elements' and the count's, which only the count read here says.
	sBytes.psType = mpcTypes->Integer(mcModule.GetPointerSize(0), false);
	sBytes.sValue = mpcFunction->Add(mpcBlock, IRO_Mul, sSize, sCount, mpcFunction->AddInteger(sSize, pcClass->mpsType->iBytes));
	sBytes.sValue = mpcFunction->Add(mpcBlock, IRO_Add, sSize, sBytes.sValue, mpcFunction->AddInteger(sSize, mcModule.GetPointerSize(0)));
	return Deallocate(pcDelete, mpcUnit->GetBinder()->FindDeallocation(pcDelete->mbGlobal ? NULL : pcClass, true, true), true, &sBytes, sBase);
}


//////////////////////////////////////////////////////////////////////////
//
// delete pointer: unless it is null, the object's destructor then operator
// delete, the class's own if it has one.  An object whose destructor is
// virtual is destroyed and freed by its own class's deleting destructor
// (CCPPClass::mpcDeleting), called through its vtable: only that knows the
// class's operator delete and size.  ::delete of one frees the whole
// object, found from its vtable, by the global operator delete.  delete[]
// of objects with destructors is DeleteArray's.  [expr.delete] 3, 9, 10
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::DeleteExpression(CCPPSyntaxDeleteExpression* pcDelete, SCPP65IRValue* psValue)
{
	SCPP65IRValue	sPointer;
	SCPP65IRValue	sBytes;
	SCPP65Type*		psType;
	CCPPFunction*	pcDeallocation;
	SIRReference	sCondition;
	SIRReference	sFree;
	CIRBlock*		pcFree;
	CIRBlock*		pcDone;
	bool			bSized;

	if (!ContextualValue(pcDelete->mpcOperand, CPPXT_ObjectPointer, &sPointer))
	{
		return false;
	}
	if (!sPointer.psType->IsPointer())
	{
		return Unsupported(__FILE__, __LINE__);
	}
	psType = sPointer.psType->psBase;
	if (psType->IsClass() && !mpcUnit->GetClasses()->CompleteClass(psType->pcClass, NULL))
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
	if (pcDelete->mbArray && mpcUnit->GetBinder()->KeepsArrayCount(psType))
	{
		if (!DeleteArray(pcDelete, &sPointer, psType->pcClass))
		{
			return false;
		}
	}
	else if (psType->IsClass() && psType->pcClass->mpcDeleting && !pcDelete->mbGlobal)
	{
		if (!CallObject(psType->pcClass->mpcDeleting, sPointer.sValue, psType->pcClass, NULL, true))
		{
			return false;
		}
	}
	else
	{
		//::delete of an object whose destructor is virtual: the whole object is what is freed, its start read from the
		//vtable before the destructors change it (a base class's pointer isn't the start of a derived object).
		sFree = sPointer.sValue;
		if (psType->IsClass() && psType->pcClass->mpcDeleting)
		{
			sFree = DynamicCast(sPointer.sValue, psType->pcClass, NULL);
		}
		if (psType->IsClass() && psType->pcClass->mpcDestructor && !CallObject(psType->pcClass->mpcDestructor, sPointer.sValue, psType->pcClass, NULL, true))
		{
			return false;
		}
		//The global operator delete that takes a size is called where the size is known: not for an array whose
		//elements have no destructor, which has no count in front of it (the one that takes the pointer alone is
		//called, as [expr.delete] 10 leaves the compiler free to), nor for ::delete of an object whose destructor
		//is virtual, whose own class only its vtable knows.
		bSized = !pcDelete->mbArray && !(psType->IsClass() && psType->pcClass->mpcDeleting);
		pcDeallocation = mpcUnit->GetBinder()->FindDeallocation((psType->IsClass() && !pcDelete->mbGlobal) ? psType->pcClass : NULL, pcDelete->mbArray, bSized);
		sBytes = ByteCount(psType->iBytes);
		if (!Deallocate(pcDelete, pcDeallocation, pcDelete->mbArray, &sBytes, sFree))
		{
			return false;
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
// Frees the memory at sFree that what was psBytes long was in: by
// pcDeallocation, the operator delete its delete calls
// (CCPPBinder::FindDeallocation), which is given the size if it takes one;
// or, when that is NULL, by the runtime's operator delete or delete[]
// (bArray).  pcFor is the syntax
// it is done for (a delete, or a class for its deleting destructor), which
// the arguments are kept by.  A deleting destructor's call of the runtime's
// is weak: the program has an operator delete if anything in it deletes,
// and needn't have one for a virtual destructor alone.  So is its call of
// the global one that takes a size, where another unit defines it: by its
// plain label, with both arguments pushed, as the runtime's is called.
// [expr.delete] 9, 10
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Deallocate(CCPPSyntax* pcFor, CCPPFunction* pcDeallocation, bool bArray, SCPP65IRValue* psBytes, SIRReference sFree)
{
	CCPPSyntax*		pcSize;
	CArraySyntax	apcArguments;
	SCPP65IRValue	sAddress;
	SCPP65IRValue	sResult;
	CChars			szLabel;
	bool			bCalled;

	if (!pcDeallocation)
	{
		CallRuntime(bArray ? "opDeleteArray" : "opDelete", IRType(IRTK_None), IRType(IRTK_Pointer), sFree, mpcCPPFunction->mbDeleting);
		return true;
	}
	if (mpcCPPFunction->mbDeleting && !pcDeallocation->mpcClass && !pcDeallocation->mpcDefinition)
	{
		szLabel.Init();
		mpcUnit->GetMangler()->AppendFunctionLabel(&szLabel, pcDeallocation, false);
		maszWeakRuntime.AddIfUnique(szLabel);
		mpcFunction->Argument(mpcBlock, IRType(IRTK_Pointer), sFree);
		mpcFunction->Argument(mpcBlock, IRTypeOf(psBytes->psType), psBytes->sValue);
		mpcFunction->Call(mpcBlock, IRType(IRTK_None), mcModule.Symbol(szLabel.Text()));
		szLabel.Kill();
		return true;
	}

	apcArguments.Init();
	apcArguments.Add(mpcUnit->GetDesugarings()->ImplicitValue(pcFor, mpcTypes->Pointer(&mpcTypes->msVoid)));
	pcSize = NULL;
	if (pcDeallocation->mapsParameters.NumElements() == 2)
	{
		pcSize = mpcUnit->GetDesugarings()->ImplicitValue(pcFor, psBytes->psType, 1);
		apcArguments.Add(pcSize);
		mcImplicitValues.Put((int64)(size)pcSize, psBytes);
	}
	sAddress.psType = mpcTypes->Pointer(&mpcTypes->msVoid);
	sAddress.sValue = sFree;
	bCalled = CallWithValue(pcDeallocation, &apcArguments, apcArguments.GetPtr(0), &sAddress, &sResult);
	if (pcSize)
	{
		mcImplicitValues.Remove((int64)(size)pcSize);
	}
	apcArguments.Kill();
	return bCalled;
}


//////////////////////////////////////////////////////////////////////////
//
// A number of bytes that is known, as the size_t an allocation or a
// deallocation function is given.
//
//////////////////////////////////////////////////////////////////////////
SCPP65IRValue CCPP65IRFrontEnd::ByteCount(int64 lliBytes)
{
	SCPP65IRValue	sBytes;

	sBytes.psType = mpcTypes->Size();
	sBytes.sValue = mpcFunction->AddInteger(IRTypeOf(sBytes.psType), lliBytes);
	return sBytes;
}


//////////////////////////////////////////////////////////////////////////
//
// The body of pcClass's deleting destructor (CCPPClass::mpcDeleting): the
// object, this, is destroyed by the class's destructor and freed by the
// class's operator delete, given the class's size if it takes one; or by
// the global one that takes a size, if the program has declared it, given
// the class's, which only this function knows (the object deleted may be
// of a class derived from the pointer's); or by the runtime's, which takes
// the pointer alone.  A program that never deletes needn't have either of
// the last two (Deallocate's weak calls).  [class.dtor] 13,
// [expr.delete] 10
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::DeletingDestructor(CCPPClass* pcClass)
{
	SCPP65IRValue	sBytes;

	if (!CallObject(pcClass->mpcDestructor, msThis, pcClass, NULL, false))
	{
		return false;
	}
	sBytes = ByteCount(pcClass->mpsType->iBytes);
	return Deallocate(pcClass->mpcSpecifier, mpcUnit->GetBinder()->FindDeallocation(pcClass, false, true), false, &sBytes, msThis);
}


//////////////////////////////////////////////////////////////////////////
//
// Makes a lambda's closure at psPlace: each field from what it captures,
// where the lambda is: a copy (constructed as a variable is from its
// initialiser), the address of what it refers to, or this.  An exception
// from a copy destroys the copies made before it.
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
	size				uiMark;

	psLambda = mpcUnit->GetClosures()->GetLambda(pcSyntax);
	if (!psLambda)
	{
		return Unsupported(__FILE__, __LINE__);
	}

	//Each capture copied is a part until the closure is made.  [except.ctor] 2
	uiMark = mcCleanups.NumEntries();
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
		else if (psField->psType->IsArray())
		{
			//An array captured by copy: the closure has its elements.  [expr.prim.lambda] 15
			if (!Place(psCapture->pcInitialiser, &sReferred) || !CopyArray(&sField, sReferred.sAddress))
			{
				return false;
			}
		}
		else if (!Construct(&sField, NULL, psCapture->pcInitialiser))
		{
			return false;
		}
		else
		{
			PushDestroyPart(sField.sAddress, sField.psType, NULL);
		}
	}
	mcCleanups.RemovePartsFrom(uiMark);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Makes the array at psPlace a copy of the array of its type at sSource:
// its bytes, or each element from the other's by its class's copying when
// its bytes alone aren't the object.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CopyArray(SCPP65IRPlace* psPlace, SIRReference sSource)
{
	SCPP65IRPlace	sElement;
	int				iOffset;

	for (sElement.psType = psPlace->psType; sElement.psType->IsArray(); sElement.psType = sElement.psType->psBase)
	{
	}
	if (!sElement.psType->IsClass() || sElement.psType->pcClass->IsTriviallyCopyable())
	{
		CopyPieces(psPlace->sAddress, sSource, psPlace->psType->iBytes);
		return true;
	}
	sElement.psBitField = NULL;
	for (iOffset = 0; iOffset < psPlace->psType->iBytes; iOffset += sElement.psType->iBytes)
	{
		sElement.sAddress = AddressAt(psPlace->sAddress, iOffset);
		if (!CopyFromAddress(&sElement, AddressAt(sSource, iOffset)))
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
	SCPP65IRMade	sTemporary;
	bool			bTemporary = false;
	SCPP65IRPlace					sPlace;
	SCPP65IRValue					sLeft;
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
		psType = mpcUnit->GetTypeChecker()->TypeOf(pcExpression);
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
		else if (!Temporary(psType, &sPlace, &sTemporary))
		{
			return false;
		}
		else
		{
			bTemporary = true;
		}
		if (!LambdaObject(&sPlace, (CCPPSyntaxLambdaExpression*)pcExpression))
		{
			return false;
		}
		if (bTemporary)
		{
			TemporaryMade(&sTemporary);
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
				return OperatorExpression(((CCPPSyntaxUnaryExpression*)pcExpression)->meOperator, ((CCPPSyntaxUnaryExpression*)pcExpression)->mpcOperand, mpcUnit->GetDesugarings()->ImplicitZero(), psValue);
			}
			//++object and --object are the object's operator, as -object is.
			//Falls through.
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
		if ((((CCPPSyntaxBinaryExpression*)pcExpression)->meOperator == CT_Comma) && !mpcUnit->GetBinder()->IsOverloadedComma((CCPPSyntaxBinaryExpression*)pcExpression))
		{
			//(x, object) with no operator, of its own: x is worked out, then the object.  [expr.comma] 1
			return Value(((CCPPSyntaxBinaryExpression*)pcExpression)->mpcLeft, &sLeft) && Value(((CCPPSyntaxBinaryExpression*)pcExpression)->mpcRight, psValue);
		}
		if (!IsCPPAssignment(((CCPPSyntaxBinaryExpression*)pcExpression)->meOperator))
		{
			return OperatorExpression(((CCPPSyntaxBinaryExpression*)pcExpression)->meOperator, ((CCPPSyntaxBinaryExpression*)pcExpression)->mpcLeft, ((CCPPSyntaxBinaryExpression*)pcExpression)->mpcRight, psValue);
		}
		return Assignment((CCPPSyntaxBinaryExpression*)pcExpression, psValue);
	case CPPS_ConstructExpression:
		pcConstruct = (CCPPSyntaxConstructExpression*)pcExpression;
		psType = mpcUnit->GetDeclarators()->GetTypeIdType(pcConstruct->mpcType);
		if (!psType || !psType->IsClass() || !Temporary(psType, &sPlace, &sTemporary))
		{
			return Unsupported(__FILE__, __LINE__);
		}
		if (!Construct(&sPlace, NULL, pcConstruct->mpcInitializer))
		{
			return false;
		}
		TemporaryMade(&sTemporary);
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
		if (!psType || !psType->IsClass() || !Temporary(psType, &sPlace, &sTemporary))
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
		TemporaryMade(&sTemporary);
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
	SCPP65IRMade	sTemporary;

	psType = psType->Unqualified();
	if (psType->IsClass())
	{
		//Made as a variable is from its initialiser (const S& s = 3 and const S& s = {1, 2}).
		if (!Temporary(psType, psPlace, &sTemporary))
		{
			return Unsupported(__FILE__, __LINE__);
		}
		if (!Construct(psPlace, NULL, pcValue))
		{
			return false;
		}
		TemporaryMade(&sTemporary);
		return true;
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
// A place in the frame for a temporary object.  psMade: it is to be
// destroyed at the end of the full expression if its class has a destructor
// (NULL for a copy of an argument's bytes, which the callee destroys); the
// caller says when it has been made (TemporaryMade), which is when it is
// registered.  One that may not be made (miConditional: in an operand of
// &&, || or ?: that may not be worked out) has its place and a flag in the
// entry block (which reaches the destroying), the flag 0 there.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Temporary(SCPP65Type* psType, SCPP65IRPlace* psPlace, SCPP65IRMade* psMade)
{
	bool		bDestructor;
	CIRBlock*	pcEntry;
	SIRType		sFlag;

	if (!mpcUnit->GetClasses()->CompleteClass(psType->pcClass, NULL))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	bDestructor = psMade && psType->pcClass->NeedsDestructor();
	psPlace->psType = psType;
	if (psMade)
	{
		psMade->bDestroy = bDestructor;
		psMade->bConditional = false;
		psMade->sFlag = IRNone();
	}
	if (!bDestructor || (miConditional == 0))
	{
		psPlace->sAddress = mpcFunction->Slot(mpcBlock, IRType(IRTK_Pointer), (uint32)psType->iBytes, 4);
		if (psMade)
		{
			psMade->sPlace = *psPlace;
		}
		return true;
	}

	pcEntry = mpcFunction->GetBlock(0);
	sFlag = IRTypeOf(&mpcTypes->msBool);
	psPlace->sAddress = mpcFunction->Slot(pcEntry, IRType(IRTK_Pointer), (uint32)psType->iBytes, 4);
	psMade->sPlace = *psPlace;
	psMade->bConditional = true;
	psMade->sFlag = mpcFunction->Slot(pcEntry, IRType(IRTK_Pointer), 1, 1);
	mpcFunction->Store(pcEntry, sFlag, mpcFunction->AddConstant(sFlag, 0), psMade->sFlag);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The temporary Temporary gave a place to has been made (its constructor,
// or the call that returns it, has finished): it is registered, to be
// destroyed at the end of the full expression (DestroyTemporaries), and a
// conditional one's flag says so from here.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::TemporaryMade(SCPP65IRMade* psMade)
{
	SIRType	sFlag;

	if (!psMade->bDestroy)
	{
		return;
	}
	if (psMade->bConditional)
	{
		sFlag = IRTypeOf(&mpcTypes->msBool);
		mpcFunction->Store(mpcBlock, sFlag, mpcFunction->AddConstant(sFlag, 1), psMade->sFlag);
	}
	mcCleanups.PushTemporary(&psMade->sPlace, psMade->bConditional, psMade->sFlag);
}


//////////////////////////////////////////////////////////////////////////
//
// Copy constructs the object at psPlace from pcSource (of its class or one
// derived from it, as which it is one of the class: CCPPBinder::ReachesBase):
// with a constructor the class declares for that (the one chosen for the
// source, so a move for what is going), or as the compiler's own does, its
// parts one by one (CopyMembers) or its bytes.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CopyConstruct(SCPP65IRPlace* psPlace, CCPPSyntax* pcSource)
{
	CCPPClass*		pcClass;
	CCPPClass*		pcSourceClass;
	CArraySyntax	apcArguments;
	bool			bResult;

	pcClass = psPlace->psType->pcClass;
	pcSourceClass = mpcUnit->GetTypeChecker()->ObjectClass(pcSource);
	if (pcSourceClass && !mpcUnit->GetBinder()->ReachesBase(pcSourceClass, pcClass, pcSource))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (mpcUnit->GetBinder()->CopiesByConstructor(pcClass, pcSource))
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
	mpcDiagnostics->ReportError(pcNode, "The object cannot be copied: its class declares a move and no copy");
	return Unsupported(__FILE__, __LINE__);
}


//////////////////////////////////////////////////////////////////////////
//
// The error that an object is assigned whose class's operator= the
// compiler may not write (CCPPClass::IsAssignmentDeleted).  Always false.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::AssignmentDeleted(CCPPSyntax* pcNode)
{
	mpcDiagnostics->ReportOnce(pcNode, "The object cannot be assigned: its class has a const or reference member, or declares a move and no copy");
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

	//The vtables are the object's once its parts' copy constructors (which store their own) have run.
	return Place(pcSource, &sSource) && CopyPlace(psPlace, &sSource, &sAddress) && CopyParts(psPlace->psType->pcClass, psPlace->sAddress, sAddress, mpcUnit->GetTypeChecker()->IsRvalue(pcSource), IRNone()) &&
		   StoreVtables(psPlace->sAddress, psPlace->psType->pcClass);
}


//////////////////////////////////////////////////////////////////////////
//
// Copies the bases and fields of the class (the objects at sDestination and
// sSource) that have copy constructors with them, in the order a copy
// constructor makes them: the virtual bases (once, by a complete object:
// sVTT is IRNone), then the other bases, then the fields.  A base
// subobject's sVTT is its entries in the VTT of the complete object it is
// part of, which its bases that have virtual bases are given theirs from
// (BaseVTT), each copied by its copy constructor's base version.
// [class.copy] 15
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CopyParts(CCPPClass* pcClass, SIRReference sDestination, SIRReference sSource, bool bMove, SIRReference sVTT)
{
	SCPPBase*			psBase;
	SCPPVirtualBase*	psVirtual;
	SCPPField*			psField;
	SCPP65Type*			psType;
	size				i;
	size				uiMark;
	int					j;
	int					iCount;
	int					iOffset;

	//Each part copied is a part until the object is: an exception from a later one's copy destroys it.  [except.ctor] 2
	uiMark = mcCleanups.NumEntries();
	for (i = 0; sVTT.IsNone() && (i < pcClass->macVirtualBases.NumElements()); i++)
	{
		psVirtual = pcClass->macVirtualBases.Get(i);
		if (!CopyPart(psVirtual->pcClass, AddressAt(sDestination, psVirtual->iOffset), VirtualBaseAddress(sSource, pcClass, psVirtual->pcClass), bMove, BaseVTT(pcClass, sVTT, psVirtual->pcClass)))
		{
			return false;
		}
		PushDestroyPart(AddressAt(sDestination, psVirtual->iOffset), psVirtual->pcClass->mpsType, psVirtual->pcClass);
	}
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		psBase = pcClass->macBases.Get(i);
		if (psBase->bVirtual)
		{
			continue;
		}
		if (!CopyPart(psBase->pcClass, AddressAt(sDestination, psBase->iOffset), AddressAt(sSource, psBase->iOffset), bMove, BaseVTT(pcClass, sVTT, psBase->pcClass)))
		{
			return false;
		}
		PushDestroyPart(AddressAt(sDestination, psBase->iOffset), psBase->pcClass->mpsType, psBase->pcClass);
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
			if (!CopyPart(psType->pcClass, AddressAt(sDestination, iOffset), AddressAt(sSource, iOffset), bMove, IRNone()))
			{
				return false;
			}
			PushDestroyPart(AddressAt(sDestination, iOffset), psType, NULL);
		}
	}
	mcCleanups.RemovePartsFrom(uiMark);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The VTT entries of pcBase, a base of pcClass, where an object of pcClass
// is being copied: found in pcClass's own, which are sVTT for a base
// subobject and its VTT for a complete object (sVTT is IRNone).  IRNone
// for a base with no virtual bases, which has no base version to give them
// to.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CCPP65IRFrontEnd::BaseVTT(CCPPClass* pcClass, SIRReference sVTT, CCPPClass* pcBase)
{
	if (!pcBase->HasVirtualBases())
	{
		return IRNone();
	}
	return AddressAt(sVTT.IsNone() ? CompleteVTT(pcClass) : sVTT, CPP65_VTABLE_ENTRY_BYTES * mpcUnit->GetClassData()->VTTIndex(pcClass, pcBase));
}


//////////////////////////////////////////////////////////////////////////
//
// An inherited constructor's call of the base's constructor it stands for,
// with its own parameters (scalars and references only).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::InheritedConstructorCall(SCPP65IRPlace* psBase)
{
	CArrayTemplate<SIRReference>	asValues;
	SCPP65Type*		psType;
	SIRReference	sValue;
	SIRReference	sAddress;
	size			ui;
	int32			iIndex;

	if (mpcCPPFunction->mpcInherited->mpcClass->HasVirtualBases())
	{
		return Unsupported(__FILE__, __LINE__);
	}
	//The IR parameters after this and the result's address, loaded before the arguments are passed: a call's arguments
	//come straight before it.
	iIndex = (msThis.IsNone() ? 0 : 1) + (msResult.IsNone() ? 0 : 1);
	asValues.Init();
	for (ui = 0; ui < mpcCPPFunction->mapsParameters.NumElements(); ui++)
	{
		psType = mpcCPPFunction->mapsParameters.GetPtr(ui);
		if (psType->IsClass() || !IsSupportedType(psType) || psType->IsVoid() || psType->IsArray())
		{
			asValues.Kill();
			return Unsupported(__FILE__, __LINE__);
		}
		sAddress = mpcFunction->Add(mpcBlock, IRO_ArgAddress, IRType(IRTK_Pointer), mpcFunction->AddInteger(IRType(IRTK_I32), iIndex));
		sValue = mpcFunction->Load(mpcBlock, IRTypeOf(psType), sAddress);
		asValues.Add(&sValue);
		iIndex++;
	}
	mpcFunction->Argument(mpcBlock, IRType(IRTK_Pointer), psBase->sAddress);
	for (ui = 0; ui < asValues.NumElements(); ui++)
	{
		mpcFunction->Argument(mpcBlock, IRTypeOf(mpcCPPFunction->mapsParameters.GetPtr(ui)), *asValues.Get(ui));
	}
	asValues.Kill();
	mpcFunction->Call(mpcBlock, IRType(IRTK_None), FunctionSymbol(mpcCPPFunction->mpcInherited), -1, true);
	TestThrown(mpcCPPFunction->mpcInherited, FunctionSymbol(mpcCPPFunction->mpcInherited));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A part (base or field) of a class: copied with its copy constructor (its
// parameters after the first given their defaults, scalars only), or its
// own parts, or left as its bytes.  sVTT: as CopyParts, for a base
// subobject with virtual bases (BaseVTT), which its copy constructor's base
// version is given.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CopyPart(CCPPClass* pcClass, SIRReference sDestination, SIRReference sSource, bool bMove, SIRReference sVTT)
{
	bool							bBaseVersion;
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
		return pcClass->IsTriviallyCopyable() || CopyParts(pcClass, sDestination, sSource, bMove, sVTT);
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
	bBaseVersion = !sVTT.IsNone();
	mpcFunction->Argument(mpcBlock, IRType(IRTK_Pointer), sDestination);
	if (bBaseVersion)
	{
		mpcFunction->Argument(mpcBlock, IRType(IRTK_Pointer), sVTT);
	}
	mpcFunction->Argument(mpcBlock, IRType(IRTK_Pointer), sSource);
	for (ui = 0; ui < asDefaults.NumElements(); ui++)
	{
		psDefault = asDefaults.Get(ui);
		mpcFunction->Argument(mpcBlock, IRTypeOf(psDefault->psType), psDefault->sValue);
	}
	asDefaults.Kill();
	mpcFunction->Call(mpcBlock, IRType(IRTK_None), FunctionSymbol(pcConstructor, bBaseVersion), -1, true);
	TestThrown(pcConstructor, FunctionSymbol(pcConstructor, bBaseVersion));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// object = pcSource for a class with no operator= of its own whose parts
// have, or that is polymorphic: as the compiler's own operator= does, each
// base and field is assigned in turn, with its class's operator= or its
// bytes.  The vtable pointers are left: the object may be a base subobject
// of another class's.  What is
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
	if (!sSource.psType->IsClass() || !mpcUnit->GetClasses()->IsDerived(sSource.psType->pcClass, psPlace->psType->pcClass, NULL) || !ReferenceAddress(&sSource, mpcTypes->Pointer(psPlace->psType), &sAddress))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	return AssignParts(psPlace->psType->pcClass, psPlace->sAddress, sAddress, mpcUnit->GetTypeChecker()->IsRvalue(pcSource), true);
}


//////////////////////////////////////////////////////////////////////////
//
// Assigns the bases and the fields of the pcClass at sSource to those of
// the one at sDestination, in the order they are declared.  With
// bVirtualBases (the object as a whole is assigned) its virtual bases
// first, once each, where each object has them; else they are left to the
// object it is a base of.  [class.copy] 28
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::AssignParts(CCPPClass* pcClass, SIRReference sDestination, SIRReference sSource, bool bMove, bool bVirtualBases)
{
	SCPPBase*			psBase;
	SCPPVirtualBase*	psVirtual;
	SCPPField*			psField;
	SCPP65Type*			psType;
	size				i;
	int					j;
	int					iOffset;

	for (i = 0; bVirtualBases && (i < pcClass->macVirtualBases.NumElements()); i++)
	{
		psVirtual = pcClass->macVirtualBases.Get(i);
		if (!AssignPart(psVirtual->pcClass, VirtualBaseAddress(sDestination, pcClass, psVirtual->pcClass), VirtualBaseAddress(sSource, pcClass, psVirtual->pcClass), bMove, false))
		{
			return false;
		}
	}
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		psBase = pcClass->macBases.Get(i);
		if (!psBase->bVirtual && !AssignPart(psBase->pcClass, AddressAt(sDestination, psBase->iOffset), AddressAt(sSource, psBase->iOffset), bMove, false))
		{
			return false;
		}
	}
	for (i = 0; i < pcClass->macFields.NumElements(); i++)
	{
		psField = pcClass->macFields.Get(i);
		psType = psField->psType->ElementType();
		if (!psType->IsClass())
		{
			CopyPieces(AddressAt(sDestination, psField->iOffset), AddressAt(sSource, psField->iOffset), psField->psType->iBytes);
			continue;
		}
		for (j = 0; j < (psField->psType->IsArray() ? psField->psType->iCount : 1); j++)
		{
			iOffset = psField->iOffset + j * psType->iBytes;
			if (!AssignPart(psType->pcClass, AddressAt(sDestination, iOffset), AddressAt(sSource, iOffset), bMove, true))
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
// or as AssignParts (bVirtualBases as it has it), or its bytes (but a
// polymorphic class's, whose vtable pointers stay).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::AssignPart(CCPPClass* pcClass, SIRReference sDestination, SIRReference sSource, bool bMove, bool bVirtualBases)
{
	CCPPFunction*	pcOperator;
	SCPP65Type*		psReturn;

	pcOperator = pcClass->GetPartCopier(true, bMove);
	if (!pcOperator)
	{
		if (pcClass->IsAssignmentDeleted())
		{
			return AssignmentDeleted(mpcValue);
		}
		if (!pcClass->IsTriviallyAssignable() || pcClass->IsPolymorphic())
		{
			return AssignParts(pcClass, sDestination, sSource, bMove, bVirtualBases);
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
	TestThrown(pcOperator, FunctionSymbol(pcOperator));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// An object made from source, a trivially copyable object of its class: the
// bytes, and the object's vtable pointers.
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
	return CopyPlace(psPlace, &sSource, &sAddress) && StoreVtables(psPlace->sAddress, psPlace->psType->pcClass);
}


//////////////////////////////////////////////////////////////////////////
//
// Copies the bytes of the object at psSource (of the place's class or one
// derived from it) to the place, a piece at a time: of a class with virtual
// bases its own part, then each virtual base's from where the source has it
// (the source may be a base subobject, whose virtual bases are elsewhere).
// *psAddress is where it was copied from, as the place's class (its base
// part).  The place's vtable pointers are the caller's to store.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CopyPlace(SCPP65IRPlace* psPlace, SCPP65IRPlace* psSource, SIRReference* psAddress)
{
	CCPPClass*			pcClass;
	SCPPVirtualBase*	psVirtual;
	size				i;

	if (!psSource->psType->IsClass() || !mpcUnit->GetClasses()->IsDerived(psSource->psType->pcClass, psPlace->psType->pcClass, NULL) ||
		!ReferenceAddress(psSource, mpcTypes->Pointer(psPlace->psType), psAddress))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	pcClass = psPlace->psType->pcClass;
	if (!pcClass->HasVirtualBases())
	{
		CopyPieces(psPlace->sAddress, *psAddress, psPlace->psType->iBytes);
		return true;
	}
	CopyPieces(psPlace->sAddress, *psAddress, pcClass->miOwnBytes);
	for (i = 0; i < pcClass->macVirtualBases.NumElements(); i++)
	{
		psVirtual = pcClass->macVirtualBases.Get(i);
		CopyPieces(AddressAt(psPlace->sAddress, psVirtual->iOffset), VirtualBaseAddress(*psAddress, pcClass, psVirtual->pcClass), psVirtual->pcClass->miOwnBytes);
	}
	return true;
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
