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
#include "BaseLib/StringHelper.h"
#include "CTIdentifier.h"
#include "CppTokeniser.h"
#include "CPPSyntaxTree.h"
#include "CPP65Generator.h"
#include "IRInliner.h"
#include <stdio.h>
#include <stdint.h>
#include "IRPrinter.h"
#include "IRVerifier.h"
#include "IROptimiser.h"
#include "IR65Backend.h"
#include "IR816Backend.h"
#include "CPP65IRFrontEnd.h"


size	CCPP65IRFrontEnd::suiWritten = 0;
size	CCPP65IRFrontEnd::suiLeft = 0;
int		CCPP65IRFrontEnd::siUnsupportedLine = 0;
char	CCPP65IRFrontEnd::sszUnsupported[1024] = "";
size	CCPP65IRFrontEnd::sauiUnsupported[8192];
bool	CCPP65IRFrontEnd::sbPrint = false;
bool	CCPP65IRFrontEnd::sbOptimise = true;
bool	CCPP65IRFrontEnd::sbInline = true;
size	CCPP65IRFrontEnd::suiAttempts = 0;
size	CCPP65IRFrontEnd::suiLimit = SIZE_MAX;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::Init(CCPP65Generator* pcGenerator)
{
	mpcGenerator = pcGenerator;
	mpcTypes = &pcGenerator->mcTypes;
	mpcFunction = NULL;
	mpcBlock = NULL;
	mpsFunction = NULL;
	msThis = IRNone();
	msResult = IRNone();
	msElide = IRNone();
	mcModule.Init();
	mcModule.AddSpace("default", 4);
	masWritten.Init();
	masJumps.Init();
	masCases.Init();
	masLabels.Init();
	masLiving.Init();
	masTemporaries.Init();
	miConditional = 0;
	mpcExit = NULL;
	mapcNone.Init();
	maszRuntime.Init();
	mauiStringBytes.Init();
	mauiStringLengths.Init();
	muiFirstString = 0;
	mbUnsupported = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::Kill(void)
{
	size	ui;

	mauiStringLengths.Kill();
	mauiStringBytes.Kill();
	maszRuntime.Kill();
	for (ui = 0; ui < masWritten.NumElements(); ui++)
	{
		masWritten.Get(ui)->szLabel.Kill();
		masWritten.Get(ui)->szCode.Kill();
	}
	masWritten.Kill();
	mcModule.Kill();
	mapcNone.Kill();
	masTemporaries.Kill();
	masLiving.Kill();
	masLabels.Kill();
	masCases.Kill();
	masJumps.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Writes psFunction's body (already set up by the generator's
// GenerateDefinition: parameters and locals have their variables) as IR and
// generates it into the generator's code.  False, with nothing changed, if
// the function uses something the front end doesn't support yet; the
// generator's own errors are not reported meanwhile (it reports them when
// it writes the function itself).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Body(SCPP65Function* psFunction)
{
	CChars					szLabel;
	bool					bResult;

	suiAttempts++;
	if (suiAttempts > suiLimit)
	{
		return false;
	}
	if (((psFunction->eKind != CPP65FK_Function) && (psFunction->eKind != CPP65FK_Constructor) && (psFunction->eKind != CPP65FK_Destructor)) ||
		!psFunction->psReturn || (!IsSupportedType(psFunction->psReturn) && !psFunction->psReturn->IsClass()) || psFunction->psReturn->IsArray() ||
		(psFunction->psReturn->IsClass() && !mpcGenerator->CompleteClass(psFunction->psReturn->psClass, NULL)))
	{
		mbUnsupported = false;
		Unsupported(__LINE__);
		suiLeft++;
		sauiUnsupported[siUnsupportedLine & 8191]++;
		return false;
	}

	szLabel.Init();
	mpcGenerator->AppendFunctionLabel(&szLabel, psFunction);
	BeginWriting(psFunction, szLabel.Text(), ReturnIRType(psFunction->psReturn), (psFunction->bVariadic ? IRA_Variadic : 0) | ((psFunction->psClass && !psFunction->bStatic) ? IRA_ThisCall : 0));

	//The generator's errors are not reported while the front end works (it reports them if it writes the function
	//itself).  What it fails to work out comes back as NULL, which the front end doesn't support; quiet errors from
	//its own probing (UsesFloat ...) are not failures.
	mpcGenerator->BeginQuiet();
	bResult = Parameters() && Locals();
	if (bResult && (psFunction->eKind == CPP65FK_Constructor))
	{
		//A member initialiser's temporaries would need destroying after it (not supported).
		bResult = ConstructorStart(psFunction->pcDefinition) && ((masTemporaries.NumElements() == 0) || Unsupported(__LINE__));
	}
	else if (bResult && (psFunction->eKind == CPP65FK_Destructor))
	{
		//The vtables are this class's while its destructor runs; returns go to the end, which destroys the fields and
		//bases.
		mpcExit = NewBlock();
		bResult = StoreVtables(msThis, psFunction->psClass);
	}
	bResult = bResult && (!psFunction->pcDefinition || Statement(psFunction->pcDefinition->mpcBody)) && DestroyLiving(0);
	if (bResult && mpcExit)
	{
		JumpTo(mpcExit);
		StartBlock(mpcExit);
		bResult = DestructorEnd();
		mpcBlock->Return();
	}
	mpcGenerator->EndQuiet();
	bResult = FinishWriting(bResult, szLabel.Text());
	szLabel.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Starts writing an IR function: the front end's state for it, and its entry block.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::BeginWriting(SCPP65Function* psFunction, const char* szLabel, SIRType sReturn, uint32 uiAttributes)
{
	mpsFunction = psFunction;
	mbUnsupported = false;
	masJumps.ReInit();
	masCases.ReInit();
	masLabels.ReInit();
	masLiving.ReInit();
	masTemporaries.ReInit();
	miConditional = 0;
	maszRuntime.ReInit();
	mpcExit = NULL;
	mauiStringBytes.ReInit();
	mauiStringLengths.ReInit();
	muiFirstString = mpcGenerator->maszStrings.NumElements();
	mcAddresses.Init(true);
	mpcFunction = mcModule.AddFunction(szLabel, sReturn, uiAttributes);
	msThis = IRNone();
	msResult = IRNone();
	msElide = IRNone();
	mpcBlock = mpcFunction->AddBlock();
}


//////////////////////////////////////////////////////////////////////////
//
// Finishes the function BeginWriting started (bResult false if writing it failed): verifies, optimises and generates
// it, and leaves a marker where its code goes in the generator's code (FinishUnit).  False, with nothing changed,
// if it didn't verify or generate.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::FinishWriting(bool bResult, const char* szLabel)
{
	CCPPErrors				cErrors;
	CIRVerifier				cVerifier;
	CIROptimiser			cOptimiser;
	CChars					szCode;
	SCPP65IRWritten*		psWritten;
	size					ui;
	size					uiStart;
	size					uiLength;

	bResult = bResult && !mbUnsupported;
	if (bResult)
	{
		//IR that doesn't verify is the front end's (or a pass's) mistake; the generator writes the function instead.
		Finish();
		if (sbOptimise)
		{
			cOptimiser.Init(&mcModule);
			cOptimiser.Optimise(mpcFunction);
			cOptimiser.Kill();
		}
		cErrors.Init();
		cVerifier.Init(&mcModule, &cErrors);
		bResult = cVerifier.VerifyFunction(mpcFunction);
		if (!bResult)
		{
			siUnsupportedLine = __LINE__;
			StrCpySafe(sszUnsupported, cErrors.Text(), sizeof(sszUnsupported));
		}
		cVerifier.Kill();
		cErrors.Kill();
		if (sbPrint)
		{
			fprintf(stderr, "Function %d\n", (int)suiAttempts);
			PrintIR(mpcFunction);
		}
	}

	if (bResult)
	{
		szCode.Init();
		bResult = GenerateCode(mpcFunction, szLabel, &szCode);
		if (bResult)
		{
			//The code waits (with a marker where it goes) for FinishUnit, which inlines across the unit.
			psWritten = masWritten.Add();
			psWritten->pcFunction = mpcFunction;
			psWritten->szLabel.Init(szLabel);
			psWritten->szCode.Init(szCode);
			mpcGenerator->mszCode.Append(IR65_FRONT_END_MARKER);
			mpcGenerator->mszCode.Append((int)(masWritten.NumElements() - 1));
			mpcGenerator->mszCode.Append(IR65_FRONT_END_MARKER);
			for (ui = 0; ui < maszRuntime.NumElements(); ui++)
			{
				mpcGenerator->maszRuntime.AddIfUnique(*maszRuntime.Get(ui));
			}
			uiStart = 0;
			for (ui = 0; ui < mauiStringLengths.NumElements(); ui++)
			{
				uiLength = *mauiStringLengths.Get(ui);
				mpcGenerator->AddString((char*)mauiStringBytes.GetData() + uiStart, uiLength);
				uiStart += uiLength;
			}
		}
		szCode.Kill();
	}

	if (bResult)
	{
		//As the generator's EndFunction leaves it.
		mpcGenerator->mpszOut = &mpcGenerator->mszCode;
		mpcGenerator->mpsFunction = NULL;
		mpcGenerator->miThisOffset = -1;
	}
	if (!bResult && mpcFunction)
	{
		mcModule.RemoveFunction(mpcFunction);
	}
	mcAddresses.Kill();
	mpcFunction = NULL;
	mpcBlock = NULL;
	mpsFunction = NULL;
	if (bResult)
	{
		suiWritten++;
	}
	else
	{
		suiLeft++;
		sauiUnsupported[siUnsupportedLine & 8191]++;
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A global's R_Initialise (constructs it from pcInitialiser) or, with bFinalise, its R_Finalise (destroys it), as a
// function of its own named szLabel that takes and returns nothing.  For a target the generator doesn't write code
// for.  False if the front end can't write it.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::GlobalRoutine(const char* szLabel, const char* szGlobal, SCPP65Type* psType, CCPPSyntax* pcInitialiser, bool bFinalise)
{
	SCPP65Function	sFunction;
	SCPP65IRPlace	sPlace;
	bool			bResult;

	memset(&sFunction, 0, sizeof(SCPP65Function));
	sFunction.eKind = CPP65FK_Function;
	sFunction.psReturn = &mpcTypes->msVoid;
	BeginWriting(&sFunction, szLabel, IRType(IRTK_None), IRA_Local);
	mpcGenerator->BeginQuiet();
	sPlace.sAddress = mcModule.Symbol(szGlobal);
	sPlace.psType = psType;
	if (bFinalise)
	{
		bResult = Destroy(sPlace.sAddress, psType);
	}
	else
	{
		bResult = Construct(&sPlace, NULL, pcInitialiser) && DestroyTemporaries(0);
	}
	mpcGenerator->EndQuiet();
	return FinishWriting(bResult, szLabel);
}


//////////////////////////////////////////////////////////////////////////
//
// Notes that the function can't be written by the front end, and the line of
// this file that found it (for finding what to support next).  Always false.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Unsupported(int iLine)
{
	if (!mbUnsupported)
	{
		siUnsupportedLine = iLine;
	}
	mbUnsupported = true;
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The IR type of a C++ value's type.  bool is a byte, an enum its integer,
// pointers, references, arrays and functions are pointers.
//
//////////////////////////////////////////////////////////////////////////
SIRType CCPP65IRFrontEnd::IRTypeOf(SCPP65Type* psType)
{
	switch (psType->eKind)
	{
	case CPP65TK_Void:
		return IRType(IRTK_None);
	case CPP65TK_Bool:
		return IRType(IRTK_I8);
	case CPP65TK_Integer:
		return IRIntegerType(psType->iBytes);
	case CPP65TK_Float:
		return IRType((psType->iBytes == 8) ? IRTK_F64 : IRTK_F32);
	case CPP65TK_MemberPointer:
		return IRType(IRTK_I32);
	default:
		return IRType(IRTK_Pointer);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// On the 65C64 a result narrower than 4 bytes comes back extended to 32 bits
// in A, so the IR returns an i32.
//
//////////////////////////////////////////////////////////////////////////
SIRType CCPP65IRFrontEnd::ReturnIRType(SCPP65Type* psType)
{
	if (mpcGenerator->mpsTarget->bWidenResults && psType->IsScalar() && !psType->IsPointer() && (psType->iBytes < 4))
	{
		return IRType(IRTK_I32);
	}
	return IRTypeOf(psType);
}


//////////////////////////////////////////////////////////////////////////
//
// The types the front end supports for values, parameters and locals.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::IsSupportedType(SCPP65Type* psType)
{
	if (!psType)
	{
		return false;
	}
	if (psType->IsReference())
	{
		return !psType->psBase->IsReference();
	}
	if (psType->IsArray())
	{
		return IsSupportedType(psType->psBase);
	}
	return psType->IsVoid() || (psType->IsScalar() && (psType->iBytes <= 8));
}


//////////////////////////////////////////////////////////////////////////
//
// The type of an array's elements (of the innermost array's); the type
// itself if it isn't an array.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65IRFrontEnd::ElementType(SCPP65Type* psType)
{
	while (psType && psType->IsArray())
	{
		psType = psType->psBase;
	}
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// True if a place (not a value) can have psType: what IsSupportedType
// allows, and classes and arrays of them (used by their address).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::IsPlaceType(SCPP65Type* psType)
{
	SCPP65Type*	psElement;

	psElement = ElementType(psType);
	return psElement && (psElement->IsClass() || IsSupportedType(psElement));
}


//////////////////////////////////////////////////////////////////////////
//
// True if a local can have psType: what IsSupportedType allows, and
// classes (and arrays of them) that are complete.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::IsLocalType(SCPP65Type* psType)
{
	SCPP65Type*	psElement;

	psElement = ElementType(psType);
	if (psElement && psElement->IsClass())
	{
		return mpcGenerator->CompleteClass(psElement->psClass, NULL);
	}
	return IsSupportedType(psType);
}


//////////////////////////////////////////////////////////////////////////
//
// The address a reference of type psReference bound to psPlace holds: a
// derived class's object is referred to as its base.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ReferenceAddress(SCPP65IRPlace* psPlace, SCPP65Type* psReference, SIRReference* psAddress)
{
	SCPP65IRValue	sPointer;

	sPointer.sValue = psPlace->sAddress;
	sPointer.psType = mpcTypes->Pointer(psPlace->psType);
	if (psPlace->psType->IsClass() && psReference->psBase->IsClass() && !Convert(&sPointer, mpcTypes->Pointer(psReference->psBase)))
	{
		return false;
	}
	*psAddress = sPointer.sValue;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIRBlock* CCPP65IRFrontEnd::NewBlock(void)
{
	return mpcFunction->AddBlock();
}


//////////////////////////////////////////////////////////////////////////
//
// Code is now written in pcBlock.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::StartBlock(CIRBlock* pcBlock)
{
	mpcBlock = pcBlock;
}


//////////////////////////////////////////////////////////////////////////
//
// Ends the current block with a jump to pcBlock, if it hasn't ended.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::JumpTo(CIRBlock* pcBlock)
{
	if (!mpcBlock->HasJump())
	{
		mpcBlock->Jump(pcBlock);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// After a return, break, continue or goto the code that follows can't be
// reached; it goes in a block of its own.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Terminated(void)
{
	StartBlock(NewBlock());
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIRReference CCPP65IRFrontEnd::Integer(SCPP65Type* psType, int64 lliValue)
{
	return mpcFunction->AddInteger(IRTypeOf(psType), lliValue);
}


//////////////////////////////////////////////////////////////////////////
//
// A pointer offset (i32 on the 65C64).
//
//////////////////////////////////////////////////////////////////////////
SIRReference CCPP65IRFrontEnd::Offset(int iBytes)
{
	return mpcFunction->AddInteger(IRIntegerType(mcModule.GetPointerSize(0)), iBytes);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIRReference CCPP65IRFrontEnd::AddressAt(SIRReference sAddress, int iOffset)
{
	if (iOffset == 0)
	{
		return sAddress;
	}
	return mpcFunction->Add(mpcBlock, IRO_PtrAdd, IRType(IRTK_Pointer), sAddress, Offset(iOffset));
}


//////////////////////////////////////////////////////////////////////////
//
// The IR parameters are the function's, in order, at their own sizes.  Each
// named one's place is where it was pushed (argaddr), as the generator's
// parameters are.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Parameters(void)
{
	size					ui;
	size					uiParameter;
	SCPP65Type*				psType;
	SCPP65Variable*			psVariable;
	int						iOffset;
	SIRReference			sAddress;
	SCPP65IRPlace			sPlace;
	CArrayTemplate<int32>	aiFirst;	//Per C++ parameter: its first IR parameter.
	int32					iFirst;

	//A member function's this comes first (in X), then the address a function that returns an object makes it at
	//(the first argument on the stack).
	if (mpcFunction->muiAttributes & IRA_ThisCall)
	{
		msThis = mpcFunction->AddParameter(IRType(IRTK_Pointer));
	}
	if (mpsFunction->psReturn->IsClass())
	{
		msResult = mpcFunction->AddParameter(IRType(IRTK_Pointer));
	}
	aiFirst.Init();
	for (ui = 0; ui < mpsFunction->mapsParameters.NumElements(); ui++)
	{
		psType = mpsFunction->mapsParameters.GetPtr(ui);
		iFirst = (int32)mpcFunction->masParameters.NumElements();
		aiFirst.Add(&iFirst);
		if (psType->IsClass())
		{
			//An object passed by value is its bytes where the caller pushed them.
			if (!mpcGenerator->CompleteClass(psType->psClass, NULL))
			{
				aiFirst.Kill();
				return Unsupported(__LINE__);
			}
			AddPieces(psType->iBytes);
			continue;
		}
		if (!IsSupportedType(psType) || psType->IsVoid() || psType->IsArray())
		{
			aiFirst.Kill();
			return Unsupported(__LINE__);
		}
		mpcFunction->AddParameter(IRTypeOf(psType));
	}

	//The generator's parameter variables are known by their offsets after the first argument (the result address
	//is first).  Object parameters are the called function's to destroy.
	for (ui = 0; ui < mpcGenerator->mapsLocals.NumElements(); ui++)
	{
		psVariable = mpcGenerator->mapsLocals.GetPtr(ui);
		if (psVariable->eStorage != CPP65S_Parameter)
		{
			continue;
		}
		iOffset = msResult.IsNone() ? 0 : 4;
		for (uiParameter = 0; uiParameter < mpsFunction->mapsParameters.NumElements(); uiParameter++)
		{
			if (iOffset == psVariable->iOffset)
			{
				break;
			}
			iOffset += mpsFunction->mapsParameters.GetPtr(uiParameter)->iBytes;
		}
		if (uiParameter == mpsFunction->mapsParameters.NumElements())
		{
			aiFirst.Kill();
			return Unsupported(__LINE__);
		}
		sAddress = mpcFunction->Add(mpcBlock, IRO_ArgAddress, IRType(IRTK_Pointer), mpcFunction->AddInteger(IRType(IRTK_I32), *aiFirst.Get(uiParameter)));
		mcAddresses.Put((int64)(size)psVariable, &sAddress);
		if (psVariable->psType->IsClass())
		{
			sPlace.sAddress = sAddress;
			sPlace.psType = psVariable->psType;
			AddLiving(&sPlace);
		}
	}
	aiFirst.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Parameters for iBytes of an object passed by value: as many i32s as fit,
// then an i16 and an i8 for what is left, so the next parameter is where
// the caller pushed it.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::AddPieces(int iBytes)
{
	int		iOffset;
	int		iPiece;

	for (iOffset = 0; iOffset < iBytes; iOffset += iPiece)
	{
		iPiece = PieceBytes(iBytes - iOffset);
		mpcFunction->AddParameter(IRIntegerType(iPiece));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// How many pieces iBytes of an object are (see PieceBytes).
//
//////////////////////////////////////////////////////////////////////////
int CCPP65IRFrontEnd::PieceCount(int iBytes)
{
	int		iOffset;
	int		iCount;

	iCount = 0;
	for (iOffset = 0; iOffset < iBytes; iOffset += PieceBytes(iBytes - iOffset))
	{
		iCount++;
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
// The bytes the next piece of an object copied a piece at a time takes,
// with iLeft bytes left: 4, 2 or 1.
//
//////////////////////////////////////////////////////////////////////////
int CCPP65IRFrontEnd::PieceBytes(int iLeft)
{
	return (iLeft >= 4) ? 4 : ((iLeft >= 2) ? 2 : 1);
}


//////////////////////////////////////////////////////////////////////////
//
// Generates a function with the target's back end: CIR65Backend, or CIR816Backend
// for the W65C816 (whose runtime routines the generator then writes).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::GenerateCode(CIRFunction* pcFunction, const char* szLabel, CChars* pszCode)
{
	CIR65Backend	cBackend;
	CIR816Backend	cBackend816;
	CArrayChars*	paszRuntime;
	size			ui;
	bool			bResult;

	if (mpcGenerator->mpsTarget->eTarget == CPPT_W65C816)
	{
		cBackend816.Init(&mcModule, &mpcGenerator->miLabels);
		bResult = cBackend816.GenerateFunction(pcFunction, szLabel, pszCode, NULL);
		paszRuntime = cBackend816.GetRuntime();
		for (ui = 0; bResult && (ui < paszRuntime->NumElements()); ui++)
		{
			mpcGenerator->maszRuntime.AddIfUnique(paszRuntime->GetText(ui));
		}
		cBackend816.Kill();
		return bResult;
	}

	cBackend.Init(&mcModule, &mpcGenerator->miLabels, &mpcGenerator->maszRuntime);
	bResult = cBackend.GenerateFunction(pcFunction, szLabel, pszCode, NULL);
	mpcGenerator->mbSignedDivide |= cBackend.UsesSignedDivide();
	mpcGenerator->mbSignedDivide64 |= cBackend.UsesSignedDivide64();
	cBackend.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// After the generator has written every function of the unit: small
// functions are inlined into the functions written as IR (callees, which
// are written after their callers, first), which are optimised and
// generated again, and each function's code replaces its marker in the
// generator's code.  A function that then fails keeps the code Body
// generated (and isn't inlined into others).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::FinishUnit(void)
{
	CIRInliner			cInliner;
	CIROptimiser		cOptimiser;
	CIRVerifier			cVerifier;
	CCPPErrors			cErrors;
	CChars				szCode;
	CChars				szAll;
	SCPP65IRWritten*	psWritten;
	size				ui;
	char*				szMarker;
	char*				szText;
	int					iIndex;
	int					iRound;
	bool				bResult;

	cInliner.Init(&mcModule, IR65_INLINE_INSTRUCTIONS, IR65_INLINE_LOOP_INSTRUCTIONS, IR65_INLINE_CALLS);
	for (ui = masWritten.NumElements(); sbInline && (ui > 0); ui--)
	{
		psWritten = masWritten.Get(ui - 1);
		if (!cInliner.InlineCalls(psWritten->pcFunction))
		{
			continue;
		}
		//Optimising can make an indirect call direct (a function pointer argument, a vtable entry), which is then
		//inlined too.
		for (iRound = 0; iRound < IR65_INLINE_ROUNDS; iRound++)
		{
			cOptimiser.Init(&mcModule);
			cOptimiser.Optimise(psWritten->pcFunction);
			cOptimiser.Kill();
			if ((iRound + 1 == IR65_INLINE_ROUNDS) || !cInliner.InlineCalls(psWritten->pcFunction))
			{
				break;
			}
		}
		if (sbPrint)
		{
			fprintf(stderr, "Inlined\n");
			PrintIR(psWritten->pcFunction);
		}
		cErrors.Init();
		cVerifier.Init(&mcModule, &cErrors);
		bResult = cVerifier.VerifyFunction(psWritten->pcFunction);
		cVerifier.Kill();
		cErrors.Kill();
		szCode.Init();
		if (bResult)
		{
			bResult = GenerateCode(psWritten->pcFunction, psWritten->szLabel.Text(), &szCode);
		}
		if (bResult)
		{
			psWritten->szCode.Kill();
			psWritten->szCode.Init(szCode);
		}
		else
		{
			mcModule.RemoveFunction(psWritten->pcFunction);
			psWritten->pcFunction = NULL;
		}
		szCode.Kill();
	}
	cInliner.Kill();

	//The markers become the code.
	szAll.Init();
	szText = mpcGenerator->mszCode.Text();
	for (;;)
	{
		szMarker = strchr(szText, IR65_FRONT_END_MARKER[0]);
		if (!szMarker)
		{
			szAll.Append(szText);
			break;
		}
		szAll.AppendSubString(szText, szMarker);
		iIndex = atoi(szMarker + 1);
		psWritten = masWritten.Get((size)iIndex);
		if (sbPrint)
		{
			fprintf(stderr, "%s\n", psWritten->szCode.Text());
		}
		szAll.Append(psWritten->szCode);
		szText = strchr(szMarker + 1, IR65_FRONT_END_MARKER[0]) + 1;
	}
	mpcGenerator->mszCode.Kill();
	mpcGenerator->mszCode.Init(&szAll);
	szAll.Kill();
	for (ui = 0; ui < masWritten.NumElements(); ui++)
	{
		masWritten.Get(ui)->szLabel.Kill();
		masWritten.Get(ui)->szCode.Kill();
	}
	masWritten.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// Each local variable is a slot.  Static locals are the generator's data.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Locals(void)
{
	size			ui;
	SCPP65Variable*	psVariable;
	SIRReference	sAddress;
	int				iAlign;

	for (ui = 0; ui < mpcGenerator->mapsLocals.NumElements(); ui++)
	{
		psVariable = mpcGenerator->mapsLocals.GetPtr(ui);
		if (psVariable->eStorage != CPP65S_Local)
		{
			continue;
		}
		if (!IsLocalType(psVariable->psType) || psVariable->psType->IsVoid())
		{
			return Unsupported(__LINE__);
		}
		iAlign = ((psVariable->psType->iBytes >= 4) || psVariable->psType->IsArray() || psVariable->psType->IsClass()) ? 4 : psVariable->psType->iBytes;
		sAddress = mpcFunction->Slot(mpcBlock, IRType(IRTK_Pointer), (uint32)psVariable->psType->iBytes, (uint32)iAlign);
		mcAddresses.Put((int64)(size)psVariable, &sAddress);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Blocks left without a jump (the end of the body, and code after a return)
// return: nothing, or 0.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::Finish(void)
{
	size		ui;
	CIRBlock*	pcBlock;
	SIRType		sType;

	sType = mpcFunction->msReturnType;
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		if (!pcBlock->HasJump())
		{
			if (sType.IsNone())
			{
				pcBlock->Return();
			}
			else
			{
				pcBlock->Return(sType, mpcFunction->AddConstant(sType, 0));
			}
		}
	}
	mpcFunction->ComputePredecessors();
}


//////////////////////////////////////////////////////////////////////////
//
// A statement (StatementOf), after which none of the temporaries it made
// may still need destroying.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Statement(CCPPSyntax* pcStatement)
{
	size	uiTemporaries;

	//Expression, declaration and return statements destroy their temporaries; one left (made in a condition, say) isn't
	//supported.
	uiTemporaries = masTemporaries.NumElements();
	if (!StatementOf(pcStatement))
	{
		return false;
	}
	if (masTemporaries.NumElements() > uiTemporaries)
	{
		return Unsupported(__LINE__);
	}
	return true;
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
	size							uiTemporaries;

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
		uiTemporaries = masTemporaries.NumElements();
		if (pcExpression->mpcExpression && !Expression(pcExpression->mpcExpression, &sValue))
		{
			return false;
		}
		return DestroyTemporaries(uiTemporaries);
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
		return BreakStatement(false);
	case CPPS_ContinueStatement:
		return BreakStatement(true);
	case CPPS_GotoStatement:
		return GotoStatement((CCPPSyntaxGotoStatement*)pcStatement);
	case CPPS_LabelStatement:
		return LabelStatement((CCPPSyntaxLabelStatement*)pcStatement);
	default:
		return Unsupported(__LINE__);
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
	if (ElementType(psPlace->psType)->psClass->psDestructor)
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
// The temporaries made since uiTemporaries, at the end of their full
// expression, last made first.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::DestroyTemporaries(size uiTemporaries)
{
	if (!DestroyPlaces(&masTemporaries, uiTemporaries))
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
// Locals already have their slots; this runs their initialisers.  Local
// function declarations, typedefs and static_asserts were dealt with by the
// generator's set up, or need nothing.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::DeclarationStatement(CCPPSyntaxDeclarationStatement* pcStatement)
{
	CCPPSyntaxSimpleDeclaration*	pcDeclaration;
	CCPPSyntaxInitDeclarator*		pcInit;
	SCPP65Variable*					psVariable;
	SCPP65IRPlace					sPlace;
	size							ui;
	size							uiTemporaries;

	if (!pcStatement->mpcDeclaration || (pcStatement->mpcDeclaration->GetKind() != CPPS_SimpleDeclaration))
	{
		return Unsupported(__LINE__);
	}
	pcDeclaration = (CCPPSyntaxSimpleDeclaration*)pcStatement->mpcDeclaration;
	if (pcDeclaration->mpcSpecifiers && (pcDeclaration->mpcSpecifiers->muiFlags & CPPDS_TYPEDEF))
	{
		return true;
	}
	for (ui = 0; ui < pcDeclaration->macpDeclarators.NumElements(); ui++)
	{
		pcInit = (CCPPSyntaxInitDeclarator*)pcDeclaration->macpDeclarators.GetPtr(ui);
		if (pcInit->mpcDeclarator->IsFunction())
		{
			return Unsupported(__LINE__);
		}
		psVariable = pcInit->mpcSymbol ? mpcGenerator->GetVariable(pcInit->mpcSymbol) : NULL;
		if (!psVariable)
		{
			return Unsupported(__LINE__);
		}
		if (psVariable->eStorage == CPP65S_Static)
		{
			continue;
		}
		if (psVariable->eStorage != CPP65S_Local)
		{
			return Unsupported(__LINE__);
		}
		sPlace.sAddress = *mcAddresses.Get((int64)(size)psVariable);
		sPlace.psType = psVariable->psType;
		uiTemporaries = masTemporaries.NumElements();
		if (ElementType(psVariable->psType)->IsClass())
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
		if (psVariable->psType->IsReference() && (masTemporaries.NumElements() > uiTemporaries))
		{
			return Unsupported(__LINE__);
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
			return Unsupported(__LINE__);
		}
		pcInitialiser = papcElements->GetPtr(0);
	}

	if (!IsSupportedType(psPlace->psType))
	{
		//A class (or array of them) is initialised by the generator.
		return Unsupported(__LINE__);
	}
	if (psPlace->psType->IsArray())
	{
		return ArrayInitialiser(psPlace, pcInitialiser);
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
			return Unsupported(__LINE__);
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
	if (!Value(pcInitialiser, &sValue) || !Convert(&sValue, psPlace->psType))
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
	if ((pcInitialiser->GetKind() == CPPS_StringExpression) && (psElement->iBytes == 1) && psElement->IsScalar())
	{
		auiBytes.Init();
		if (!mpcGenerator->DecodeString(pcInitialiser, &auiBytes) || (auiBytes.NumElements() > (size)psPlace->psType->iCount))
		{
			auiBytes.Kill();
			return Unsupported(__LINE__);
		}
		for (ui = 0; ui < auiBytes.NumElements(); ui++)
		{
			sPlace.sAddress = AddressAt(psPlace->sAddress, (int)ui);
			sPlace.psType = psElement;
			sValue.sValue = Integer(psElement, auiBytes.GetValue(ui));
			sValue.psType = psElement;
			Store(&sPlace, &sValue);
		}
		ZeroElements(psPlace->sAddress, (int)ui, psElement, psPlace->psType->iCount - (int)ui);
		auiBytes.Kill();
		return true;
	}
	if (pcInitialiser->GetKind() != CPPS_InitializerList)
	{
		return Unsupported(__LINE__);
	}
	papcElements = &((CCPPSyntaxInitializerList*)pcInitialiser)->macpElements;
	uiNext = 0;
	if (!ArrayElements(psPlace, papcElements, &uiNext))
	{
		return false;
	}
	return (uiNext >= papcElements->NumElements()) || Unsupported(__LINE__);
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
		return Unsupported(__LINE__);
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
				return Unsupported(__LINE__);
			}
			pcElement = papcOne->GetPtr(0);
		}
		if (!Value(pcElement, &sValue) || !Convert(&sValue, psElement))
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
// the array at sAddress.
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
	if (!Condition(pcIf->mpcCondition, pcThen, pcElse))
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
	if (!Condition(pcWhile->mpcCondition, pcBody, pcEnd))
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
	if (!Condition(pcDo->mpcCondition, pcBody, pcEnd))
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
	pcTest = NewBlock();
	pcBody = NewBlock();
	pcNext = NewBlock();
	pcEnd = NewBlock();
	JumpTo(pcTest);
	StartBlock(pcTest);
	if (pcFor->mpcCondition)
	{
		if (!Condition(pcFor->mpcCondition, pcBody, pcEnd))
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
	if (pcFor->mpcIncrement && !Expression(pcFor->mpcIncrement, &sValue))
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

	if (!pcSwitch->mpcCondition || (pcSwitch->mpcCondition->GetKind() == CPPS_SimpleDeclaration))
	{
		return Unsupported(__LINE__);
	}
	if (!Value(pcSwitch->mpcCondition, &sValue))
	{
		return false;
	}
	if (!sValue.psType->IsInteger())
	{
		return Unsupported(__LINE__);
	}
	if (!Convert(&sValue, mpcTypes->Promote(sValue.psType)))
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
			return Unsupported(__LINE__);
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
				return Unsupported(__LINE__);
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
	return Unsupported(__LINE__);
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

	psReturn = mpsFunction->psReturn;
	sType = mpcFunction->msReturnType;
	uiTemporaries = masTemporaries.NumElements();
	//The value is worked out, then the living objects are destroyed.  An object is made at the result address, which
	//is returned.
	if (psReturn->IsClass())
	{
		if (!pcReturn->mpcValue)
		{
			return Unsupported(__LINE__);
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
		return Unsupported(__LINE__);
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
		if (!Value(pcReturn->mpcValue, &sValue) || !Convert(&sValue, psReturn))
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
bool CCPP65IRFrontEnd::BreakStatement(bool bContinue)
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
	return Unsupported(__LINE__);
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
		return Unsupported(__LINE__);
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
		return Unsupported(__LINE__);
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

	pcToken = mpcGenerator->mpcTokeniser->Get(iToken);
	if (!pcToken || !pcToken->IsIdentifier())
	{
		return NULL;
	}
	lliName = ((CCTIdentifier*)pcToken)->mlliIdentifier;
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
// An expression's value (a reference's is its object's, an array's the
// address of its first element).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Value(CCPPSyntax* pcExpression, SCPP65IRValue* psValue)
{
	SCPP65Type*						psType;
	SCPP65IRPlace					sPlace;
	CCPPSyntaxNamedCastExpression*	pcNamedCast;
	CCPPSymbol*						pcSymbol;

	pcExpression = mpcGenerator->SkipParentheses(pcExpression);
	if (!pcExpression || mbUnsupported)
	{
		return Unsupported(__LINE__);
	}
	psType = mpcGenerator->TypeOf(pcExpression);
	if (!psType || (!IsSupportedType(psType) && !psType->IsFunction() && !psType->IsClass()))
	{
		return Unsupported(__LINE__);
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
		return ConstantValue(pcExpression, psValue) || Unsupported(__LINE__);
	case CPPS_StringExpression:
		return StringLiteral(pcExpression, psValue);
	case CPPS_IdExpression:
		pcSymbol = ((CCPPSyntaxIdExpression*)pcExpression)->mpcName->mpcSymbol;
		if (mpcGenerator->IsFunctionName((CCPPSyntaxIdExpression*)pcExpression))
		{
			return FunctionName((CCPPSyntaxIdExpression*)pcExpression, psValue);
		}
		if (pcSymbol && (pcSymbol->meKind == CPPSK_Function))
		{
			return FunctionAddress((CCPPSyntaxIdExpression*)pcExpression, NULL, psValue);
		}
		if (pcSymbol && (pcSymbol->meKind == CPPSK_FunctionTemplate))
		{
			return Unsupported(__LINE__);
		}
		//A name without a symbol (a member in a template the parser could not resolve) is a place.
		if (pcSymbol && ((pcSymbol->meKind == CPPSK_Enumerator) || (pcSymbol->meKind == CPPSK_ValueParameter)))
		{
			return ConstantValue(pcExpression, psValue) || Unsupported(__LINE__);
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
	case CPPS_MemberExpression:
		break;
	case CPPS_ThisExpression:
		if (msThis.IsNone())
		{
			return Unsupported(__LINE__);
		}
		psValue->sValue = msThis;
		psValue->psType = mpcTypes->Pointer(mpsFunction->psClass->psType);
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
			return Unsupported(__LINE__);
		}
		return CastExpression(pcNamedCast->mpcType, pcNamedCast->mpcOperand, psValue);
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
		return Unsupported(__LINE__);
	}
	psValue->sValue = mpcFunction->Load(mpcBlock, IRTypeOf(psPlace->psType), psPlace->sAddress);
	psValue->psType = psPlace->psType;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The value must already be the place's type.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::Store(SCPP65IRPlace* psPlace, SCPP65IRValue* psValue)
{
	mpcFunction->Store(mpcBlock, IRTypeOf(psPlace->psType), psValue->sValue, psPlace->sAddress);
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
	if (psType->IsReference())
	{
		psType = psType->psBase;
	}
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
		return Unsupported(__LINE__);
	}
	StringData((char*)auiBytes.GetData(), auiBytes.NumElements(), psValue);
	auiBytes.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// __func__ and the other names of the function: a string, as the
// generator's IdExpression.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::FunctionName(CCPPSyntaxIdExpression* pcId, SCPP65IRValue* psValue)
{
	CChars	szName;

	szName.Init();
	if (!mpcGenerator->FunctionNameText(mpcGenerator->GetComponentName(pcId->mpcName->GetLast()), &szName))
	{
		szName.Kill();
		return Unsupported(__LINE__);
	}
	StringData(szName.Text(), szName.Length(), psValue);
	szName.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A function's name used as a value is its address, as the generator's
// FunctionAddress: Class::f and, in a member function, f may be a static
// member function (of the class or its bases).  An overloaded name needs
// the type of the function wanted (psTarget, from a cast).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::FunctionAddress(CCPPSyntaxIdExpression* pcId, SCPP65Type* psTarget, SCPP65IRValue* psValue)
{
	SCPP65Function*		psFunction;
	SCPP65Function*		psFound;
	SCPP65Class*		psClass;
	CCPPSyntaxName*		pcName;
	int64				lliName;
	size				ui;
	bool				bOverloaded;
	bool				bQualified;

	pcName = pcId->mpcName;
	lliName = pcName->mpcSymbol->mlliName;
	psFound = NULL;
	bOverloaded = false;
	psClass = NULL;
	bQualified = pcName->macpComponents.NumElements() >= 2;
	if (bQualified)
	{
		psClass = mpcGenerator->GetComponentClass((CCPPSyntaxNameComponent*)pcName->macpComponents.GetPtr(pcName->macpComponents.NumElements() - 2), NULL, pcId);
	}
	else if (mpsFunction->psClass)
	{
		psClass = mpsFunction->psClass;
	}
	if (psClass && mpcGenerator->CompleteClass(psClass, pcId))
	{
		psFound = mpcGenerator->FindStaticFunction(psClass, lliName, psTarget, &bOverloaded);
	}
	for (ui = 0; !psFound && !bQualified && (ui < mpcGenerator->mapsFunctions.NumElements()); ui++)
	{
		psFunction = mpcGenerator->mapsFunctions.GetPtr(ui);
		if (!psFunction->psClass && (psFunction->lliName == lliName) && !psFunction->psTemplate)
		{
			if (psTarget && psTarget->Equals(FunctionType(psFunction)))
			{
				psFound = psFunction;
				bOverloaded = false;
				break;
			}
			bOverloaded |= (psFound != NULL);
			psFound = psFunction;
		}
	}
	if (bOverloaded || !psFound)
	{
		return Unsupported(__LINE__);
	}
	psValue->sValue = FunctionSymbol(psFound);
	psValue->psType = mpcTypes->Pointer(FunctionType(psFound));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// &Class::member, as the generator's MemberAddress: a pointer to a member
// function is its address (a thunk that calls through the vtable for a
// virtual one) and a pointer to a field its offset.  Returns false with
// *pbMember clear if the name isn't a non-static member (it may be a static
// one), and with it set after an Unsupported.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::MemberAddress(CCPPSyntaxIdExpression* pcId, SCPP65IRValue* psValue, bool* pbMember)
{
	CCPPSyntaxName*		pcName;
	SCPP65Class*		psClass;
	SCPP65Class*		psFoundClass;
	SCPP65Function*		psFound;
	SCPP65Field*		psField;
	SIRReference		sSymbol;
	CChars				szThunk;
	int64				lliName;
	int					iOffset;

	*pbMember = false;
	pcName = pcId->mpcName;
	psClass = mpcGenerator->GetComponentClass((CCPPSyntaxNameComponent*)pcName->macpComponents.GetPtr(pcName->macpComponents.NumElements() - 2), NULL, pcId);
	lliName = mpcGenerator->GetFunctionName(pcName->GetLast());
	if (!psClass || (lliName == -1) || !mpcGenerator->CompleteClass(psClass, pcId))
	{
		return false;
	}

	psFound = NULL;
	psFoundClass = psClass;
	if (!MemberFunction(psClass, lliName, &psFound, &psFoundClass))
	{
		*pbMember = true;
		return Unsupported(__LINE__);
	}
	if (psFound)
	{
		if (psFound->bStatic)
		{
			return false;
		}
		*pbMember = true;
		if (psFound->bVirtual)
		{
			szThunk.Init();
			mpcGenerator->AppendVirtualCallThunk(&szThunk, psFound);
			sSymbol = mcModule.Symbol(szThunk.Text());
			szThunk.Kill();
		}
		else
		{
			sSymbol = FunctionSymbol(psFound);
		}
		psValue->sValue = ConvertIR(sSymbol, IRType(IRTK_Pointer), IRType(IRTK_I32), false);
		psValue->psType = mpcTypes->MemberPointer(psFoundClass, FunctionType(psFound));
		return true;
	}

	psField = mpcGenerator->FindField(psClass, lliName, &iOffset);
	if (!psField)
	{
		return false;
	}
	*pbMember = true;
	psValue->sValue = mpcFunction->AddInteger(IRType(IRTK_I32), iOffset);
	psValue->psType = mpcTypes->MemberPointer(psClass, psField->psType);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The member function of the name in psClass, or else in its bases (the
// first that has one), and the class it is in, as the generator's
// MemberFunctionAddress.  Returns false for an overloaded name.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::MemberFunction(SCPP65Class* psClass, int64 lliName, SCPP65Function** ppsFound, SCPP65Class** ppsClass)
{
	SCPP65Function*		psFunction;
	size				ui;

	*ppsFound = NULL;
	for (ui = 0; ui < mpcGenerator->mapsFunctions.NumElements(); ui++)
	{
		psFunction = mpcGenerator->mapsFunctions.GetPtr(ui);
		if ((psFunction->psClass == psClass) && (psFunction->lliName == lliName) && (psFunction->eKind == CPP65FK_Function) && !psFunction->psTemplate)
		{
			if (*ppsFound)
			{
				return false;
			}
			*ppsFound = psFunction;
		}
	}
	if (*ppsFound)
	{
		*ppsClass = psClass;
		return true;
	}
	for (ui = 0; ui < psClass->macBases.NumElements(); ui++)
	{
		if (!MemberFunction(psClass->macBases.Get(ui)->psClass, lliName, ppsFound, ppsClass))
		{
			return false;
		}
		if (*ppsFound)
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
	SCPP65Class*	psClass;

	*ppsMember = mpcGenerator->TypeOf(pcBinary->mpcRight);
	if (!*ppsMember || !(*ppsMember)->IsMemberPointer())
	{
		return Unsupported(__LINE__);
	}
	if (pcBinary->meOperator == CT_MemberByPointerFunctionPointerIndirection)
	{
		if (!Value(pcBinary->mpcLeft, &sPointer))
		{
			return false;
		}
		if (!sPointer.psType->IsPointer() || !sPointer.psType->psBase->IsClass())
		{
			return Unsupported(__LINE__);
		}
		*psObject = sPointer.sValue;
		psClass = sPointer.psType->psBase->psClass;
	}
	else
	{
		if (!Place(pcBinary->mpcLeft, &sPlace))
		{
			return false;
		}
		if (!sPlace.psType->IsClass())
		{
			return Unsupported(__LINE__);
		}
		*psObject = sPlace.sAddress;
		psClass = sPlace.psType->psClass;
	}
	*psObject = AddressAt(*psObject, BaseOffset(psClass, (*ppsMember)->psClass));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// (object.*function)(...) and (pointer->*function)(...): the object is
// this.  The object and the pointer are worked out before the arguments
// (the generator works out the arguments first).
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
		return Unsupported(__LINE__);
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

	if (!MemberPointerObject(pcBinary, &sObject, &psMember))
	{
		return false;
	}
	if (psMember->psBase->IsFunction() || !IsPlaceType(psMember->psBase))
	{
		return Unsupported(__LINE__);
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
	SCPP65Variable*					psVariable;
	SCPP65IRValue					sPointer;
	SCPP65IRValue					sIndex;
	SCPP65IRValue					sAddress;
	SCPP65Type*						psType;

	pcExpression = mpcGenerator->SkipParentheses(pcExpression);
	switch (pcExpression->GetKind())
	{
	case CPPS_IdExpression:
		pcId = (CCPPSyntaxIdExpression*)pcExpression;
		psVariable = pcId->mpcName->mpcSymbol ? mpcGenerator->GetVariable(pcId->mpcName->mpcSymbol) : NULL;
		if (psVariable)
		{
			return VariablePlace(psVariable, psPlace);
		}
		//A member of this (a name in a template the parser could not resolve too).
		if (msThis.IsNone() || (pcId->mpcName->macpComponents.NumElements() != 1) || (pcId->mpcName->mpcSymbol && (pcId->mpcName->mpcSymbol->meKind != CPPSK_Field)))
		{
			return Unsupported(__LINE__);
		}
		return FieldPlace(msThis, mpsFunction->psClass, mpcGenerator->GetComponentName(pcId->mpcName->GetLast()), pcId->mpcName->mpcSymbol, psPlace);

	case CPPS_MemberExpression:
		return MemberPlace((CCPPSyntaxMemberExpression*)pcExpression, psPlace);

	case CPPS_UnaryExpression:
		pcUnary = (CCPPSyntaxUnaryExpression*)pcExpression;
		if (pcUnary->meOperator != CT_Multiply)
		{
			return Unsupported(__LINE__);
		}
		if (!Value(pcUnary->mpcOperand, &sPointer))
		{
			return false;
		}
		if (!sPointer.psType->IsPointer() || sPointer.psType->psBase->IsVoid() || !IsPlaceType(sPointer.psType->psBase))
		{
			return Unsupported(__LINE__);
		}
		psPlace->sAddress = sPointer.sValue;
		psPlace->psType = sPointer.psType->psBase;
		return true;

	case CPPS_SubscriptExpression:
		pcSubscript = (CCPPSyntaxSubscriptExpression*)pcExpression;
		psType = mpcGenerator->TypeOf(pcSubscript->mpcArray);
		if (!psType || psType->IsClass())
		{
			return Unsupported(__LINE__);
		}
		if (!Value(pcSubscript->mpcIndex, &sIndex) || !Value(pcSubscript->mpcArray, &sPointer))
		{
			return false;
		}
		if (!Operate(CT_Add, &sPointer, &sIndex, &sAddress))
		{
			return false;
		}
		if (!sAddress.psType->IsPointer() || sAddress.psType->psBase->IsVoid() || !IsPlaceType(sAddress.psType->psBase))
		{
			return Unsupported(__LINE__);
		}
		psPlace->sAddress = sAddress.sValue;
		psPlace->psType = sAddress.psType->psBase;
		return true;

	case CPPS_CallExpression:
		//The type of a call that returns a reference to a class is the class.
		psType = mpcGenerator->TypeOf(pcExpression);
		if (!psType || !(psType->IsReference() || psType->IsClass()))
		{
			return Unsupported(__LINE__);
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
			return Unsupported(__LINE__);
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

		//An operator function's object (a temporary it made, or the object it refers to).
		psType = mpcGenerator->TypeOf(pcExpression);
		if (!psType || !(psType->IsReference() || psType->IsClass()) || !Value(pcExpression, &sAddress))
		{
			return Unsupported(__LINE__);
		}
		psPlace->sAddress = sAddress.sValue;
		psPlace->psType = sAddress.psType->IsReference() ? sAddress.psType->psBase : sAddress.psType;
		return psPlace->psType->IsClass() || Unsupported(__LINE__);

	default:
		return Unsupported(__LINE__);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A local's slot, a parameter's argaddr or a global's (or static local's)
// label.  A reference's place is what it refers to.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::VariablePlace(SCPP65Variable* psVariable, SCPP65IRPlace* psPlace)
{
	SIRReference*	psAddress;
	SCPP65Place		sPlace;
	CChars			szLabel;

	if (!IsPlaceType(psVariable->psType))
	{
		return Unsupported(__LINE__);
	}
	if ((psVariable->eStorage == CPP65S_Local) || (psVariable->eStorage == CPP65S_Parameter))
	{
		psAddress = mcAddresses.Get((int64)(size)psVariable);
		if (!psAddress)
		{
			return Unsupported(__LINE__);
		}
		psPlace->sAddress = *psAddress;
	}
	else
	{
		mpcGenerator->VariablePlace(psVariable, &sPlace);
		szLabel.Init();
		mpcGenerator->AppendPlaceLabel(&szLabel, &sPlace);
		psPlace->sAddress = mcModule.Symbol(szLabel.Text());
		szLabel.Kill();
	}
	psPlace->psType = psVariable->psType;
	if (psPlace->psType->IsReference())
	{
		psPlace->sAddress = mpcFunction->Load(mpcBlock, IRType(IRTK_Pointer), psPlace->sAddress);
		psPlace->psType = psPlace->psType->psBase;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::UnaryExpression(CCPPSyntaxUnaryExpression* pcUnary, SCPP65IRValue* psValue)
{
	SCPP65IRPlace	sPlace;
	SCPP65IRValue	sOperand;
	CCPPSyntax*		pcOperand;
	SCPP65Type*		psType;
	SIRType			sType;
	bool			bMember;

	switch (pcUnary->meOperator)
	{
	case CT_Increment:
	case CT_Decrement:
		return IncrementExpression(pcUnary, psValue);
	case CT_LogicalNot:
		return ConditionValue(pcUnary, psValue);
	case CT_Multiply:
		if (!Place(pcUnary, &sPlace))
		{
			return false;
		}
		return Load(&sPlace, psValue);
	case CT_BitwiseAnd:
		pcOperand = mpcGenerator->SkipParentheses(pcUnary->mpcOperand);
		bMember = false;
		if ((pcOperand->GetKind() == CPPS_IdExpression) && (((CCPPSyntaxIdExpression*)pcOperand)->mpcName->macpComponents.NumElements() >= 2) &&
			MemberAddress((CCPPSyntaxIdExpression*)pcOperand, psValue, &bMember))
		{
			return true;
		}
		if (bMember)
		{
			return false;
		}
		if (mpcGenerator->IsObject(pcOperand) && mpcGenerator->HasMember(mpcGenerator->ObjectClass(pcOperand), mpcGenerator->GetOperatorName(CT_BitwiseAnd, false)))
		{
			//&object with the object's operator&.
			return Unsupported(__LINE__);
		}
		if ((pcOperand->GetKind() == CPPS_IdExpression) && ((CCPPSyntaxIdExpression*)pcOperand)->mpcName->mpcSymbol && (((CCPPSyntaxIdExpression*)pcOperand)->mpcName->mpcSymbol->meKind == CPPSK_Function))
		{
			return FunctionAddress((CCPPSyntaxIdExpression*)pcOperand, NULL, psValue);
		}
		if ((pcOperand->GetKind() == CPPS_IdExpression) && (((CCPPSyntaxIdExpression*)pcOperand)->mpcName->macpComponents.NumElements() >= 2))
		{
			return Unsupported(__LINE__);
		}
		if (!Place(pcOperand, &sPlace))
		{
			return false;
		}
		psValue->sValue = sPlace.sAddress;
		psValue->psType = mpcTypes->Pointer(sPlace.psType);
		return true;
	case CT_Add:
	case CT_Subtract:
	case CT_BitwiseNot:
		break;
	default:
		return Unsupported(__LINE__);
	}

	//Operators on constants are worked out by the IR (the generator's Constant does its arithmetic in signed 64
	//bits, which isn't right for every type).
	if (!Value(pcUnary->mpcOperand, &sOperand))
	{
		return false;
	}
	if (sOperand.psType->IsFloat() && (pcUnary->meOperator != CT_BitwiseNot))
	{
		*psValue = sOperand;
		if (pcUnary->meOperator == CT_Subtract)
		{
			psValue->sValue = mpcFunction->Add(mpcBlock, IRO_FNeg, IRTypeOf(sOperand.psType), sOperand.sValue);
		}
		return true;
	}
	if (!sOperand.psType->IsInteger())
	{
		return Unsupported(__LINE__);
	}
	psType = mpcTypes->Promote(sOperand.psType);
	if (!Convert(&sOperand, psType))
	{
		return false;
	}
	sType = IRTypeOf(psType);
	psValue->psType = psType;
	switch (pcUnary->meOperator)
	{
	case CT_Subtract:
		psValue->sValue = mpcFunction->Add(mpcBlock, IRO_Neg, sType, sOperand.sValue);
		break;
	case CT_BitwiseNot:
		psValue->sValue = mpcFunction->Add(mpcBlock, IRO_Not, sType, sOperand.sValue);
		break;
	default:
		psValue->sValue = sOperand.sValue;
		break;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// ++x and --x give the new value, x++ and x-- the old one.  Pointers step by
// the size of what they point to.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::IncrementExpression(CCPPSyntaxUnaryExpression* pcUnary, SCPP65IRValue* psValue)
{
	SCPP65IRPlace	sPlace;
	SCPP65IRValue	sOld;
	SCPP65IRValue	sOne;
	SCPP65IRValue	sNew;
	SCPP65Type*		psType;

	if (!Place(pcUnary->mpcOperand, &sPlace) || !Load(&sPlace, &sOld))
	{
		return false;
	}
	psType = sPlace.psType;
	if (!psType->IsScalar() || (psType->eKind == CPP65TK_Bool))
	{
		return Unsupported(__LINE__);
	}
	if (psType->IsFloat())
	{
		sOne.sValue = mpcFunction->AddConstant(IRTypeOf(psType), (uint64)mpcGenerator->FloatBits(psType, 1.0));
		sOne.psType = psType;
	}
	else
	{
		sOne.sValue = Integer(&mpcTypes->msInt, 1);
		sOne.psType = &mpcTypes->msInt;
	}
	if (!Operate((pcUnary->meOperator == CT_Increment) ? CT_Add : CT_Subtract, &sOld, &sOne, &sNew) || !Convert(&sNew, psType))
	{
		return false;
	}
	Store(&sPlace, &sNew);
	*psValue = pcUnary->mbPostfix ? sOld : sNew;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::BinaryExpression(CCPPSyntaxBinaryExpression* pcBinary, SCPP65IRValue* psValue)
{
	SCPP65IRValue	sLeft;
	SCPP65IRValue	sRight;
	SCPP65IRPlace	sPlace;

	switch (pcBinary->meOperator)
	{
	case CT_MemberFunctionPointerIndirection:
	case CT_MemberByPointerFunctionPointerIndirection:
		//object.*field (a member function can only be called).
		return MemberPointerPlace(pcBinary, &sPlace) && Load(&sPlace, psValue);
	case CT_Assignment:
	case CT_AssignmentAdd:
	case CT_AssignmentSubtract:
	case CT_AssignmentMultiply:
	case CT_AssignmentDivide:
	case CT_AssignmentModulus:
	case CT_AssignmentBitwiseAnd:
	case CT_AssignmentBitwiseOr:
	case CT_AssignmentBitwiseXor:
	case CT_AssignmentLeftShift:
	case CT_AssignmentRightShift:
		return Assignment(pcBinary, psValue);
	case CT_Comma:
		return Value(pcBinary->mpcLeft, &sLeft) && Value(pcBinary->mpcRight, psValue);
	case CT_LogicalAnd:
	case CT_LogicalOr:
	case CT_EqualTo:
	case CT_NotEqualTo:
	case CT_LessThan:
	case CT_LessThanEqualTo:
	case CT_GreaterThan:
	case CT_GreaterThanEqualTo:
		return ConditionValue(pcBinary, psValue);
	default:
		if (mpcGenerator->IsObject(pcBinary->mpcLeft) || mpcGenerator->IsObject(pcBinary->mpcRight))
		{
			return OperatorExpression(pcBinary->meOperator, pcBinary->mpcLeft, pcBinary->mpcRight, psValue);
		}
		if (!Value(pcBinary->mpcRight, &sRight) || !Value(pcBinary->mpcLeft, &sLeft))
		{
			return false;
		}
		return Operate(pcBinary->meOperator, &sLeft, &sRight, psValue);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// x = y and x op= y.  The value is the one stored.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Assignment(CCPPSyntaxBinaryExpression* pcBinary, SCPP65IRValue* psValue)
{
	SCPP65IRPlace	sPlace;
	SCPP65IRValue	sRight;
	SCPP65IRValue	sOld;
	ECppTokens		eOperator;
	SCPP65Type*		psLeft;

	psLeft = mpcGenerator->TypeOf(pcBinary->mpcLeft);
	if (psLeft && psLeft->IsClass())
	{
		//object op= value calls the class's operator if it has one; object = object copies the bytes otherwise.
		if (mpcGenerator->HasMember(psLeft->psClass, mpcGenerator->GetOperatorName(pcBinary->meOperator, false)))
		{
			return OperatorCall(pcBinary->mpcLeft, pcBinary->meOperator, pcBinary->mpcRight, psValue);
		}
		if (pcBinary->meOperator != CT_Assignment)
		{
			return Unsupported(__LINE__);
		}
		if (!Place(pcBinary->mpcLeft, &sPlace) || !sPlace.psType->IsClass() || !CopyObject(&sPlace, pcBinary->mpcRight))
		{
			return false;
		}
		psValue->sValue = sPlace.sAddress;
		psValue->psType = sPlace.psType;
		return true;
	}
	if (!Value(pcBinary->mpcRight, &sRight) || !Place(pcBinary->mpcLeft, &sPlace))
	{
		return false;
	}
	if (!sPlace.psType->IsScalar() || (sPlace.psType->IsMemberPointer() && (pcBinary->meOperator != CT_Assignment)))
	{
		return Unsupported(__LINE__);
	}
	if (pcBinary->meOperator == CT_Assignment)
	{
		if (!Convert(&sRight, sPlace.psType))
		{
			return false;
		}
		Store(&sPlace, &sRight);
		*psValue = sRight;
		return true;
	}

	switch (pcBinary->meOperator)
	{
	case CT_AssignmentAdd:			eOperator = CT_Add;			break;
	case CT_AssignmentSubtract:		eOperator = CT_Subtract;	break;
	case CT_AssignmentMultiply:		eOperator = CT_Multiply;	break;
	case CT_AssignmentDivide:		eOperator = CT_Divide;		break;
	case CT_AssignmentModulus:		eOperator = CT_Modulus;		break;
	case CT_AssignmentBitwiseAnd:	eOperator = CT_BitwiseAnd;	break;
	case CT_AssignmentBitwiseOr:	eOperator = CT_BitwiseOr;	break;
	case CT_AssignmentBitwiseXor:	eOperator = CT_BitwiseXor;	break;
	case CT_AssignmentLeftShift:	eOperator = CT_LeftShift;	break;
	default:						eOperator = CT_RightShift;	break;
	}
	if (!Load(&sPlace, &sOld) || !Operate(eOperator, &sOld, &sRight, psValue) || !Convert(psValue, sPlace.psType))
	{
		return false;
	}
	Store(&sPlace, psValue);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// left op right for the arithmetic, bitwise and shift operators.  Pointer +
// integer and pointer - integer are scaled by the size pointed to; pointer -
// pointer is divided by it.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Operate(ECppTokens eOperator, SCPP65IRValue* psLeft, SCPP65IRValue* psRight, SCPP65IRValue* psResult)
{
	SCPP65Type*		psType;
	SCPP65IRValue	sLeft;
	SCPP65IRValue	sRight;
	SCPP65IRValue	sIndex;
	SCPP65IRValue	sPointer;
	SIRType			sType;
	SIRType			sOffset;
	SIRReference	sScaled;
	SIRReference	sDifference;
	uint16			uiOpcode;
	int				iBytes;

	sLeft = *psLeft;
	sRight = *psRight;
	if (sLeft.psType->IsFloat() || sRight.psType->IsFloat())
	{
		if (sLeft.psType->IsPointer() || sRight.psType->IsPointer())
		{
			return Unsupported(__LINE__);
		}
		psType = mpcGenerator->FloatArithmetic(sLeft.psType, sRight.psType);
		switch (eOperator)
		{
		case CT_Add:		uiOpcode = IRO_FAdd;	break;
		case CT_Subtract:	uiOpcode = IRO_FSub;	break;
		case CT_Multiply:	uiOpcode = IRO_FMul;	break;
		case CT_Divide:		uiOpcode = IRO_FDiv;	break;
		default:			return Unsupported(__LINE__);
		}
		if (!Convert(&sLeft, psType) || !Convert(&sRight, psType))
		{
			return false;
		}
		psResult->sValue = mpcFunction->Add(mpcBlock, uiOpcode, IRTypeOf(psType), sLeft.sValue, sRight.sValue);
		psResult->psType = psType;
		return true;
	}

	if (sLeft.psType->IsPointer() || sRight.psType->IsPointer())
	{
		sOffset = IRIntegerType(mcModule.GetPointerSize(0));
		if (((eOperator == CT_Add) || (eOperator == CT_Subtract)) && sLeft.psType->IsPointer() != sRight.psType->IsPointer())
		{
			if ((eOperator == CT_Subtract) && !sLeft.psType->IsPointer())
			{
				return Unsupported(__LINE__);
			}
			sPointer = sLeft.psType->IsPointer() ? sLeft : sRight;
			sIndex = sLeft.psType->IsPointer() ? sRight : sLeft;
			if (!sIndex.psType->IsInteger() || sPointer.psType->psBase->IsVoid() || sPointer.psType->psBase->IsFunction())
			{
				return Unsupported(__LINE__);
			}
			//The index widens to the pointer's size (an int is narrower on the W65C816).
			if (!Convert(&sIndex, mpcTypes->Integer(mcModule.GetPointerSize(0), sIndex.psType->bSigned)))
			{
				return false;
			}
			iBytes = sPointer.psType->GetElementBytes();
			sScaled = (iBytes == 1) ? sIndex.sValue : mpcFunction->Add(mpcBlock, IRO_Mul, sOffset, sIndex.sValue, mpcFunction->AddInteger(sOffset, iBytes));
			if (eOperator == CT_Subtract)
			{
				sScaled = mpcFunction->Add(mpcBlock, IRO_Neg, sOffset, sScaled);
			}
			psResult->sValue = mpcFunction->Add(mpcBlock, IRO_PtrAdd, IRType(IRTK_Pointer), sPointer.sValue, sScaled);
			psResult->psType = sPointer.psType;
			return true;
		}
		if ((eOperator == CT_Subtract) && sLeft.psType->IsPointer() && sRight.psType->IsPointer())
		{
			sDifference = mpcFunction->Add(mpcBlock, IRO_Sub, sOffset,
				mpcFunction->Add(mpcBlock, IRO_PtrToI, sOffset, sLeft.sValue),
				mpcFunction->Add(mpcBlock, IRO_PtrToI, sOffset, sRight.sValue));
			iBytes = sLeft.psType->GetElementBytes();
			if (iBytes != 1)
			{
				sDifference = mpcFunction->Add(mpcBlock, IRO_SDiv, sOffset, sDifference, mpcFunction->AddInteger(sOffset, iBytes));
			}
			//ptrdiff_t: as wide as a pointer (int on the 65C64, long on the W65C816).
			psResult->sValue = sDifference;
			psResult->psType = mpcTypes->Integer(mcModule.GetPointerSize(0), true);
			return true;
		}
		return Unsupported(__LINE__);
	}

	if (!sLeft.psType->IsInteger() || !sRight.psType->IsInteger())
	{
		return Unsupported(__LINE__);
	}
	if ((eOperator == CT_LeftShift) || (eOperator == CT_RightShift))
	{
		psType = mpcTypes->Promote(sLeft.psType);
	}
	else
	{
		psType = mpcTypes->Arithmetic(sLeft.psType, sRight.psType);
	}
	if (!Convert(&sLeft, psType) || !Convert(&sRight, psType))
	{
		return false;
	}
	switch (eOperator)
	{
	case CT_Add:			uiOpcode = IRO_Add;									break;
	case CT_Subtract:		uiOpcode = IRO_Sub;									break;
	case CT_Multiply:		uiOpcode = IRO_Mul;									break;
	case CT_Divide:			uiOpcode = psType->bSigned ? IRO_SDiv : IRO_UDiv;	break;
	case CT_Modulus:		uiOpcode = psType->bSigned ? IRO_SRem : IRO_URem;	break;
	case CT_BitwiseAnd:		uiOpcode = IRO_And;									break;
	case CT_BitwiseOr:		uiOpcode = IRO_Or;									break;
	case CT_BitwiseXor:		uiOpcode = IRO_Xor;									break;
	case CT_LeftShift:		uiOpcode = IRO_Shl;									break;
	case CT_RightShift:		uiOpcode = psType->bSigned ? IRO_AShr : IRO_LShr;	break;
	default:				return Unsupported(__LINE__);
	}
	sType = IRTypeOf(psType);
	psResult->sValue = mpcFunction->Add(mpcBlock, uiOpcode, sType, sLeft.sValue, sRight.sValue);
	psResult->psType = psType;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// An i1: the comparison of left and right.  Pointers compare unsigned;
// integers after the usual arithmetic conversions; floats as floats.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Compare(ECppTokens eOperator, SCPP65IRValue* psLeft, SCPP65IRValue* psRight, SIRReference* psCondition)
{
	SCPP65Type*		psType;
	SCPP65IRValue	sLeft;
	SCPP65IRValue	sRight;
	bool			bSigned;
	bool			bFloat;
	uint16			uiOpcode;

	sLeft = *psLeft;
	sRight = *psRight;
	bFloat = false;
	bSigned = false;
	if (sLeft.psType->IsFloat() || sRight.psType->IsFloat())
	{
		if (sLeft.psType->IsPointer() || sRight.psType->IsPointer())
		{
			return Unsupported(__LINE__);
		}
		psType = mpcGenerator->FloatArithmetic(sLeft.psType, sRight.psType);
		bFloat = true;
	}
	else if (sLeft.psType->IsPointer() || sRight.psType->IsPointer())
	{
		psType = sLeft.psType->IsPointer() ? sLeft.psType : sRight.psType;
	}
	else if ((sLeft.psType->IsMemberPointer() || sRight.psType->IsMemberPointer()) && ((eOperator == CT_EqualTo) || (eOperator == CT_NotEqualTo)))
	{
		//Pointers to members (an address or an offset, 0 for none) are equal or not.
		psType = sLeft.psType->IsMemberPointer() ? sLeft.psType : sRight.psType;
	}
	else if (sLeft.psType->IsInteger() && sRight.psType->IsInteger())
	{
		psType = mpcTypes->Arithmetic(sLeft.psType, sRight.psType);
		bSigned = psType->bSigned;
	}
	else
	{
		return Unsupported(__LINE__);
	}
	if (!Convert(&sLeft, psType) || !Convert(&sRight, psType))
	{
		return false;
	}

	switch (eOperator)
	{
	case CT_EqualTo:				uiOpcode = bFloat ? IRO_FEq : IRO_Eq;									break;
	case CT_NotEqualTo:				uiOpcode = bFloat ? IRO_FNe : IRO_Ne;									break;
	case CT_LessThan:				uiOpcode = bFloat ? IRO_FLt : (bSigned ? IRO_SLt : IRO_ULt);			break;
	case CT_LessThanEqualTo:		uiOpcode = bFloat ? IRO_FLe : (bSigned ? IRO_SLe : IRO_ULe);			break;
	case CT_GreaterThan:			uiOpcode = bFloat ? IRO_FGt : (bSigned ? IRO_SGt : IRO_UGt);			break;
	case CT_GreaterThanEqualTo:		uiOpcode = bFloat ? IRO_FGe : (bSigned ? IRO_SGe : IRO_UGe);			break;
	default:						return Unsupported(__LINE__);
	}
	*psCondition = mpcFunction->Add(mpcBlock, uiOpcode, IRTypeOf(psType), sLeft.sValue, sRight.sValue);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// c ? x : y.  The two values meet in a phi.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ConditionalExpression(CCPPSyntaxConditionalExpression* pcConditional, SCPP65IRValue* psValue)
{
	SCPP65Type*		psTrue;
	SCPP65Type*		psFalse;
	SCPP65Type*		psType;
	CIRBlock*		pcTrue;
	CIRBlock*		pcFalse;
	CIRBlock*		pcEnd;
	CIRBlock*		pcTrueEnd;
	SCPP65IRValue	sTrue;
	SCPP65IRValue	sFalse;

	psTrue = mpcGenerator->TypeOf(pcConditional->mpcTrue);
	psFalse = mpcGenerator->TypeOf(pcConditional->mpcFalse);
	if (!psTrue || !psFalse || psTrue->IsReference() || psFalse->IsReference() || psTrue->IsClass() || psFalse->IsClass())
	{
		return Unsupported(__LINE__);
	}
	if ((psTrue->IsFloat() && psFalse->IsScalar() && !psFalse->IsPointer()) || (psFalse->IsFloat() && psTrue->IsScalar() && !psTrue->IsPointer()))
	{
		psType = mpcGenerator->FloatArithmetic(psTrue, psFalse);
	}
	else if ((psTrue == psFalse) && psTrue->pcEnum)
	{
		psType = psTrue;
	}
	else if (psTrue->IsInteger() && psFalse->IsInteger())
	{
		psType = mpcTypes->Arithmetic(psTrue, psFalse);
	}
	else if (psTrue->IsVoid() || psFalse->IsVoid())
	{
		psType = &mpcTypes->msVoid;
	}
	else if (psTrue->IsArray() || psFalse->IsArray())
	{
		return Unsupported(__LINE__);
	}
	else
	{
		psType = psTrue->IsPointer() ? psTrue : psFalse;
	}

	pcTrue = NewBlock();
	pcFalse = NewBlock();
	pcEnd = NewBlock();
	if (!Condition(pcConditional->mpcCondition, pcTrue, pcFalse))
	{
		return false;
	}
	//A temporary made by only one of them would need a flag to say if it is to be destroyed (not supported).
	miConditional++;
	StartBlock(pcTrue);
	if (!Value(pcConditional->mpcTrue, &sTrue) || !Convert(&sTrue, psType))
	{
		return false;
	}
	pcTrueEnd = mpcBlock;
	JumpTo(pcEnd);
	StartBlock(pcFalse);
	if (!Value(pcConditional->mpcFalse, &sFalse) || !Convert(&sFalse, psType))
	{
		return false;
	}
	miConditional--;
	JumpTo(pcEnd);
	psValue->psType = psType;
	if (!psType->IsVoid())
	{
		psValue->sValue = Phi(pcEnd, IRTypeOf(psType), pcTrueEnd, sTrue.sValue, mpcBlock, sFalse.sValue);
	}
	else
	{
		psValue->sValue = IRNone();
	}
	StartBlock(pcEnd);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A condition as a bool: 1 or 0 from a phi.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ConditionValue(CCPPSyntax* pcExpression, SCPP65IRValue* psValue)
{
	CIRBlock*	pcTrue;
	CIRBlock*	pcFalse;
	CIRBlock*	pcEnd;
	SIRType		sType;

	pcTrue = NewBlock();
	pcFalse = NewBlock();
	pcEnd = NewBlock();
	if (!Condition(pcExpression, pcTrue, pcFalse))
	{
		return false;
	}
	pcTrue->Jump(pcEnd);
	pcFalse->Jump(pcEnd);
	sType = IRTypeOf(&mpcTypes->msBool);
	psValue->sValue = Phi(pcEnd, sType, pcTrue, mpcFunction->AddConstant(sType, 1), pcFalse, mpcFunction->AddConstant(sType, 0));
	psValue->psType = &mpcTypes->msBool;
	StartBlock(pcEnd);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Branches to pcTrue or pcFalse.  && and || only work out their right
// operand when they need to.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Condition(CCPPSyntax* pcExpression, CIRBlock* pcTrue, CIRBlock* pcFalse)
{
	CCPPSyntaxBinaryExpression*	pcBinary;
	CCPPSyntaxUnaryExpression*	pcUnary;
	CIRBlock*					pcRight;
	SCPP65IRValue				sLeft;
	SCPP65IRValue				sRight;
	SCPP65IRValue				sValue;
	SIRReference				sCondition;
	bool						bResult;

	pcExpression = mpcGenerator->SkipParentheses(pcExpression);
	if (!pcExpression)
	{
		return Unsupported(__LINE__);
	}
	if (pcExpression->GetKind() == CPPS_UnaryExpression)
	{
		pcUnary = (CCPPSyntaxUnaryExpression*)pcExpression;
		if (pcUnary->meOperator == CT_LogicalNot)
		{
			if (mpcGenerator->IsObject(pcUnary->mpcOperand))
			{
				return Unsupported(__LINE__);
			}
			return Condition(pcUnary->mpcOperand, pcFalse, pcTrue);
		}
	}
	else if (pcExpression->GetKind() == CPPS_BinaryExpression)
	{
		pcBinary = (CCPPSyntaxBinaryExpression*)pcExpression;
		switch (pcBinary->meOperator)
		{
		case CT_LogicalAnd:
		case CT_LogicalOr:
			pcRight = NewBlock();
			if (!Condition(pcBinary->mpcLeft, (pcBinary->meOperator == CT_LogicalAnd) ? pcRight : pcTrue, (pcBinary->meOperator == CT_LogicalAnd) ? pcFalse : pcRight))
			{
				return false;
			}
			StartBlock(pcRight);
			miConditional++;
			bResult = Condition(pcBinary->mpcRight, pcTrue, pcFalse);
			miConditional--;
			return bResult;
		case CT_EqualTo:
		case CT_NotEqualTo:
		case CT_LessThan:
		case CT_LessThanEqualTo:
		case CT_GreaterThan:
		case CT_GreaterThanEqualTo:
			if (mpcGenerator->IsObject(pcBinary->mpcLeft) || mpcGenerator->IsObject(pcBinary->mpcRight))
			{
				//An operator function's result.
				if (!OperatorExpression(pcBinary->meOperator, pcBinary->mpcLeft, pcBinary->mpcRight, &sValue) || !sValue.psType->IsScalar() || !Truth(&sValue, &sCondition))
				{
					return Unsupported(__LINE__);
				}
				mpcBlock->Branch(sCondition, pcTrue, pcFalse);
				return true;
			}
			if (!Value(pcBinary->mpcRight, &sRight) || !Value(pcBinary->mpcLeft, &sLeft) || !Compare(pcBinary->meOperator, &sLeft, &sRight, &sCondition))
			{
				return false;
			}
			mpcBlock->Branch(sCondition, pcTrue, pcFalse);
			return true;
		default:
			break;
		}
	}
	else if (pcExpression->GetKind() == CPPS_SimpleDeclaration)
	{
		return Unsupported(__LINE__);
	}

	if (mpcGenerator->IsObject(pcExpression) || !Value(pcExpression, &sValue) || !Truth(&sValue, &sCondition))
	{
		return Unsupported(__LINE__);
	}
	mpcBlock->Branch(sCondition, pcTrue, pcFalse);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// An i1 that is 1 when a scalar is not zero (+0 and -0 are zero).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Truth(SCPP65IRValue* psValue, SIRReference* psCondition)
{
	SIRType	sType;

	if (!psValue->psType->IsScalar())
	{
		return Unsupported(__LINE__);
	}
	sType = IRTypeOf(psValue->psType);
	if (psValue->psType->IsFloat())
	{
		*psCondition = mpcFunction->Add(mpcBlock, IRO_FNe, sType, psValue->sValue, mpcFunction->AddConstant(sType, 0));
	}
	else
	{
		*psCondition = mpcFunction->Add(mpcBlock, IRO_Ne, sType, psValue->sValue, mpcFunction->AddConstant(sType, 0));
	}
	return true;
}


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
		if (pcName->macpComponents.NumElements() >= 2)
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

	//__builtin_ functions, on a target the generator doesn't write code for (the 65C64's are the generator's).
	lliName = pcSymbol ? pcSymbol->mlliName : mpcGenerator->GetFunctionName(pcComponent);
	if (!mpcGenerator->mpsTarget->bDirectGenerator && (!pcSymbol || (pcSymbol->meKind == CPPSK_Function)) && mpcGenerator->IsBuiltin(lliName))
	{
		return Builtin(pcCall, lliName, psValue);
	}

	psFunction = NULL;
	if (pcSymbol && (pcSymbol->meKind == CPPSK_FunctionTemplate))
	{
		psTemplate = mpcGenerator->GetCallTemplate(pcSymbol, pcCall->macpArguments.NumElements());
		if (!psTemplate)
		{
			return Unsupported(__LINE__);
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
		psFunction = mpcGenerator->FindFunction(NULL, lliName, CPP65FK_Function, &pcCall->macpArguments, false, pcCall);
	}
	else if (pcSymbol)
	{
		return CallThroughPointer(pcCall, pcCallee, psValue);
	}
	if (!psFunction || (psFunction->psClass && !psFunction->bStatic))
	{
		return Unsupported(__LINE__);
	}

	sSymbol = FunctionSymbol(psFunction);
	return Call(psFunction, sSymbol, NULL, &pcCall->macpArguments, psValue);
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
	CChars			szName;
	char*			szIdentifier;
	int				iLength;
	double			dValue;
	SCPP65Type*		psType;
	SCPP65IRValue	sArgument;
	SIRType			sType;
	SIRType			sInt;
	CIRBlock*		pcEntry;
	CIRBlock*		pcLoop;
	CIRBlock*		pcBody;
	CIRBlock*		pcZero;
	CIRBlock*		pcDone;
	SIRPhi*			psX;
	SIRPhi*			psCount;
	SIRPhi*			psResult;
	SIRReference	sX2;
	SIRReference	sCount2;
	SIRReference	sTest;
	int				iBits;
	int				iArguments;
	bool			bResult;

	if (mpcGenerator->FloatBuiltin(pcCall, &dValue, &psType))
	{
		psValue->sValue = (psType->iBytes == 4) ? mpcFunction->AddFloat((float)dValue) : mpcFunction->AddDouble(dValue);
		psValue->psType = psType;
		return true;
	}

	szIdentifier = mpcGenerator->mpcTokeniser->GetIdentifier(lliName, &iLength);
	szName.Init();
	szName.Append(szIdentifier + 10, iLength - 10);
	iArguments = (int)pcCall->macpArguments.NumElements();
	bResult = true;
	psValue->sValue = IRNone();
	psValue->psType = &mpcTypes->msVoid;
	if ((szName.Equals("unreachable") || szName.Equals("trap")) && (iArguments == 0))
	{
		mpcBlock->Unreachable();
		StartBlock(NewBlock());
	}
	else if (szName.Equals("ia32_pause") && (iArguments == 0))
	{
	}
	else if (szName.Equals("expect") && (iArguments == 2))
	{
		bResult = Value(pcCall->macpArguments.GetPtr(0), psValue) && Convert(psValue, mpcTypes->Promote(psValue->psType));
	}
	else
	{
		psType = &mpcTypes->msUnsignedInt;
		if (szName.EndsWith("ll"))
		{
			psType = &mpcTypes->msUnsignedLongLong;
			szName.SetLength(szName.Length() - 2);
		}
		else if (szName.EndsWith('l'))
		{
			psType = mpcTypes->Long(false);
			szName.SetLength(szName.Length() - 1);
		}
		if ((!szName.Equals("ctz") && !szName.Equals("clz") && !szName.Equals("popcount")) || (iArguments != 1) ||
			!Value(pcCall->macpArguments.GetPtr(0), &sArgument) || !Convert(&sArgument, psType))
		{
			szName.Kill();
			return Unsupported(__LINE__);
		}

		//loop: x and the count; ctz and clz stop at the first set bit from their end (0 counts the width), popcount
		//clears the lowest set bit until there are none.
		sType = IRTypeOf(psType);
		sInt = IRTypeOf(&mpcTypes->msInt);
		iBits = psType->iBytes * 8;
		pcEntry = mpcBlock;
		pcLoop = NewBlock();
		pcBody = NewBlock();
		pcZero = szName.Equals("popcount") ? NULL : NewBlock();
		pcDone = NewBlock();
		sTest = mpcFunction->Add(mpcBlock, IRO_Eq, sType, sArgument.sValue, mpcFunction->AddInteger(sType, 0));
		mpcBlock->Branch(sTest, szName.Equals("popcount") ? pcDone : pcZero, pcLoop);
		StartBlock(pcLoop);
		//Adding a phi can move the block's others: both are found again after.
		mpcBlock->AddPhi(sType, mpcFunction->AddTemporary(sType));
		mpcBlock->AddPhi(sInt, mpcFunction->AddTemporary(sInt));
		psX = mpcBlock->masPhis.Get(mpcBlock->masPhis.NumElements() - 2);
		psCount = mpcBlock->masPhis.Get(mpcBlock->masPhis.NumElements() - 1);
		if (szName.Equals("popcount"))
		{
			sTest = mpcFunction->Add(mpcBlock, IRO_Eq, sType, psX->sResult, mpcFunction->AddInteger(sType, 0));
		}
		else
		{
			sTest = mpcFunction->Add(mpcBlock, IRO_And, sType, psX->sResult, mpcFunction->AddConstant(sType, szName.Equals("ctz") ? 1 : ((uint64)1 << (iBits - 1))));
			sTest = mpcFunction->Add(mpcBlock, IRO_Ne, sType, sTest, mpcFunction->AddInteger(sType, 0));
		}
		mpcBlock->Branch(sTest, pcDone, pcBody);
		StartBlock(pcBody);
		if (szName.Equals("popcount"))
		{
			sX2 = mpcFunction->Add(mpcBlock, IRO_Sub, sType, psX->sResult, mpcFunction->AddInteger(sType, 1));
			sX2 = mpcFunction->Add(mpcBlock, IRO_And, sType, psX->sResult, sX2);
		}
		else
		{
			sX2 = mpcFunction->Add(mpcBlock, szName.Equals("ctz") ? IRO_LShr : IRO_Shl, sType, psX->sResult, mpcFunction->AddInteger(sType, 1));
		}
		sCount2 = mpcFunction->Add(mpcBlock, IRO_Add, sInt, psCount->sResult, mpcFunction->AddInteger(sInt, 1));
		mpcBlock->Jump(pcLoop);
		pcLoop->AddPhiArgument(psX, pcEntry, sArgument.sValue);
		pcLoop->AddPhiArgument(psX, pcBody, sX2);
		pcLoop->AddPhiArgument(psCount, pcEntry, mpcFunction->AddInteger(sInt, 0));
		pcLoop->AddPhiArgument(psCount, pcBody, sCount2);
		if (pcZero)
		{
			StartBlock(pcZero);
			mpcBlock->Jump(pcDone);
		}
		StartBlock(pcDone);
		psResult = mpcBlock->AddPhi(sInt, mpcFunction->AddTemporary(sInt));
		mpcBlock->AddPhiArgument(psResult, pcLoop, psCount->sResult);
		if (szName.Equals("popcount"))
		{
			mpcBlock->AddPhiArgument(psResult, pcEntry, mpcFunction->AddInteger(sInt, 0));
		}
		else
		{
			mpcBlock->AddPhiArgument(psResult, pcZero, mpcFunction->AddInteger(sInt, iBits));
		}
		psValue->sValue = psResult->sResult;
		psValue->psType = &mpcTypes->msInt;
	}
	szName.Kill();
	return bResult || Unsupported(__LINE__);
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
	if (!psType || !psType->IsPointer() || !psType->psBase->IsFunction() || !Value(pcCallee, &sCallee))
	{
		return Unsupported(__LINE__);
	}
	msElide = sElide;
	return Call(NULL, sCallee.sValue, psType->psBase, &pcCall->macpArguments, psValue);
}


//////////////////////////////////////////////////////////////////////////
//
// f(args...), as the generator's ExpandPackCall: the call is written with
// args... replaced by a name for each of the pack's elements, then given
// back its arguments.  Only the function parameter pack can be expanded.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ExpandPackCall(CCPPSyntaxCallExpression* pcCall, SCPP65IRValue* psValue)
{
	CArraySyntax				apcArguments;
	CCPPSyntax*					pcArgument;
	CCPPSyntax*					pcPattern;
	CCPPSyntaxIdExpression*		pcId;
	CCPPSyntaxName*				pcName;
	CCPPSyntaxNameComponent*	pcComponent;
	CCPPSyntaxMemory*			pcMemory;
	size						ui;
	size						uiElement;
	bool						bResult;

	apcArguments.Init();
	apcArguments.Copy(&pcCall->macpArguments);
	pcCall->macpArguments.ReInit();
	pcMemory = mpcGenerator->mpcTree->GetMemory();
	for (ui = 0; ui < apcArguments.NumElements(); ui++)
	{
		pcArgument = apcArguments.GetPtr(ui);
		if (pcArgument->GetKind() != CPPS_PackExpansionExpression)
		{
			pcCall->macpArguments.Add(pcArgument);
			continue;
		}
		pcPattern = mpcGenerator->SkipParentheses(((CCPPSyntaxPackExpansionExpression*)pcArgument)->mpcPattern);
		if (!mpcGenerator->mpcPackParameter || (pcPattern->GetKind() != CPPS_IdExpression) || (((CCPPSyntaxIdExpression*)pcPattern)->mpcName->mpcSymbol != mpcGenerator->mpcPackParameter))
		{
			pcCall->macpArguments.ReInit();
			pcCall->macpArguments.Copy(&apcArguments);
			apcArguments.Kill();
			return Unsupported(__LINE__);
		}
		for (uiElement = 0; uiElement < mpcGenerator->mapcPackElements.NumElements(); uiElement++)
		{
			pcId = pcMemory->CreateIdExpression(pcCall);
			pcId->SetTokens(pcPattern->miFirstToken, pcPattern->miLastToken);
			pcName = pcMemory->CreateName(pcId);
			pcName->SetTokens(pcPattern->miFirstToken, pcPattern->miLastToken);
			pcName->mpcSymbol = mpcGenerator->mapcPackElements.GetPtr(uiElement);
			pcComponent = pcMemory->CreateNameComponent(pcName);
			pcComponent->SetTokens(pcPattern->miFirstToken, pcPattern->miLastToken);
			pcComponent->meKind = ((CCPPSyntaxIdExpression*)pcPattern)->mpcName->GetLast()->meKind;
			pcComponent->miName = ((CCPPSyntaxIdExpression*)pcPattern)->mpcName->GetLast()->miName;
			pcComponent->mpcSymbol = pcName->mpcSymbol;
			pcName->macpComponents.Add(pcComponent);
			pcId->mpcName = pcName;
			pcCall->macpArguments.Add(pcId);
		}
	}

	bResult = CallExpression(pcCall, psValue);
	pcCall->macpArguments.ReInit();
	pcCall->macpArguments.Copy(&apcArguments);
	apcArguments.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The arguments are worked out last first (as the generator does), each
// converted to its parameter's type; missing ones are the defaults.  A
// variadic function's extra arguments are promoted.  A reference gets the
// argument's address.  The result comes back as the IR return type and is
// narrowed to the C++ type.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Call(SCPP65Function* psFunction, SIRReference sCallee, SCPP65Type* psFunctionType, CArraySyntax* papcArguments, SCPP65IRValue* psValue, CCPPSyntax* pcObject, bool bArrow, SCPP65Class* psObjectClass, bool bVirtual, SIRReference sGivenObject)
{
	CArrayCPP65Types*				papsParameters;
	CArraySyntax*					papcDefaults;
	SCPP65Type*						psReturn;
	bool							bVariadic;
	size							uiArguments;
	size							uiParameters;
	size							ui;
	CCPPSyntax*						pcArgument;
	SCPP65Type*						psParameter;
	SCPP65IRValue					sArgument;
	SCPP65IRPlace					sPlace;
	CArrayTemplate<SCPP65IRValue>	asArguments;
	SIRType							sResultType;
	SIRReference					sResult;
	SIRReference					sObject;
	SIRReference					sVtable;
	bool							bThisCall;
	int								iOffset;
	int								iPiece;
	SIRReference					sMade;
	SIRType							sPiece;
	CArrayIRReferences				asPieces;		//Every argument's value, an object's in pieces.
	CArrayIRReferences				asLoaded;		//The pieces of the objects passed by value...
	CArrayTemplate<int32>			aiLoaded;		//...and per argument where its pieces start, -1 for none.
	int32							iLoaded;
	SIRReference					sLoaded;
	SCPP65IRPlace					sCopy;

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
		return Unsupported(__LINE__);
	}
	uiArguments = papcArguments->NumElements();
	uiParameters = papsParameters ? papsParameters->NumElements() : 0;
	if ((uiArguments > uiParameters) && !bVariadic)
	{
		return Unsupported(__LINE__);
	}

	asArguments.Init();
	asArguments.GrowToAtLeastNumElements(uiArguments > uiParameters ? uiArguments : uiParameters, true, 0);
	asLoaded.Init();
	aiLoaded.Init();
	aiLoaded.GrowToAtLeastNumElements(asArguments.NumElements(), true, 0xFF);
	for (ui = uiArguments; ui > uiParameters; ui--)
	{
		pcArgument = papcArguments->GetPtr(ui - 1);
		if (!Value(pcArgument, &sArgument))
		{
			asArguments.Kill();
			asLoaded.Kill();
			aiLoaded.Kill();
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
			asArguments.Kill();
			asLoaded.Kill();
			aiLoaded.Kill();
			return Unsupported(__LINE__);
		}
		if (!Convert(&sArgument, psParameter))
		{
			asArguments.Kill();
			asLoaded.Kill();
			aiLoaded.Kill();
			return false;
		}
		*asArguments.Get(ui - 1) = sArgument;
	}
	for (ui = uiParameters; ui > 0; ui--)
	{
		pcArgument = (ui <= uiArguments) ? papcArguments->GetPtr(ui - 1) : (papcDefaults ? papcDefaults->GetPtr(ui - 1) : NULL);
		psParameter = papsParameters->GetPtr(ui - 1);
		if (!pcArgument || psParameter->IsArray() || (!IsSupportedType(psParameter) && !psParameter->IsClass()) ||
			(psParameter->IsClass() && (bVariadic || !mpcGenerator->CompleteClass(psParameter->psClass, NULL) || !mpcGenerator->IsTriviallyCopyable(psParameter->psClass))))
		{
			asArguments.Kill();
			asLoaded.Kill();
			aiLoaded.Kill();
			return Unsupported(__LINE__);
		}
		if (psParameter->IsClass())
		{
			//An object passed by value is copied a piece at a time (only objects whose copy is their bytes).
			if (!Value(pcArgument, &sArgument))
			{
				asArguments.Kill();
				asLoaded.Kill();
				aiLoaded.Kill();
				return false;
			}
			sPlace.sAddress = sArgument.sValue;
			sPlace.psType = sArgument.psType->IsReference() ? sArgument.psType->psBase : sArgument.psType;
			if (!sPlace.psType->IsClass() || !mpcGenerator->IsDerived(sPlace.psType->psClass, psParameter->psClass, NULL) || !ReferenceAddress(&sPlace, mpcTypes->Pointer(psParameter), &sArgument.sValue))
			{
				asArguments.Kill();
				asLoaded.Kill();
				aiLoaded.Kill();
				return Unsupported(__LINE__);
			}
			sArgument.psType = psParameter;

			//It is copied as it is worked out (the generator constructs it on the stack then), through a temporary for a
			//class with vtables so the copy has its own class's.
			if (mpcGenerator->IsPolymorphic(psParameter->psClass))
			{
				if (!Temporary(psParameter, &sCopy, false) || !CopyBytes(&sCopy, &sArgument))
				{
					asArguments.Kill();
					asLoaded.Kill();
					aiLoaded.Kill();
					return false;
				}
				sArgument.sValue = sCopy.sAddress;
			}
			*aiLoaded.Get(ui - 1) = (int32)asLoaded.NumElements();
			for (iOffset = 0; iOffset < psParameter->iBytes; iOffset += iPiece)
			{
				iPiece = PieceBytes(psParameter->iBytes - iOffset);
				sPiece = IRIntegerType(iPiece);
				sLoaded = mpcFunction->Load(mpcBlock, sPiece, AddressAt(sArgument.sValue, iOffset));
				asLoaded.Add(&sLoaded);
			}
		}
		else if (psParameter->IsReference())
		{
			if (!Place(pcArgument, &sPlace) || !ReferenceAddress(&sPlace, psParameter, &sArgument.sValue))
			{
				asArguments.Kill();
				asLoaded.Kill();
				aiLoaded.Kill();
				return false;
			}
			sArgument.psType = psParameter;
		}
		else if (!Value(pcArgument, &sArgument) || !Convert(&sArgument, psParameter))
		{
			asArguments.Kill();
			asLoaded.Kill();
			aiLoaded.Kill();
			return false;
		}
		*asArguments.Get(ui - 1) = sArgument;
	}

	//An object returned is made at the address passed first on the stack (a temporary's, or msElide's).
	if (psReturn->IsClass() && sMade.IsNone())
	{
		if (!Temporary(psReturn, &sPlace, true))
		{
			asArguments.Kill();
			asLoaded.Kill();
			aiLoaded.Kill();
			return false;
		}
		sMade = sPlace.sAddress;
	}

	//The arguments in order, objects passed by value as their pieces.
	asPieces.Init();
	for (ui = 0; ui < asArguments.NumElements(); ui++)
	{
		iLoaded = *aiLoaded.Get(ui);
		if (iLoaded < 0)
		{
			asPieces.Add(&asArguments.Get(ui)->sValue);
			continue;
		}
		asPieces.InsertBlockAt(asLoaded.Get(iLoaded), PieceCount(asArguments.Get(ui)->psType->iBytes), asPieces.NumElements());
	}
	aiLoaded.Kill();
	asLoaded.Kill();
	asArguments.Kill();

	//A member function's object, after the arguments as the generator does: a base's function gets the base's
	//address, and a virtual one is called through the object's vtable.
	bThisCall = psFunction ? (psFunction->psClass && !psFunction->bStatic) : !sGivenObject.IsNone();
	sObject = IRNone();
	if (bThisCall)
	{
		if (pcObject)
		{
			if (!ObjectAddress(pcObject, bArrow, &sObject))
			{
				asPieces.Kill();
				return false;
			}
		}
		else if (!sGivenObject.IsNone())
		{
			sObject = sGivenObject;
		}
		else
		{
			if (msThis.IsNone())
			{
				asPieces.Kill();
				return Unsupported(__LINE__);
			}
			sObject = msThis;
			psObjectClass = mpsFunction->psClass;
		}
		if (psFunction)
		{
			sObject = AddressAt(sObject, BaseOffset(psObjectClass, psFunction->psClass));
		}
		if (bVirtual && psFunction && psFunction->bVirtual)
		{
			sVtable = mpcFunction->Load(mpcBlock, IRType(IRTK_Pointer), AddressAt(sObject, psFunction->psClass->iVtableOffset));
			sCallee = mpcFunction->Load(mpcBlock, IRType(IRTK_Pointer), AddressAt(sVtable, 4 * psFunction->iSlot));
		}
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

	sResultType = ReturnIRType(psReturn);
	sResult = mpcFunction->Call(mpcBlock, sResultType, sCallee, bVariadic ? (int)uiParameters : -1, bThisCall);
	if (psReturn->IsClass())
	{
		psValue->sValue = sMade;
		psValue->psType = psReturn;
		return true;
	}
	if (psReturn->IsReference())
	{
		psValue->sValue = sResult;
		psValue->psType = psReturn;
		return true;
	}
	psValue->psType = psReturn;
	psValue->sValue = psReturn->IsVoid() ? IRNone() : ConvertIR(sResult, sResultType, IRTypeOf(psReturn), false);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// (T)x and the named casts, to scalars and void.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CastExpression(CCPPSyntaxTypeId* pcTypeId, CCPPSyntax* pcOperand, SCPP65IRValue* psValue)
{
	SCPP65Type*		psTo;
	CCPPSyntax*		pcFunction;

	psTo = mpcGenerator->GetTypeIdType(pcTypeId);
	if (!psTo || psTo->IsClass() || psTo->IsReference())
	{
		return Unsupported(__LINE__);
	}
	pcFunction = mpcGenerator->SkipParentheses(pcOperand);
	if ((pcFunction->GetKind() == CPPS_UnaryExpression) && (((CCPPSyntaxUnaryExpression*)pcFunction)->meOperator == CT_BitwiseAnd))
	{
		pcFunction = mpcGenerator->SkipParentheses(((CCPPSyntaxUnaryExpression*)pcFunction)->mpcOperand);
	}
	if ((pcFunction->GetKind() == CPPS_IdExpression) && ((CCPPSyntaxIdExpression*)pcFunction)->mpcName->mpcSymbol && (((CCPPSyntaxIdExpression*)pcFunction)->mpcName->mpcSymbol->meKind == CPPSK_Function))
	{
		//(FPointer)&f chooses the overload of f with the pointer's type.
		if (!psTo->IsPointer() || !psTo->psBase->IsFunction() || !FunctionAddress((CCPPSyntaxIdExpression*)pcFunction, psTo->psBase, psValue))
		{
			return Unsupported(__LINE__);
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
	return psTo->IsScalar() ? Convert(psValue, psTo) : Unsupported(__LINE__);
}


//////////////////////////////////////////////////////////////////////////
//
// Converts a value to psTo, as the generator's Convert does.
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
	if (psFrom->IsMemberPointer() || psTo->IsMemberPointer())
	{
		return ConvertMemberPointer(psValue, psTo);
	}
	if (psFrom->IsVoid() || !psFrom->IsScalar() || !psTo->IsScalar())
	{
		return Unsupported(__LINE__);
	}
	//A pointer to a derived class points at its base, which may be further on (a null pointer stays null).  The
	//derived class is laid out to know where its base is, even if nothing else has needed it whole yet.
	if (psFrom->IsPointer() && psTo->IsPointer() && psFrom->psBase->IsClass() && psTo->psBase->IsClass() && (psFrom->psBase->psClass != psTo->psBase->psClass) &&
		mpcGenerator->IsDerived(psFrom->psBase->psClass, psTo->psBase->psClass, NULL) && mpcGenerator->CompleteClass(psFrom->psBase->psClass, NULL) &&
		mpcGenerator->IsDerived(psFrom->psBase->psClass, psTo->psBase->psClass, &iOffset) && (iOffset != 0))
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
			return Unsupported(__LINE__);
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
// Conversions to and from pointers to members, as the generator's (an
// address or an offset, 4 bytes): an integer (0, null) is one as it is, one
// is a bool when it isn't 0, a pointer to a field of a base is one to the
// derived class's field (the base's offset added), and other pointers to
// members (to members of unrelated classes, by a cast) keep the value.
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
		//Otherwise (a cast between unrelated classes, say) it is the same value.
		iOffset = BaseOffset(psTo->psClass, psFrom->psClass);
		if ((iOffset != 0) && psFrom->psBase->IsFunction())
		{
			return Unsupported(__LINE__);
		}
		if (iOffset != 0)
		{
			psValue->sValue = mpcFunction->Add(mpcBlock, IRO_Add, IRType(IRTK_I32), psValue->sValue, mpcFunction->AddInteger(IRType(IRTK_I32), iOffset));
		}
	}
	else if (!psFrom->Equals(psTo))
	{
		return Unsupported(__LINE__);
	}
	psValue->psType = psTo;
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
	if (psType->IsReference())
	{
		psType = psType->psBase;
	}
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

	psType = mpcGenerator->TypeOf(pcMember->mpcObject);
	if (!psType)
	{
		return Unsupported(__LINE__);
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
		return Unsupported(__LINE__);
	}
	return FieldPlace(sObject, psType->psClass, mpcGenerator->GetComponentName(pcMember->mpcName->GetLast()), NULL, psPlace);
}


//////////////////////////////////////////////////////////////////////////
//
// The field lliName (or pcSymbol's) of the object of psClass at sObject; a
// reference field's place is what it refers to.  A static member is its
// variable.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::FieldPlace(SIRReference sObject, SCPP65Class* psClass, int64 lliName, CCPPSymbol* pcSymbol, SCPP65IRPlace* psPlace)
{
	SCPP65Field*	psField;
	SCPP65Variable*	psVariable;
	int				iOffset;

	if (!psClass || !mpcGenerator->CompleteClass(psClass, NULL))
	{
		return Unsupported(__LINE__);
	}
	psField = pcSymbol ? mpcGenerator->FindFieldSymbol(psClass, pcSymbol, &iOffset) : mpcGenerator->FindField(psClass, lliName, &iOffset);
	if (!psField)
	{
		psVariable = mpcGenerator->FindStatic(psClass, lliName);
		if (!psVariable)
		{
			return Unsupported(__LINE__);
		}
		return VariablePlace(psVariable, psPlace);
	}
	if (!IsSupportedType(psField->psType) && !ElementType(psField->psType)->IsClass())
	{
		return Unsupported(__LINE__);
	}
	psPlace->sAddress = AddressAt(sObject, iOffset);
	psPlace->psType = psField->psType;
	if (psPlace->psType->IsReference())
	{
		psPlace->sAddress = mpcFunction->Load(mpcBlock, IRType(IRTK_Pointer), psPlace->sAddress);
		psPlace->psType = psPlace->psType->psBase;
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
			return Unsupported(__LINE__);
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
			return Unsupported(__LINE__);
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
		if (pcName->macpComponents.NumElements() == 2)
		{
			psClass = mpcGenerator->GetComponentClass((CCPPSyntaxNameComponent*)pcName->macpComponents.GetPtr(0), NULL, pcCallee);
			bVirtual = false;
		}
		else
		{
			psClass = mpsFunction->psClass;
		}
		bThis = true;
	}

	lliName = mpcGenerator->GetFunctionName(pcName->GetLast());
	if (!psClass || (lliName == -1) || (pcName->macpComponents.NumElements() > 2))
	{
		return Unsupported(__LINE__);
	}
	psLookup = psClass;
	if (pcObject && (pcName->macpComponents.NumElements() == 2))
	{
		psLookup = mpcGenerator->QualifierClass(psClass, (CCPPSyntaxNameComponent*)pcName->macpComponents.GetPtr(0), pcCallee);
		if (!psLookup)
		{
			return Unsupported(__LINE__);
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
	psTemplate = mpcGenerator->CompleteClass(psLookup, pcCall) ? mpcGenerator->FindMemberTemplate(psLookup, lliName, pcCall->macpArguments.NumElements(), iConst) : NULL;
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
		return Unsupported(__LINE__);
	}
	if (bThis && !psFunction->bStatic && (!mpsFunction->psClass || msThis.IsNone() || !mpcGenerator->IsDerived(mpsFunction->psClass, psFunction->psClass, NULL)))
	{
		return Unsupported(__LINE__);
	}

	sSymbol = FunctionSymbol(psFunction);
	return Call(psFunction, sSymbol, NULL, &pcCall->macpArguments, psValue, pcObject, bArrow, pcObject ? psClass : NULL, bVirtual);
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

	psType = mpcGenerator->TypeOf(pcObject);
	if (psType && psType->IsReference())
	{
		psType = psType->psBase;
	}
	if (!psType || !psType->IsClass() || !mpcGenerator->CompleteClass(psType->psClass, pcObject))
	{
		return Unsupported(__LINE__);
	}
	apcArguments.Init();
	if (pcArgument)
	{
		apcArguments.Add(pcArgument);
	}
	psFunction = mpcGenerator->FindFunction(psType->psClass, mpcGenerator->GetOperatorName(eOperator, false), CPP65FK_Function, &apcArguments, true, pcObject, mpcGenerator->IsConstObject(pcObject));
	if (!psFunction || psFunction->bStatic)
	{
		apcArguments.Kill();
		return Unsupported(__LINE__);
	}
	bResult = Call(psFunction, FunctionSymbol(psFunction), NULL, &apcArguments, psValue, pcObject, false, psType->psClass, mpcGenerator->SkipParentheses(pcObject)->GetKind() != CPPS_IdExpression);
	apcArguments.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// left op right, or op operand when pcRight is NULL, with an object
// operand, as the generator's OperatorExpression: the left's member
// operator if one matches, otherwise a free operator function.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::OperatorExpression(ECppTokens eOperator, CCPPSyntax* pcLeft, CCPPSyntax* pcRight, SCPP65IRValue* psValue)
{
	SCPP65Type*		psType;
	SCPP65Function*	psFunction;
	CArraySyntax	apcArguments;
	int64			lliName;
	bool			bResult;

	lliName = mpcGenerator->GetOperatorName(eOperator, false);
	psType = mpcGenerator->TypeOf(pcLeft);
	if (psType && psType->IsReference())
	{
		psType = psType->psBase;
	}
	apcArguments.Init();
	if (psType && psType->IsClass() && mpcGenerator->CompleteClass(psType->psClass, NULL) && mpcGenerator->HasMember(psType->psClass, lliName))
	{
		if (pcRight)
		{
			apcArguments.Add(pcRight);
		}
		psFunction = mpcGenerator->FindFunction(psType->psClass, lliName, CPP65FK_Function, &apcArguments, true, pcLeft, mpcGenerator->IsConstObject(pcLeft));
		apcArguments.Kill();
		if (psFunction)
		{
			return OperatorCall(pcLeft, eOperator, pcRight, psValue);
		}
		apcArguments.Init();
	}

	//operator op(left, right), declared or from a function template.
	apcArguments.Add(pcLeft);
	if (pcRight)
	{
		apcArguments.Add(pcRight);
	}
	psFunction = mpcGenerator->FindFreeOperator(lliName, &apcArguments, pcLeft);
	if (!psFunction)
	{
		apcArguments.Kill();
		return Unsupported(__LINE__);
	}
	bResult = Call(psFunction, FunctionSymbol(psFunction), NULL, &apcArguments, psValue);
	apcArguments.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::PrintIR(CIRFunction* pcFunction)
{
	CIRPrinter	cPrinter;
	CChars		szText;

	szText.Init();
	cPrinter.Init(&mcModule, &szText);
	cPrinter.PrintFunction(pcFunction);
	cPrinter.Kill();
	fprintf(stderr, "%s\n", szText.Text());
	szText.Kill();
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
		if (!ConstructClass(&sPlace, psBase->psClass, papcArguments))
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
	DescribeVtable(psClass, sLabel.Index());
	if (psClass->iVtableOffset != -1)
	{
		mpcFunction->Store(mpcBlock, IRType(IRTK_Pointer), sLabel, AddressAt(sObject, psClass->iVtableOffset));
	}
	iEntry = (int)psClass->mapsVirtuals.NumElements();
	for (ui = 0; ui < psClass->macVtables.NumElements(); ui++)
	{
		psVtable = psClass->macVtables.Get(ui);
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
		for (j = 0; j < psVtable->mapsFunctions.NumElements(); j++)
		{
			AddVtableEntry(pcVtable, psVtable->mapsFunctions.GetPtr(j), psVtable->maiAdjust.GetValue(j));
		}
	}
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
			sElement.psType = psType->psBase;
			for (i = 0; i < psType->iCount; i++)
			{
				sElement.sAddress = AddressAt(psPlace->sAddress, i * psType->psBase->iBytes);
				if (!Construct(&sElement, NULL, NULL))
				{
					return false;
				}
			}
			return true;
		}
		sZero.sValue = mpcFunction->AddConstant(IRTypeOf(psType), 0);
		sZero.psType = psType;
		Store(psPlace, &sZero);
		return true;
	}

	if (pcInitialiser)
	{
		if (pcInitialiser->GetKind() == CPPS_ParenthesisedInitializer)
		{
			papcArguments = &((CCPPSyntaxParenthesisedInitializer*)pcInitialiser)->macpElements;
			pcInitialiser = NULL;
		}
		else if (pcInitialiser->GetKind() == CPPS_InitializerList)
		{
			papcArguments = &((CCPPSyntaxInitializerList*)pcInitialiser)->macpElements;
			pcInitialiser = NULL;
		}
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
		if (psSource && psSource->IsClass() && (psSource->psClass == psClass) && (mpcGenerator->SkipParentheses(pcInitialiser)->GetKind() == CPPS_CallExpression))
		{
			//A call that makes an object of the class makes it here (as C++17 requires).
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
// A phi in pcBlock of sValue1 from pcFrom1 and sValue2 from pcFrom2.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CCPP65IRFrontEnd::Phi(CIRBlock* pcBlock, SIRType sType, CIRBlock* pcFrom1, SIRReference sValue1, CIRBlock* pcFrom2, SIRReference sValue2)
{
	SIRReference	sResult;
	SIRPhi*			psPhi;

	sResult = mpcFunction->AddTemporary(sType);
	psPhi = pcBlock->AddPhi(sType, sResult);
	pcBlock->AddPhiArgument(psPhi, pcFrom1, sValue1);
	pcBlock->AddPhiArgument(psPhi, pcFrom2, sValue2);
	return sResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The symbol of a function's label; the generator writes the function.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CCPP65IRFrontEnd::FunctionSymbol(SCPP65Function* psFunction)
{
	CChars			szLabel;
	SIRReference	sSymbol;

	mpcGenerator->RequireFunction(psFunction);
	szLabel.Init();
	mpcGenerator->AppendFunctionLabel(&szLabel, psFunction);
	sSymbol = mcModule.Symbol(szLabel.Text());
	szLabel.Kill();
	return sSymbol;
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
// (DestroyTemporaries), unless it may not be made (miConditional: not
// supported).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Temporary(SCPP65Type* psType, SCPP65IRPlace* psPlace, bool bDestroy)
{
	bool	bDestructor;

	if (!mpcGenerator->CompleteClass(psType->psClass, NULL))
	{
		return Unsupported(__LINE__);
	}
	bDestructor = bDestroy && mpcGenerator->NeedsDestructor(psType->psClass);
	if (bDestructor && (miConditional > 0))
	{
		return Unsupported(__LINE__);
	}
	psPlace->sAddress = mpcFunction->Slot(mpcBlock, IRType(IRTK_Pointer), (uint32)psType->iBytes, 4);
	psPlace->psType = psType;
	if (bDestructor)
	{
		masTemporaries.Add(psPlace);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Copy constructs the object at psPlace from pcSource (of its class or one
// derived from it), as the generator's CopyConstruct: with the class's copy
// constructor, or its bytes.  An implicit copy constructor that copies
// members with their own is left to the generator.
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
		return Unsupported(__LINE__);
	}
	return CopyObject(psPlace, pcSource);
}


//////////////////////////////////////////////////////////////////////////
//
// object = source, as the generator's CopyObject: the bytes (whatever the
// class), keeping the destination's vtable pointers.
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
// The function type of psFunction (what a pointer to it points to).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65IRFrontEnd::FunctionType(SCPP65Function* psFunction)
{
	return mpcTypes->Function(psFunction->psReturn, &psFunction->mapsParameters, psFunction->bVariadic);
}


//////////////////////////////////////////////////////////////////////////
//
// How far into an object of psDerived its base psBase is; 0 if psBase isn't
// a base of it (or either is NULL).
//
//////////////////////////////////////////////////////////////////////////
int CCPP65IRFrontEnd::BaseOffset(SCPP65Class* psDerived, SCPP65Class* psBase)
{
	int	iOffset;

	if (!psDerived || !psBase || (psDerived == psBase) || !mpcGenerator->CompleteClass(psDerived, NULL) || !mpcGenerator->IsDerived(psDerived, psBase, &iOffset))
	{
		return 0;
	}
	return iOffset;
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
