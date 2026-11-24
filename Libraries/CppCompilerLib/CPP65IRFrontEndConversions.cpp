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


//The IR front end's conversions: casts, dynamic_cast, conversions between types, base class addresses and member
//pointers.


//////////////////////////////////////////////////////////////////////////
//
// (T)x, T(x) and the named casts (pcCast), to scalars and void.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CastExpression(CCPPSyntax* pcCast, CCPPSyntaxTypeId* pcTypeId, CCPPSyntax* pcOperand, SCPP65IRValue* psValue)
{
	SCPP65Type*		psTo;
	bool			bAddress;

	psTo = mpcUnit->GetDeclarators()->GetTypeIdType(pcTypeId);
	if (!psTo || psTo->IsClass() || psTo->IsReference())
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (mbConstants && IsReinterpretCast(pcCast))
	{
		//[expr.const] 2.13
		mpcUnit->GetConstants()->SetReason("It uses reinterpret_cast");
		return Unsupported(__FILE__, __LINE__);
	}

	//(FPointer)&f and (MPointer)&Class::f choose the overload of f with the pointer's type; (void*)&f is the function's address.
	if (!OverloadAddress(pcOperand, psTo, psValue, &bAddress))
	{
		return false;
	}
	if (bAddress)
	{
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
		mpcUnit->GetWarner()->WideningArithmetic(pcOperand, psValue->psType, psTo);
	}

	//A base's pointer cast to a derived class's (but by reinterpret_cast) points at the derived object.
	if (!IsReinterpretCast(pcCast) && psTo->IsPointer() && psValue->psType && psValue->psType->IsPointer() && psTo->psBase->IsClass() && psValue->psType->psBase->IsClass() &&
		(psTo->psBase->pcClass != psValue->psType->psBase->pcClass) && mpcUnit->IsDerived(psTo->psBase->pcClass, psValue->psType->psBase->pcClass, NULL))
	{
		if (!DerivedAddress(&psValue->sValue, psValue->psType->psBase->pcClass, psTo->psBase->pcClass, true, pcCast))
		{
			return false;
		}
		psValue->psType = psTo;
		return true;
	}
	return psTo->IsScalar() ? Convert(psValue, psTo) : Unsupported(__FILE__, __LINE__);
}


//////////////////////////////////////////////////////////////////////////
//
// static_cast<To*>(from) (or to To&, or a cast that is one): the address of
// the pcTo object whose pcFrom base is at *psAddress, a fixed distance
// back (a null pointer stays null with bNull).  A virtual base is nowhere
// fixed in the class, so a cast from one is an error.  [expr.static.cast]
// 2, 11
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::DerivedAddress(SIRReference* psAddress, CCPPClass* pcFrom, CCPPClass* pcTo, bool bNull, CCPPSyntax* pcCast)
{
	int		iOffset;

	if (!mpcUnit->GetClasses()->CompleteClass(pcTo, NULL) || !mpcUnit->IsDerived(pcTo, pcFrom, &iOffset))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (iOffset == CPP_VIRTUAL_PATH)
	{
		mpcDiagnostics->ReportOnce(pcCast, "A pointer or reference to a virtual base can't be cast to a class derived from it");
		return Unsupported(__FILE__, __LINE__);
	}
	if (iOffset != 0)
	{
		*psAddress = bNull ? BaseAddress(*psAddress, -iOffset) : AddressAt(*psAddress, -iOffset);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the cast is reinterpret_cast<T>(x), which moves no pointer.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::IsReinterpretCast(CCPPSyntax* pcCast)
{
	return (pcCast->GetKind() == CPPS_NamedCastExpression) && (((CCPPSyntaxNamedCastExpression*)pcCast)->meCast == CT_reinterpret_cast);
}


//////////////////////////////////////////////////////////////////////////
//
// dynamic_cast<T*>(p) and dynamic_cast<void*>(p).  A cast to the class or one
// of its bases is a conversion; any other looks up the object's type info.
// dynamic_cast<T&> is a place (DynamicCastPlace).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::DynamicCastExpression(CCPPSyntaxNamedCastExpression* pcCast, SCPP65IRValue* psValue)
{
	SCPP65Type*		psTo;
	CCPPClass*	pcFrom;
	CCPPClass*	pcTarget;

	psTo = mpcUnit->GetDeclarators()->GetTypeIdType(pcCast->mpcType);
	if (!psTo)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (psTo->IsReference())
	{
		//A reference cast's place is DynamicCastPlace's.
		return Unsupported(__FILE__, __LINE__);
	}
	if (!psTo->IsPointer() || !(psTo->psBase->IsClass() || psTo->psBase->IsVoid()))
	{
		mpcDiagnostics->ReportOnce(pcCast, "dynamic_cast needs a pointer to a class or to void");
		return Unsupported(__FILE__, __LINE__);
	}
	if (!Value(pcCast->mpcOperand, psValue))
	{
		return false;
	}
	if (!psValue->psType->IsPointer() || !psValue->psType->psBase->IsClass())
	{
		mpcDiagnostics->ReportError(pcCast, "The operand of dynamic_cast must be a pointer to a class");
		return Unsupported(__FILE__, __LINE__);
	}

	pcFrom = psValue->psType->psBase->pcClass;
	if (!mpcUnit->GetClasses()->CompleteClass(pcFrom, pcCast))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	pcTarget = NULL;
	if (psTo->psBase->IsClass())
	{
		pcTarget = psTo->psBase->pcClass;
		if (!mpcUnit->GetClasses()->CompleteClass(pcTarget, pcCast))
		{
			return Unsupported(__FILE__, __LINE__);
		}
		if (mpcUnit->IsDerived(pcFrom, pcTarget, NULL))
		{
			return Convert(psValue, psTo);
		}
	}
	if (!pcFrom->IsPolymorphic())
	{
		mpcDiagnostics->ReportOnce(pcCast, "The operand of dynamic_cast is not polymorphic");
		return Unsupported(__FILE__, __LINE__);
	}

	psValue->sValue = DynamicCast(psValue->sValue, pcFrom, pcTarget);
	psValue->psType = psTo;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The object sAddress (a pcFrom, NULL stays NULL) as a pcTo, NULL if it isn't
// one.  A NULL pcTo gives the start of the whole object (dynamic_cast<void*>),
// read from the vtable here.  Any other is looked for by R_DynamicCast, a
// routine the unit writes once (DynamicCastRoutine), which is given the two
// classes' type infos and, when pcFrom is a base of pcTo in one way, public
// and through no virtual base, where in a pcTo it is: no two pcTo objects
// can share it then, so a pcTo found there is the answer at once.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CCPP65IRFrontEnd::DynamicCast(SIRReference sAddress, CCPPClass* pcFrom, CCPPClass* pcTo)
{
	SIRType			asTypes[4];
	SIRReference	asArguments[4];
	int				iDown;

	if (!pcTo)
	{
		return WholeObject(sAddress, pcFrom);
	}
	asTypes[0] = IRType(IRTK_Pointer);
	asTypes[1] = IRType(IRTK_Pointer);
	asTypes[2] = IRType(IRTK_Pointer);
	asTypes[3] = IRIntegerType(mcModule.GetPointerSize(0));
	asArguments[0] = sAddress;
	asArguments[1] = TypeInfoSymbol(pcFrom);
	asArguments[2] = TypeInfoSymbol(pcTo);
	asArguments[3] = mpcFunction->AddInteger(asTypes[3], mpcUnit->IsOnePublicBase(pcTo, pcFrom, &iDown) ? iDown : CPP65_DYNAMIC_CAST_NO_PLACE);
	mpcUnit->GetGlobals()->UseDynamicCast();
	return CallNoThrow(CPP65_DYNAMIC_CAST, IRType(IRTK_Pointer), asTypes, asArguments, 4);
}


//////////////////////////////////////////////////////////////////////////
//
// The class's type info (written if it hasn't been), as a symbol.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CCPP65IRFrontEnd::TypeInfoSymbol(CCPPClass* pcClass)
{
	SIRReference	sSymbol;
	CChars			szLabel;

	mpcUnit->GetClassData()->EnsureTypeInfo(pcClass);
	szLabel.Init();
	mpcUnit->GetMangler()->AppendTypeInfoLabel(&szLabel, pcClass);
	sSymbol = mcModule.Symbol(szLabel.Text());
	szLabel.Kill();
	if (pcClass->IsUnnamed())
	{
		mcModule.SetLocal(sSymbol.Index());
	}
	return sSymbol;
}


//////////////////////////////////////////////////////////////////////////
//
// The start of the whole object that the pcFrom at sAddress is, or is a
// base of (dynamic_cast<void*>): NULL stays NULL.  The part's vtable has
// the distance from its vtable pointer (CPP65_VTABLE_TOP_OFFSET).
//
//////////////////////////////////////////////////////////////////////////
SIRReference CCPP65IRFrontEnd::WholeObject(SIRReference sAddress, CCPPClass* pcFrom)
{
	CIRBlock*		pcStart;
	CIRBlock*		pcLook;
	CIRBlock*		pcEnd;
	SIRType			sPointer;
	SIRReference	sNullPointer;
	SIRReference	sCondition;
	SIRReference	sTop;

	sPointer = IRType(IRTK_Pointer);
	sNullPointer = mpcFunction->AddConstant(sPointer, 0);
	pcLook = NewBlock();
	pcEnd = NewBlock();

	pcStart = mpcBlock;
	sCondition = mpcFunction->Add(mpcBlock, IRO_Eq, sPointer, sAddress, sNullPointer);
	mpcBlock->Branch(sCondition, pcEnd, pcLook);

	StartBlock(pcLook);
	sTop = ObjectTop(AddressAt(sAddress, pcFrom->GetVtablePointerOffset()), NULL);
	JumpTo(pcEnd);
	StartBlock(pcEnd);
	return Phi(pcEnd, sPointer, pcStart, sNullPointer, pcLook, sTop);
}


//////////////////////////////////////////////////////////////////////////
//
// The start of the whole object whose part's vtable pointer is at
// sVtablePointer, and that part's vtable (*psVtable, if wanted).
//
//////////////////////////////////////////////////////////////////////////
SIRReference CCPP65IRFrontEnd::ObjectTop(SIRReference sVtablePointer, SIRReference* psVtable)
{
	SIRType			sPointer;
	SIRType			sWord;
	SIRReference	sVtable;
	SIRReference	sTop;

	sPointer = IRType(IRTK_Pointer);
	sWord = IRIntegerType(CPP65_VTABLE_ENTRY_BYTES);
	sVtable = mpcFunction->Load(mpcBlock, sPointer, sVtablePointer);
	sTop = ConvertIR(mpcFunction->Load(mpcBlock, sWord, AddressAt(sVtable, CPP65_VTABLE_TOP_OFFSET)), sWord, IRIntegerType(mcModule.GetPointerSize(0)), true);
	if (psVtable)
	{
		*psVtable = sVtable;
	}
	return mpcFunction->Add(mpcBlock, IRO_PtrAdd, sPointer, sVtablePointer, sTop);
}


//////////////////////////////////////////////////////////////////////////
//
// R_DynamicCast, which every dynamic_cast that looks for its answer calls
// (DynamicCast): the object at the first parameter, of the class whose type
// info is the second, as an object of the class whose type info is the
// third, or NULL.  Written once in a unit that casts, in a section the
// linker keeps one of (CCPPUnit::WriteDynamicCast).
//
// The object's vtable pointer (its place is in its class's type info's own
// entry) finds the start of the whole object and the whole object's type
// info, which lists every subobject.  [expr.dynamic.cast] 8: if the object
// is a public base of a target object, and of one only, it is that object
// (a target found in the list with the object where the fourth parameter
// says its base is; CPP65_DYNAMIC_CAST_NO_PLACE when two targets could share
// it).  Otherwise the object must be a public base of the whole object (its
// own entry in the list, the source class at its address, says), and the
// whole object must have one target subobject, reached through public
// bases: one found twice, or found where a base on the way isn't public, is
// no answer.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::DynamicCastRoutine(const char* szLabel)
{
	CCPPFunction	cFunction;
	CIRBlock*		pcStart;
	CIRBlock*		pcLook;
	CIRBlock*		pcLoop;
	CIRBlock*		pcCheck;
	CIRBlock*		pcMatch;
	CIRBlock*		pcPlace;
	CIRBlock*		pcAccess;
	CIRBlock*		pcFirst;
	CIRBlock*		pcTake;
	CIRBlock*		pcSpoil;
	CIRBlock*		pcNext;
	CIRBlock*		pcDone;
	CIRBlock*		pcSpoilt;
	CIRBlock*		pcEnd;
	CIRBlock*		pcIsSource;
	CIRBlock*		pcSourceHere;
	CIRBlock*		pcSourcePublic;
	CIRBlock*		pcTarget;
	CIRBlock*		pcUnspoilt;
	SIRType			sPointer;
	SIRType			sOffset;
	SIRType			sWord;
	SIRReference	sAddress;
	SIRReference	sSource;
	SIRReference	sTarget;
	SIRReference	sDown;
	SIRReference	sHasPlace;
	SIRReference	sNullPointer;
	SIRReference	sSpoilt;
	SIRReference	sCondition;
	SIRReference	sVtableOffset;
	SIRReference	sVtable;
	SIRReference	sTop;
	SIRReference	sInfo;
	SIRReference	sEntry;
	SIRReference	sFound;
	SIRReference	sFrom;
	SIRReference	sNextFrom;
	SIRReference	sHere;
	SIRReference	sClass;
	SIRReference	sCandidate;
	SIRReference	sFlags;
	SIRReference	sNextEntry;
	SIRReference	sNextFound;
	SIRReference	sResult;
	SIRPhi*			psPhi;

	memset(&cFunction, 0, sizeof(CCPPFunction));
	cFunction.meKind = CPPFK_Function;
	cFunction.mpsReturn = mpcTypes->Pointer(&mpcTypes->msVoid);
	sPointer = IRType(IRTK_Pointer);
	sOffset = IRIntegerType(mcModule.GetPointerSize(0));
	sWord = IRIntegerType(CPP65_VTABLE_ENTRY_BYTES);
	BeginWriting(&cFunction, szLabel, sPointer, IRA_NoInline);
	mbRoutine = true;
	sAddress = mpcFunction->AddParameter(sPointer);
	sSource = mpcFunction->AddParameter(sPointer);
	sTarget = mpcFunction->AddParameter(sPointer);
	sDown = mpcFunction->AddParameter(sOffset);

	sNullPointer = mpcFunction->AddConstant(sPointer, 0);
	pcLook = NewBlock();
	pcEnd = NewBlock();
	pcStart = mpcBlock;
	sCondition = mpcFunction->Add(mpcBlock, IRO_Eq, sPointer, sAddress, sNullPointer);
	mpcBlock->Branch(sCondition, pcEnd, pcLook);

	//The start of the whole object, and its type info's entries.
	StartBlock(pcLook);
	sFlags = mpcFunction->Load(mpcBlock, sWord, AddressAt(sSource, CPP65_TYPE_INFO_ENTRIES_OFFSET + CPP65_TYPE_INFO_FLAGS_OFFSET));
	sVtableOffset = mpcFunction->Add(mpcBlock, IRO_LShr, sWord, sFlags, mpcFunction->AddConstant(sWord, CPP65_TYPE_INFO_VTABLE_SHIFT));
	sVtableOffset = ConvertIR(mpcFunction->Add(mpcBlock, IRO_Sub, sWord, sVtableOffset, mpcFunction->AddConstant(sWord, 1)), sWord, sOffset, true);
	sTop = ObjectTop(mpcFunction->Add(mpcBlock, IRO_PtrAdd, sPointer, sAddress, sVtableOffset), &sVtable);
	sInfo = AddressAt(mpcFunction->Load(mpcBlock, sPointer, AddressAt(sVtable, CPP65_VTABLE_TYPE_INFO_OFFSET)), CPP65_TYPE_INFO_ENTRIES_OFFSET);
	sHasPlace = mpcFunction->Add(mpcBlock, IRO_Ne, sOffset, sDown, mpcFunction->AddInteger(sOffset, CPP65_DYNAMIC_CAST_NO_PLACE));

	//After the type info's header, each entry is a subobject's class's type info, its offset and its flags; a zero ends
	//them.  sFound is the one target found so far: null for none, and sSpoilt (an address no object has) once the
	//answer can only be null.  sFrom is whether the object's own entry has been met and is public (null until then).
	sSpoilt = mpcFunction->AddConstant(sPointer, 1);
	pcLoop = NewBlock();
	pcCheck = NewBlock();
	pcMatch = NewBlock();
	pcPlace = NewBlock();
	pcAccess = NewBlock();
	pcFirst = NewBlock();
	pcTake = NewBlock();
	pcSpoil = NewBlock();
	pcNext = NewBlock();
	pcDone = NewBlock();
	pcSpoilt = NewBlock();
	pcIsSource = NewBlock();
	pcSourceHere = NewBlock();
	pcSourcePublic = NewBlock();
	pcTarget = NewBlock();
	pcUnspoilt = NewBlock();
	JumpTo(pcLoop);

	StartBlock(pcLoop);
	sEntry = LoopPhi(pcLoop, sPointer, pcLook, sInfo);
	sFound = LoopPhi(pcLoop, sPointer, pcLook, sNullPointer);
	sFrom = LoopPhi(pcLoop, sPointer, pcLook, sNullPointer);
	sClass = mpcFunction->Load(mpcBlock, sPointer, sEntry);
	sCondition = mpcFunction->Add(mpcBlock, IRO_Eq, sPointer, sClass, sNullPointer);
	mpcBlock->Branch(sCondition, pcDone, pcCheck);

	//The object's own entry: a source class's, where the object is.
	StartBlock(pcCheck);
	sCondition = mpcFunction->Add(mpcBlock, IRO_Eq, sPointer, sClass, sSource);
	mpcBlock->Branch(sCondition, pcIsSource, pcTarget);

	StartBlock(pcIsSource);
	sHere = ConvertIR(mpcFunction->Load(mpcBlock, sWord, AddressAt(sEntry, CPP65_TYPE_INFO_OFFSET_OFFSET)), sWord, sOffset, true);
	sHere = mpcFunction->Add(mpcBlock, IRO_PtrAdd, sPointer, sTop, sHere);
	sCondition = mpcFunction->Add(mpcBlock, IRO_Eq, sPointer, sHere, sAddress);
	mpcBlock->Branch(sCondition, pcSourceHere, pcTarget);

	StartBlock(pcSourceHere);
	sFlags = mpcFunction->Load(mpcBlock, sWord, AddressAt(sEntry, CPP65_TYPE_INFO_FLAGS_OFFSET));
	sFlags = mpcFunction->Add(mpcBlock, IRO_And, sWord, sFlags, mpcFunction->AddConstant(sWord, CPP65_TYPE_INFO_PUBLIC));
	sCondition = mpcFunction->Add(mpcBlock, IRO_Ne, sWord, sFlags, mpcFunction->AddConstant(sWord, 0));
	mpcBlock->Branch(sCondition, pcSourcePublic, pcTarget);

	StartBlock(pcSourcePublic);
	JumpTo(pcTarget);

	StartBlock(pcTarget);
	sNextFrom = mpcFunction->AddTemporary(sPointer);
	psPhi = pcTarget->AddPhi(sPointer, sNextFrom);
	pcTarget->AddPhiArgument(psPhi, pcCheck, sFrom);
	pcTarget->AddPhiArgument(psPhi, pcIsSource, sFrom);
	pcTarget->AddPhiArgument(psPhi, pcSourceHere, sFrom);
	pcTarget->AddPhiArgument(psPhi, pcSourcePublic, sSpoilt);
	sCondition = mpcFunction->Add(mpcBlock, IRO_Eq, sPointer, sClass, sTarget);
	mpcBlock->Branch(sCondition, pcMatch, pcNext);

	//A target subobject.  With the object where its source base is, it is the answer, whatever else there is.
	StartBlock(pcMatch);
	sCandidate = ConvertIR(mpcFunction->Load(mpcBlock, sWord, AddressAt(sEntry, CPP65_TYPE_INFO_OFFSET_OFFSET)), sWord, sOffset, true);
	sCandidate = mpcFunction->Add(mpcBlock, IRO_PtrAdd, sPointer, sTop, sCandidate);
	mpcBlock->Branch(sHasPlace, pcPlace, pcAccess);

	StartBlock(pcPlace);
	sCondition = mpcFunction->Add(mpcBlock, IRO_Eq, sPointer, mpcFunction->Add(mpcBlock, IRO_PtrAdd, sPointer, sCandidate, sDown), sAddress);
	mpcBlock->Branch(sCondition, pcEnd, pcAccess);

	//A second one, or one not reached through public bases, spoils the answer.
	StartBlock(pcAccess);
	sCondition = mpcFunction->Add(mpcBlock, IRO_Eq, sPointer, sFound, sNullPointer);
	mpcBlock->Branch(sCondition, pcFirst, pcSpoil);

	StartBlock(pcFirst);
	sFlags = mpcFunction->Load(mpcBlock, sWord, AddressAt(sEntry, CPP65_TYPE_INFO_FLAGS_OFFSET));
	sFlags = mpcFunction->Add(mpcBlock, IRO_And, sWord, sFlags, mpcFunction->AddConstant(sWord, CPP65_TYPE_INFO_PUBLIC));
	sCondition = mpcFunction->Add(mpcBlock, IRO_Ne, sWord, sFlags, mpcFunction->AddConstant(sWord, 0));
	mpcBlock->Branch(sCondition, pcTake, pcSpoil);

	StartBlock(pcTake);
	JumpTo(pcNext);

	StartBlock(pcSpoil);
	JumpTo(pcNext);

	StartBlock(pcNext);
	sNextFound = mpcFunction->AddTemporary(sPointer);
	psPhi = pcNext->AddPhi(sPointer, sNextFound);
	pcNext->AddPhiArgument(psPhi, pcTarget, sFound);
	pcNext->AddPhiArgument(psPhi, pcTake, sCandidate);
	pcNext->AddPhiArgument(psPhi, pcSpoil, sSpoilt);
	sNextEntry = AddressAt(sEntry, CPP65_TYPE_INFO_ENTRY_BYTES);
	JumpTo(pcLoop);
	ClosePhi(pcLoop, sEntry, pcNext, sNextEntry);
	ClosePhi(pcLoop, sFound, pcNext, sNextFound);
	ClosePhi(pcLoop, sFrom, pcNext, sNextFrom);

	//The list's end: sFound is the answer if it wasn't spoilt and the object was seen to be a public base.
	StartBlock(pcDone);
	sCondition = mpcFunction->Add(mpcBlock, IRO_Eq, sPointer, sFound, sSpoilt);
	mpcBlock->Branch(sCondition, pcSpoilt, pcUnspoilt);

	StartBlock(pcUnspoilt);
	sCondition = mpcFunction->Add(mpcBlock, IRO_Eq, sPointer, sFrom, sNullPointer);
	mpcBlock->Branch(sCondition, pcSpoilt, pcEnd);

	StartBlock(pcSpoilt);
	JumpTo(pcEnd);

	StartBlock(pcEnd);
	sResult = mpcFunction->AddTemporary(sPointer);
	psPhi = pcEnd->AddPhi(sPointer, sResult);
	pcEnd->AddPhiArgument(psPhi, pcStart, sNullPointer);
	pcEnd->AddPhiArgument(psPhi, pcPlace, sCandidate);
	pcEnd->AddPhiArgument(psPhi, pcUnspoilt, sFound);
	pcEnd->AddPhiArgument(psPhi, pcSpoilt, sNullPointer);
	mpcBlock->Return(sPointer, sResult);
	return FinishWriting(true, szLabel);
}


//////////////////////////////////////////////////////////////////////////
//
// Converts a value to psTo where no cast asks for it (a variable given its
// value, an argument, what a function returns, an assignment): an enum
// class's value stays one, and an enum is given its own values
// (CCPP65TypeChecker::KeepsEnum); an object's explicit conversion function
// isn't used.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ConvertImplicit(SCPP65IRValue* psValue, SCPP65Type* psTo, CCPPSyntax* pcNode)
{
	bool	bKept;

	if (psValue->psType && psTo && (psValue->psType->IsScopedEnum() || psTo->WithoutReference()->pcEnum))
	{
		mpcDiagnostics->BeginLoud();
		bKept = mpcUnit->GetTypeChecker()->KeepsEnum(psValue->psType, psTo, pcNode);
		mpcDiagnostics->EndLoud();
		if (!bKept)
		{
			return false;
		}
	}
	if (psValue->psType && psTo && (psValue->psType->WithoutReference()->IsNullPointer() || psTo->WithoutReference()->IsNullPointer()))
	{
		//std::nullptr_t converts only as it may; pcNode is the value converted (a literal 0 becomes one).
		mpcDiagnostics->BeginLoud();
		bKept = mpcUnit->GetTypeChecker()->KeepsNullPointer(psValue->psType, psTo, pcNode, pcNode);
		mpcDiagnostics->EndLoud();
		if (!bKept)
		{
			return false;
		}
	}
	if (psValue->psType && psTo && IsObjectValue(psValue) && psTo->IsScalar())
	{
		return ConvertObject(psValue, psTo, false);
	}
	return Convert(psValue, psTo);
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
		mpcUnit->IsDerived(psFrom->psBase->pcClass, psTo->psBase->pcClass, NULL) && mpcUnit->GetClasses()->CompleteClass(psFrom->psBase->pcClass, NULL) &&
		mpcUnit->IsDerived(psFrom->psBase->pcClass, psTo->psBase->pcClass, &iOffset) && (iOffset != 0))
	{
		psValue->sValue = BaseAddress(psValue->sValue, psFrom->psBase->pcClass, psTo->psBase->pcClass, true);
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
// A pointer moved lliOffset bytes on, or a pointer to a member (an integer)
// with lliOffset added, as its conversion to a base's or a derived class's
// moves it; null stays null.  [conv.ptr] 3, [conv.mem] 2
//
//////////////////////////////////////////////////////////////////////////
SIRReference CCPP65IRFrontEnd::MovedUnlessNull(SIRType sType, SIRReference sValue, int64 lliOffset)
{
	CIRBlock*		pcAdd;
	CIRBlock*		pcEnd;
	CIRBlock*		pcFrom;
	SIRReference	sNull;
	SIRReference	sMoved;

	pcAdd = NewBlock();
	pcEnd = NewBlock();
	pcFrom = mpcBlock;
	sNull = mpcFunction->Add(mpcBlock, IRO_Eq, sType, sValue, mpcFunction->AddConstant(sType, 0));
	mpcBlock->Branch(sNull, pcEnd, pcAdd);
	StartBlock(pcAdd);
	sMoved = sType.IsPointer() ? AddressAt(sValue, (int)lliOffset) : mpcFunction->Add(mpcBlock, IRO_Add, sType, sValue, mpcFunction->AddInteger(sType, lliOffset));
	JumpTo(pcEnd);
	StartBlock(pcEnd);
	return Phi(pcEnd, sType, pcFrom, sValue, pcAdd, sMoved);
}


//////////////////////////////////////////////////////////////////////////
//
// The address of a derived object's base iOffset bytes in; null stays
// null.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CCPP65IRFrontEnd::BaseAddress(SIRReference sAddress, int iOffset)
{
	return MovedUnlessNull(IRType(IRTK_Pointer), sAddress, iOffset);
}


//////////////////////////////////////////////////////////////////////////
//
// The address of the pcTo part of the pcFrom object at sAddress (with bNull
// a null pointer stays null): a fixed offset further on, or through a
// virtual base the offset the object's vtable part gives
// (VirtualBaseAddress), then where pcTo is in that base.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CCPP65IRFrontEnd::BaseAddress(SIRReference sAddress, CCPPClass* pcFrom, CCPPClass* pcTo, bool bNull)
{
	SCPPVirtualBase*	psVirtual;
	CIRBlock*			pcAdd;
	CIRBlock*			pcEnd;
	CIRBlock*			pcStart;
	SIRReference		sNull;
	SIRReference		sMoved;
	int					iOffset;
	int					iBelow;

	if (!pcFrom || !pcTo || (pcFrom == pcTo) || !mpcUnit->GetClasses()->CompleteClass(pcFrom, NULL) || !mpcUnit->IsDerived(pcFrom, pcTo, &iOffset))
	{
		return sAddress;
	}
	if (iOffset != CPP_VIRTUAL_PATH)
	{
		return (bNull && (iOffset != 0)) ? BaseAddress(sAddress, iOffset) : AddressAt(sAddress, iOffset);
	}
	psVirtual = mpcUnit->VirtualBaseOf(pcFrom, pcTo, &iBelow);
	if (!psVirtual)
	{
		Unsupported(__FILE__, __LINE__);
		return sAddress;
	}
	if (!bNull)
	{
		return AddressAt(VirtualBaseAddress(sAddress, pcFrom, psVirtual->pcClass), iBelow);
	}
	pcAdd = NewBlock();
	pcEnd = NewBlock();
	pcStart = mpcBlock;
	sNull = mpcFunction->Add(mpcBlock, IRO_Eq, IRType(IRTK_Pointer), sAddress, mpcFunction->AddConstant(IRType(IRTK_Pointer), 0));
	mpcBlock->Branch(sNull, pcEnd, pcAdd);
	StartBlock(pcAdd);
	sMoved = AddressAt(VirtualBaseAddress(sAddress, pcFrom, psVirtual->pcClass), iBelow);
	pcAdd = mpcBlock;
	JumpTo(pcEnd);
	StartBlock(pcEnd);
	return Phi(pcEnd, IRType(IRTK_Pointer), pcStart, sAddress, pcAdd, sMoved);
}


//////////////////////////////////////////////////////////////////////////
//
// The virtual base pcBase of the pcClass object at sAddress: the distance
// the object's vtable part for pcClass has for it
// (CPP65_VTABLE_VIRTUAL_OFFSET).
//
//////////////////////////////////////////////////////////////////////////
SIRReference CCPP65IRFrontEnd::VirtualBaseAddress(SIRReference sAddress, CCPPClass* pcClass, CCPPClass* pcBase)
{
	SIRReference	sVtable;
	SIRReference	sOffset;
	SIRType			sEntry;

	sEntry = IRIntegerType(CPP65_VTABLE_ENTRY_BYTES);
	sVtable = mpcFunction->Load(mpcBlock, IRType(IRTK_Pointer), AddressAt(sAddress, pcClass->GetVtablePointerOffset()));
	sOffset = mpcFunction->Load(mpcBlock, sEntry, AddressAt(sVtable, CPP65_VTABLE_VIRTUAL_OFFSET(pcClass->GetVirtualBaseIndex(pcBase))));
	sOffset = ConvertIR(sOffset, sEntry, IRIntegerType(mcModule.GetPointerSize(0)), true);
	return mpcFunction->Add(mpcBlock, IRO_PtrAdd, IRType(IRTK_Pointer), sAddress, sOffset);
}


//////////////////////////////////////////////////////////////////////////
//
// Conversions to and from pointers to members (a data member's offset, or
// a member function's address with what its this is moved by): an integer
// (0, null) is one as it is, one is a bool when it isn't 0, a pointer to a
// member of a base is one to the derived class's member (the base's offset
// added: to a field's offset, to a function's this) and back again by a
// cast (taken off), and other pointers to members (to members of unrelated
// classes, by a cast) keep the value.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ConvertMemberPointer(SCPP65IRValue* psValue, SCPP65Type* psTo)
{
	SCPP65Type*		psFrom;
	SIRReference	sCondition;
	int				iOffset;

	psFrom = psValue->psType;
	if (psTo->IsMemberPointer() && psFrom->IsNullPointer())
	{
		//A std::nullptr_t is the null pointer to a member, which is 0.  [conv.mem] 1
		psValue->sValue = mpcFunction->AddInteger(IRTypeOf(psTo), 0);
	}
	else if (psTo->IsMemberPointer() && psFrom->IsInteger())
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
		if ((BaseOffset(psTo->pcClass, psFrom->pcClass) == CPP_VIRTUAL_PATH) || (BaseOffset(psFrom->pcClass, psTo->pcClass) == CPP_VIRTUAL_PATH))
		{
			//A member of a virtual base has no fixed offset from the derived class.  [conv.mem] 2
			mpcDiagnostics->ReportError(mpcValue, "A pointer to a member of a virtual base can't be converted to a pointer to a member of a class derived from it");
			return Unsupported(__FILE__, __LINE__);
		}
		iOffset = BaseOffset(psTo->pcClass, psFrom->pcClass) - BaseOffset(psFrom->pcClass, psTo->pcClass);
		if (iOffset != 0)
		{
			psValue->sValue = MovedUnlessNull(IRTypeOf(psFrom), psValue->sValue, psFrom->IsMemberFunctionPointer() ? ((int64)iOffset << CPP_MEMBER_FUNCTION_MOVE_SHIFT) : iOffset);
		}
	}
	else if (!psFrom->Equals(psTo))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	psValue->psType = psTo;
	return true;
}
