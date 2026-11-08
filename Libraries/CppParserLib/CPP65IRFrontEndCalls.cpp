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


//The IR front end's calls: of functions, through pointers, of members, operators and destructors, the builtins,
//and the runtime's routines.


#define IR65_MAX_BUILTIN_ARGUMENTS	4		//The most arguments a builtin passed on to a function takes (CallWithArguments).


//////////////////////////////////////////////////////////////////////////
//
// Calls of functions (by name, or instances of function templates) and of
// pointers to functions.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CallExpression(CCPPSyntaxCallExpression* pcCall, SCPP65IRValue* psValue)
{
	CCPPSyntax*					pcCallee;
	CCPPSyntaxName*				pcName;
	CCPPSyntaxNameComponent*	pcComponent;
	CCPPSymbol*					pcSymbol;
	SCPP65Function*				psFunction;
	SCPP65Template*				psTemplate;
	SIRReference				sSymbol;
	int64						lliName;
	SCPP65Type*					psOperandType;

	if (mpcGenerator->HasPackExpansion(&pcCall->macpArguments))
	{
		return ExpandPackCall(pcCall, psValue);
	}
	pcCallee = mpcGenerator->SkipParentheses(pcCall->mpcCallee);
	if (pcCallee->GetKind() == CPPS_MemberExpression)
	{
		return MemberCall(pcCall, pcCallee, psValue);
	}
	if (mpcGenerator->IsMemberPointerOperator(pcCallee))
	{
		return MemberPointerCall(pcCall, (CCPPSyntaxBinaryExpression*)pcCallee, psValue);
	}
	pcSymbol = NULL;
	pcName = NULL;
	pcComponent = NULL;
	if (pcCallee->GetKind() == CPPS_IdExpression)
	{
		pcName = ((CCPPSyntaxIdExpression*)pcCallee)->mpcName;
		pcSymbol = pcName->mpcSymbol;
		pcComponent = pcName->GetLast();
		if (pcName->IsQualifiedByClass())
		{
			return MemberCall(pcCall, pcCallee, psValue);
		}
	}

	//f() in a member function may be a member, even when the parser could not resolve it in a template.
	if (pcName && !pcName->mbGlobal && mpsFunction->psClass)
	{
		if (pcSymbol && (pcSymbol->meKind == CPPSK_FunctionTemplate))
		{
			if (mpcGenerator->FindMemberTemplate(mpsFunction->psClass, pcSymbol->mlliName, pcCall->macpArguments.NumElements()) || mpcGenerator->HasMember(mpsFunction->psClass, pcSymbol->mlliName))
			{
				return MemberCall(pcCall, pcCallee, psValue);
			}
		}
		else
		{
			lliName = pcSymbol ? pcSymbol->mlliName : mpcGenerator->GetFunctionName(pcComponent);
			if ((!pcSymbol || (pcSymbol->meKind == CPPSK_Function)) && mpcGenerator->HasMember(mpsFunction->psClass, lliName))
			{
				return MemberCall(pcCall, pcCallee, psValue);
			}
		}
	}

	//A call through an expression (table[i](...), (*pf)(...), GetF()(...)): a pointer to a function.
	if (!pcName)
	{
		//(*pf)() is pf(); (*ppf)() calls the function pointer ppf points to, so it keeps its *.
		if ((pcCallee->GetKind() == CPPS_UnaryExpression) && (((CCPPSyntaxUnaryExpression*)pcCallee)->meOperator == CT_Multiply))
		{
			psOperandType = mpcGenerator->TypeOf(((CCPPSyntaxUnaryExpression*)pcCallee)->mpcOperand);
			if (psOperandType && psOperandType->IsPointer() && psOperandType->psBase->IsFunction())
			{
				pcCallee = mpcGenerator->SkipParentheses(((CCPPSyntaxUnaryExpression*)pcCallee)->mpcOperand);
			}
		}
		return CallThroughPointer(pcCall, pcCallee, psValue);
	}

	//__builtin_ functions.
	lliName = pcSymbol ? pcSymbol->mlliName : mpcGenerator->GetFunctionName(pcComponent);
	if ((!pcSymbol || (pcSymbol->meKind == CPPSK_Function)) && mpcGenerator->IsBuiltin(lliName))
	{
		return Builtin(pcCall, lliName, psValue);
	}

	psFunction = NULL;
	if (pcSymbol && (pcSymbol->meKind == CPPSK_FunctionTemplate))
	{
		psTemplate = mpcGenerator->GetCallTemplate(pcSymbol, pcSymbol->mlliName, pcCall, (pcComponent && pcComponent->mbTemplateArguments) ? &pcComponent->macpTemplateArguments : NULL);
		if (!psTemplate)
		{
			return Unsupported(__FILE__, __LINE__);
		}
		psFunction = mpcGenerator->InstantiateFunction(psTemplate, (pcComponent && pcComponent->mbTemplateArguments) ? &pcComponent->macpTemplateArguments : NULL, &pcCall->macpArguments, pcCall);
	}
	else if (pcSymbol && (pcSymbol->meKind == CPPSK_Function))
	{
		lliName = pcSymbol->mlliName;
		if (mpcGenerator->IsInlineMemcpy(lliName, &pcCall->macpArguments))
		{
			return InlineMemcpy(&pcCall->macpArguments, mpcGenerator->TypeOf(pcCall), psValue);
		}
		psFunction = mpcGenerator->FindNamedFunction(pcName, lliName, &pcCall->macpArguments, pcCall);
	}
	else if (pcSymbol)
	{
		return CallThroughPointer(pcCall, pcCallee, psValue);
	}
	else
	{
		//Nothing declares it here: it may be in a namespace of its arguments'.
		psFunction = mpcGenerator->FindAssociatedFunction(pcName, lliName, pcCall);
	}
	if (!psFunction || (psFunction->psClass && !psFunction->bStatic))
	{
		return Unsupported(__FILE__, __LINE__);
	}

	sSymbol = FunctionSymbol(psFunction);
	return Call(psFunction, sSymbol, NULL, &pcCall->macpArguments, psValue);
}


//////////////////////////////////////////////////////////////////////////
//
// __builtin_clz or __builtin_ctz (bLeading) of psArgument (psType wide): the
// width when it is 0, else narrowed by halves to a byte (NarrowCount, each
// half's bits counted) and the byte's leading or trailing zeros looked up in
// a 256 byte table, instead of a loop of up to 15 shifts.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CountTable(bool bLeading, SCPP65IRValue* psArgument, SCPP65Type* psType, SCPP65IRValue* psValue)
{
	SIRType			sType;
	SIRType			sInt;
	SIRType			sByte;
	SIRReference	sTest;
	SIRReference	sCount;
	SIRReference	sByteValue;
	SIRReference	sAddress;
	SIRReference	sZeros;
	SCPP65IRValue	sTable;
	CIRBlock*		pcZero;
	CIRBlock*		pcNarrow;
	CIRBlock*		pcCounted;
	CIRBlock*		pcDone;
	char			acZeros[256];
	int				iBits;
	int				i;
	int				iBit;

	sType = IRTypeOf(psType);
	sInt = IRTypeOf(&mpcTypes->msInt);
	sByte = IRType(IRTK_I8);
	iBits = psType->iBytes * 8;
	pcZero = NewBlock();
	pcNarrow = NewBlock();
	pcDone = NewBlock();
	sTest = mpcFunction->Add(mpcBlock, IRO_Eq, sType, psArgument->sValue, mpcFunction->AddInteger(sType, 0));
	mpcBlock->Branch(sTest, pcZero, pcNarrow);

	StartBlock(pcNarrow);
	sByteValue = psArgument->sValue;
	sCount = mpcFunction->AddInteger(sInt, 0);
	while (iBits > 8)
	{
		NarrowCount(bLeading, &sByteValue, &sCount, &iBits, sInt);
	}
	acZeros[0] = 8;
	for (i = 1; i < 256; i++)
	{
		for (iBit = 0; (iBit < 8) && !(i & (bLeading ? (0x80 >> iBit) : (1 << iBit))); iBit++)
		{
		}
		acZeros[i] = (char)iBit;
	}
	StringData(acZeros, 256, &sTable);
	sAddress = mpcFunction->Add(mpcBlock, IRO_ZExt, IRIntegerType(mcModule.GetPointerSize(0)), sByteValue);
	sAddress = mpcFunction->Add(mpcBlock, IRO_PtrAdd, IRType(IRTK_Pointer), sTable.sValue, sAddress);
	sZeros = mpcFunction->Load(mpcBlock, sByte, sAddress);
	sZeros = mpcFunction->Add(mpcBlock, IRO_ZExt, sInt, sZeros);
	sCount = mpcFunction->Add(mpcBlock, IRO_Add, sInt, sCount, sZeros);
	pcCounted = mpcBlock;
	mpcBlock->Jump(pcDone);

	StartBlock(pcZero);
	mpcBlock->Jump(pcDone);

	StartBlock(pcDone);
	psValue->sValue = Phi(pcDone, sInt, pcCounted, sCount, pcZero, mpcFunction->AddInteger(sInt, psType->iBytes * 8));
	psValue->psType = &mpcTypes->msInt;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The half of *psValue (*piBits wide and not 0) that ctz (the low half) or
// clz (the high half) counts from, unless it is 0: then the other half, and
// the half's bits are added to *psCount.  *psValue and *piBits become the
// half's.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::NarrowCount(bool bLeading, SIRReference* psValue, SIRReference* psCount, int* piBits, SIRType sInt)
{
	SIRType			sType;
	SIRType			sHalf;
	SIRReference	sHigh;
	SIRReference	sLow;
	SIRReference	sFirst;
	SIRReference	sOther;
	SIRReference	sTest;
	SIRReference	sCounted;
	CIRBlock*		pcFrom;
	CIRBlock*		pcOther;
	CIRBlock*		pcJoin;
	int				iHalf;

	iHalf = *piBits / 2;
	sType = mpcFunction->TypeOf(*psValue);
	sHalf = IRType((iHalf == 32) ? IRTK_I32 : (iHalf == 16) ? IRTK_I16 : IRTK_I8);
	sHigh = mpcFunction->Add(mpcBlock, IRO_LShr, sType, *psValue, mpcFunction->AddInteger(sType, iHalf));
	sHigh = mpcFunction->Add(mpcBlock, IRO_Trunc, sHalf, sHigh);
	sLow = mpcFunction->Add(mpcBlock, IRO_Trunc, sHalf, *psValue);
	sFirst = bLeading ? sHigh : sLow;
	sOther = bLeading ? sLow : sHigh;
	sTest = mpcFunction->Add(mpcBlock, IRO_Eq, sHalf, sFirst, mpcFunction->AddInteger(sHalf, 0));
	pcFrom = mpcBlock;
	pcOther = NewBlock();
	pcJoin = NewBlock();
	mpcBlock->Branch(sTest, pcOther, pcJoin);
	StartBlock(pcOther);
	sCounted = mpcFunction->Add(mpcBlock, IRO_Add, sInt, *psCount, mpcFunction->AddInteger(sInt, iHalf));
	mpcBlock->Jump(pcJoin);
	StartBlock(pcJoin);
	*psValue = Phi(pcJoin, sHalf, pcFrom, sFirst, pcOther, sOther);
	*psCount = Phi(pcJoin, sInt, pcFrom, *psCount, pcOther, sCounted);
	*piBits = iHalf;
}


//////////////////////////////////////////////////////////////////////////
//
// The __builtin_ functions the generator has (CCPP65Generator::Builtin), as IR: the float constants (huge_val,
// inf, nan), expect, unreachable and trap, ia32_pause, and ctz, clz and popcount (of unsigned int, l: unsigned long,
// ll: unsigned long long) as loops.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Builtin(CCPPSyntaxCallExpression* pcCall, int64 lliName, SCPP65IRValue* psValue)
{
	const SCPP65Builtin*	psBuiltin;
	char*			szLabel;
	double			dValue;
	SCPP65Type*		psType;
	SCPP65IRValue	sArgument;
	SIRType			sType;
	SIRType			sInt;
	CIRBlock*		pcEntry;
	CIRBlock*		pcLoop;
	CIRBlock*		pcBody;
	CIRBlock*		pcZero;
	CIRBlock*		pcNarrow;
	CIRBlock*		pcDone;
	SIRReference	sX;
	SIRReference	sCounted;
	SIRReference	sX2;
	SIRReference	sCount2;
	SIRReference	sCount;
	SIRReference	sTest;
	int				iBits;
	bool			bPopcount;
	bool			bClz;

	if (mpcGenerator->FloatBuiltin(pcCall, &dValue, &psType))
	{
		psValue->sValue = (psType->iBytes == 4) ? mpcFunction->AddFloat((float)dValue) : mpcFunction->AddDouble(dValue);
		psValue->psType = psType;
		return true;
	}

	psBuiltin = mpcGenerator->FindBuiltin(lliName);
	if (!psBuiltin || ((int)pcCall->macpArguments.NumElements() != psBuiltin->iArguments) ||
		(psBuiltin->szW65C816Only && (mpcGenerator->mpsTarget->eTarget != CPPT_W65C816)))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	psValue->sValue = IRNone();
	psValue->psType = &mpcTypes->msVoid;
	switch (psBuiltin->eBuiltin)
	{
	case CPP65B_Unreachable:
		mpcBlock->Unreachable();
		StartBlock(NewBlock());
		return true;
	case CPP65B_Pause:
		return true;
	case CPP65B_Expect:
		return Value(pcCall->macpArguments.GetPtr(0), psValue) && Convert(psValue, mpcTypes->Promote(psValue->psType));
	case CPP65B_Memcpy:
		//Copied in place for a small constant count, as memcpy is.
		if (mpcGenerator->IsInlineMemcpy(lliName, &pcCall->macpArguments))
		{
			return InlineMemcpy(&pcCall->macpArguments, mpcGenerator->TypeOf(pcCall), psValue);
		}
		return BuiltinMemcpy(pcCall, psValue);
	case CPP65B_BCDAdd:
	case CPP65B_BCDSubtract:
		return BuiltinBCD(pcCall, psBuiltin->eBuiltin == CPP65B_BCDSubtract, psValue);
	case CPP65B_Alloca:
		//Frames are fixed in size (and the W65C816's stack is small); malloc instead.
		mpcGenerator->ReportError(pcCall, "__builtin_alloca is not supported");
		return Unsupported(__FILE__, __LINE__);
	default:
		break;
	}

	//ctz, clz and popcount.
	bPopcount = psBuiltin->eBuiltin == CPP65B_Popcount;
	bClz = psBuiltin->eBuiltin == CPP65B_Clz;
	psType = mpcGenerator->BuiltinOperandType(psBuiltin);
	if (!Value(pcCall->macpArguments.GetPtr(0), &sArgument) || !Convert(&sArgument, psType))
	{
		return Unsupported(__FILE__, __LINE__);
	}

	//loop: x and the count; ctz and clz stop at the first set bit from their end (0 counts the width), popcount
	//clears the lowest set bit until there are none.  ctz and clz of more than 16 bits are narrowed first
	//(NarrowCount), so the loop goes round at most 15 times on 16 bits.  On the W65C816 those of 16 and 32 bits
	//are the runtime's R816_Clz16 ... (a byte table read with long,X), of 64 bits narrowed to a byte whose count
	//is looked up (CountTable).
	if (!bPopcount && (mpcGenerator->mpsTarget->eTarget == CPPT_W65C816) && (psType->iBytes <= 4))
	{
		szLabel = (char*)(bClz ? ((psType->iBytes == 4) ? "R816_Clz32" : "R816_Clz16") : ((psType->iBytes == 4) ? "R816_Ctz32" : "R816_Ctz16"));
		maszRuntime.AddIfUnique(szLabel);
		mpcFunction->Argument(mpcBlock, IRTypeOf(psType), sArgument.sValue);
		psValue->sValue = mpcFunction->Call(mpcBlock, IRTypeOf(&mpcTypes->msInt), mcModule.Symbol(szLabel));
		psValue->psType = &mpcTypes->msInt;
		return true;
	}
	if (!bPopcount && (mpcGenerator->mpsTarget->eTarget == CPPT_W65C816))
	{
		return CountTable(bClz, &sArgument, psType, psValue) || Unsupported(__FILE__, __LINE__);
	}
	sType = IRTypeOf(psType);
	sInt = IRTypeOf(&mpcTypes->msInt);
	iBits = psType->iBytes * 8;
	pcLoop = NewBlock();
	pcBody = NewBlock();
	pcZero = bPopcount ? NULL : NewBlock();
	pcDone = NewBlock();
	sTest = mpcFunction->Add(mpcBlock, IRO_Eq, sType, sArgument.sValue, mpcFunction->AddInteger(sType, 0));
	sCount = mpcFunction->AddInteger(sInt, 0);
	if (pcZero && (iBits > 16))
	{
		pcNarrow = NewBlock();
		mpcBlock->Branch(sTest, pcZero, pcNarrow);
		StartBlock(pcNarrow);
		while (iBits > 16)
		{
			NarrowCount(bClz, &sArgument.sValue, &sCount, &iBits, sInt);
		}
		sType = IRType(IRTK_I16);
		mpcBlock->Jump(pcLoop);
	}
	else
	{
		mpcBlock->Branch(sTest, bPopcount ? pcDone : pcZero, pcLoop);
	}
	pcEntry = mpcBlock;
	StartBlock(pcLoop);
	sX = LoopPhi(pcLoop, sType, pcEntry, sArgument.sValue);
	sCounted = LoopPhi(pcLoop, sInt, pcEntry, sCount);
	if (bPopcount)
	{
		sTest = mpcFunction->Add(mpcBlock, IRO_Eq, sType, sX, mpcFunction->AddInteger(sType, 0));
	}
	else
	{
		sTest = mpcFunction->Add(mpcBlock, IRO_And, sType, sX, mpcFunction->AddConstant(sType, !bClz ? 1 : ((uint64)1 << (iBits - 1))));
		sTest = mpcFunction->Add(mpcBlock, IRO_Ne, sType, sTest, mpcFunction->AddInteger(sType, 0));
	}
	mpcBlock->Branch(sTest, pcDone, pcBody);
	StartBlock(pcBody);
	if (bPopcount)
	{
		sX2 = mpcFunction->Add(mpcBlock, IRO_Sub, sType, sX, mpcFunction->AddInteger(sType, 1));
		sX2 = mpcFunction->Add(mpcBlock, IRO_And, sType, sX, sX2);
	}
	else
	{
		sX2 = mpcFunction->Add(mpcBlock, !bClz ? IRO_LShr : IRO_Shl, sType, sX, mpcFunction->AddInteger(sType, 1));
	}
	sCount2 = mpcFunction->Add(mpcBlock, IRO_Add, sInt, sCounted, mpcFunction->AddInteger(sInt, 1));
	mpcBlock->Jump(pcLoop);
	ClosePhi(pcLoop, sX, pcBody, sX2);
	ClosePhi(pcLoop, sCounted, pcBody, sCount2);
	if (pcZero)
	{
		StartBlock(pcZero);
		mpcBlock->Jump(pcDone);
	}
	StartBlock(pcDone);
	psValue->sValue = Phi(pcDone, sInt, pcLoop, sCounted, bPopcount ? pcEntry : pcZero, mpcFunction->AddInteger(sInt, bPopcount ? 0 : psType->iBytes * 8));
	psValue->psType = &mpcTypes->msInt;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A call through pcCallee, a pointer to a function (a variable, a field,
// table[i], GetF()).  A call in the callee isn't the one the object being
// initialised is made by (msElide).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CallThroughPointer(CCPPSyntaxCallExpression* pcCall, CCPPSyntax* pcCallee, SCPP65IRValue* psValue)
{
	SCPP65Type*		psType;
	SCPP65IRValue	sCallee;
	SIRReference	sElide;

	psType = mpcGenerator->TypeOf(pcCallee);
	sElide = msElide;
	msElide = IRNone();
	if (!psType || !psType->IsPointer() || !psType->psBase->IsFunction())
	{
		return Unsupported(__FILE__, __LINE__);
	}
	//The pointer before the arguments.  [expr.call] 8
	if (!Value(pcCallee, &sCallee))
	{
		return false;
	}
	msElide = sElide;
	return Call(NULL, sCallee.sValue, psType->psBase, &pcCall->macpArguments, psValue);
}


//////////////////////////////////////////////////////////////////////////
//
// f(args...): written with the generator's ExpandPackArguments, which replaces
// args... by a name for each of the pack's elements.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ExpandPackCall(CCPPSyntaxCallExpression* pcCall, SCPP65IRValue* psValue)
{
	CArraySyntax	apcArguments;
	bool			bResult;

	if (mpcGenerator->ExpandPackArguments(pcCall, &apcArguments))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	bResult = CallExpression(pcCall, psValue);
	mpcGenerator->RestoreArguments(pcCall, &apcArguments);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The address a member function is called with in *psObject: pcObject's
// (. or ->), sGivenObject, or this; moved to the function's class if that
// is a base.  A virtual call's *psCallee is the object's vtable's entry.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ThisObject(SCPP65Function* psFunction, CCPPSyntax* pcObject, bool bArrow, SCPP65Class* psObjectClass, bool bVirtual, SIRReference sGivenObject, SIRReference* psObject, SIRReference* psCallee)
{
	SIRReference	sVtable;

	if (pcObject)
	{
		if (!ObjectAddress(pcObject, bArrow, psObject))
		{
			return false;
		}
	}
	else if (!sGivenObject.IsNone())
	{
		*psObject = sGivenObject;
	}
	else
	{
		if (msThis.IsNone())
		{
			return Unsupported(__FILE__, __LINE__);
		}
		*psObject = msThis;
		psObjectClass = mpsFunction->psClass;
	}
	if (psFunction)
	{
		*psObject = AddressAt(*psObject, BaseOffset(psObjectClass, psFunction->psClass));
	}
	if (bVirtual && psFunction && psFunction->bVirtual)
	{
		sVtable = mpcFunction->Load(mpcBlock, IRType(IRTK_Pointer), AddressAt(*psObject, psFunction->psClass->iVtableOffset));
		*psCallee = mpcFunction->Load(mpcBlock, IRType(IRTK_Pointer), AddressAt(sVtable, CPP65_VTABLE_ENTRY_BYTES * psFunction->iSlot));
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A member function's object is worked out first (ThisObject; last if
// bObjectLast, for a = b's operator), then the arguments (GatherArguments).
// A reference gets the argument's address.  The result comes back as the
// IR return type and is narrowed to the C++ type.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Call(SCPP65Function* psFunction, SIRReference sCallee, SCPP65Type* psFunctionType, CArraySyntax* papcArguments, SCPP65IRValue* psValue, CCPPSyntax* pcObject, bool bArrow, SCPP65Class* psObjectClass, bool bVirtual, SIRReference sGivenObject, bool bObjectLast)
{
	CArrayCPP65Types*	papsParameters;
	CArraySyntax*		papcDefaults;
	SCPP65Type*			psReturn;
	bool				bVariadic;
	size				uiParameters;
	size				ui;
	SIRReference		sObject;
	bool				bThisCall;
	SIRReference		sMade;
	CArrayIRReferences	asPieces;		//Every argument's value, an object's in pieces.

	if (psFunction)
	{
		papsParameters = &psFunction->mapsParameters;
		papcDefaults = &psFunction->mapcDefaults;
		psReturn = psFunction->psReturn;
		bVariadic = psFunction->bVariadic;
	}
	else
	{
		papsParameters = psFunctionType->papsParameters;
		papcDefaults = NULL;
		psReturn = psFunctionType->psBase;
		bVariadic = psFunctionType->bVariadic;
	}
	//Where an object the call returns is made (msElide is the next call's, not the arguments').
	sMade = msElide;
	msElide = IRNone();
	if ((!IsSupportedType(psReturn) && !psReturn->IsClass()) || psReturn->IsArray() || (psReturn->IsClass() && (bVariadic || !mpcGenerator->CompleteClass(psReturn->psClass, NULL))))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	uiParameters = papsParameters ? papsParameters->NumElements() : 0;
	if ((papcArguments->NumElements() > uiParameters) && !bVariadic)
	{
		return Unsupported(__FILE__, __LINE__);
	}

	//A member function's object before its arguments (after them for an assignment operator's).  [expr.call] 8
	bThisCall = psFunction ? (psFunction->psClass && !psFunction->bStatic) : !sGivenObject.IsNone();
	sObject = IRNone();
	if (bThisCall && !bObjectLast && !ThisObject(psFunction, pcObject, bArrow, psObjectClass, bVirtual, sGivenObject, &sObject, &sCallee))
	{
		return false;
	}

	asPieces.Init();
	if (!ArgumentPieces(papcArguments, papsParameters, papcDefaults, bVariadic, psReturn, &sMade, &asPieces))
	{
		asPieces.Kill();
		return false;
	}
	if (bThisCall && bObjectLast && !ThisObject(psFunction, pcObject, bArrow, psObjectClass, bVirtual, sGivenObject, &sObject, &sCallee))
	{
		asPieces.Kill();
		return false;
	}
	if (bThisCall)
	{
		mpcFunction->Argument(mpcBlock, IRType(IRTK_Pointer), sObject);
	}
	if (psReturn->IsClass())
	{
		mpcFunction->Argument(mpcBlock, IRType(IRTK_Pointer), sMade);
	}
	for (ui = 0; ui < asPieces.NumElements(); ui++)
	{
		mpcFunction->Argument(mpcBlock, mpcFunction->TypeOf(*asPieces.Get(ui)), *asPieces.Get(ui));
	}
	asPieces.Kill();

	CallResult(psReturn, sMade, mpcFunction->Call(mpcBlock, ReturnIRType(psReturn), sCallee, bVariadic ? (int)uiParameters : -1, bThisCall), psValue);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The arguments of a call worked out (GatherArguments), and their values in
// order in pasPieces, objects passed by value as their pieces.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ArgumentPieces(CArraySyntax* papcArguments, CArrayCPP65Types* papsParameters, CArraySyntax* papcDefaults, bool bVariadic, SCPP65Type* psReturn, SIRReference* psMade, CArrayIRReferences* pasPieces)
{
	CArrayTemplate<SCPP65IRValue>	asArguments;
	CArrayIRReferences				asLoaded;		//The pieces of the objects passed by value...
	CArrayTemplate<int32>			aiLoaded;		//...and per argument where its pieces start, -1 for none.
	size							uiArguments;
	size							uiParameters;
	size							ui;
	int32							iLoaded;
	bool							bResult;

	uiArguments = papcArguments->NumElements();
	uiParameters = papsParameters ? papsParameters->NumElements() : 0;
	asArguments.Init();
	asArguments.GrowToAtLeastNumElements(uiArguments > uiParameters ? uiArguments : uiParameters, true, 0);
	asLoaded.Init();
	aiLoaded.Init();
	aiLoaded.GrowToAtLeastNumElements(asArguments.NumElements(), true, 0xFF);
	bResult = GatherArguments(papcArguments, papsParameters, papcDefaults, bVariadic, psReturn, psMade, &asArguments, &asLoaded, &aiLoaded);
	for (ui = 0; bResult && (ui < asArguments.NumElements()); ui++)
	{
		iLoaded = *aiLoaded.Get(ui);
		if (iLoaded < 0)
		{
			pasPieces->Add(&asArguments.Get(ui)->sValue);
		}
		else
		{
			pasPieces->InsertBlockAt(asLoaded.Get(iLoaded), PieceCount(asArguments.Get(ui)->psType->iBytes), pasPieces->NumElements());
		}
	}
	aiLoaded.Kill();
	asLoaded.Kill();
	asArguments.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A call's value: an object is where it was made (sMade), a reference its
// address, and anything else is sResult, as the IR return type, narrowed to
// the C++ type.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::CallResult(SCPP65Type* psReturn, SIRReference sMade, SIRReference sResult, SCPP65IRValue* psValue)
{
	psValue->psType = psReturn;
	if (psReturn->IsClass())
	{
		psValue->sValue = sMade;
	}
	else if (psReturn->IsReference())
	{
		psValue->sValue = sResult;
	}
	else
	{
		psValue->sValue = psReturn->IsVoid() ? IRNone() : ConvertIR(sResult, ReturnIRType(psReturn), IRTypeOf(psReturn), false);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Call's arguments, last first: each converted to its parameter's type
// (missing ones are the defaults; a variadic function's extra ones are
// promoted), an object passed by value loaded as its pieces into pasLoaded
// (paiLoaded: per argument where its pieces start, -1 for none), and the
// temporary an object returned is made in (*psMade, unless it is given).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::GatherArguments(CArraySyntax* papcArguments, CArrayCPP65Types* papsParameters, CArraySyntax* papcDefaults, bool bVariadic, SCPP65Type* psReturn, SIRReference* psMade,
									   CArrayTemplate<SCPP65IRValue>* pasArguments, CArrayIRReferences* pasLoaded, CArrayTemplate<int32>* paiLoaded)
{
	size			uiArguments;
	size			uiParameters;
	size			ui;
	CCPPSyntax*		pcArgument;
	SCPP65Type*		psParameter;
	SCPP65IRValue	sArgument;
	SCPP65IRPlace	sPlace;
	SCPP65IRPlace	sCopy;
	SIRType			sPiece;
	SIRReference	sLoaded;
	int				iOffset;
	int				iPiece;

	uiArguments = papcArguments->NumElements();
	uiParameters = papsParameters ? papsParameters->NumElements() : 0;
	for (ui = uiArguments; ui > uiParameters; ui--)
	{
		pcArgument = papcArguments->GetPtr(ui - 1);
		if (!Value(pcArgument, &sArgument))
		{
			return false;
		}
		if (sArgument.psType->eKind == CPP65TK_Bool)
		{
			psParameter = &mpcTypes->msInt;
		}
		else if (sArgument.psType->IsFloat())
		{
			psParameter = &mpcTypes->msDouble;
		}
		else if (sArgument.psType->IsInteger())
		{
			psParameter = mpcTypes->Promote(sArgument.psType);
		}
		else if (sArgument.psType->IsPointer())
		{
			psParameter = sArgument.psType;
		}
		else
		{
			return Unsupported(__FILE__, __LINE__);
		}
		if (!Convert(&sArgument, psParameter))
		{
			return false;
		}
		*pasArguments->Get(ui - 1) = sArgument;
	}
	for (ui = uiParameters; ui > 0; ui--)
	{
		pcArgument = (ui <= uiArguments) ? papcArguments->GetPtr(ui - 1) : (papcDefaults ? papcDefaults->GetPtr(ui - 1) : NULL);
		psParameter = papsParameters->GetPtr(ui - 1);
		if (pcArgument && psParameter->IsClass() && (!bVariadic || (ui < uiParameters)) && mpcGenerator->CompleteClass(psParameter->psClass, NULL) && IsPassedByAddress(psParameter))
		{
			//A copy made here (and destroyed at the end of the full expression) is passed by its address.  Not a variadic
			//function's last named parameter, whose address va_start takes for the arguments after it.
			if (!Temporary(psParameter, &sCopy, true) || !Construct(&sCopy, NULL, pcArgument))
			{
				return false;
			}
			sArgument.sValue = sCopy.sAddress;
			sArgument.psType = mpcTypes->Pointer(psParameter);
			*pasArguments->Get(ui - 1) = sArgument;
			continue;
		}
		if (!pcArgument || psParameter->IsArray() || (!IsSupportedType(psParameter) && !psParameter->IsClass()) ||
			(psParameter->IsClass() && (bVariadic || !mpcGenerator->CompleteClass(psParameter->psClass, NULL) || !mpcGenerator->IsTriviallyCopyable(psParameter->psClass))))
		{
			return Unsupported(__FILE__, __LINE__);
		}
		mpcGenerator->ConversionOverflow(pcArgument, psParameter);
		if (psParameter->IsClass())
		{
			//An object passed by value is copied a piece at a time (only objects whose copy is their bytes).  One made
			//for the call (not an object lvalue: a call's, Class(...)'s, or one a converting constructor makes) is the
			//parameter, which the called function destroys, so it's made in a temporary the caller doesn't destroy.
			if (!IsObjectLvalue(pcArgument) || !IsObjectExpression(pcArgument))
			{
				if (!Temporary(psParameter, &sCopy, false) || !Construct(&sCopy, NULL, pcArgument))
				{
					return false;
				}
				sArgument.sValue = sCopy.sAddress;
				sArgument.psType = psParameter;
			}
			else if (!Value(pcArgument, &sArgument))
			{
				return false;
			}
			sPlace.sAddress = sArgument.sValue;
			sPlace.psType = sArgument.psType->WithoutReference();
			if (!sPlace.psType->IsClass() || !mpcGenerator->IsDerived(sPlace.psType->psClass, psParameter->psClass, NULL) || !ReferenceAddress(&sPlace, mpcTypes->Pointer(psParameter), &sArgument.sValue))
			{
				return Unsupported(__FILE__, __LINE__);
			}
			sArgument.psType = psParameter;

			//It is copied as it is worked out (the generator constructs it on the stack then), through a temporary for a
			//class with vtables so the copy has its own class's.
			if (mpcGenerator->IsPolymorphic(psParameter->psClass))
			{
				if (!Temporary(psParameter, &sCopy, false) || !CopyBytes(&sCopy, &sArgument))
				{
					return false;
				}
				sArgument.sValue = sCopy.sAddress;
			}
			*paiLoaded->Get(ui - 1) = (int32)pasLoaded->NumElements();
			for (iOffset = 0; iOffset < psParameter->iBytes; iOffset += iPiece)
			{
				iPiece = PieceBytes(psParameter->iBytes - iOffset);
				sPiece = IRIntegerType(iPiece);
				sLoaded = mpcFunction->Load(mpcBlock, sPiece, AddressAt(sArgument.sValue, iOffset));
				pasLoaded->Add(&sLoaded);
			}
		}
		else if (psParameter->IsReference())
		{
			if (mpcGenerator->BindsToTemporary(psParameter, pcArgument))
			{
				if (!ValueTemporary(pcArgument, psParameter->psBase, &sPlace))
				{
					return false;
				}
				sArgument.sValue = sPlace.sAddress;
			}
			else if (!Place(pcArgument, &sPlace) || !ReferenceAddress(&sPlace, psParameter, &sArgument.sValue))
			{
				return false;
			}
			sArgument.psType = psParameter;
		}
		else if (!ValueFor(pcArgument, psParameter, &sArgument) || !Convert(&sArgument, psParameter))
		{
			return false;
		}
		*pasArguments->Get(ui - 1) = sArgument;
	}

	//An object returned is made at the address passed first on the stack (a temporary's, or msElide's).
	if (psReturn->IsClass() && psMade->IsNone())
	{
		if (!Temporary(psReturn, &sPlace, true))
		{
			return false;
		}
		*psMade = sPlace.sAddress;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// object.f(), pointer->f(), f() and Class::f() in a member function, as the
// generator's MemberCall: calls through a pointer or this are virtual; calls
// on an object variable and qualified calls are not.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::MemberCall(CCPPSyntaxCallExpression* pcCall, CCPPSyntax* pcCallee, SCPP65IRValue* psValue)
{
	CCPPSyntaxMemberExpression*	pcMember;
	CCPPSyntaxName*				pcName;
	CCPPSyntaxNameComponent*	pcComponent;
	CCPPSyntax*					pcObject;
	SCPP65Class*				psClass;
	SCPP65Class*				psLookup;
	SCPP65Function*				psFunction;
	SCPP65Template*				psTemplate;
	SCPP65Type*					psType;
	SIRReference				sSymbol;
	int64						lliName;
	int							iOffset;
	int							iConst;
	bool						bVirtual;
	bool						bArrow;
	bool						bThis;

	psClass = NULL;
	pcObject = NULL;
	bVirtual = true;
	bThis = false;
	bArrow = false;
	if (pcCallee->GetKind() == CPPS_MemberExpression)
	{
		pcMember = (CCPPSyntaxMemberExpression*)pcCallee;
		psType = mpcGenerator->TypeOf(pcMember->mpcObject);
		if (!psType)
		{
			return Unsupported(__FILE__, __LINE__);
		}
		bArrow = pcMember->mbArrow;
		if (bArrow)
		{
			psType = ArrowClass(pcMember->mpcObject, psType);
		}
		else if (psType->IsReference())
		{
			psType = psType->psBase;
		}
		if (!psType || !psType->IsClass() || !mpcGenerator->CompleteClass(psType->psClass, pcCallee))
		{
			return Unsupported(__FILE__, __LINE__);
		}
		psClass = psType->psClass;
		pcObject = pcMember->mpcObject;
		pcName = pcMember->mpcName;
		if (!bArrow && (mpcGenerator->SkipParentheses(pcObject)->GetKind() == CPPS_IdExpression))
		{
			bVirtual = false;
		}
	}
	else
	{
		pcName = ((CCPPSyntaxIdExpression*)pcCallee)->mpcName;
		if (pcName->IsQualifiedByClass())
		{
			psClass = mpcGenerator->GetComponentClass(pcName->GetQualifier(), NULL, pcCallee);
			bVirtual = false;
		}
		else
		{
			psClass = mpsFunction->psClass;
		}
		bThis = true;
	}

	if (pcName->GetLast()->meKind == CPPNC_Destructor)
	{
		return DestructorCall(pcCall, psClass, pcObject, bArrow, pcName->IsQualifiedByClass() ? false : bVirtual, psValue);
	}

	lliName = mpcGenerator->GetFunctionName(pcName->GetLast());
	if (!psClass || (lliName == -1))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	psLookup = psClass;
	if (pcObject && pcName->IsQualifiedByClass())
	{
		psLookup = mpcGenerator->QualifierClass(psClass, pcName->GetQualifier(), pcCallee);
		if (!psLookup)
		{
			return Unsupported(__FILE__, __LINE__);
		}
		bVirtual = false;
	}
	if (pcObject && !mpcGenerator->HasMember(psLookup, lliName) && mpcGenerator->FindField(psLookup, lliName, &iOffset))
	{
		//object.pf() calls through a field that points to a function.
		return CallThroughPointer(pcCall, pcCallee, psValue);
	}

	pcComponent = pcName->GetLast();
	iConst = pcObject ? (bArrow ? mpcGenerator->IsConstPointer(pcObject) : mpcGenerator->IsConstObject(pcObject)) : mpcGenerator->IsConstThis();
	psTemplate = mpcGenerator->CompleteClass(psLookup, pcCall) ? mpcGenerator->ChooseMemberTemplate(psLookup, lliName, pcCall, pcComponent->mbTemplateArguments ? &pcComponent->macpTemplateArguments : NULL, iConst) : NULL;
	psFunction = NULL;
	if (!psTemplate || !pcComponent->mbTemplateArguments)
	{
		psFunction = mpcGenerator->FindFunction(psLookup, lliName, CPP65FK_Function, &pcCall->macpArguments, true, pcCall, iConst);
	}
	if (!psFunction && psTemplate)
	{
		psFunction = mpcGenerator->InstantiateFunction(psTemplate, pcComponent->mbTemplateArguments ? &pcComponent->macpTemplateArguments : NULL, &pcCall->macpArguments, pcCall);
	}
	if (!psFunction)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (bThis && !psFunction->bStatic && (!mpsFunction->psClass || msThis.IsNone() || !mpcGenerator->IsDerived(mpsFunction->psClass, psFunction->psClass, NULL)))
	{
		return Unsupported(__FILE__, __LINE__);
	}

	sSymbol = FunctionSymbol(psFunction);
	return Call(psFunction, sSymbol, NULL, &pcCall->macpArguments, psValue, pcObject, bArrow, pcObject ? psClass : NULL, bVirtual);
}


//////////////////////////////////////////////////////////////////////////
//
// An explicit destructor call, p->~X(), o.~X() or ~X() (this's): the class's
// destructor runs on the object (through its vtable unless the call is
// qualified or on a named object), or nothing happens if it has none.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::DestructorCall(CCPPSyntaxCallExpression* pcCall, SCPP65Class* psClass, CCPPSyntax* pcObject, bool bArrow, bool bVirtual, SCPP65IRValue* psValue)
{
	SIRReference	sObject;

	if (!psClass || (pcCall->macpArguments.NumElements() != 0))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	psValue->psType = &mpcTypes->msVoid;
	psValue->sValue = IRNone();
	if (!psClass->psDestructor)
	{
		return true;
	}

	if (pcObject)
	{
		if (!ObjectAddress(pcObject, bArrow, &sObject))
		{
			return false;
		}
	}
	else
	{
		if (msThis.IsNone())
		{
			return Unsupported(__FILE__, __LINE__);
		}
		sObject = msThis;
	}
	return CallObject(psClass->psDestructor, sObject, psClass, NULL, bVirtual);
}


//////////////////////////////////////////////////////////////////////////
//
// object op argument (pcArgument NULL for none): a call of the object's
// member operator, as the generator's OperatorCall.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::OperatorCall(CCPPSyntax* pcObject, ECppTokens eOperator, CCPPSyntax* pcArgument, SCPP65IRValue* psValue)
{
	SCPP65Type*		psType;
	SCPP65Function*	psFunction;
	CArraySyntax	apcArguments;
	bool			bResult;

	psType = mpcGenerator->ValueTypeOf(pcObject);
	if (!psType || !psType->IsClass() || !mpcGenerator->CompleteClass(psType->psClass, pcObject))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	apcArguments.Init();
	if (pcArgument)
	{
		apcArguments.Add(pcArgument);
	}
	psFunction = mpcGenerator->FindOperator(psType->psClass, mpcGenerator->GetOperatorName(eOperator, false), &apcArguments, pcObject, mpcGenerator->IsConstObject(pcObject));
	if (!psFunction || psFunction->bStatic)
	{
		apcArguments.Kill();
		return Unsupported(__FILE__, __LINE__);
	}
	//a = b and a op= b work out b first, other operators their left first, as the built in ones do.  [over.match.oper] 2
	bResult = Call(psFunction, FunctionSymbol(psFunction), NULL, &apcArguments, psValue, pcObject, false, psType->psClass, mpcGenerator->SkipParentheses(pcObject)->GetKind() != CPPS_IdExpression,
				   IRNone(), mpcGenerator->IsAssignmentOperator(eOperator));
	apcArguments.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Calls a member function of the object at sObject (of psObjectClass, which
// is moved on to the function's class if that is a base).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CallObject(SCPP65Function* psFunction, SIRReference sObject, SCPP65Class* psObjectClass, CArraySyntax* papcArguments, bool bVirtual)
{
	SIRReference	sSymbol;
	SCPP65IRValue	sValue;

	sSymbol = FunctionSymbol(psFunction);
	return Call(psFunction, sSymbol, NULL, papcArguments ? papcArguments : &mapcNone, &sValue, NULL, false, psObjectClass, bVirtual, sObject);
}


//////////////////////////////////////////////////////////////////////////
//
// __builtin_memcpy(Dest, Source, Count) is a call of Libc65's memcpy.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::BuiltinMemcpy(CCPPSyntaxCallExpression* pcCall, SCPP65IRValue* psValue)
{
	SCPP65Type*		apsParameter[3];

	apsParameter[0] = mpcTypes->Pointer(&mpcTypes->msVoid);
	apsParameter[1] = apsParameter[0];
	apsParameter[2] = mpcTypes->Long(false);
	mcModule.SetRegisterCall(mcModule.AddSymbol("_memcpy"));
	return CallWithArguments(pcCall, apsParameter, 3, "_memcpy", apsParameter[0], psValue);
}


//////////////////////////////////////////////////////////////////////////
//
// The call's iCount arguments converted to apsParameters' types, passed to
// the function szSymbol, whose result is of psResult's type.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CallWithArguments(CCPPSyntaxCallExpression* pcCall, SCPP65Type** apsParameters, int iCount, const char* szSymbol, SCPP65Type* psResult, SCPP65IRValue* psValue)
{
	SCPP65IRValue	asArgument[IR65_MAX_BUILTIN_ARGUMENTS];
	int				i;

	for (i = 0; i < iCount; i++)
	{
		if (!Value(pcCall->macpArguments.GetPtr(i), &asArgument[i]) || !Convert(&asArgument[i], apsParameters[i]))
		{
			return false;
		}
	}
	for (i = 0; i < iCount; i++)
	{
		mpcFunction->Argument(mpcBlock, IRTypeOf(apsParameters[i]), asArgument[i].sValue);
	}
	psValue->sValue = mpcFunction->Call(mpcBlock, IRTypeOf(psResult), mcModule.Symbol(szSymbol));
	psValue->psType = psResult;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// __builtin_bcd_add(Dest, Source, Words, Carry) and __builtin_bcd_subtract
// (Words packed BCD words of Source added to or subtracted from Dest's,
// returning the carry or borrow; the W65C816's decimal mode) are calls of
// the runtime routines R816_BcdAdd and R816_BcdSubtract.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::BuiltinBCD(CCPPSyntaxCallExpression* pcCall, bool bSubtract, SCPP65IRValue* psValue)
{
	SCPP65Type*		apsParameter[4];
	char*			szLabel;

	apsParameter[0] = mpcTypes->Pointer(&mpcTypes->msVoid);
	apsParameter[1] = apsParameter[0];
	apsParameter[2] = &mpcTypes->msUnsignedInt;
	apsParameter[3] = &mpcTypes->msUnsignedInt;
	szLabel = (char*)(bSubtract ? "R816_BcdSubtract" : "R816_BcdAdd");
	maszRuntime.AddIfUnique(szLabel);
	return CallWithArguments(pcCall, apsParameter, 4, szLabel, &mpcTypes->msUnsignedInt, psValue);
}


//////////////////////////////////////////////////////////////////////////
//
// Calls a runtime routine (_ and its name) with one argument; the generator
// adds it to the unit if the function is written.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CCPP65IRFrontEnd::CallRuntime(const char* szName, SIRType sResult, SIRType sArgument, SIRReference sValue)
{
	CChars			szLabel;
	SIRReference	sSymbol;

	szLabel.Init("_");
	szLabel.Append(szName);
	maszRuntime.AddIfUnique(szLabel);
	sSymbol = mcModule.Symbol(szLabel.Text());
	szLabel.Kill();
	mpcFunction->Argument(mpcBlock, sArgument, sValue);
	return mpcFunction->Call(mpcBlock, sResult, sSymbol);
}
