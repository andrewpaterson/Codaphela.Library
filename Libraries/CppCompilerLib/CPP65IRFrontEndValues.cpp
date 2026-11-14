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


//The IR front end's values and places: an expression's value, loads and stores, constants and strings, variables,
//members and member pointers, subscripts and functions' addresses.


//////////////////////////////////////////////////////////////////////////
//
// An expression worked out for its value or its effects (a void call).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Expression(CCPPSyntax* pcExpression, SCPP65IRValue* psValue)
{
	return Value(pcExpression, psValue);
}


//////////////////////////////////////////////////////////////////////////
//
// An expression's value for psTo (NULL when it isn't known): a function's name
// for a pointer to a function is the address of the overload of that type,
// otherwise as Value.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ValueFor(CCPPSyntax* pcExpression, SCPP65Type* psTo, SCPP65IRValue* psValue)
{
	CCPPSyntaxIdExpression*	pcFunction;

	pcFunction = mpcGenerator->FunctionNameFor(pcExpression, psTo);
	if (pcFunction)
	{
		if (!FunctionAddress(pcFunction, psTo->psBase, psValue))
		{
			return false;
		}
		psValue->psType = psTo;
		return true;
	}
	return Value(pcExpression, psValue);
}


//////////////////////////////////////////////////////////////////////////
//
// An expression's value (a reference's is its object's, an array's the
// address of its first element).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Value(CCPPSyntax* pcExpression, SCPP65IRValue* psValue)
{
	CCPPClass*	pcEnumeratorClass;
	SCPP65Type*						psType;
	SCPP65IRPlace					sPlace;
	CCPPSyntaxNamedCastExpression*	pcNamedCast;
	CCPPSymbol*						pcSymbol;

	pcExpression = mpcGenerator->SkipParentheses(pcExpression);
	if (!pcExpression || mbUnsupported)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	mpcValue = pcExpression;
	psType = mpcGenerator->TypeOf(pcExpression);
	if (!psType)
	{
		//Usually C++ that is wrong (a call no function matches, or more than one): the generator says what.
		mpcGenerator->ReportTypeOf(pcExpression);
		return Unsupported(__FILE__, __LINE__);
	}
	if (!IsSupportedType(psType) && !psType->IsFunction() && !psType->IsClass())
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (psType->IsClass())
	{
		return ObjectValue(pcExpression, psValue);
	}

	switch (pcExpression->GetKind())
	{
	case CPPS_LiteralExpression:
	case CPPS_SizeofExpression:
	case CPPS_TypeTraitExpression:
		return ConstantValue(pcExpression, psValue) || Unsupported(__FILE__, __LINE__);
	case CPPS_StringExpression:
		return StringLiteral(pcExpression, psValue);
	case CPPS_IdExpression:
		pcSymbol = ((CCPPSyntaxIdExpression*)pcExpression)->mpcName->mpcSymbol;
		if (mpcGenerator->IsFunctionName((CCPPSyntaxIdExpression*)pcExpression))
		{
			return FunctionName((CCPPSyntaxIdExpression*)pcExpression, psValue);
		}
		if (!pcSymbol)
		{
			pcSymbol = mpcGenerator->QualifiedEnumerator(((CCPPSyntaxIdExpression*)pcExpression)->mpcName, pcExpression, &pcEnumeratorClass);
		}
		if (pcSymbol && (pcSymbol->meKind == CPPSK_Function))
		{
			return FunctionAddress((CCPPSyntaxIdExpression*)pcExpression, NULL, psValue);
		}
		if (pcSymbol && (pcSymbol->meKind == CPPSK_FunctionTemplate))
		{
			return FunctionTemplateAddress((CCPPSyntaxIdExpression*)pcExpression, psValue);
		}
		//A name without a symbol (a member in a template the parser could not resolve) is a place.
		if (pcSymbol && ((pcSymbol->meKind == CPPSK_Enumerator) || (pcSymbol->meKind == CPPSK_ValueParameter)))
		{
			return ConstantValue(pcExpression, psValue) || Unsupported(__FILE__, __LINE__);
		}
		break;
	case CPPS_UnaryExpression:
		return UnaryExpression((CCPPSyntaxUnaryExpression*)pcExpression, psValue);
	case CPPS_BinaryExpression:
		return BinaryExpression((CCPPSyntaxBinaryExpression*)pcExpression, psValue);
	case CPPS_ConditionalExpression:
		return ConditionalExpression((CCPPSyntaxConditionalExpression*)pcExpression, psValue);
	case CPPS_CallExpression:
		return CallExpression((CCPPSyntaxCallExpression*)pcExpression, psValue);
	case CPPS_SubscriptExpression:
		if (IsObjectSubscript((CCPPSyntaxSubscriptExpression*)pcExpression))
		{
			//object[index] is a call to the object's operator[], which may return a value.
			if (!OperatorCall(((CCPPSyntaxSubscriptExpression*)pcExpression)->mpcArray, CT_OpenSquareBracket, ((CCPPSyntaxSubscriptExpression*)pcExpression)->mpcIndex, psValue))
			{
				return false;
			}
			if (!psValue->psType->IsReference())
			{
				return true;
			}
			sPlace.sAddress = psValue->sValue;
			sPlace.psType = psValue->psType->psBase;
			return Load(&sPlace, psValue);
		}
		break;
	case CPPS_MemberExpression:
		break;
	case CPPS_ThisExpression:
		if (msThis.IsNone())
		{
			return Unsupported(__FILE__, __LINE__);
		}
		psValue->sValue = msThis;
		psValue->psType = mpcTypes->Pointer(mpcCPPFunction->mpcClass->mpsType);
		return true;
	case CPPS_NewExpression:
		return NewExpression((CCPPSyntaxNewExpression*)pcExpression, psValue);
	case CPPS_DeleteExpression:
		return DeleteExpression((CCPPSyntaxDeleteExpression*)pcExpression, psValue);
	case CPPS_CastExpression:
		return CastExpression(((CCPPSyntaxCastExpression*)pcExpression)->mpcType, ((CCPPSyntaxCastExpression*)pcExpression)->mpcOperand, psValue);
	case CPPS_NamedCastExpression:
		pcNamedCast = (CCPPSyntaxNamedCastExpression*)pcExpression;
		if (pcNamedCast->meCast == CT_dynamic_cast)
		{
			return DynamicCastExpression(pcNamedCast, psValue);
		}
		return CastExpression(pcNamedCast->mpcType, pcNamedCast->mpcOperand, psValue);
	case CPPS_ConstructExpression:
		return ScalarConstruct((CCPPSyntaxConstructExpression*)pcExpression, psValue);
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
// A place's value: an array gives the address of its first element.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Load(SCPP65IRPlace* psPlace, SCPP65IRValue* psValue)
{
	if (psPlace->psType->IsArray())
	{
		psValue->sValue = psPlace->sAddress;
		psValue->psType = mpcTypes->Pointer(psPlace->psType->psBase);
		return true;
	}
	if (psPlace->psType->IsClass())
	{
		//An object used as a value is its address (as in the generator).
		psValue->sValue = psPlace->sAddress;
		psValue->psType = psPlace->psType;
		return true;
	}
	if (!psPlace->psType->IsScalar())
	{
		return Unsupported(__FILE__, __LINE__);
	}
	psValue->sValue = mpcFunction->Load(mpcBlock, IRTypeOf(psPlace->psType), psPlace->sAddress, psPlace->psType->bVolatile);
	psValue->psType = psPlace->psType->Unqualified();
	if (psPlace->psBitField)
	{
		psValue->sValue = BitFieldValue(psPlace->psType, psPlace->psBitField->iBitOffset, psPlace->psBitField->iBitWidth, psValue->sValue);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The value of the iWidth bits from iOffset of sUnit, of psType: shifted
// down and masked, or for a signed type shifted to the top and back down
// so its top bit is extended.  A bit-field's value from its unit, or (with
// an offset of 0) a value assigned to it as the bit-field keeps it.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CCPP65IRFrontEnd::BitFieldValue(SCPP65Type* psType, int iOffset, int iWidth, SIRReference sUnit)
{
	SIRType		sType;
	int			iBits;
	int			iAbove;

	sType = IRTypeOf(psType);
	iBits = psType->iBytes * 8;
	if (psType->bSigned && (psType->eKind != CPP65TK_Bool))
	{
		iAbove = iBits - iOffset - iWidth;
		if (iAbove > 0)
		{
			sUnit = mpcFunction->Add(mpcBlock, IRO_Shl, sType, sUnit, mpcFunction->AddInteger(sType, iAbove));
		}
		if (iWidth < iBits)
		{
			sUnit = mpcFunction->Add(mpcBlock, IRO_AShr, sType, sUnit, mpcFunction->AddInteger(sType, iBits - iWidth));
		}
		return sUnit;
	}
	if (iOffset > 0)
	{
		sUnit = mpcFunction->Add(mpcBlock, IRO_LShr, sType, sUnit, mpcFunction->AddInteger(sType, iOffset));
	}
	if (iWidth < iBits)
	{
		sUnit = mpcFunction->Add(mpcBlock, IRO_And, sType, sUnit, mpcFunction->AddInteger(sType, (int64)(((uint64)1 << iWidth) - 1)));
	}
	return sUnit;
}


//////////////////////////////////////////////////////////////////////////
//
// The value must already be the place's type.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::Store(SCPP65IRPlace* psPlace, SCPP65IRValue* psValue)
{
	SIRType			sType;
	SIRReference	sUnit;
	SIRReference	sBits;
	uint64			ulliMask;
	int				iBits;

	sType = IRTypeOf(psPlace->psType);
	if (!psPlace->psBitField)
	{
		mpcFunction->Store(mpcBlock, sType, psValue->sValue, psPlace->sAddress, psPlace->psType->bVolatile);
		return;
	}

	//A bit-field: its bits of the unit replaced, the others kept.
	iBits = psPlace->psType->iBytes * 8;
	ulliMask = ((psPlace->psBitField->iBitWidth < 64) ? (((uint64)1 << psPlace->psBitField->iBitWidth) - 1) : ~(uint64)0) << psPlace->psBitField->iBitOffset;
	if (iBits < 64)
	{
		ulliMask &= ((uint64)1 << iBits) - 1;
	}
	sBits = psValue->sValue;
	if (psPlace->psBitField->iBitOffset > 0)
	{
		sBits = mpcFunction->Add(mpcBlock, IRO_Shl, sType, sBits, mpcFunction->AddInteger(sType, psPlace->psBitField->iBitOffset));
	}
	if (psPlace->psBitField->iBitWidth < iBits)
	{
		sBits = mpcFunction->Add(mpcBlock, IRO_And, sType, sBits, mpcFunction->AddInteger(sType, (int64)ulliMask));
		sUnit = mpcFunction->Load(mpcBlock, sType, psPlace->sAddress, psPlace->psType->bVolatile);
		sUnit = mpcFunction->Add(mpcBlock, IRO_And, sType, sUnit, mpcFunction->AddInteger(sType, (int64)(~ulliMask & ((iBits < 64) ? (((uint64)1 << iBits) - 1) : ~(uint64)0))));
		sBits = mpcFunction->Add(mpcBlock, IRO_Or, sType, sUnit, sBits);
	}
	mpcFunction->Store(mpcBlock, sType, sBits, psPlace->sAddress, psPlace->psType->bVolatile);
}


//////////////////////////////////////////////////////////////////////////
//
// The value an assignment to psPlace gives when it is a bit-field: what
// was stored, cut to the bit-field's width, as reading it back would give.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::BitFieldResult(SCPP65IRPlace* psPlace, SCPP65IRValue* psValue)
{
	if (psPlace->psBitField)
	{
		psValue->sValue = BitFieldValue(psPlace->psType, 0, psPlace->psBitField->iBitWidth, psValue->sValue);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A constant expression the generator can work out: an integer of its type,
// or a float literal's bits.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ConstantValue(CCPPSyntax* pcExpression, SCPP65IRValue* psValue)
{
	SCPP65Type*	psType;
	int64		lliValue;
	double		dValue;

	psType = (pcExpression->GetKind() == CPPS_LiteralExpression) ? mpcGenerator->Literal((CCPPSyntaxLiteralExpression*)pcExpression) : mpcGenerator->TypeOf(pcExpression);
	if (!psType)
	{
		return false;
	}
	psType = psType->WithoutReference();
	if (psType->IsFloat())
	{
		if (!mpcGenerator->FloatConstant(pcExpression, &dValue))
		{
			return false;
		}
		psValue->sValue = mpcFunction->AddConstant(IRTypeOf(psType), (uint64)mpcGenerator->FloatBits(psType, dValue));
		psValue->psType = psType;
		return true;
	}
	if (!psType->IsScalar() || !mpcGenerator->Constant(pcExpression, &lliValue))
	{
		return false;
	}
	psValue->sValue = Integer(psType, lliValue);
	psValue->psType = psType;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// String literals are the generator's StrN data, numbered after the ones it
// has; they are given to it only if the function is written.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::StringLiteral(CCPPSyntax* pcString, SCPP65IRValue* psValue)
{
	CArrayUint8	auiBytes;

	auiBytes.Init();
	if (!mpcGenerator->DecodeString(pcString, &auiBytes))
	{
		auiBytes.Kill();
		return Unsupported(__FILE__, __LINE__);
	}

	//StringData's data ends with one zero byte.
	auiBytes.GrowToAtLeastNumElements(auiBytes.NumElements() + mpcGenerator->StringElementBytes(pcString) - 1, true, 0);
	StringData((char*)auiBytes.GetData(), auiBytes.NumElements(), psValue);
	psValue->psType = mpcTypes->Pointer(mpcGenerator->StringElementType(pcString));
	auiBytes.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Type(), Type{}, Type(value) or Type{value} of a type that isn't a class: zero
// (nothing, of void), or the value cast to it.  [expr.type.conv]
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ScalarConstruct(CCPPSyntaxConstructExpression* pcConstruct, SCPP65IRValue* psValue)
{
	CArraySyntax*	papcElements;
	SCPP65Type*		psType;

	papcElements = mpcGenerator->ConstructElements(pcConstruct);
	if (papcElements && (papcElements->NumElements() == 1))
	{
		return CastExpression(pcConstruct->mpcType, papcElements->GetPtr(0), psValue);
	}
	psType = mpcGenerator->GetTypeIdType(pcConstruct->mpcType);
	if (!psType || !papcElements || (papcElements->NumElements() != 0) || !(psType->IsScalar() || psType->IsVoid()))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	psValue->psType = psType;
	psValue->sValue = psType->IsVoid() ? IRNone() : mpcFunction->AddConstant(IRTypeOf(psType), 0);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// __func__ and the other names of the function: a string.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::FunctionName(CCPPSyntaxIdExpression* pcId, SCPP65IRValue* psValue)
{
	CChars	szName;

	szName.Init();
	if (!mpcGenerator->FunctionNameText(mpcGenerator->GetComponentName(pcId->mpcName->GetLast()), &szName))
	{
		szName.Kill();
		return Unsupported(__FILE__, __LINE__);
	}
	StringData(szName.Text(), szName.Length(), psValue);
	szName.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A function's name used as a value is its address: Class::f and, in a member function, f may be a static
// member function (of the class or its bases).  An overloaded name needs
// the type of the function wanted (psTarget, from a cast).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::FunctionAddress(CCPPSyntaxIdExpression* pcId, SCPP65Type* psTarget, SCPP65IRValue* psValue)
{
	CCPPFunction*		pcFound;
	CCPPSyntaxName*		pcName;
	int64				lliName;
	bool				bOverloaded;

	pcName = pcId->mpcName;
	lliName = pcName->mpcSymbol->mlliName;
	pcFound = mpcGenerator->FindAddressedFunction(pcName, lliName, psTarget, pcId, &bOverloaded);
	if (bOverloaded || !pcFound)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	psValue->sValue = FunctionSymbol(pcFound);
	psValue->psType = mpcTypes->Pointer(FunctionType(pcFound));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// &Class::member: a pointer to a member
// function is its address (a thunk that calls through the vtable for a
// virtual one) and a pointer to a field its offset.  Returns false with
// *pbMember clear if the name isn't a non-static member (it may be a static
// one), and with it set after an Unsupported.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::MemberAddress(CCPPSyntaxIdExpression* pcId, SCPP65IRValue* psValue, bool* pbMember)
{
	CCPPSyntaxName*		pcName;
	CCPPClass*		pcClass;
	CCPPClass*		pcFoundClass;
	CCPPFunction*		pcFound;
	SCPPField*		psField;
	SIRReference		sSymbol;
	CChars				szThunk;
	int64				lliName;
	int					iOffset;

	*pbMember = false;
	pcName = pcId->mpcName;
	pcClass = mpcGenerator->GetComponentClass(pcName->GetQualifier(), NULL, pcId);
	lliName = mpcGenerator->GetFunctionName(pcName->GetLast());
	if (!pcClass || (lliName == -1) || !mpcGenerator->CompleteClass(pcClass, pcId))
	{
		return false;
	}

	pcFound = NULL;
	pcFoundClass = pcClass;
	if (!MemberFunction(pcClass, lliName, &pcFound, &pcFoundClass))
	{
		*pbMember = true;
		return Unsupported(__FILE__, __LINE__);
	}
	if (pcFound)
	{
		if (pcFound->mbStatic)
		{
			return false;
		}
		*pbMember = true;
		if (pcFound->mbVirtual)
		{
			szThunk.Init();
			mpcGenerator->AppendVirtualCallThunk(&szThunk, pcFound);
			sSymbol = mcModule.Symbol(szThunk.Text());
			szThunk.Kill();
		}
		else
		{
			sSymbol = FunctionSymbol(pcFound);
		}
		psValue->sValue = ConvertIR(sSymbol, IRType(IRTK_Pointer), IRType(IRTK_I32), false);
		psValue->psType = mpcTypes->MemberPointer(pcFoundClass, FunctionType(pcFound));
		return true;
	}

	psField = pcClass->FindField(lliName, &iOffset);
	if (!psField)
	{
		return false;
	}
	*pbMember = true;
	psValue->sValue = mpcFunction->AddInteger(IRType(IRTK_I32), iOffset);
	psValue->psType = mpcTypes->MemberPointer(pcClass, psField->psType);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The member function of the name in pcClass, or else in its bases (the
// first that has one), and the class it is in.  Returns false for an overloaded name.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::MemberFunction(CCPPClass* pcClass, int64 lliName, CCPPFunction** ppcFound, CCPPClass** ppcClass)
{
	CCPPFunction*		pcFunction;
	size				ui;

	*ppcFound = NULL;
	for (pcFunction = mpcGenerator->FirstFunctionOfName(lliName); pcFunction; pcFunction = pcFunction->mpcNextOfName)
	{
		if ((pcFunction->mpcClass == pcClass) && (pcFunction->meKind == CPPFK_Function) && !pcFunction->mpcTemplate)
		{
			if (*ppcFound)
			{
				return false;
			}
			*ppcFound = pcFunction;
		}
	}
	if (*ppcFound)
	{
		*ppcClass = pcClass;
		return true;
	}
	for (ui = 0; ui < pcClass->macBases.NumElements(); ui++)
	{
		if (!MemberFunction(pcClass->macBases.Get(ui)->pcClass, lliName, ppcFound, ppcClass))
		{
			return false;
		}
		if (*ppcFound)
		{
			return true;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The object of object.*member or pointer->*member (moved on to the member
// pointer's class if that is a base) and the member pointer's type.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::MemberPointerObject(CCPPSyntaxBinaryExpression* pcBinary, SIRReference* psObject, SCPP65Type** ppsMember)
{
	SCPP65IRValue	sPointer;
	SCPP65IRPlace	sPlace;
	CCPPClass*	pcClass;

	*ppsMember = mpcGenerator->TypeOf(pcBinary->mpcRight);
	if (!*ppsMember || !(*ppsMember)->IsMemberPointer())
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (pcBinary->meOperator == CT_MemberByPointerFunctionPointerIndirection)
	{
		if (!Value(pcBinary->mpcLeft, &sPointer))
		{
			return false;
		}
		if (!sPointer.psType->IsPointer() || !sPointer.psType->psBase->IsClass())
		{
			return Unsupported(__FILE__, __LINE__);
		}
		*psObject = sPointer.sValue;
		pcClass = sPointer.psType->psBase->pcClass;
	}
	else
	{
		if (!Place(pcBinary->mpcLeft, &sPlace))
		{
			return false;
		}
		if (!sPlace.psType->IsClass())
		{
			return Unsupported(__FILE__, __LINE__);
		}
		*psObject = sPlace.sAddress;
		pcClass = sPlace.psType->pcClass;
	}
	*psObject = AddressAt(*psObject, BaseOffset(pcClass, (*ppsMember)->pcClass));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// (object.*function)(...) and (pointer->*function)(...): the object is
// this.  The object and the pointer are worked out before the arguments,
// as the callee is.  [expr.call] 8
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::MemberPointerCall(CCPPSyntaxCallExpression* pcCall, CCPPSyntaxBinaryExpression* pcBinary, SCPP65IRValue* psValue)
{
	SCPP65Type*		psMember;
	SCPP65IRValue	sFunction;
	SIRReference	sObject;

	if (!MemberPointerObject(pcBinary, &sObject, &psMember))
	{
		return false;
	}
	if (!psMember->psBase->IsFunction())
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (!Value(pcBinary->mpcRight, &sFunction))
	{
		return false;
	}
	return Call(NULL, ConvertIR(sFunction.sValue, IRType(IRTK_I32), IRType(IRTK_Pointer), false), psMember->psBase, &pcCall->macpArguments, psValue, NULL, false, NULL, false, sObject);
}


//////////////////////////////////////////////////////////////////////////
//
// object.*field and pointer->*field: the object's address plus the
// field's offset.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::MemberPointerPlace(CCPPSyntaxBinaryExpression* pcBinary, SCPP65IRPlace* psPlace)
{
	SCPP65Type*		psMember;
	SCPP65IRValue	sOffset;
	SIRReference	sObject;

	psPlace->psBitField = NULL;

	if (!MemberPointerObject(pcBinary, &sObject, &psMember))
	{
		return false;
	}
	if (psMember->psBase->IsFunction() || !IsPlaceType(psMember->psBase))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (!Value(pcBinary->mpcRight, &sOffset))
	{
		return false;
	}
	psPlace->sAddress = mpcFunction->Add(mpcBlock, IRO_PtrAdd, IRType(IRTK_Pointer), sObject, sOffset.sValue);
	psPlace->psType = psMember->psBase;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A string's address: its bytes become the generator's string data (with a
// zero after them) if the function is written.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::StringData(char* pcBytes, size uiLength, SCPP65IRValue* psValue)
{
	CChars	szLabel;

	if (uiLength > 0)
	{
		mauiStringBytes.InsertBlockAfterEnd((uint8*)pcBytes, uiLength);
	}
	mauiStringLengths.Add(&uiLength);

	szLabel.Init("Str");
	szLabel.Append((int)(muiFirstString + mauiStringLengths.NumElements() - 1));
	psValue->sValue = mcModule.Symbol(szLabel.Text());
	mcModule.SetLocal(psValue->sValue.Index());
	psValue->psType = mpcTypes->Pointer(&mpcTypes->msChar);
	szLabel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Variables, *pointer, array[index], and calls that return references.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Place(CCPPSyntax* pcExpression, SCPP65IRPlace* psPlace)
{
	CCPPSyntaxIdExpression*			pcId;
	CCPPSyntaxUnaryExpression*		pcUnary;
	CCPPSyntaxSubscriptExpression*	pcSubscript;
	CCPPVariable*					pcVariable;
	SCPP65IRValue					sPointer;
	SCPP65IRValue					sIndex;
	SCPP65IRValue					sAddress;
	SCPP65Type*						psType;
	SCPP65Type*						psArray;

	psPlace->psBitField = NULL;

	pcExpression = mpcGenerator->SkipParentheses(pcExpression);
	switch (pcExpression->GetKind())
	{
	case CPPS_IdExpression:
		pcId = (CCPPSyntaxIdExpression*)pcExpression;
		pcVariable = pcId->mpcName->mpcSymbol ? mpcGenerator->GetVariable(pcId->mpcName->mpcSymbol) : NULL;
		if (pcVariable)
		{
			return VariablePlace(pcVariable, psPlace);
		}
		//A member of this (a name in a template the parser could not resolve too).
		if (msThis.IsNone() || (pcId->mpcName->macpComponents.NumElements() != 1) || (pcId->mpcName->mpcSymbol && (pcId->mpcName->mpcSymbol->meKind != CPPSK_Field)))
		{
			return Unsupported(__FILE__, __LINE__);
		}
		return FieldPlace(msThis, mpcCPPFunction->mpcClass, mpcGenerator->GetComponentName(pcId->mpcName->GetLast()), pcId->mpcName->mpcSymbol, psPlace);

	case CPPS_MemberExpression:
		return MemberPlace((CCPPSyntaxMemberExpression*)pcExpression, psPlace);

	case CPPS_UnaryExpression:
		pcUnary = (CCPPSyntaxUnaryExpression*)pcExpression;
		if (pcUnary->meOperator != CT_Multiply)
		{
			//-object, +object, ~object: the temporary its operator made.
			psType = mpcGenerator->TypeOf(pcExpression);
			if (!psType || !psType->IsClass() || !ObjectValue(pcExpression, &sAddress))
			{
				return Unsupported(__FILE__, __LINE__);
			}
			psPlace->sAddress = sAddress.sValue;
			psPlace->psType = sAddress.psType;
			return true;
		}
		if (!Value(pcUnary->mpcOperand, &sPointer))
		{
			return false;
		}
		if (!sPointer.psType->IsPointer() || sPointer.psType->psBase->IsVoid() || !IsPlaceType(sPointer.psType->psBase))
		{
			return Unsupported(__FILE__, __LINE__);
		}
		psPlace->sAddress = sPointer.sValue;
		psPlace->psType = sPointer.psType->psBase;
		return true;

	case CPPS_SubscriptExpression:
		pcSubscript = (CCPPSyntaxSubscriptExpression*)pcExpression;
		if (IsObjectSubscript(pcSubscript))
		{
			//object[index] is a call to the object's operator[], which must return a reference here.
			if (!OperatorCall(pcSubscript->mpcArray, CT_OpenSquareBracket, pcSubscript->mpcIndex, &sAddress))
			{
				return false;
			}
			if (!sAddress.psType->IsReference())
			{
				return Unsupported(__FILE__, __LINE__);
			}
			psPlace->sAddress = sAddress.sValue;
			psPlace->psType = sAddress.psType->psBase;
			return true;
		}
		if (!mpcGenerator->TypeOf(pcSubscript->mpcArray))
		{
			return Unsupported(__FILE__, __LINE__);
		}
		//The array before the index.  [expr.sub] 1
		if (!ArrayValue(pcSubscript->mpcArray, &sPointer, &psArray) || !Value(pcSubscript->mpcIndex, &sIndex))
		{
			return false;
		}
		if (!(psArray && NarrowSubscript(psArray, &sPointer, &sIndex, &sAddress)) && !Operate(CT_Add, &sPointer, &sIndex, &sAddress))
		{
			return false;
		}
		if (!sAddress.psType->IsPointer() || sAddress.psType->psBase->IsVoid() || !IsPlaceType(sAddress.psType->psBase))
		{
			return Unsupported(__FILE__, __LINE__);
		}
		psPlace->sAddress = sAddress.sValue;
		psPlace->psType = sAddress.psType->psBase;
		return true;

	case CPPS_CallExpression:
		//The type of a call that returns a reference to a class is the class.
		psType = mpcGenerator->TypeOf(pcExpression);
		if (!psType || !(psType->IsReference() || psType->IsClass()))
		{
			return Unsupported(__FILE__, __LINE__);
		}
		if (!CallExpression((CCPPSyntaxCallExpression*)pcExpression, &sAddress))
		{
			return false;
		}
		psPlace->sAddress = sAddress.sValue;
		if (sAddress.psType->IsReference())
		{
			psPlace->psType = sAddress.psType->psBase;
		}
		else if (sAddress.psType->IsClass())
		{
			//A temporary the call made.
			psPlace->psType = sAddress.psType;
		}
		else
		{
			return Unsupported(__FILE__, __LINE__);
		}
		return true;

	case CPPS_ConstructExpression:
		if (!ObjectValue(pcExpression, &sAddress))
		{
			return false;
		}
		psPlace->sAddress = sAddress.sValue;
		psPlace->psType = sAddress.psType;
		return true;

	case CPPS_BinaryExpression:
		if (mpcGenerator->IsMemberPointerOperator(pcExpression))
		{
			return MemberPointerPlace((CCPPSyntaxBinaryExpression*)pcExpression, psPlace);
		}

		//(x = y) and (x op= y) of a scalar are x's place after the assignment.
		psType = mpcGenerator->TypeOf(((CCPPSyntaxBinaryExpression*)pcExpression)->mpcLeft);
		if (mpcGenerator->IsAssignmentOperator(((CCPPSyntaxBinaryExpression*)pcExpression)->meOperator) && psType && !psType->IsClass() &&
			!(psType->IsReference() && psType->psBase->IsClass()))
		{
			return Assignment((CCPPSyntaxBinaryExpression*)pcExpression, &sAddress, psPlace);
		}

		//An operator function's object (a temporary it made, or the object it refers to).
		psType = mpcGenerator->TypeOf(pcExpression);
		if (!psType || !(psType->IsReference() || psType->IsClass()) || !Value(pcExpression, &sAddress))
		{
			return Unsupported(__FILE__, __LINE__);
		}
		psPlace->sAddress = sAddress.sValue;
		psPlace->psType = sAddress.psType->WithoutReference();
		return psPlace->psType->IsClass() || Unsupported(__FILE__, __LINE__);

	case CPPS_CastExpression:
	case CPPS_NamedCastExpression:
	case CPPS_ConditionalExpression:
		psType = mpcGenerator->TypeOf(pcExpression);
		if (!psType || !psType->IsClass() || !ObjectValue(pcExpression, &sAddress))
		{
			return Unsupported(__FILE__, __LINE__);
		}
		psPlace->sAddress = sAddress.sValue;
		psPlace->psType = sAddress.psType;
		return true;

	default:
		return Unsupported(__FILE__, __LINE__);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A local's slot, a parameter's argaddr or a global's (or static local's)
// label.  A reference's place is what it refers to.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::VariablePlace(CCPPVariable* pcVariable, SCPP65IRPlace* psPlace)
{
	SIRReference*	psAddress;
	CChars			szLabel;

	psPlace->psBitField = NULL;

	if (!IsPlaceType(pcVariable->mpsType))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if ((pcVariable->meStorage == CPPVS_Local) || (pcVariable->meStorage == CPPVS_Parameter))
	{
		psAddress = mcAddresses.Get((int64)(size)pcVariable);
		if (!psAddress)
		{
			return Unsupported(__FILE__, __LINE__);
		}
		psPlace->sAddress = *psAddress;
	}
	else
	{
		szLabel.Init();
		mpcGenerator->AppendVariableLabel(&szLabel, pcVariable);
		psPlace->sAddress = mcModule.Symbol(szLabel.Text());
		szLabel.Kill();
		if (pcVariable->mbRestrict)
		{
			mcModule.SetRestrict(psPlace->sAddress.Index());
		}
		if (pcVariable->mbInternal || (pcVariable->meStorage == CPPVS_Static))
		{
			mcModule.SetLocal(psPlace->sAddress.Index());
		}
	}
	psPlace->psType = pcVariable->mpsType;
	if (psPlace->psType->IsReference())
	{
		psPlace->sAddress = mpcFunction->Load(mpcBlock, IRType(IRTK_Pointer), psPlace->sAddress);
		psPlace->psType = psPlace->psType->psBase;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of what the left of -> (pcObject, of psType) points to: through
// its operator-> for an object of a class.  NULL if it isn't a class.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65IRFrontEnd::ArrowClass(CCPPSyntax* pcObject, SCPP65Type* psType)
{
	psType = psType->WithoutReference();
	if (psType->IsClass())
	{
		psType = mpcGenerator->ArrowType(pcObject, pcObject);
	}
	return (psType && psType->IsPointer() && psType->psBase->IsClass()) ? psType->psBase : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// object.field and pointer->field (a static member is its variable).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::MemberPlace(CCPPSyntaxMemberExpression* pcMember, SCPP65IRPlace* psPlace)
{
	SCPP65Type*		psType;
	SIRReference	sObject;

	psPlace->psBitField = NULL;

	psType = mpcGenerator->TypeOf(pcMember->mpcObject);
	if (!psType)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (pcMember->mbArrow)
	{
		psType = ArrowClass(pcMember->mpcObject, psType);
	}
	else if (psType->IsReference())
	{
		psType = psType->psBase;
	}
	if (!psType || !psType->IsClass() || !ObjectAddress(pcMember->mpcObject, pcMember->mbArrow, &sObject))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (!FieldPlace(sObject, psType->pcClass, mpcGenerator->GetComponentName(pcMember->mpcName->GetLast()), NULL, psPlace))
	{
		return false;
	}

	//A member of a volatile object is volatile.
	if (psType->bVolatile)
	{
		psPlace->psType = mpcTypes->Volatile(psPlace->psType);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The field lliName (or pcSymbol's) of the object of pcClass at sObject; a
// reference field's place is what it refers to.  A static member is its
// variable.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::FieldPlace(SIRReference sObject, CCPPClass* pcClass, int64 lliName, CCPPSymbol* pcSymbol, SCPP65IRPlace* psPlace)
{
	SCPPField*	psField;
	CCPPVariable*	pcVariable;
	int				iOffset;

	psPlace->psBitField = NULL;

	if (!pcClass || !mpcGenerator->CompleteClass(pcClass, NULL))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	psField = pcSymbol ? pcClass->FindFieldSymbol(pcSymbol, &iOffset) : pcClass->FindField(lliName, &iOffset);
	if (!psField)
	{
		pcVariable = mpcGenerator->FindStatic(pcClass, lliName);
		if (!pcVariable)
		{
			return Unsupported(__FILE__, __LINE__);
		}
		return VariablePlace(pcVariable, psPlace);
	}
	if (!IsSupportedType(psField->psType) && !ElementType(psField->psType)->IsClass())
	{
		return Unsupported(__FILE__, __LINE__);
	}
	psPlace->sAddress = AddressAt(sObject, iOffset);
	psPlace->psType = psField->psType;
	psPlace->psBitField = psField->IsBitField() ? psField : NULL;
	if (psPlace->psType->IsReference())
	{
		psPlace->sAddress = mpcFunction->Load(mpcBlock, IRType(IRTK_Pointer), psPlace->sAddress);
		psPlace->psType = psPlace->psType->psBase;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether array[index] subscripts an object (a call of its operator[]).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::IsObjectSubscript(CCPPSyntaxSubscriptExpression* pcSubscript)
{
	SCPP65Type*	psType;

	psType = mpcGenerator->ValueTypeOf(pcSubscript->mpcArray);
	return psType && psType->IsClass();
}


//////////////////////////////////////////////////////////////////////////
//
// The value of the array (or pointer) in array[index], as Value, and in
// *ppsArray the array's own type when it is a place that is an array (a
// variable, member or element), else NULL.  For those Value is the place
// loaded (an array's first element's address), so they are done the same.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ArrayValue(CCPPSyntax* pcArray, SCPP65IRValue* psPointer, SCPP65Type** ppsArray)
{
	SCPP65IRPlace	sPlace;
	CCPPSymbol*		pcSymbol;
	bool			bPlace;

	*ppsArray = NULL;
	pcArray = mpcGenerator->SkipParentheses(pcArray);
	switch (pcArray->GetKind())
	{
	case CPPS_MemberExpression:
		bPlace = true;
		break;
	case CPPS_SubscriptExpression:
		bPlace = !IsObjectSubscript((CCPPSyntaxSubscriptExpression*)pcArray);
		break;
	case CPPS_IdExpression:
		pcSymbol = ((CCPPSyntaxIdExpression*)pcArray)->mpcName->mpcSymbol;
		bPlace = pcSymbol && ((pcSymbol->meKind == CPPSK_Variable) || (pcSymbol->meKind == CPPSK_Field) || (pcSymbol->meKind == CPPSK_Parameter)) &&
				 !mpcGenerator->IsFunctionName((CCPPSyntaxIdExpression*)pcArray);
		break;
	default:
		bPlace = false;
		break;
	}
	if (!bPlace)
	{
		return Value(pcArray, psPointer);
	}
	if (!Place(pcArray, &sPlace))
	{
		return false;
	}
	if (sPlace.psType->IsArray())
	{
		*ppsArray = sPlace.psType;
	}
	return Load(&sPlace, psPointer);
}


//////////////////////////////////////////////////////////////////////////
//
// array[index] on the W65C816, for an array (not a pointer) of at least two
// elements and less than 64KB: an index outside the array is undefined, so
// the offset fits in 16 bits and is worked out in them, then zero extended
// once (marked IRF_InArray), instead of widening the index to 32 bits and scaling it there.  An
// array of 0 or 1 elements may be a struct's open ended tail, so it isn't.
// False (nothing written) when the subscript isn't one of these.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::NarrowSubscript(SCPP65Type* psArray, SCPP65IRValue* psPointer, SCPP65IRValue* psIndex, SCPP65IRValue* psAddress)
{
	SIRType			sWord;
	SIRReference	sOffset;
	int				iBytes;

	if (!mpcGenerator->mpcTarget->NarrowsSubscripts() || !psArray->IsArray() || (psArray->iCount < 2) || (psArray->iBytes >= 0x10000) ||
		!psIndex->psType->IsInteger() || !psPointer->psType->IsPointer())
	{
		return false;
	}
	if (!Convert(psIndex, mpcTypes->Integer(2, false)))
	{
		return false;
	}
	sWord = IRType(IRTK_I16);
	iBytes = psPointer->psType->GetElementBytes();
	sOffset = (iBytes == 1) ? psIndex->sValue : mpcFunction->Add(mpcBlock, IRO_Mul, sWord, psIndex->sValue, mpcFunction->AddInteger(sWord, iBytes));
	sOffset = mpcFunction->Add(mpcBlock, IRO_ZExt, IRIntegerType(mcModule.GetPointerSize(0)), sOffset);
	psAddress->sValue = mpcFunction->Add(mpcBlock, IRO_PtrAdd, IRType(IRTK_Pointer), psPointer->sValue, sOffset);
	if ((mpcBlock->masInstructions.NumElements() > 0) && mpcBlock->masInstructions.Tail()->sResult.Equals(psAddress->sValue) && (mpcBlock->masInstructions.Tail()->uiOpcode == IRO_PtrAdd))
	{
		mpcBlock->masInstructions.Tail()->uiFlags |= IRF_InArray;
	}
	psAddress->psType = psPointer->psType;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The address of a function template's instance for explicit arguments, as
// the generator's FunctionTemplateAddress: &ComparePrimitive<int> or
// ComparePrimitive<int>.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::FunctionTemplateAddress(CCPPSyntaxIdExpression* pcId, SCPP65IRValue* psValue)
{
	CCPPSyntaxNameComponent*	pcComponent;
	CCPPTemplate*				pcTemplate;
	CCPPFunction*				pcFunction;

	pcComponent = pcId->mpcName->GetLast();
	pcTemplate = mpcGenerator->GetTemplate(pcId->mpcName->mpcSymbol);
	if (!pcTemplate || !pcComponent->mbTemplateArguments)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	pcFunction = mpcGenerator->InstantiateFunction(pcTemplate, &pcComponent->macpTemplateArguments, NULL, pcId);
	if (!pcFunction)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	psValue->sValue = FunctionSymbol(pcFunction);
	psValue->psType = mpcTypes->Pointer(FunctionType(pcFunction));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The symbol of a function's label; the generator writes the function.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CCPP65IRFrontEnd::FunctionSymbol(CCPPFunction* pcFunction)
{
	CChars			szLabel;
	SIRReference	sSymbol;

	mpcGenerator->RequireFunction(pcFunction);
	szLabel.Init();
	mpcGenerator->AppendFunctionLabel(&szLabel, pcFunction);
	sSymbol = mcModule.Symbol(szLabel.Text());
	szLabel.Kill();
	MarkFunctionSymbol(pcFunction, sSymbol.Index());
	return sSymbol;
}


//////////////////////////////////////////////////////////////////////////
//
// Marks a function's symbol, where it is called and where it is defined:
// the unit's own (a static function or a member of a class without a name)
// and, for a function that isn't variadic (C linkage too), that a back end
// may pass its arguments in registers (CIRModule::SetRegisterCall: the
// W65C816's).  Not one with an object passed by value: as its bytes
// (AddPieces) the called function takes the object's address (IRO_ArgAddress
// of its first piece), so its pieces must stay together where the caller
// pushed them, and the register rule would put the first piece (or two) in
// registers.  Any object parameter counts (not only those passed as their
// bytes), so every unit decides the same from the declaration alone, even
// where the class isn't complete.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::MarkFunctionSymbol(CCPPFunction* pcFunction, uint32 uiSymbol)
{
	size	ui;
	bool	bPieces;

	if (pcFunction->IsUnitFunction())
	{
		mcModule.SetLocal(uiSymbol);
	}
	bPieces = false;
	for (ui = 0; ui < pcFunction->mapsParameters.NumElements(); ui++)
	{
		bPieces |= pcFunction->mapsParameters.GetPtr(ui)->IsClass();
	}
	if (!pcFunction->mbVariadic && !bPieces)
	{
		//C linkage too: this compiler is the only one that sees the source (the user, 2026-11-03).
		mcModule.SetRegisterCall(uiSymbol);
	}
}
