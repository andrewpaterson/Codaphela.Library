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


//The IR front end's conversions: casts, dynamic_cast, conversions between types, base class addresses and member
//pointers.


//////////////////////////////////////////////////////////////////////////
//
// (T)x and the named casts, to scalars and void.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CastExpression(CCPPSyntaxTypeId* pcTypeId, CCPPSyntax* pcOperand, SCPP65IRValue* psValue)
{
	SCPP65Type*		psTo;
	CCPPSyntax*		pcFunction;

	psTo = mpcGenerator->GetDeclarators()->GetTypeIdType(pcTypeId);
	if (!psTo || psTo->IsClass() || psTo->IsReference())
	{
		return Unsupported(__FILE__, __LINE__);
	}
	pcFunction = mpcGenerator->FunctionNameFor(pcOperand, psTo);
	if (pcFunction)
	{
		//(FPointer)&f chooses the overload of f with the pointer's type.  (MPointer)&Class::f is a member pointer
		//converted below, (void*)&f the function's address.
		if (!FunctionAddress((CCPPSyntaxIdExpression*)pcFunction, psTo->psBase, psValue))
		{
			return Unsupported(__FILE__, __LINE__);
		}
		psValue->psType = psTo;
		return true;
	}
	if (!Value(pcOperand, psValue))
	{
		return false;
	}
	if (psTo->IsVoid())
	{
		psValue->psType = psTo;
		return true;
	}

	//(int32)((ui + 1) * 7) still multiplies in ui's width.
	if (psTo->IsInteger() && psValue->psType && psValue->psType->IsInteger())
	{
		mpcGenerator->WideningArithmetic(pcOperand, psValue->psType, psTo);
	}
	return psTo->IsScalar() ? Convert(psValue, psTo) : Unsupported(__FILE__, __LINE__);
}


//////////////////////////////////////////////////////////////////////////
//
// dynamic_cast<T*>(p) and dynamic_cast<void*>(p).  A cast to the class or one
// of its bases is a conversion; any other looks up the object's type info.
// dynamic_cast<T&> (which would need exceptions) is not supported.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::DynamicCastExpression(CCPPSyntaxNamedCastExpression* pcCast, SCPP65IRValue* psValue)
{
	SCPP65Type*		psTo;
	CCPPClass*	pcFrom;
	CCPPClass*	pcTarget;

	psTo = mpcGenerator->GetDeclarators()->GetTypeIdType(pcCast->mpcType);
	if (!psTo)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (psTo->IsReference())
	{
		mpcGenerator->ReportError(pcCast, "dynamic_cast to a reference is not supported");
		return Unsupported(__FILE__, __LINE__);
	}
	if (!psTo->IsPointer() || !(psTo->psBase->IsClass() || psTo->psBase->IsVoid()))
	{
		mpcGenerator->ReportError(pcCast, "dynamic_cast needs a pointer to a class or to void");
		return Unsupported(__FILE__, __LINE__);
	}
	if (!Value(pcCast->mpcOperand, psValue))
	{
		return false;
	}
	if (!psValue->psType->IsPointer() || !psValue->psType->psBase->IsClass())
	{
		mpcGenerator->ReportError(pcCast, "The operand of dynamic_cast must be a pointer to a class");
		return Unsupported(__FILE__, __LINE__);
	}

	pcFrom = psValue->psType->psBase->pcClass;
	if (!mpcGenerator->CompleteClass(pcFrom, pcCast))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	pcTarget = NULL;
	if (psTo->psBase->IsClass())
	{
		pcTarget = psTo->psBase->pcClass;
		if (!mpcGenerator->CompleteClass(pcTarget, pcCast))
		{
			return Unsupported(__FILE__, __LINE__);
		}
		if (mpcGenerator->IsDerived(pcFrom, pcTarget, NULL))
		{
			return Convert(psValue, psTo);
		}
	}
	if (!pcFrom->IsPolymorphic())
	{
		mpcGenerator->ReportError(pcCast, "The operand of dynamic_cast is not polymorphic");
		return Unsupported(__FILE__, __LINE__);
	}

	psValue->sValue = DynamicCast(psValue->sValue, pcFrom, pcTarget);
	psValue->psType = psTo;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The object sAddress (a psFrom, NULL stays NULL) as a psTo, NULL if it isn't
// one, or ambiguously one.  A NULL psTo gives the start of the whole object
// (dynamic_cast<void*>).  The vtable pointer finds the start and the type info
// (its part's header, CPP65_VTABLE_TOP_OFFSET); the type info lists every subobject.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CCPP65IRFrontEnd::DynamicCast(SIRReference sAddress, CCPPClass* pcFrom, CCPPClass* pcTo)
{
	CIRBlock*		pcStart;
	CIRBlock*		pcLook;
	CIRBlock*		pcLoop;
	CIRBlock*		pcCheck;
	CIRBlock*		pcMatch;
	CIRBlock*		pcFirst;
	CIRBlock*		pcNext;
	CIRBlock*		pcEnd;
	SIRType			sPointer;
	SIRType			sOffset;
	SIRReference	sNullPointer;
	SIRReference	sCondition;
	SIRReference	sVtablePointer;
	SIRReference	sVtable;
	SIRReference	sTop;
	SIRReference	sInfo;
	SIRReference	sTarget;
	SIRReference	sEntry;
	SIRReference	sFound;
	SIRReference	sClass;
	SIRReference	sCandidate;
	SIRReference	sNextEntry;
	SIRReference	sNextFound;
	SIRReference	sResult;
	SIRPhi*			psPhi;
	CChars			szLabel;

	sPointer = IRType(IRTK_Pointer);
	sOffset = IRIntegerType(mcModule.GetPointerSize(0));
	sNullPointer = mpcFunction->AddConstant(sPointer, 0);
	pcLook = NewBlock();
	pcEnd = NewBlock();

	pcStart = mpcBlock;
	sCondition = mpcFunction->Add(mpcBlock, IRO_Eq, sPointer, sAddress, sNullPointer);
	mpcBlock->Branch(sCondition, pcEnd, pcLook);

	//The start of the whole object.
	StartBlock(pcLook);
	sVtablePointer = AddressAt(sAddress, pcFrom->GetVtablePointerOffset());
	sVtable = mpcFunction->Load(mpcBlock, sPointer, sVtablePointer);
	sTop = ConvertIR(mpcFunction->Load(mpcBlock, IRIntegerType(CPP65_VTABLE_ENTRY_BYTES), AddressAt(sVtable, CPP65_VTABLE_TOP_OFFSET)), IRIntegerType(CPP65_VTABLE_ENTRY_BYTES), sOffset, true);
	sTop = mpcFunction->Add(mpcBlock, IRO_PtrAdd, sPointer, sVtablePointer, sTop);
	if (!pcTo)
	{
		JumpTo(pcEnd);
		StartBlock(pcEnd);
		return Phi(pcEnd, sPointer, pcStart, sNullPointer, pcLook, sTop);
	}

	mpcGenerator->EnsureTypeInfo(pcTo);
	szLabel.Init();
	mpcGenerator->GetMangler()->AppendTypeInfoLabel(&szLabel, pcTo);
	sTarget = mcModule.Symbol(szLabel.Text());
	szLabel.Kill();
	if (pcTo->IsUnnamed())
	{
		mcModule.SetLocal(sTarget.Index());
	}
	sInfo = mpcFunction->Load(mpcBlock, sPointer, AddressAt(sVtable, CPP65_VTABLE_TYPE_INFO_OFFSET));

	//Each entry is a subobject's class's type info then its offset; a zero ends them.  sFound is the match so far.
	pcLoop = NewBlock();
	pcCheck = NewBlock();
	pcMatch = NewBlock();
	pcFirst = NewBlock();
	pcNext = NewBlock();
	JumpTo(pcLoop);

	StartBlock(pcLoop);
	sEntry = LoopPhi(pcLoop, sPointer, pcLook, sInfo);
	sFound = LoopPhi(pcLoop, sPointer, pcLook, sNullPointer);
	sClass = mpcFunction->Load(mpcBlock, sPointer, sEntry);
	sCondition = mpcFunction->Add(mpcBlock, IRO_Eq, sPointer, sClass, sNullPointer);
	mpcBlock->Branch(sCondition, pcEnd, pcCheck);

	StartBlock(pcCheck);
	sCondition = mpcFunction->Add(mpcBlock, IRO_Eq, sPointer, sClass, sTarget);
	mpcBlock->Branch(sCondition, pcMatch, pcNext);

	//A second match: the class is an ambiguous base of the object.
	StartBlock(pcMatch);
	sCondition = mpcFunction->Add(mpcBlock, IRO_Ne, sPointer, sFound, sNullPointer);
	mpcBlock->Branch(sCondition, pcEnd, pcFirst);

	StartBlock(pcFirst);
	sCandidate = ConvertIR(mpcFunction->Load(mpcBlock, IRIntegerType(CPP65_VTABLE_ENTRY_BYTES), AddressAt(sEntry, CPP65_TYPE_INFO_OFFSET_OFFSET)), IRIntegerType(CPP65_VTABLE_ENTRY_BYTES), sOffset, true);
	sCandidate = mpcFunction->Add(mpcBlock, IRO_PtrAdd, sPointer, sTop, sCandidate);
	JumpTo(pcNext);

	StartBlock(pcNext);
	sNextFound = Phi(pcNext, sPointer, pcCheck, sFound, pcFirst, sCandidate);
	sNextEntry = AddressAt(sEntry, CPP65_TYPE_INFO_ENTRY_BYTES);
	JumpTo(pcLoop);
	ClosePhi(pcLoop, sEntry, pcNext, sNextEntry);
	ClosePhi(pcLoop, sFound, pcNext, sNextFound);

	StartBlock(pcEnd);
	sResult = mpcFunction->AddTemporary(sPointer);
	psPhi = pcEnd->AddPhi(sPointer, sResult);
	pcEnd->AddPhiArgument(psPhi, pcStart, sNullPointer);
	pcEnd->AddPhiArgument(psPhi, pcLoop, sFound);
	pcEnd->AddPhiArgument(psPhi, pcMatch, sNullPointer);
	return sResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Converts a value to psTo.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Convert(SCPP65IRValue* psValue, SCPP65Type* psTo)
{
	SCPP65Type*		psFrom;
	SIRType			sFrom;
	SIRType			sTo;
	SIRReference	sCondition;
	int				iOffset;

	psFrom = psValue->psType;
	if (psTo->IsVoid())
	{
		psValue->psType = psTo;
		return true;
	}
	if (IsObjectValue(psValue) && psTo->IsScalar())
	{
		return ConvertObject(psValue, psTo);
	}
	if (psFrom->IsMemberPointer() || psTo->IsMemberPointer())
	{
		return ConvertMemberPointer(psValue, psTo);
	}
	if (psFrom->IsVoid() || !psFrom->IsScalar() || !psTo->IsScalar())
	{
		return Unsupported(__FILE__, __LINE__);
	}
	//A pointer to a derived class points at its base, which may be further on (a null pointer stays null).  The
	//derived class is laid out to know where its base is, even if nothing else has needed it whole yet.
	if (psFrom->IsPointer() && psTo->IsPointer() && psFrom->psBase->IsClass() && psTo->psBase->IsClass() && (psFrom->psBase->pcClass != psTo->psBase->pcClass) &&
		mpcGenerator->IsDerived(psFrom->psBase->pcClass, psTo->psBase->pcClass, NULL) && mpcGenerator->CompleteClass(psFrom->psBase->pcClass, NULL) &&
		mpcGenerator->IsDerived(psFrom->psBase->pcClass, psTo->psBase->pcClass, &iOffset) && (iOffset != 0))
	{
		psValue->sValue = BaseAddress(psValue->sValue, iOffset);
		psValue->psType = psTo;
		return true;
	}

	sFrom = IRTypeOf(psFrom);
	sTo = IRTypeOf(psTo);
	if ((psTo->eKind == CPP65TK_Bool) && (psFrom->eKind != CPP65TK_Bool))
	{
		if (!Truth(psValue, &sCondition))
		{
			return false;
		}
		psValue->sValue = mpcFunction->Add(mpcBlock, IRO_ZExt, sTo, sCondition);
		psValue->psType = psTo;
		return true;
	}

	if (psFrom->IsFloat() || psTo->IsFloat())
	{
		if (psFrom->IsPointer() || psTo->IsPointer())
		{
			return Unsupported(__FILE__, __LINE__);
		}
		if (psFrom->IsFloat() && psTo->IsFloat())
		{
			if (!sFrom.Equals(sTo))
			{
				psValue->sValue = mpcFunction->Add(mpcBlock, sTo.Is(IRTK_F64) ? IRO_FExt : IRO_FTrunc, sTo, psValue->sValue);
			}
		}
		else if (psTo->IsFloat())
		{
			psValue->sValue = mpcFunction->Add(mpcBlock, (psFrom->bSigned && (psFrom->eKind == CPP65TK_Integer)) ? IRO_SIToF : IRO_UIToF, sTo, psValue->sValue);
		}
		else
		{
			psValue->sValue = mpcFunction->Add(mpcBlock, psTo->bSigned ? IRO_FToSI : IRO_FToUI, sTo, psValue->sValue);
		}
		psValue->psType = psTo;
		return true;
	}

	psValue->sValue = ConvertIR(psValue->sValue, sFrom, sTo, psFrom->bSigned && (psFrom->eKind == CPP65TK_Integer));
	psValue->psType = psTo;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Between integers and pointers: wider is extended (signed or not), narrower
// truncated, the same size reinterpreted.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CCPP65IRFrontEnd::ConvertIR(SIRReference sValue, SIRType sFrom, SIRType sTo, bool bSigned)
{
	SIRType	sFromInteger;
	SIRType	sToInteger;

	if (sFrom.Equals(sTo))
	{
		return sValue;
	}
	sFromInteger = sFrom.IsPointer() ? IRIntegerType(mcModule.GetPointerSize(sFrom.uiSpace)) : sFrom;
	sToInteger = sTo.IsPointer() ? IRIntegerType(mcModule.GetPointerSize(sTo.uiSpace)) : sTo;
	if (sFrom.IsPointer())
	{
		sValue = mpcFunction->Add(mpcBlock, IRO_PtrToI, sFromInteger, sValue);
	}
	if (sToInteger.IntegerBits() > sFromInteger.IntegerBits())
	{
		sValue = mpcFunction->Add(mpcBlock, bSigned ? IRO_SExt : IRO_ZExt, sToInteger, sValue);
	}
	else if (sToInteger.IntegerBits() < sFromInteger.IntegerBits())
	{
		sValue = mpcFunction->Add(mpcBlock, IRO_Trunc, sToInteger, sValue);
	}
	if (sTo.IsPointer())
	{
		sValue = mpcFunction->Add(mpcBlock, IRO_IToPtr, sTo, sValue);
	}
	return sValue;
}


//////////////////////////////////////////////////////////////////////////
//
// The address of a derived object's base iOffset bytes in; null stays
// null.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CCPP65IRFrontEnd::BaseAddress(SIRReference sAddress, int iOffset)
{
	CIRBlock*		pcAdd;
	CIRBlock*		pcEnd;
	CIRBlock*		pcFrom;
	SIRReference	sNull;
	SIRReference	sMoved;

	pcAdd = NewBlock();
	pcEnd = NewBlock();
	pcFrom = mpcBlock;
	sNull = mpcFunction->Add(mpcBlock, IRO_Eq, IRType(IRTK_Pointer), sAddress, mpcFunction->AddConstant(IRType(IRTK_Pointer), 0));
	mpcBlock->Branch(sNull, pcEnd, pcAdd);
	StartBlock(pcAdd);
	sMoved = AddressAt(sAddress, iOffset);
	JumpTo(pcEnd);
	StartBlock(pcEnd);
	return Phi(pcEnd, IRType(IRTK_Pointer), pcFrom, sAddress, pcAdd, sMoved);
}


//////////////////////////////////////////////////////////////////////////
//
// Conversions to and from pointers to members (an address or an offset, 4
// bytes): an integer (0, null) is one as it is, one
// is a bool when it isn't 0, a pointer to a field of a base is one to the
// derived class's field (the base's offset added) and back again by a cast
// (taken off), and other pointers to members (to members of unrelated
// classes, by a cast) keep the value.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ConvertMemberPointer(SCPP65IRValue* psValue, SCPP65Type* psTo)
{
	SCPP65Type*		psFrom;
	SIRReference	sCondition;
	int				iOffset;

	psFrom = psValue->psType;
	if (psTo->IsMemberPointer() && psFrom->IsInteger())
	{
		psValue->sValue = ConvertIR(psValue->sValue, IRTypeOf(psFrom), IRTypeOf(psTo), psFrom->bSigned);
	}
	else if (psFrom->IsMemberPointer() && (psTo->eKind == CPP65TK_Bool))
	{
		if (!Truth(psValue, &sCondition))
		{
			return false;
		}
		psValue->sValue = ConvertIR(sCondition, IRType(IRTK_I1), IRTypeOf(psTo), false);
	}
	else if (psFrom->IsMemberPointer() && psTo->IsMemberPointer() && !psFrom->Equals(psTo))
	{
		//Base to derived adds the base's offset, derived to base takes it off.  Otherwise (a cast between unrelated
		//classes, say) it is the same value.
		iOffset = BaseOffset(psTo->pcClass, psFrom->pcClass) - BaseOffset(psFrom->pcClass, psTo->pcClass);
		if ((iOffset != 0) && psFrom->psBase->IsFunction())
		{
			return Unsupported(__FILE__, __LINE__);
		}
		if (iOffset != 0)
		{
			psValue->sValue = mpcFunction->Add(mpcBlock, IRO_Add, IRType(IRTK_I32), psValue->sValue, mpcFunction->AddInteger(IRType(IRTK_I32), iOffset));
		}
	}
	else if (!psFrom->Equals(psTo))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	psValue->psType = psTo;
	return true;
}
