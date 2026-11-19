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
#include "CPPConstants.h"


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
// for a pointer to a function is the address of the overload of that type, and
// an object is converted by its conversion function to psTo (never an explicit
// one: a cast and a condition convert the value, with ConvertObject), and a
// list in braces is its one value or zero.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ValueFor(CCPPSyntax* pcExpression, SCPP65Type* psTo, SCPP65IRValue* psValue)
{
	CCPPBoundConversion		cBound;
	CArraySyntax*			papcElements;
	bool					bAddress;

	if ((pcExpression->GetKind() == CPPS_InitializerList) && !psTo->IsClass())
	{
		//{} is zero and {value} the value.  [dcl.init.list] 3
		papcElements = mpcUnit->ListElements(pcExpression);
		if (papcElements->NumElements() == 0)
		{
			psValue->sValue = mpcFunction->AddConstant(IRTypeOf(psTo), 0);
			psValue->psType = psTo;
			return true;
		}
		if (papcElements->NumElements() > 1)
		{
			return Unsupported(__FILE__, __LINE__);
		}
		return ValueFor(papcElements->GetPtr(0), psTo, psValue) && ConvertImplicit(psValue, psTo, papcElements->GetPtr(0));
	}
	if (!OverloadAddress(pcExpression, psTo, psValue, &bAddress))
	{
		return false;
	}
	if (bAddress)
	{
		return true;
	}
	if (!Value(pcExpression, psValue))
	{
		return false;
	}

	//An object wanted as a value of another type, where nothing says so (an argument, an initialiser, what is
	//assigned or returned): by the conversion function the binder chose, which is never an explicit one.
	if (IsObjectValue(psValue))
	{
		mpcUnit->GetBinder()->BindConversion(pcExpression, psTo, false, &cBound);
		if (cBound.meHow == CPPBV_Function)
		{
			return ConvertObjectBy(psValue, &cBound, psTo);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// f, &f or &Class::f wanted as a psTo, a pointer to a function or to a
// member function: the address of the overload of psTo's type, with
// *pbAddress set.  *pbAddress is clear for any other expression or type.
// Returns false after an Unsupported.  [over.over] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::OverloadAddress(CCPPSyntax* pcExpression, SCPP65Type* psTo, SCPP65IRValue* psValue, bool* pbAddress)
{
	CCPPSyntaxIdExpression*	pcName;

	pcName = mpcUnit->FunctionNameFor(pcExpression, psTo);
	*pbAddress = pcName != NULL;
	if (pcName)
	{
		if (!FunctionAddress(pcName, psTo->psBase, psValue))
		{
			return Unsupported(__FILE__, __LINE__);
		}
		psValue->psType = psTo;
		return true;
	}
	pcName = mpcUnit->MemberFunctionNameFor(pcExpression, psTo);
	if (pcName && MemberAddress(pcName, psTo->psBase, psValue, pbAddress))
	{
		//A member of a base is converted to a member of the class psTo is for.
		return Convert(psValue, psTo);
	}
	return !*pbAddress;
}


//////////////////////////////////////////////////////////////////////////
//
// An expression's value (a reference's is its object's, an array's the
// address of its first element).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Value(CCPPSyntax* pcExpression, SCPP65IRValue* psValue)
{
	CCPPBoundName					cBound;
	SCPP65Type*						psType;
	SCPP65IRPlace					sPlace;
	CCPPSyntaxNamedCastExpression*	pcNamedCast;

	pcExpression = mpcUnit->SkipParentheses(pcExpression);
	if (!pcExpression || mbUnsupported)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	mpcValue = pcExpression;
	psType = mpcUnit->TypeOf(pcExpression);
	if (!psType)
	{
		//Usually C++ that is wrong (a call no function matches, or more than one): the generator says what.
		mpcUnit->ReportTypeOf(pcExpression);
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
	if (IsReferenceCast(pcExpression))
	{
		//What a cast to a reference refers to, read.
		return Place(pcExpression, &sPlace) && Load(&sPlace, psValue);
	}

	switch (pcExpression->GetKind())
	{
	case CPPS_ImplicitArgument:
		return ImplicitArgument((CCPPSyntaxImplicitArgument*)pcExpression, psValue);
	case CPPS_LiteralExpression:
		return mpcUnit->UserLiteralToken(pcExpression) ? LiteralCall(pcExpression, psValue) : (ConstantValue(pcExpression, psValue) || Unsupported(__FILE__, __LINE__));
	case CPPS_SizeofExpression:
	case CPPS_TypeTraitExpression:
	case CPPS_NoexceptExpression:
		return ConstantValue(pcExpression, psValue) || Unsupported(__FILE__, __LINE__);
	case CPPS_StringExpression:
		return mpcUnit->UserLiteralToken(pcExpression) ? LiteralCall(pcExpression, psValue) : StringLiteral(pcExpression, psValue);
	case CPPS_IdExpression:
		mpcUnit->GetBinder()->BindName((CCPPSyntaxIdExpression*)pcExpression, &cBound);
		switch (cBound.meWhat)
		{
		case CPPBN_FunctionName:
			return FunctionName((CCPPSyntaxIdExpression*)pcExpression, psValue);
		case CPPBN_Function:
			return FunctionAddress((CCPPSyntaxIdExpression*)pcExpression, NULL, psValue);
		case CPPBN_FunctionTemplate:
			return FunctionTemplateAddress((CCPPSyntaxIdExpression*)pcExpression, psValue);
		case CPPBN_Constant:
		case CPPBN_EnclosingConstant:
			return ConstantValue(pcExpression, psValue) || Unsupported(__FILE__, __LINE__);
		case CPPBN_TemplateArgument:
			return Value(cBound.mpcArgument, psValue);
		case CPPBN_Variable:
			//A static const member initialised in its class is its value (CCPP65TypeChecker::IdExpression).
			if (mpcUnit->IsConstantMember(cBound.mpcVariable) && ConstantValue(pcExpression, psValue))
			{
				return true;
			}
			break;
		default:
			//A variable or a field: a place, which is loaded.
			break;
		}
		break;
	case CPPS_UnaryExpression:
		return UnaryExpression((CCPPSyntaxUnaryExpression*)pcExpression, psValue);
	case CPPS_BinaryExpression:
		return BinaryExpression((CCPPSyntaxBinaryExpression*)pcExpression, psValue);
	case CPPS_ConditionalExpression:
		return ConditionalExpression((CCPPSyntaxConditionalExpression*)pcExpression, psValue);
	case CPPS_CallExpression:
		if (!CallExpression((CCPPSyntaxCallExpression*)pcExpression, psValue))
		{
			return false;
		}
		if (psValue->psType && psValue->psType->IsReference() && psValue->psType->psBase->IsScalar())
		{
			//The value of a call that returns a reference to a number or a pointer is what it refers to, read
			//([conv.lval]); its place is Place's.  (A reference to an object stays one: IsObjectValue.)
			sPlace.sAddress = psValue->sValue;
			sPlace.psType = psValue->psType->psBase;
			sPlace.psBitField = NULL;
			return Load(&sPlace, psValue);
		}
		return true;
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
		if (mpcUnit->MemberEnumerator((CCPPSyntaxMemberExpression*)pcExpression))
		{
			return MemberEnumerator((CCPPSyntaxMemberExpression*)pcExpression, psValue);
		}
		break;
	case CPPS_ThisExpression:
		if (msThis.IsNone())
		{
			return Unsupported(__FILE__, __LINE__);
		}
		if (!mpcUnit->ThisClass())
		{
			return Unsupported(__FILE__, __LINE__);
		}
		psValue->sValue = msThis;
		psValue->psType = mpcTypes->Pointer(mpcUnit->ThisClass()->mpsType);
		return true;
	case CPPS_NewExpression:
		return NewExpression((CCPPSyntaxNewExpression*)pcExpression, psValue);
	case CPPS_DeleteExpression:
		return DeleteExpression((CCPPSyntaxDeleteExpression*)pcExpression, psValue);
	case CPPS_CastExpression:
		return CastExpression(pcExpression, ((CCPPSyntaxCastExpression*)pcExpression)->mpcType, ((CCPPSyntaxCastExpression*)pcExpression)->mpcOperand, psValue);
	case CPPS_NamedCastExpression:
		pcNamedCast = (CCPPSyntaxNamedCastExpression*)pcExpression;
		if (pcNamedCast->meCast == CT_dynamic_cast)
		{
			return DynamicCastExpression(pcNamedCast, psValue);
		}
		return CastExpression(pcNamedCast, pcNamedCast->mpcType, pcNamedCast->mpcOperand, psValue);
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
// A place's value: an array gives the address of its first element, and a
// function its own.
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
	if (psPlace->psType->IsFunction())
	{
		//A function used as a value is a pointer to it.  [conv.func]
		psValue->sValue = psPlace->sAddress;
		psValue->psType = mpcTypes->Pointer(psPlace->psType);
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

	psType = (pcExpression->GetKind() == CPPS_LiteralExpression) ? mpcUnit->Literal((CCPPSyntaxLiteralExpression*)pcExpression) : mpcUnit->TypeOf(pcExpression);
	if (!psType)
	{
		return false;
	}
	psType = psType->WithoutReference();
	if (psType->IsFloat())
	{
		if (!mpcUnit->FloatConstant(pcExpression, &dValue))
		{
			return false;
		}
		psValue->sValue = mpcFunction->AddConstant(IRTypeOf(psType), (uint64)mpcUnit->FloatBits(psType, dValue));
		psValue->psType = psType;
		return true;
	}
	if (!psType->IsScalar() || !mpcUnit->Constant(pcExpression, &lliValue))
	{
		return false;
	}
	psValue->sValue = Integer(psType, lliValue);
	psValue->psType = psType;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// An argument the program doesn't write: the value the call is given
// (CallWithValue), the characters of a user-defined literal's string or its
// spelling, or a constant.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ImplicitArgument(CCPPSyntaxImplicitArgument* pcArgument, SCPP65IRValue* psValue)
{
	CChars	szSpelling;

	if (mcImplicitValues.Get((int64)(size)pcArgument))
	{
		*psValue = *mcImplicitValues.Get((int64)(size)pcArgument);
		return true;
	}
	if (pcArgument->mpcString)
	{
		if (!StringLiteral(pcArgument->mpcString, psValue))
		{
			return false;
		}
		psValue->psType = pcArgument->mpsType;
		return true;
	}
	if (pcArgument->miSpelling >= 0)
	{
		szSpelling.Init();
		mpcUnit->UserLiteralSpelling(pcArgument->miSpelling, &szSpelling);
		StringData(szSpelling.Text(), szSpelling.Length(), psValue);
		psValue->psType = pcArgument->mpsType;
		szSpelling.Kill();
		return true;
	}
	return ConstantValue(pcArgument, psValue) || Unsupported(__FILE__, __LINE__);
}


//////////////////////////////////////////////////////////////////////////
//
// A user-defined literal: its literal operator called with what it is
// given (CCPPUnit::GetLiteralCall).  [lex.ext]
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::LiteralCall(CCPPSyntax* pcLiteral, SCPP65IRValue* psValue)
{
	SCPPLiteralCall*	psCall;

	psCall = mpcUnit->GetLiteralCall(pcLiteral);
	if (!psCall)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	return Call(psCall->pcFunction, FunctionSymbol(psCall->pcFunction), NULL, &psCall->pcArguments->macpElements, psValue, NULL, false, NULL, false, IRNone());
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
	if (!mpcUnit->DecodeString(pcString, &auiBytes))
	{
		auiBytes.Kill();
		return Unsupported(__FILE__, __LINE__);
	}

	//StringData's data ends with one zero byte.
	auiBytes.GrowToAtLeastNumElements(auiBytes.NumElements() + mpcUnit->StringElementBytes(pcString) - 1, true, 0);
	StringData((char*)auiBytes.GetData(), auiBytes.NumElements(), psValue);
	psValue->psType = mpcTypes->Pointer(mpcTypes->Const(mpcUnit->StringElementType(pcString)));
	auiBytes.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// object.Enumerator and pointer->Enumerator: the object's expression is
// worked out, and the value is the enumerator's.  [expr.ref] 1, 4
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::MemberEnumerator(CCPPSyntaxMemberExpression* pcMember, SCPP65IRValue* psValue)
{
	CCPPSymbol*		pcEnumerator;
	SCPP65IRValue	sObject;
	int64			lliValue;

	pcEnumerator = mpcUnit->MemberEnumerator(pcMember);
	if (!Value(pcMember->mpcObject, &sObject))
	{
		return false;
	}
	psValue->psType = mpcUnit->EnumeratorType(pcEnumerator);
	if (!psValue->psType || !mpcUnit->EnumeratorValue(pcEnumerator, &lliValue))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	psValue->sValue = Integer(psValue->psType, lliValue);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The value of an expression where an integer is wanted (a switch's
// condition, a new array's bound) or, with bPointer, a pointer to an object
// (delete's operand): its own, or what an object's conversion function
// gives (CCPPBinder::ContextualType).  [conv] 5
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ContextualValue(CCPPSyntax* pcExpression, bool bPointer, SCPP65IRValue* psValue)
{
	SCPP65Type*	psTo;

	if (!mpcUnit->IsObject(pcExpression))
	{
		return Operand(pcExpression, psValue);
	}
	psTo = mpcUnit->GetBinder()->ContextualType(pcExpression, bPointer);
	return psTo ? ValueFor(pcExpression, psTo, psValue) : Unsupported(__FILE__, __LINE__);
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

	papcElements = mpcUnit->ConstructElements(pcConstruct);
	if (papcElements && (papcElements->NumElements() == 1))
	{
		return CastExpression(pcConstruct, pcConstruct->mpcType, papcElements->GetPtr(0), psValue);
	}
	psType = mpcUnit->GetDeclarators()->GetTypeIdType(pcConstruct->mpcType);
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
	if (!mpcUnit->FunctionNameText(mpcUnit->GetComponentName(pcId->mpcName->GetLast()), &szName))
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
	pcFound = mpcUnit->GetBinder()->FindAddressedFunction(pcName, lliName, psTarget, pcId, &bOverloaded);
	if (bOverloaded || !pcFound)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	psValue->sValue = FunctionSymbol(pcFound);
	psValue->psType = mpcTypes->Pointer(mpcUnit->FunctionType(pcFound));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// &Class::member: a pointer to a member
// function (the one of psTarget's type if it is overloaded) is its address (a thunk that calls through the vtable for a
// virtual one) and a pointer to a field its offset.  Returns false with
// *pbMember clear if the name isn't a non-static member (it may be a static
// one), and with it set after an Unsupported.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::MemberAddress(CCPPSyntaxIdExpression* pcId, SCPP65Type* psTarget, SCPP65IRValue* psValue, bool* pbMember)
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
	bool				bOverloaded;

	*pbMember = false;
	pcName = pcId->mpcName;
	pcClass = mpcUnit->GetComponentClass(pcName->GetQualifier(), NULL, pcId);
	lliName = mpcUnit->GetFunctionName(pcName->GetLast());
	if (!pcClass || (lliName == -1) || !mpcUnit->CompleteClass(pcClass, pcId))
	{
		return false;
	}

	pcFound = mpcUnit->GetBinder()->FindMemberFunction(pcClass, lliName, psTarget, &pcFoundClass, &bOverloaded);
	if (bOverloaded)
	{
		*pbMember = true;
		return Unsupported(__FILE__, __LINE__);
	}
	if (pcFound)
	{
		*pbMember = true;
		if (pcFound->mbVirtual)
		{
			szThunk.Init();
			mpcUnit->AppendVirtualCallThunk(&szThunk, pcFound);
			sSymbol = mcModule.Symbol(szThunk.Text());
			szThunk.Kill();
		}
		else
		{
			sSymbol = FunctionSymbol(pcFound);
		}
		psValue->sValue = ConvertIR(sSymbol, IRType(IRTK_Pointer), IRType(IRTK_I32), false);
		psValue->psType = mpcTypes->MemberPointer(pcFoundClass, mpcUnit->FunctionType(pcFound));
		return true;
	}

	psField = pcClass->FindField(lliName, &iOffset);
	if (!psField)
	{
		return false;
	}
	*pbMember = true;
	psValue->sValue = mpcFunction->AddInteger(IRType(IRTK_I32), iOffset + CPP_MEMBER_POINTER_BIAS);
	psValue->psType = mpcTypes->MemberPointer(pcClass, psField->psType);
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

	*ppsMember = mpcUnit->TypeOf(pcBinary->mpcRight);
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
	*psObject = BaseAddress(*psObject, pcClass, (*ppsMember)->pcClass, false);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// (object.*function)(...) and (pointer->*function)(...): the object is
// this.  The object and the pointer are worked out before the arguments,
// as the callee is.  [expr.call] 8
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::MemberPointerCall(CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments, CCPPSyntaxBinaryExpression* pcBinary, SCPP65IRValue* psValue)
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
	return Call(NULL, ConvertIR(sFunction.sValue, IRType(IRTK_I32), IRType(IRTK_Pointer), false), psMember->psBase, papcArguments, psValue, NULL, false, NULL, false, sObject);
}


//////////////////////////////////////////////////////////////////////////
//
// object.*field and pointer->*field: the object's address plus the
// field's offset, which the pointer to the member is with its bias.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::MemberPointerPlace(CCPPSyntaxBinaryExpression* pcBinary, SCPP65IRPlace* psPlace)
{
	SCPP65Type*		psMember;
	SCPP65IRValue	sMember;
	SIRReference	sObject;
	SIRReference	sOffset;

	psPlace->psBitField = NULL;

	if (!MemberPointerObject(pcBinary, &sObject, &psMember))
	{
		return false;
	}
	if (psMember->psBase->IsFunction() || !IsPlaceType(psMember->psBase))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (!Value(pcBinary->mpcRight, &sMember))
	{
		return false;
	}
	sOffset = mpcFunction->Add(mpcBlock, IRO_Sub, IRType(IRTK_I32), sMember.sValue, mpcFunction->AddInteger(IRType(IRTK_I32), CPP_MEMBER_POINTER_BIAS));
	psPlace->sAddress = mpcFunction->Add(mpcBlock, IRO_PtrAdd, IRType(IRTK_Pointer), sObject, sOffset);
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
	CChars		szLabel;
	CIRGlobal*	pcGlobal;

	if (mbConstants)
	{
		//A constant's string is data of its own module, which the interpreter reads, with the zero byte the
		//unit's data would end it with (CCPPUnitData::AddString).
		szLabel.Init();
		mpcUnit->GetConstants()->NewName("__string", &szLabel);
		pcGlobal = mcModule.AddGlobal(szLabel.Text(), 1, IRA_Local | IRA_Constant);
		pcGlobal->AddBytes((const uint8*)pcBytes, uiLength);
		pcGlobal->AddZero(1);
		psValue->sValue = mcModule.Symbol(szLabel.Text());
		psValue->psType = mpcTypes->Pointer(&mpcTypes->msChar);
		szLabel.Kill();
		return;
	}

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
// True if the expression is a cast to a reference that is what it casts:
// (T&)x, static_cast<T&&>(x).  dynamic_cast<T&> is not one: it has a
// check to make.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::IsReferenceCast(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	if ((pcExpression->GetKind() != CPPS_CastExpression) && (pcExpression->GetKind() != CPPS_NamedCastExpression))
	{
		return false;
	}
	psType = mpcUnit->DeclaredTypeOf(pcExpression);
	return psType && psType->IsReference();
}


//////////////////////////////////////////////////////////////////////////
//
// The place a cast to a reference, static_cast<T&&>(x), refers to: x's,
// or for an object of another class of its hierarchy that class's part
// (dynamic_cast<T&>'s, DynamicCastPlace), or the object of a derived class
// that x is a base of (DerivedAddress, but for reinterpret_cast).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ReferenceCastPlace(CCPPSyntax* pcCast, SCPP65Type* psReference, SCPP65IRPlace* psPlace)
{
	CCPPSyntax*		pcOperand;
	SCPP65IRPlace	sOperand;

	pcOperand = (pcCast->GetKind() == CPPS_CastExpression) ? ((CCPPSyntaxCastExpression*)pcCast)->mpcOperand : ((CCPPSyntaxNamedCastExpression*)pcCast)->mpcOperand;
	if ((pcCast->GetKind() == CPPS_NamedCastExpression) && (((CCPPSyntaxNamedCastExpression*)pcCast)->meCast == CT_dynamic_cast))
	{
		return DynamicCastPlace(pcCast, psReference, psPlace);
	}
	if (!Place(pcOperand, &sOperand))
	{
		return false;
	}
	psPlace->psType = psReference->psBase;
	psPlace->psBitField = NULL;
	if (!IsReinterpretCast(pcCast) && sOperand.psType->IsClass() && psReference->psBase->IsClass() && (sOperand.psType->pcClass != psReference->psBase->pcClass) &&
		mpcUnit->IsDerived(psReference->psBase->pcClass, sOperand.psType->pcClass, NULL))
	{
		psPlace->sAddress = sOperand.sAddress;
		return DerivedAddress(&psPlace->sAddress, sOperand.psType->pcClass, psReference->psBase->pcClass, false, pcCast);
	}
	return ReferenceAddress(&sOperand, psReference, &psPlace->sAddress);
}


//////////////////////////////////////////////////////////////////////////
//
// dynamic_cast<T&>(x): the T of the object x is part of, found as
// dynamic_cast<T*>(&x) finds it.  If there is none the program stops
// (R_BadCast), as there is nothing to throw.  [expr.dynamic.cast] 9
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::DynamicCastPlace(CCPPSyntax* pcCast, SCPP65Type* psReference, SCPP65IRPlace* psPlace)
{
	CCPPSyntax*		pcOperand;
	SCPP65IRPlace	sOperand;
	CCPPClass*		pcFrom;
	CCPPClass*		pcTo;
	CIRBlock*		pcFailed;
	CIRBlock*		pcFound;
	SIRReference	sCondition;

	pcOperand = ((CCPPSyntaxNamedCastExpression*)pcCast)->mpcOperand;
	if (!psReference->psBase->IsClass() || !Place(pcOperand, &sOperand) || !sOperand.psType->IsClass())
	{
		return Unsupported(__FILE__, __LINE__);
	}
	pcFrom = sOperand.psType->pcClass;
	pcTo = psReference->psBase->pcClass;
	if (!mpcUnit->CompleteClass(pcFrom, NULL) || !mpcUnit->CompleteClass(pcTo, NULL))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	psPlace->psType = psReference->psBase;
	psPlace->psBitField = NULL;
	if (mpcUnit->IsDerived(pcFrom, pcTo, NULL))
	{
		return ReferenceAddress(&sOperand, psReference, &psPlace->sAddress);
	}
	if (!pcFrom->IsPolymorphic())
	{
		mpcUnit->ReportError(pcCast, "The operand of dynamic_cast is not polymorphic");
		return Unsupported(__FILE__, __LINE__);
	}

	psPlace->sAddress = DynamicCast(sOperand.sAddress, pcFrom, pcTo);
	pcFailed = NewBlock();
	pcFound = NewBlock();
	sCondition = mpcFunction->Add(mpcBlock, IRO_Eq, IRType(IRTK_Pointer), psPlace->sAddress, mpcFunction->AddConstant(IRType(IRTK_Pointer), 0));
	mpcBlock->Branch(sCondition, pcFailed, pcFound);
	StartBlock(pcFailed);
	mpcUnit->UseBadCast();
	mpcFunction->Call(mpcBlock, IRType(IRTK_None), mcModule.Symbol("R_BadCast"));
	JumpTo(pcFound);
	StartBlock(pcFound);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// typeid's std::type_info: the type info of its type (CCPPUnit::
// AppendTypeidLabel), or for an lvalue of a polymorphic class its object's
// own, which its vtable points to (a null one, *p of a null p, stops the
// program at R_BadCast, there being no bad_typeid to throw).  [expr.typeid]
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::TypeidPlace(CCPPSyntaxTypeidExpression* pcTypeid, SCPP65IRPlace* psPlace)
{
	SCPP65Type*		psType;
	SCPP65IRPlace	sObject;
	SIRReference	sVtable;
	SIRReference	sCondition;
	CIRBlock*		pcNull;
	CIRBlock*		pcObject;
	CChars			szLabel;
	bool			bDynamic;

	psType = mpcUnit->TypeidType(pcTypeid, &bDynamic);
	if (!psType || !mpcUnit->TypeInfoClass())
	{
		return Unsupported(__FILE__, __LINE__);
	}
	psPlace->psType = mpcTypes->Const(mpcUnit->TypeInfoClass()->mpsType);
	psPlace->psBitField = NULL;
	if (bDynamic)
	{
		if (!Place(pcTypeid->mpcOperand, &sObject))
		{
			return false;
		}
		pcNull = NewBlock();
		pcObject = NewBlock();
		sCondition = mpcFunction->Add(mpcBlock, IRO_Eq, IRType(IRTK_Pointer), sObject.sAddress, mpcFunction->AddConstant(IRType(IRTK_Pointer), 0));
		mpcBlock->Branch(sCondition, pcNull, pcObject);
		StartBlock(pcNull);
		mpcUnit->UseBadCast();
		mpcFunction->Call(mpcBlock, IRType(IRTK_None), mcModule.Symbol("R_BadCast"));
		JumpTo(pcObject);
		StartBlock(pcObject);
		sVtable = mpcFunction->Load(mpcBlock, IRType(IRTK_Pointer), AddressAt(sObject.sAddress, sObject.psType->pcClass->GetVtablePointerOffset()));
		psPlace->sAddress = mpcFunction->Load(mpcBlock, IRType(IRTK_Pointer), AddressAt(sVtable, CPP65_VTABLE_TYPE_INFO_OFFSET));
		return true;
	}
	//An expression that isn't an lvalue of a polymorphic class isn't worked out.  [expr.typeid] 4
	szLabel.Init();
	mpcUnit->AppendTypeidLabel(&szLabel, psType);
	psPlace->sAddress = mcModule.Symbol(szLabel.Text());
	if (psType->IsClass() && psType->pcClass->IsUnnamed())
	{
		mcModule.SetLocal(psPlace->sAddress.Index());
	}
	szLabel.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Variables, *pointer, array[index], and calls that return references.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Place(CCPPSyntax* pcExpression, SCPP65IRPlace* psPlace)
{
	CCPPBoundName					cBound;
	CCPPSyntaxIdExpression*			pcId;
	CCPPSyntaxUnaryExpression*		pcUnary;
	CCPPSyntaxBinaryExpression*		pcBinary;
	CCPPSyntaxSubscriptExpression*	pcSubscript;
	SCPP65IRValue					sPointer;
	SCPP65IRValue					sIndex;
	SCPP65IRValue					sAddress;
	SCPP65Type*						psType;
	SCPP65Type*						psArray;

	psPlace->psBitField = NULL;

	pcExpression = mpcUnit->SkipParentheses(pcExpression);
	switch (pcExpression->GetKind())
	{
	case CPPS_IdExpression:
		pcId = (CCPPSyntaxIdExpression*)pcExpression;
		mpcUnit->GetBinder()->BindName(pcId, &cBound);
		if (cBound.meWhat == CPPBN_Variable)
		{
			return VariablePlace(cBound.mpcVariable, psPlace);
		}
		if (cBound.meWhat == CPPBN_TemplateArgument)
		{
			return Place(cBound.mpcArgument, psPlace);
		}
		if (cBound.meWhat == CPPBN_Function)
		{
			//A function's name is the function, an lvalue: what a reference to a function refers to.
			if (!FunctionAddress(pcId, NULL, &sAddress))
			{
				return false;
			}
			psPlace->sAddress = sAddress.sValue;
			psPlace->psType = sAddress.psType->psBase;
			return true;
		}
		if (cBound.meWhat == CPPBN_Capture)
		{
			//What a lambda captures is its closure's field (a reference's is what it refers to).
			return !msClosure.IsNone() ? FieldAt(msClosure, cBound.mpsField, cBound.miOffset, psPlace) : Unsupported(__FILE__, __LINE__);
		}
		//A field of this (or of the base a qualifier names), where there is one.
		if ((cBound.meWhat != CPPBN_Field) || msThis.IsNone() || !mpcUnit->CompleteClass(cBound.mpcClass, NULL))
		{
			return Unsupported(__FILE__, __LINE__);
		}
		return BoundFieldAt(msThis, &cBound, psPlace);

	case CPPS_MemberExpression:
		return MemberPlace((CCPPSyntaxMemberExpression*)pcExpression, psPlace);

	case CPPS_UnaryExpression:
		pcUnary = (CCPPSyntaxUnaryExpression*)pcExpression;
		if (((pcUnary->meOperator == CT_Increment) || (pcUnary->meOperator == CT_Decrement)) && !pcUnary->mbPostfix && !mpcUnit->IsObject(pcUnary->mpcOperand))
		{
			//++x and --x are x, once it is stepped.  [expr.pre.incr] 1
			return IncrementExpression(pcUnary, &sAddress, psPlace);
		}
		if (pcUnary->meOperator != CT_Multiply)
		{
			//-object, +object, ~object: the temporary its operator made.
			psType = mpcUnit->TypeOf(pcExpression);
			if (!psType || !psType->IsClass() || !ObjectValue(pcExpression, &sAddress))
			{
				return Unsupported(__FILE__, __LINE__);
			}
			psPlace->sAddress = sAddress.sValue;
			psPlace->psType = sAddress.psType;
			return true;
		}
		if (mpcUnit->IsObject(pcUnary->mpcOperand))
		{
			//*object: what its operator's reference refers to, or the object it returns.
			if (!OperatorExpression(CT_Multiply, pcUnary->mpcOperand, NULL, &sAddress))
			{
				return false;
			}
			psPlace->sAddress = sAddress.sValue;
			psPlace->psType = sAddress.psType->WithoutReference();
			return sAddress.psType->IsReference() || sAddress.psType->IsClass() || Unsupported(__FILE__, __LINE__);
		}
		if (!Value(pcUnary->mpcOperand, &sPointer))
		{
			return false;
		}
		//*pointer; a function's is the function, which is its pointer again as a value, so (**f)() is f().
		if (!sPointer.psType->IsPointer() || sPointer.psType->psBase->IsVoid() || !(IsPlaceType(sPointer.psType->psBase) || sPointer.psType->psBase->IsFunction()))
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
		if (!mpcUnit->TypeOf(pcSubscript->mpcArray))
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

	case CPPS_StringExpression:
		if (mpcUnit->IsStringLiteral(pcExpression))
		{
			//The array of its characters, where its data is.
			if (!StringLiteral(pcExpression, &sAddress))
			{
				return false;
			}
			psPlace->sAddress = sAddress.sValue;
			psPlace->psType = mpcUnit->StringArrayType(pcExpression);
			return true;
		}
	case CPPS_CallExpression:
	case CPPS_LiteralExpression:
		//What a call (or a user-defined literal's) that returns a reference refers to, or the object it makes.
		psType = mpcUnit->DeclaredTypeOf(pcExpression);
		if (!psType || !(psType->IsReference() || psType->IsClass()) || ((pcExpression->GetKind() != CPPS_CallExpression) && !mpcUnit->UserLiteralToken(pcExpression)))
		{
			return Unsupported(__FILE__, __LINE__);
		}
		if ((pcExpression->GetKind() == CPPS_CallExpression) ? !CallExpression((CCPPSyntaxCallExpression*)pcExpression, &sAddress) : !LiteralCall(pcExpression, &sAddress))
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

	case CPPS_TypeidExpression:
		return TypeidPlace((CCPPSyntaxTypeidExpression*)pcExpression, psPlace);

	case CPPS_ConstructExpression:
		if (!ObjectValue(pcExpression, &sAddress))
		{
			return false;
		}
		psPlace->sAddress = sAddress.sValue;
		psPlace->psType = sAddress.psType;
		return true;

	case CPPS_BinaryExpression:
		if (mpcUnit->IsMemberPointerOperator(pcExpression))
		{
			return MemberPointerPlace((CCPPSyntaxBinaryExpression*)pcExpression, psPlace);
		}

		//(x, y) is y, once x is worked out.  [expr.comma] 1
		pcBinary = (CCPPSyntaxBinaryExpression*)pcExpression;
		if ((pcBinary->meOperator == CT_Comma) && !mpcUnit->IsOverloadedComma(pcBinary))
		{
			return Value(pcBinary->mpcLeft, &sAddress) && Place(pcBinary->mpcRight, psPlace);
		}

		//(x = y) and (x op= y) of a scalar are x's place after the assignment.
		psType = mpcUnit->TypeOf(((CCPPSyntaxBinaryExpression*)pcExpression)->mpcLeft);
		if (mpcUnit->IsAssignmentOperator(((CCPPSyntaxBinaryExpression*)pcExpression)->meOperator) && psType && !psType->IsClass() &&
			!(psType->IsReference() && psType->psBase->IsClass()))
		{
			return Assignment((CCPPSyntaxBinaryExpression*)pcExpression, &sAddress, psPlace);
		}

		//An operator function's object (a temporary it made, or the object it refers to).
		psType = mpcUnit->TypeOf(pcExpression);
		if (!psType || !(psType->IsReference() || psType->IsClass()) || !Value(pcExpression, &sAddress))
		{
			return Unsupported(__FILE__, __LINE__);
		}
		psPlace->sAddress = sAddress.sValue;
		psPlace->psType = sAddress.psType->WithoutReference();
		return psPlace->psType->IsClass() || Unsupported(__FILE__, __LINE__);

	case CPPS_CastExpression:
	case CPPS_NamedCastExpression:
		if (IsReferenceCast(pcExpression))
		{
			return ReferenceCastPlace(pcExpression, mpcUnit->DeclaredTypeOf(pcExpression), psPlace);
		}
		//Any other cast is the object it makes, as below.
	case CPPS_ConditionalExpression:
		psType = mpcUnit->TypeOf(pcExpression);
		if ((pcExpression->GetKind() == CPPS_ConditionalExpression) && psType && !psType->IsClass())
		{
			return ConditionalPlace((CCPPSyntaxConditionalExpression*)pcExpression, psPlace);
		}
		if (!psType || !psType->IsClass() || !ObjectValue(pcExpression, &sAddress))
		{
			return Unsupported(__FILE__, __LINE__);
		}
		psPlace->sAddress = sAddress.sValue;
		psPlace->psType = sAddress.psType;
		return true;

	case CPPS_LambdaExpression:
		//Its closure (a temporary), called at once: [](int a) { ... }(1).
		if (!ObjectValue(pcExpression, &sAddress))
		{
			return false;
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
		mpcUnit->AppendVariableLabel(&szLabel, pcVariable);
		psPlace->sAddress = mcModule.Symbol(szLabel.Text());
		if (mbConstants && (pcVariable->meStorage == CPPVS_Global))
		{
			//The interpreter may ask what its data is.  Only a global's can be a constant's (IsConstantVariable): a
			//static local's variable is freed once its function is written, so it isn't kept.
			mpcUnit->GetConstants()->NoteVariable(szLabel.Text(), pcVariable);
		}
		szLabel.Kill();
		if (pcVariable->mbRestrict)
		{
			mcModule.SetRestrict(psPlace->sAddress.Index());
		}
		if (pcVariable->mbInternal || ((pcVariable->meStorage == CPPVS_Static) && !pcVariable->mpcSharedIn))
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
		psType = mpcUnit->ArrowType(pcObject, pcObject);
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
	CCPPBoundName	cBound;
	SCPP65Type*		psType;
	SIRReference	sObject;

	psPlace->psBitField = NULL;

	psType = mpcUnit->TypeOf(pcMember->mpcObject);
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
	if (!mpcUnit->GetBinder()->BindMember(psType->pcClass, pcMember->mpcName, pcMember, &cBound))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (!((cBound.meWhat == CPPBN_Field) ? BoundFieldAt(sObject, &cBound, psPlace) : VariablePlace(cBound.mpcVariable, psPlace)))
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
// The field a name or object.name is bound to, in the object at sObject: in
// it or the base its qualifier names (CCPPBoundName::mpcQualifier), or in
// the virtual base of that the binder found it in (mpcVirtualBase), which is
// where that class's vtable part says (not by a path from the object's
// class, which another base of the same class may be on).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::BoundFieldAt(SIRReference sObject, CCPPBoundName* pcBound, SCPP65IRPlace* psPlace)
{
	CCPPClass*	pcClass;

	pcClass = pcBound->mpcClass;
	if (pcBound->mpcQualifier)
	{
		sObject = BaseAddress(sObject, pcClass, pcBound->mpcQualifier, false);
		pcClass = pcBound->mpcQualifier;
	}
	if (pcBound->mpcVirtualBase)
	{
		sObject = VirtualBaseAddress(sObject, pcClass, pcBound->mpcVirtualBase);
	}
	return FieldAt(sObject, pcBound->mpsField, pcBound->miOffset, psPlace);
}


//////////////////////////////////////////////////////////////////////////
//
// The place of a field at iOffset in the object at sObject: what a reference field
// refers to.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::FieldAt(SIRReference sObject, SCPPField* psField, int iOffset, SCPP65IRPlace* psPlace)
{
	psPlace->psBitField = NULL;
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

	psType = mpcUnit->ValueTypeOf(pcSubscript->mpcArray);
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
	pcArray = mpcUnit->SkipParentheses(pcArray);
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
				 !mpcUnit->IsFunctionName((CCPPSyntaxIdExpression*)pcArray);
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

	if (!mpcUnit->GetTarget()->NarrowsSubscripts() || !psArray->IsArray() || (psArray->iCount < 2) || (psArray->iBytes >= 0x10000) ||
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
	pcTemplate = mpcUnit->GetTemplate(pcId->mpcName->mpcSymbol);
	if (!pcTemplate || !pcComponent->mbTemplateArguments)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	pcFunction = mpcUnit->InstantiateFunction(pcTemplate, &pcComponent->macpTemplateArguments, NULL, pcId);
	if (!pcFunction)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	psValue->sValue = FunctionSymbol(pcFunction);
	psValue->psType = mpcTypes->Pointer(mpcUnit->FunctionType(pcFunction));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The symbol of a function's label; the generator writes the function.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CCPP65IRFrontEnd::FunctionSymbol(CCPPFunction* pcFunction, bool bBaseVersion)
{
	CChars			szLabel;
	SIRReference	sSymbol;

	szLabel.Init();
	mpcUnit->GetMangler()->AppendFunctionLabel(&szLabel, pcFunction, bBaseVersion);
	if (mbConstants)
	{
		//A constant's call is run when the program is compiled: the function is only asked for there.
		mpcUnit->GetConstants()->NoteFunction(szLabel.Text(), pcFunction);
	}
	else
	{
		mpcUnit->RequireFunction(pcFunction);
	}
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
