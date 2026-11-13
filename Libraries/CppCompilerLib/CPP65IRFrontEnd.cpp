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
#include <stdio.h>
#include <stdint.h>
#include "BaseLib/StringHelper.h"
#include "SourceFile.h"
#include "PreprocessedTokens.h"
#include "CPP65Generator.h"
#include "IRPrinter.h"
#include "IRVerifier.h"
#include "IROptimiser.h"
#include "IR65Backend.h"
#include "CPP65IRFrontEnd.h"


//The IR front end: a function's IR from its syntax tree (Body), written for the back end (FinishWriting); IR types,
//blocks, parameters, source lines and what isn't supported.  The rest is in CPP65IRFrontEndUnit, Statements, Values,
//Operators, Conversions, Calls and Objects.


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::Init(CCPP65Generator* pcGenerator)
{
	muiAttempts = 0;
	mpcGenerator = pcGenerator;
	mpcTypes = &pcGenerator->mcTypes;
	mpcFunction = NULL;
	mpcBlock = NULL;
	mpcCPPFunction = NULL;
	msThis = IRNone();
	msResult = IRNone();
	msElide = IRNone();
	mpcValue = NULL;
	mpcProfiledCaller = NULL;
	mbProfiledCaller = false;
	mcModule.Init();
	mcModule.AddDefaultSpace(pcGenerator->mpcTarget->GetPointerBytes());
	mpcBackend = pcGenerator->mpcTarget->NewBackend();
	mcPipeline.Init();
	pcGenerator->mpcTarget->BuildPipeline(&mcPipeline);
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
	maszMissing.Init();
	maszNotImported.Init();
	mauiStringBytes.Init();
	mauiStringLengths.Init();
	muiFirstString = 0;
	mbUnsupported = false;
	mbFloatReported = false;
	mszUnsupportedFile = "";
	miUnsupportedLine = 0;
	mbCodeGiven = false;
	mszUnsupported[0] = '\0';
}


//////////////////////////////////////////////////////////////////////////
//
// What the unit's functions are counted and timed in: the generator's.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::SetStatistics(CCPPCompileStatistics* pcStatistics)
{
	mcModule.SetStatistics(pcStatistics);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::Kill(void)
{
	size				ui;
	SCPP65IRWritten*	psWritten;

	mauiStringLengths.Kill();
	mauiStringBytes.Kill();
	maszNotImported.Kill();
	maszMissing.Kill();
	maszRuntime.Kill();
	for (ui = 0; ui < masWritten.NumElements(); ui++)
	{
		psWritten = masWritten.Get(ui);
		psWritten->szLabel.Kill();
		FreeWritten(psWritten);

		//Code not given to the unit's assembly (a unit generated again, or one that failed) leaves the store.
		if (!mbCodeGiven)
		{
			CCPP65Assembly::RemoveCode(mpcGenerator->mpcSpillStore, mpcGenerator->muiSpillUnit, ui, 0, psWritten->uiPieces);
		}
	}
	masWritten.Kill();
	SafeFree(mpcBackend);
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
// Writes pcFunction's body (already set up by the generator's
// GenerateDefinition: parameters and locals have their variables) as IR and
// generates it into the generator's code.  False, with nothing changed, if
// the function uses something the front end doesn't support yet; the
// generator's own errors are not reported meanwhile (it reports them when
// it writes the function itself).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Body(CCPPFunction* pcFunction)
{
	CChars					szLabel;
	bool					bResult;

	muiAttempts++;
	if (muiAttempts > mpcGenerator->mpcOptions->muiFunctionLimit)
	{
		return false;
	}
	if (((pcFunction->meKind != CPPFK_Function) && (pcFunction->meKind != CPPFK_Constructor) && (pcFunction->meKind != CPPFK_Destructor)) ||
		!pcFunction->mpsReturn || (!IsSupportedType(pcFunction->mpsReturn) && !pcFunction->mpsReturn->IsClass()) || pcFunction->mpsReturn->IsArray() ||
		(pcFunction->mpsReturn->IsClass() && !mpcGenerator->CompleteClass(pcFunction->mpsReturn->pcClass, NULL)))
	{
		mbUnsupported = false;
		Unsupported(__FILE__, __LINE__);
		return false;
	}

	szLabel.Init();
	mpcGenerator->AppendFunctionLabel(&szLabel, pcFunction);
	BeginWriting(pcFunction, szLabel.Text(), ReturnIRType(pcFunction->mpsReturn), (pcFunction->mbVariadic ? IRA_Variadic : 0) | ((pcFunction->mpcClass && !pcFunction->mbStatic) ? IRA_ThisCall : 0));

	//The generator's errors are not reported while the front end works (it reports them if it writes the function
	//itself).  What it fails to work out comes back as NULL, which the front end doesn't support; quiet errors from
	//its own probing (UsesFloat ...) are not failures.
	mpcGenerator->BeginQuiet();
	bResult = Parameters() && Locals();
	if (bResult && (pcFunction->meKind == CPPFK_Constructor))
	{
		//A member initialiser's temporaries would need destroying after it (not supported).
		bResult = ConstructorStart(pcFunction->mpcDefinition) && ((masTemporaries.NumElements() == 0) || Unsupported(__FILE__, __LINE__));
	}
	else if (bResult && (pcFunction->meKind == CPPFK_Destructor))
	{
		//The vtables are this class's while its destructor runs; returns go to the end, which destroys the fields and
		//bases.
		mpcExit = NewBlock();
		bResult = StoreVtables(msThis, pcFunction->mpcClass);
	}
	bResult = bResult && (!pcFunction->mpcDefinition || Statement(pcFunction->mpcDefinition->mpcBody)) && DestroyLiving(0);
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
void CCPP65IRFrontEnd::BeginWriting(CCPPFunction* pcFunction, const char* szLabel, SIRType sReturn, uint32 uiAttributes)
{
	//The prologue's instructions (parameters, locals, a constructor's bases) are for the function's first line.
	mcModule.muiSource = (pcFunction && pcFunction->mpcDefinition && (pcFunction->mpcDefinition->miFirstToken >= 0)) ? (uint32)pcFunction->mpcDefinition->miFirstToken + 1 : 0;
	mpcCPPFunction = pcFunction;
	mbUnsupported = false;
	mbFloatReported = false;
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
	mcAnonymousAddresses.Init(true);
	mpcFunction = mcModule.AddFunction(szLabel, sReturn, uiAttributes);
	if (pcFunction)
	{
		MarkFunctionSymbol(pcFunction, mpcFunction->muiSymbol);
	}
	msThis = IRNone();
	msResult = IRNone();
	msElide = IRNone();
	mpcValue = NULL;
	mpcBlock = mpcFunction->AddBlock();
}


//////////////////////////////////////////////////////////////////////////
//
// Warns of what the optimiser found (SetWarnings: a line each) at the function's definition.  The IR has no lines,
// so the warning is the function's.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::OptimiserWarnings(CChars* pszWarnings)
{
	CChars	szMessage;

	if (pszWarnings->Empty() || !mpcCPPFunction || !mpcCPPFunction->mpcDefinition)
	{
		return;
	}

	//A comparison the generator already warned about in the function (a 16 bit variable compared with 0xFFFFFFFF) is
	//the same one, seen before the values were followed.
	if (mpcGenerator->HasWarning(mpcCPPFunction->mpcDefinition, "Comparison is always"))
	{
		return;
	}
	pszWarnings->RemoveLastCharacter();
	szMessage.Init("In this function, once the values are followed: ");
	szMessage.Append(pszWarnings);
	mpcGenerator->Warning(mpcCPPFunction->mpcDefinition, szMessage.Text());
	szMessage.Kill();
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
	CIROptimiser			cOptimiser;
	CChars					szCode;
	CChars					szWarnings;
	SCPP65IRWritten*		psWritten;
	CIRWrittenFunction*		pcWritten;
	SAsm65816FunctionProfile*	psProfiled;
	size					ui;
	size					uiStart;
	size					uiLength;
	uint32					uiFingerprint;

	//Instructions the optimiser adds have no source of their own.
	mcModule.muiSource = 0;
	bResult = bResult && !mbUnsupported;
	uiFingerprint = 0;
	if (bResult)
	{
		//IR that doesn't verify is the front end's (or a pass's) mistake: the function is an error.
		Finish();
		if (mpcGenerator->mbLineTable || InliningProfile())
		{
			uiFingerprint = Fingerprint(mpcFunction);
		}
		if (mpcGenerator->mpcOptions->mbOptimise)
		{
			InitOptimiser(&cOptimiser);
			if (InliningProfile())
			{
				//The function isn't written yet: CallCount is told its fingerprint here.
				psProfiled = InliningProfile()->Get(szLabel);
				mpcProfiledCaller = mpcFunction;
				mbProfiledCaller = psProfiled && ((psProfiled->uiHash == 0) || (psProfiled->uiHash == uiFingerprint));
			}
			szWarnings.Init();
			cOptimiser.SetWarnings(&szWarnings);
			mcModule.Begin(CPPCP_Optimise);
			cOptimiser.Optimise(mpcFunction);
			mcModule.End(CPPCP_Optimise);
			cOptimiser.Kill();
			mpcProfiledCaller = NULL;
			OptimiserWarnings(&szWarnings);
			szWarnings.Kill();
		}
		bResult = VerifyIR(mpcFunction, __FILE__, __LINE__);
		if (mpcGenerator->mpcOptions->mbPrintIR)
		{
			fprintf(stderr, "Function %d\n", (int)muiAttempts);
			PrintIR(mpcFunction);
		}
	}

	if (bResult)
	{
		szCode.Init();
		mcModule.Begin(CPPCP_BackEnd);
		bResult = GenerateCode(mpcFunction, szLabel, &szCode, &pcWritten);
		mcModule.End(CPPCP_BackEnd);
		if (bResult)
		{
			//The code waits (with a marker where it goes) for FinishUnit, which inlines across the unit.
			psWritten = masWritten.Add();
			psWritten->pcFunction = mpcFunction;
			psWritten->szLabel.Init(szLabel);
			psWritten->uiPieces = 0;
			CCPP65Assembly::PutCode(mpcGenerator->mpcSpillStore, mpcGenerator->muiSpillUnit, masWritten.NumElements() - 1, szCode.Text(), szCode.Length(), &psWritten->uiPieces);
			psWritten->pcBackend = pcWritten;
			psWritten->uiFingerprint = uiFingerprint;
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
		mpcGenerator->mpcFunction = NULL;
		mpcGenerator->mbThis = false;
	}
	if (!bResult && mpcFunction)
	{
		mcModule.RemoveFunction(mpcFunction);
	}
	mcAddresses.Kill();
	mcAnonymousAddresses.Kill();
	mpcFunction = NULL;
	mpcBlock = NULL;
	mpcCPPFunction = NULL;
	if (bResult)
	{
		mcModule.Count(CPPCC_Functions);
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
	CCPPFunction	cFunction;
	SCPP65IRPlace	sPlace;
	bool			bResult;

	memset(&cFunction, 0, sizeof(CCPPFunction));
	cFunction.meKind = CPPFK_Function;
	cFunction.mpsReturn = &mpcTypes->msVoid;
	BeginWriting(&cFunction, szLabel, IRType(IRTK_None), IRA_Local);
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
bool CCPP65IRFrontEnd::Unsupported(const char* szFile, int iLine)
{
	if (!mbUnsupported)
	{
		SetUnsupported(szFile, iLine, NULL);
	}
	mbUnsupported = true;
	mpcGenerator->mpcStatistics->Count(CPPCC_UnsupportedExits);
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A bit-field's address was wanted (&s.iBits, or a reference that isn't
// const bound to it): it has none.  [expr.unary.op] 5, [dcl.init.ref] 5
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::BitFieldAddress(const char* szFile, int iLine)
{
	if (!mbUnsupported)
	{
		SetUnsupported(szFile, iLine, "A bit-field has no address: it can't be pointed to or bound to a reference that isn't const");
	}
	return Unsupported(szFile, iLine);
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
		NoFloats();
		return IRType((psType->iBytes == 8) ? IRTK_F64 : IRTK_F32);
	case CPP65TK_MemberPointer:
		return IRType(IRTK_I32);
	default:
		return IRType(IRTK_Pointer);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A floating point value is in the code being written: with SetNoFloats an
// error at the statement (once a function), as every float or double the
// code uses comes through IRTypeOf.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::NoFloats(void)
{
	int		iToken;

	if (!mpcGenerator->mbNoFloats || mbFloatReported)
	{
		return;
	}
	mbFloatReported = true;
	iToken = (mcModule.muiSource > 0) ? (int)mcModule.muiSource - 1 : -1;
	if ((iToken < 0) && mpcCPPFunction && mpcCPPFunction->mpcDefinition)
	{
		iToken = mpcCPPFunction->mpcDefinition->miFirstToken;
	}
	mpcGenerator->ReportErrorAt(iToken, "Floating point is used here, and this build has none (CCPP65Builder::SetNoFloats: the kernal has no floats)");
}


//////////////////////////////////////////////////////////////////////////
//
// On the 65C64 a result narrower than 4 bytes comes back extended to 32 bits
// in A, so the IR returns an i32.
//
//////////////////////////////////////////////////////////////////////////
SIRType CCPP65IRFrontEnd::ReturnIRType(SCPP65Type* psType)
{
	if (mpcGenerator->mpcTarget->WidensResults() && psType->IsScalar() && !psType->IsPointer() && (psType->iBytes < 4))
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
		return mpcGenerator->CompleteClass(psElement->pcClass, NULL);
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
	SCPP65IRValue	sValue;

	if (psPlace->psBitField)
	{
		//A const reference to a bit-field refers to a temporary holding its value.  [dcl.init.ref] 5
		if (!psReference->psBase->bConst || psReference->psBase->bVolatile || !Load(psPlace, &sValue))
		{
			return BitFieldAddress(__FILE__, __LINE__);
		}
		*psAddress = mpcFunction->Slot(mpcBlock, IRType(IRTK_Pointer), (uint32)psPlace->psType->iBytes, (uint32)psPlace->psType->iBytes);
		mpcFunction->Store(mpcBlock, IRTypeOf(psPlace->psType), sValue.sValue, *psAddress);
		return true;
	}

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
// named one's place is where it was pushed (argaddr).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Parameters(void)
{
	size					ui;
	size					uiParameter;
	SCPP65Type*				psType;
	CCPPVariable*			pcVariable;
	SIRReference			sAddress;
	SCPP65IRPlace			sPlace;
	CArrayTemplate<int32>	aiFirst;	//Per C++ parameter: its first IR parameter.
	CArrayTemplate<SIRReference>	asByAddress;	//Per C++ parameter: the IR parameter of an object passed by address.
	int32					iFirst;

	//A member function's this comes first (in X), then the address a function that returns an object makes it at
	//(the first argument on the stack).
	if (mpcFunction->muiAttributes & IRA_ThisCall)
	{
		msThis = mpcFunction->AddParameter(IRType(IRTK_Pointer));
	}
	if (mpcCPPFunction->mpsReturn->IsClass())
	{
		msResult = mpcFunction->AddParameter(IRType(IRTK_Pointer));
	}
	aiFirst.Init();
	asByAddress.Init();
	for (ui = 0; ui < mpcCPPFunction->mapsParameters.NumElements(); ui++)
	{
		psType = mpcCPPFunction->mapsParameters.GetPtr(ui);
		iFirst = (int32)mpcFunction->masParameters.NumElements();
		aiFirst.Add(&iFirst);
		sAddress = IRNone();
		if (psType->IsClass())
		{
			//An object passed by value is its bytes where the caller pushed them, or its address (IsPassedByAddress).
			if (!mpcGenerator->CompleteClass(psType->pcClass, NULL))
			{
				aiFirst.Kill();
				asByAddress.Kill();
				return Unsupported(__FILE__, __LINE__);
			}
			if (IsPassedByAddress(psType))
			{
				sAddress = mpcFunction->AddParameter(IRType(IRTK_Pointer));
			}
			else
			{
				AddPieces(psType->iBytes);
			}
			asByAddress.Add(&sAddress);
			continue;
		}
		asByAddress.Add(&sAddress);
		if (!IsSupportedType(psType) || psType->IsVoid() || psType->IsArray())
		{
			aiFirst.Kill();
			asByAddress.Kill();
			return Unsupported(__FILE__, __LINE__);
		}
		mpcFunction->AddParameter(IRTypeOf(psType));
	}

	//The generator's parameter variables are known by their index in the parameters.  Object parameters are the
	//called function's to destroy.
	for (ui = 0; ui < mpcGenerator->mapcLocals.NumElements(); ui++)
	{
		pcVariable = mpcGenerator->mapcLocals.GetPtr(ui);
		if (pcVariable->meStorage != CPPVS_Parameter)
		{
			continue;
		}
		uiParameter = (size)pcVariable->miOffset;
		if (uiParameter >= mpcCPPFunction->mapsParameters.NumElements())
		{
			aiFirst.Kill();
			asByAddress.Kill();
			return Unsupported(__FILE__, __LINE__);
		}
		if (pcVariable->mbRestrict)
		{
			mpcFunction->SetRestrictParameter((size)*aiFirst.Get(uiParameter));
		}
		if (!asByAddress.Get(uiParameter)->IsNone())
		{
			//The caller's copy, which the caller destroys.
			mcAddresses.Put((int64)(size)pcVariable, asByAddress.Get(uiParameter));
			continue;
		}
		sAddress = mpcFunction->Add(mpcBlock, IRO_ArgAddress, IRType(IRTK_Pointer), mpcFunction->AddInteger(IRType(IRTK_I32), *aiFirst.Get(uiParameter)));
		mcAddresses.Put((int64)(size)pcVariable, &sAddress);
		if (pcVariable->mpsType->IsClass())
		{
			sPlace.sAddress = sAddress;
			sPlace.psType = pcVariable->mpsType;
			AddLiving(&sPlace);
		}
	}
	aiFirst.Kill();
	asByAddress.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an object passed by value is passed by its address: one whose
// copy isn't its bytes (it has a copy constructor, or a base or field does).  The caller copies it to a
// temporary, passes its address and destroys it at the end of the full
// expression, as the Itanium C++ ABI does; others are their bytes, pushed.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::IsPassedByAddress(SCPP65Type* psType)
{
	return psType->IsClass() && !mpcGenerator->IsTriviallyCopyable(psType->pcClass);
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
// Generates a function with the target's back end, in the generator's arena.
// The runtime routines its code calls are added to the generator's, and what
// the back end kept about it (for CIRBackend::FinishFunction) is *ppcWritten,
// the caller's to free: NULL if nothing, or if the function wasn't written.
// The back end's errors (a frame too big ...) are why it wasn't.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::GenerateCode(CIRFunction* pcFunction, const char* szLabel, CChars* pszCode, CIRWrittenFunction** ppcWritten)
{
	CCPPErrors		cErrors;
	CArrayChars*	paszRuntime;
	size			ui;
	bool			bResult;

	*ppcWritten = NULL;
	if (!mpcBackend)
	{
		return Unsupported(__FILE__, __LINE__);
	}

	cErrors.Init();
	mpcBackend->Init(&mcModule, &mpcGenerator->miLabels, mpcGenerator->GetBackendScratch());
	mpcBackend->SetOptions(mpcGenerator->mpcOptions);
	mpcBackend->SetSourceMarkers(mpcGenerator->mbListing || mpcGenerator->mbLineTable);
	bResult = mpcBackend->GenerateFunction(pcFunction, szLabel, pszCode, &cErrors);
	if (bResult)
	{
		*ppcWritten = mpcBackend->TakeWritten();
		paszRuntime = mpcBackend->GetRuntime();
		for (ui = 0; ui < paszRuntime->NumElements(); ui++)
		{
			mpcGenerator->maszRuntime.AddIfUnique(paszRuntime->GetText(ui));
		}
	}
	else
	{
		SetUnsupported(__FILE__, __LINE__, cErrors.Text());
	}
	mpcBackend->Kill();
	cErrors.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// What the back end kept about a written function is freed.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::FreeWritten(SCPP65IRWritten* psWritten)
{
	if (psWritten->pcBackend)
	{
		psWritten->pcBackend->Kill();
		free(psWritten->pcBackend);
		psWritten->pcBackend = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The named functions the unit's inliner looked for and found neither in
// the unit nor in the generator's imports.
//
//////////////////////////////////////////////////////////////////////////
CArrayChars* CCPP65IRFrontEnd::GetMissingCallees(void)
{
	return &maszMissing;
}


//////////////////////////////////////////////////////////////////////////
//
// The names the inliner looked for in the imports and didn't find
// (InlineUnit), which the unit's turn checks (CIRImports::Missed).
//
//////////////////////////////////////////////////////////////////////////
CArrayChars* CCPP65IRFrontEnd::GetNotImported(void)
{
	return &maszNotImported;
}


//////////////////////////////////////////////////////////////////////////
//
// The back ends (and CIRPrinter) write a marker line (IR_SOURCE_MARKER and
// the source token + 1) where the instructions' source changes.  Where it
// is another line than the last's, with bComments (a listing) it becomes a
// comment with the file, line and text of the line, and with bLines a
// .line directive with the file's full name (the linker's line table);
// otherwise they go.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::SourceComments(CChars* pszCode, bool bComments, bool bLines)
{
	CChars				szOut;
	const char*			szText;
	const char*			szEnd;
	const char*			szLine;
	CSourceFile*		pcLastFile;
	int					iLastLine;
	CSourceFile*		pcFile;
	int					iLine;
	size				uiMarker;

	if (!pszCode->Contains(IR_SOURCE_MARKER))
	{
		return;
	}

	uiMarker = strlen(IR_SOURCE_MARKER);
	szOut.Init();
	pcLastFile = NULL;
	iLastLine = -1;
	for (szText = pszCode->Text(); *szText != '\0'; szText = (*szEnd == '\n') ? szEnd + 1 : szEnd)
	{
		szEnd = strchr(szText, '\n');
		if (!szEnd)
		{
			szEnd = szText + strlen(szText);
		}
		if (strncmp(szText, IR_SOURCE_MARKER, uiMarker) != 0)
		{
			szOut.AppendSubString(szText, (*szEnd == '\n') ? szEnd + 1 : szEnd);
			continue;
		}
		if ((!bComments && !bLines) || !SourceLine((int)strtol(szText + uiMarker, NULL, 10) - 1, &pcFile, &iLine))
		{
			continue;
		}
		if ((pcFile == pcLastFile) && (iLine == iLastLine))
		{
			continue;
		}
		pcLastFile = pcFile;
		iLastLine = iLine;

		//\t\t.line "C:\\Path\\File.cpp", 12 (a backslash or quote in the name escaped).
		if (bLines)
		{
			szOut.Append("\t\t.line \"");
			for (szLine = pcFile->FullName(); *szLine != '\0'; szLine++)
			{
				if ((*szLine == '\\') || (*szLine == '"'))
				{
					szOut.Append('\\');
				}
				szOut.Append(*szLine);
			}
			szOut.Append("\", ");
			szOut.Append(iLine + 1);
			szOut.AppendNewLine();
		}
		if (!bComments)
		{
			continue;
		}

		//\t\t; File.cpp(12): the line, without its indent.
		szLine = pcFile->GetContents() + pcFile->GetOffset(iLine, 0);
		while ((*szLine == ' ') || (*szLine == '\t'))
		{
			szLine++;
		}
		szOut.Append("\t\t; ");
		szOut.Append(pcFile->ShortName());
		szOut.Append('(');
		szOut.Append(iLine + 1);
		szOut.Append("): ");
		for (; (*szLine != '\0') && (*szLine != '\n') && (*szLine != '\r'); szLine++)
		{
			szOut.Append(*szLine);
		}
		szOut.AppendNewLine();
	}
	pszCode->Kill();
	pszCode->Init(&szOut);
	szOut.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Where the token iToken is in the source: a token a macro made is where
// the outermost macro was used.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::SourceLine(int iToken, CSourceFile** ppcFile, int* piLine)
{
	SPreprocessedToken*	psSource;
	SMacroExpansion*	psExpansion;
	CPreprocessedTokens*	pcSource;

	if ((iToken < 0) || ((size)iToken >= mpcGenerator->mpcTokeniser->NumTokens()))
	{
		return false;
	}
	psSource = mpcGenerator->mpcTokeniser->GetSource(mpcGenerator->mpcTokeniser->Get(iToken));
	if (!psSource)
	{
		return false;
	}
	*ppcFile = psSource->pcFile;
	*piLine = psSource->iLine;
	pcSource = mpcGenerator->mpcTokeniser->mpcSource;
	if (psSource->iExpansion >= 0)
	{
		psExpansion = pcSource->GetExpansion((size)psSource->iExpansion);
		while (psExpansion->iParent >= 0)
		{
			psExpansion = pcSource->GetExpansion((size)psExpansion->iParent);
		}
		*ppcFile = psExpansion->pcFile;
		*piLine = psExpansion->iLine;
	}
	return (*ppcFile != NULL) && (*piLine >= 0) && ((size)*piLine < (*ppcFile)->NumLines());
}


//////////////////////////////////////////////////////////////////////////
//
// Each local variable is a slot.  Static locals are the generator's data.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Locals(void)
{
	size			ui;
	CCPPVariable*	pcVariable;
	SIRReference	sAddress;
	SIRReference*	psAnonymousAddress;
	int				iAlign;

	mcAnonymousAddresses.Kill();
	mcAnonymousAddresses.Init(true);
	for (ui = 0; ui < mpcGenerator->mapcLocals.NumElements(); ui++)
	{
		pcVariable = mpcGenerator->mapcLocals.GetPtr(ui);
		if (pcVariable->meStorage != CPPVS_Local)
		{
			continue;
		}
		if (!IsLocalType(pcVariable->mpsType) || pcVariable->mpsType->IsVoid())
		{
			return Unsupported(__FILE__, __LINE__);
		}
		if (pcVariable->mpcAnonymous)
		{
			//The members of an anonymous union or struct share its slot (made for its first member).
			psAnonymousAddress = mcAnonymousAddresses.Get((int64)(size)pcVariable->mpcAnonymous);
			if (!psAnonymousAddress)
			{
				sAddress = mpcFunction->Slot(mpcBlock, IRType(IRTK_Pointer), (uint32)pcVariable->mpcAnonymous->mpsType->iBytes, 4);
				mcAnonymousAddresses.Put((int64)(size)pcVariable->mpcAnonymous, &sAddress);
				psAnonymousAddress = mcAnonymousAddresses.Get((int64)(size)pcVariable->mpcAnonymous);
			}
			sAddress = AddressAt(*psAnonymousAddress, pcVariable->miOffset);
			mcAddresses.Put((int64)(size)pcVariable, &sAddress);
			continue;
		}
		iAlign = ((pcVariable->mpsType->iBytes >= 4) || pcVariable->mpsType->IsArray() || pcVariable->mpsType->IsClass()) ? 4 : pcVariable->mpsType->iBytes;
		sAddress = mpcFunction->Slot(mpcBlock, IRType(IRTK_Pointer), (uint32)pcVariable->mpsType->iBytes, (uint32)iAlign);
		mcAddresses.Put((int64)(size)pcVariable, &sAddress);
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
// pcFunction's IR as text (with each instruction's source token when
// bSources) appended to pszText.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::PrintFunction(CIRFunction* pcFunction, CChars* pszText, bool bSources)
{
	CIRPrinter	cPrinter;

	cPrinter.Init(&mcModule, pszText);
	cPrinter.SetSources(bSources);
	cPrinter.PrintFunction(pcFunction);
	cPrinter.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The function's IR (with its source lines as comments) to stderr, for
// debugging (CCPPCompileOptions::mbPrintIR).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::PrintIR(CIRFunction* pcFunction)
{
	CChars	szText;

	szText.Init();
	PrintFunction(pcFunction, &szText, true);
	SourceComments(&szText, true, false);
	fprintf(stderr, "%s\n", szText.Text());
	szText.Kill();
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
// A phi at the top of loop pcLoop, sStart from pcEntry; its value round the
// loop is given when the loop's end is written (ClosePhi).  The phi is
// known by its result: adding a phi can move the block's others.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CCPP65IRFrontEnd::LoopPhi(CIRBlock* pcLoop, SIRType sType, CIRBlock* pcEntry, SIRReference sStart)
{
	SIRReference	sResult;

	sResult = mpcFunction->AddTemporary(sType);
	pcLoop->AddPhiArgument(pcLoop->AddPhi(sType, sResult), pcEntry, sStart);
	return sResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The value LoopPhi's sPhi has round the loop: sNext from pcLatch.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::ClosePhi(CIRBlock* pcLoop, SIRReference sPhi, CIRBlock* pcLatch, SIRReference sNext)
{
	pcLoop->AddPhiArgument(pcLoop->FindPhi(sPhi), pcLatch, sNext);
}


//////////////////////////////////////////////////////////////////////////
//
// Starts a loop round an index of sType from sStart: the loop's block tests
// uiTest(index, sLimit), going on to the loop's body (started here) or
// out.  Returns the index.  The body ends with EndCountedLoop.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CCPP65IRFrontEnd::BeginCountedLoop(SCPP65IRCountedLoop* psLoop, SIRType sType, SIRReference sStart, uint16 uiTest, SIRReference sLimit)
{
	CIRBlock*	pcEntry;
	CIRBlock*	pcBody;

	pcEntry = mpcBlock;
	psLoop->pcLoop = NewBlock();
	pcBody = NewBlock();
	psLoop->pcDone = NewBlock();
	JumpTo(psLoop->pcLoop);
	StartBlock(psLoop->pcLoop);
	psLoop->sIndex = LoopPhi(psLoop->pcLoop, sType, pcEntry, sStart);
	mpcBlock->Branch(mpcFunction->Add(mpcBlock, uiTest, sType, psLoop->sIndex, sLimit), pcBody, psLoop->pcDone);
	StartBlock(pcBody);
	return psLoop->sIndex;
}


//////////////////////////////////////////////////////////////////////////
//
// Ends BeginCountedLoop's body: the index is sNext going round again, and
// the code goes on after the loop.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::EndCountedLoop(SCPP65IRCountedLoop* psLoop, SIRReference sNext)
{
	CIRBlock*	pcLatch;

	pcLatch = mpcBlock;
	JumpTo(psLoop->pcLoop);
	ClosePhi(psLoop->pcLoop, psLoop->sIndex, pcLatch, sNext);
	StartBlock(psLoop->pcDone);
}


//////////////////////////////////////////////////////////////////////////
//
// Inits pcOptimiser with the target's pipeline and settings (CCPPTarget::
// BuildPipeline and ConfigureOptimiser), the target's int and, when the
// target is guided by one, the profile.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::InitOptimiser(CIROptimiser* pcOptimiser)
{
	pcOptimiser->Init(&mcModule);
	pcOptimiser->SetPipeline(&mcPipeline);
	mpcGenerator->mpcTarget->ConfigureOptimiser(pcOptimiser);
	pcOptimiser->SetLibraryInt(IRTypeOf(&mpcTypes->msInt));
	if (InliningProfile())
	{
		pcOptimiser->SetProfile(this);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Why a function was left to the generator: the front end's file (__FILE__,
// kept without its directory) and line that found it and, when there is
// one, the reason (szReason, or NULL to keep the last).  The generator tells
// it with the function's error (CCPP65Generator::AppendUnsupported).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::SetUnsupported(const char* szFile, int iLine, const char* szReason)
{
	const char*	szName;

	for (szName = szFile; *szFile != '\0'; szFile++)
	{
		if ((*szFile == '/') || (*szFile == '\\'))
		{
			szName = szFile + 1;
		}
	}
	mszUnsupportedFile = szName;
	miUnsupportedLine = iLine;
	if (szReason)
	{
		StrCpySafe(mszUnsupported, szReason, sizeof(mszUnsupported));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Why a function was left to the generator: " (CPP65IRFrontEndCalls.cpp
// line N)" and its reason (forgotten once told).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::AppendUnsupported(CChars* psz)
{
	psz->Append(" (");
	psz->Append(mszUnsupportedFile);
	psz->Append(" line ");
	psz->Append(miUnsupportedLine);
	psz->Append(")");
	if (mszUnsupported[0] != '\0')
	{
		psz->Append(": ");
		psz->Append(mszUnsupported);
		mszUnsupported[0] = '\0';
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The reason is forgotten (the function's own error says why).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::ClearUnsupported(void)
{
	mszUnsupported[0] = '\0';
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcFunction's IR is valid.  When it isn't, the verifier's errors
// and the caller's file and line are kept as the reason (SetUnsupported).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::VerifyIR(CIRFunction* pcFunction, const char* szFile, int iLine)
{
	CCPPErrors	cErrors;
	CIRVerifier	cVerifier;
	bool		bResult;

	cErrors.Init();
	cVerifier.Init(&mcModule, &cErrors);
	bResult = cVerifier.VerifyFunction(pcFunction);
	if (!bResult)
	{
		SetUnsupported(szFile, iLine, cErrors.Text());
	}
	cVerifier.Kill();
	cErrors.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The function type of pcFunction (what a pointer to it points to).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65IRFrontEnd::FunctionType(CCPPFunction* pcFunction)
{
	return mpcTypes->Function(pcFunction->mpsReturn, &pcFunction->mapsParameters, pcFunction->mbVariadic);
}


//////////////////////////////////////////////////////////////////////////
//
// How far into an object of pcDerived its base psBase is; 0 if psBase isn't
// a base of it (or either is NULL).
//
//////////////////////////////////////////////////////////////////////////
int CCPP65IRFrontEnd::BaseOffset(CCPPClass* pcDerived, CCPPClass* pcBase)
{
	int	iOffset;

	if (!pcDerived || !pcBase || (pcDerived == pcBase) || !mpcGenerator->CompleteClass(pcDerived, NULL) || !mpcGenerator->IsDerived(pcDerived, pcBase, &iOffset))
	{
		return 0;
	}
	return iOffset;
}
