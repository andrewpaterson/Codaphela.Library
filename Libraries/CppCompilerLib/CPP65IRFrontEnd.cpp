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
#include "CPPUnit.h"
#include "IRPrinter.h"
#include "IRVerifier.h"
#include "IROptimiser.h"
#include "IR65Backend.h"
#include "CPP65IRFrontEnd.h"
#include "CPPConstants.h"
#include "CPP65DebugWriter.h"
#include "CPPTypeWriter.h"


//The IR front end: a function's IR from its syntax tree (Body), written for the back end (FinishWriting); IR types,
//blocks, parameters, source lines and what isn't supported.  The rest is in CPP65IRFrontEndUnit, Statements, Values,
//Operators, Conversions, Calls and Objects.


//////////////////////////////////////////////////////////////////////////
//
// return &function; the body of a closure's conversion to a pointer to a
// function, whose address is its invoker's.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ReturnAddressOf(CCPPFunction* pcFunction)
{
	mpcBlock->Return(IRType(IRTK_Pointer), FunctionSymbol(pcFunction));
	return Terminated();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::Init(CCPPUnit* pcUnit, bool bConstants)
{
	mbConstants = bConstants;
	mpcConstant = NULL;
	muiAttempts = 0;
	mpcUnit = pcUnit;
	miDebugLabels = 0;
	mpcDiagnostics = pcUnit->GetDiagnostics();
	mpcTypes = pcUnit->GetTypes();
	mpcFunction = NULL;
	mpcBlock = NULL;
	mpcCPPFunction = NULL;
	msThis = IRNone();
	msResult = IRNone();
	msElide = IRNone();
	msVTT = IRNone();
	msVTTArgument = IRNone();
	mbBaseVersion = false;
	mpcBaseObject = NULL;
	mpcValue = NULL;
	mpcProfiledCaller = NULL;
	mbProfiledCaller = false;
	mcModule.Init();
	mcModule.AddDefaultSpace(pcUnit->GetTarget()->GetPointerBytes());
	mpcBackend = bConstants ? NULL : pcUnit->GetTarget()->NewBackend();
	mcPipeline.Init();
	pcUnit->GetTarget()->BuildPipeline(&mcPipeline);
	masWritten.Init();
	masJumps.Init();
	masCases.Init();
	masLabels.Init();
	masPendingGotos.Init();
	masGotoLocals.Init();
	mcCleanups.Init();
	masTries.Init();
	mpcThrowExit = NULL;
	mpcUnexpected = NULL;
	mpcTerminate = NULL;
	mbUnwinding = false;
	muiLandingTop = MAX_SIZE;
	mbConstructorHandler = false;
	mbListLocal = false;
	mbRoutine = false;
	miConditional = 0;
	mpcExit = NULL;
	mapcNone.Init();
	maszRuntime.Init();
	maszWeakRuntime.Init();
	maszUnitWeak.Init();
	maszMissing.Init();
	maszTested.Init();
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
	size				uj;
	SCPP65IRWritten*	psWritten;

	mauiStringLengths.Kill();
	mauiStringBytes.Kill();
	maszNotImported.Kill();
	maszTested.Kill();
	maszMissing.Kill();
	maszUnitWeak.Kill();
	maszWeakRuntime.Kill();
	maszRuntime.Kill();
	for (ui = 0; ui < masWritten.NumElements(); ui++)
	{
		psWritten = masWritten.Get(ui);
		psWritten->szLabel.Kill();
		psWritten->szName.Kill();
		for (uj = 0; uj < psWritten->asVariables.NumElements(); uj++)
		{
			psWritten->asVariables.Get(uj)->szName.Kill();
		}
		psWritten->asVariables.Kill();
		psWritten->szStatics.Kill();
		FreeWritten(psWritten);

		//Code not given to the unit's assembly (a unit generated again, or one that failed) leaves the store.
		if (!mbCodeGiven)
		{
			CCPP65Assembly::RemoveCode(mpcUnit->GetSpillStore(), mpcUnit->GetSpillUnit(), ui, 0, psWritten->uiPieces);
		}
	}
	masWritten.Kill();
	SafeFree(mpcBackend);
	mcModule.Kill();
	mapcNone.Kill();
	masTries.Kill();
	mcCleanups.Kill();
	masGotoLocals.Kill();
	masPendingGotos.Kill();
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
	bool	bResult;

	muiAttempts++;
	if (!mbConstants && (muiAttempts > mpcUnit->GetOptions()->muiFunctionLimit))
	{
		return false;
	}
	if (((pcFunction->meKind != CPPFK_Function) && (pcFunction->meKind != CPPFK_Constructor) && (pcFunction->meKind != CPPFK_Destructor)) ||
		!pcFunction->mpsReturn || (!IsSupportedType(pcFunction->mpsReturn) && !pcFunction->mpsReturn->IsClass()) || pcFunction->mpsReturn->IsArray() ||
		(pcFunction->mpsReturn->IsClass() && !mpcUnit->GetClasses()->CompleteClass(pcFunction->mpsReturn->pcClass, NULL)))
	{
		mbUnsupported = false;
		Unsupported(__FILE__, __LINE__);
		return false;
	}

	//A constructor or destructor of a class with virtual bases has a base version too, for a base subobject, which
	//leaves the virtual bases alone.
	mbBaseVersion = false;
	bResult = WriteBody(pcFunction);
	if (bResult && pcFunction->mpcClass && ((pcFunction->meKind == CPPFK_Constructor) || (pcFunction->meKind == CPPFK_Destructor)) && pcFunction->mpcClass->HasVirtualBases())
	{
		mbBaseVersion = true;
		bResult = WriteBody(pcFunction);
		mbBaseVersion = false;
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Writes the function's body, its base version if mbBaseVersion (Body).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::WriteBody(CCPPFunction* pcFunction)
{
	CChars					szLabel;
	CCPPSyntaxTryStatement*	pcTry;
	bool					bResult;
	bool					bCopy;

	szLabel.Init();
	mpcUnit->GetMangler()->AppendFunctionLabel(&szLabel, pcFunction, mbBaseVersion);
	BeginWriting(pcFunction, szLabel.Text(), ReturnIRType(pcFunction->mpsReturn), (pcFunction->mbVariadic ? IRA_Variadic : 0) | ((pcFunction->mpcClass && !pcFunction->mbStatic) ? IRA_ThisCall : 0) |
		(mpcUnit->GetAttributes()->HasAttribute(pcFunction->mpcDeclarator, "noinline") ? IRA_NoInline : 0) | (IsWeak(pcFunction) ? IRA_Weak : 0));
	if (mpcUnit->GetOptions()->mbExceptions && mpcUnit->GetExceptions()->IsNoexcept(pcFunction))
	{
		//No exception leaves it (WayOut): a caller's test of it folds away.
		mcModule.SetNoThrow(mpcFunction->muiSymbol, true);
	}

	//The generator's errors are not reported while the front end works (it reports them if it writes the function
	//itself).  What it fails to work out comes back as NULL, which the front end doesn't support; quiet errors from
	//its own probing (UsesFloat ...) are not failures.
	mpcDiagnostics->BeginQuiet();
	bCopy = pcFunction->mbImplicit && pcFunction->IsCopy();
	bResult = Parameters() && Locals();
	pcTry = (pcFunction->mpcDefinition && pcFunction->mpcDefinition->mpcBody && (pcFunction->mpcDefinition->mpcBody->GetKind() == CPPS_TryStatement)) ? (CCPPSyntaxTryStatement*)pcFunction->mpcDefinition->mpcBody : NULL;
	if (pcFunction->mpcCovariantOf)
	{
		bResult = bResult && CovariantThunk(pcFunction);
	}
	else if (pcTry)
	{
		bResult = bResult && FunctionTryBlock(pcFunction, pcTry);
	}
	else if (bResult && (pcFunction->meKind == CPPFK_Constructor) && !bCopy)
	{
		bResult = ConstructorStart(pcFunction->mpcDefinition);
	}
	else if (bResult && (pcFunction->meKind == CPPFK_Destructor))
	{
		//The vtables are this class's while its destructor runs; returns go to the end, which destroys the fields and
		//bases.
		mpcExit = NewBlock();
		bResult = StoreVtables(msThis, pcFunction->mpcClass, mbBaseVersion);
		DestructorStart();
	}
	if (pcTry || pcFunction->mpcCovariantOf)
	{
	}
	else if (pcFunction->mpcAddressOf)
	{
		//A closure's conversion to a pointer to a function.
		bResult = bResult && ReturnAddressOf(pcFunction->mpcAddressOf);
	}
	else if (pcFunction->mbDeleting)
	{
		bResult = bResult && DeletingDestructor(pcFunction->mpcClass);
	}
	else if (bCopy)
	{
		bResult = bResult && ImplicitCopy(pcFunction);
	}
	else
	{
		bResult = bResult && (!pcFunction->mpcDefinition || Statement(pcFunction->mpcDefinition->mpcBody)) && DestroyLiving(0);
	}
	if (bResult && mpcExit)
	{
		JumpTo(mpcExit);
		StartBlock(mpcExit);
		bResult = DestructorEnd();
		mpcBlock->Return();
	}
	mpcDiagnostics->EndQuiet();
	bResult = FinishWriting(bResult, szLabel.Text());
	szLabel.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A covariant thunk's body: its overrider called with its arguments (this
// and scalars), and the pointer it returns moved to the overridden
// function's result type's class; a null pointer stays null.
// [class.virtual] 7, 8
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::CovariantThunk(CCPPFunction* pcFunction)
{
	CArrayTemplate<SIRReference>	asValues;
	CCPPFunction*					pcOverrider;
	SCPP65Type*						psType;
	SIRReference					sAddress;
	SIRReference					sValue;
	SIRReference					sResult;
	SIRReference					sMoved;
	SIRType							sOffsetType;
	SIRType							sReturn;
	size							ui;
	int32							iIndex;

	pcOverrider = pcFunction->mpcCovariantOf;
	if (msThis.IsNone() || !msResult.IsNone() || pcFunction->mbVariadic)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	iIndex = 1;
	asValues.Init();
	for (ui = 0; ui < pcFunction->mapsParameters.NumElements(); ui++)
	{
		psType = pcFunction->mapsParameters.GetPtr(ui);
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
	mpcFunction->Argument(mpcBlock, IRType(IRTK_Pointer), msThis);
	for (ui = 0; ui < asValues.NumElements(); ui++)
	{
		mpcFunction->Argument(mpcBlock, IRTypeOf(pcFunction->mapsParameters.GetPtr(ui)), *asValues.Get(ui));
	}
	asValues.Kill();
	sReturn = ReturnIRType(pcFunction->mpsReturn);
	sResult = mpcFunction->Call(mpcBlock, sReturn, FunctionSymbol(pcOverrider), -1, true);
	TestThrown(pcOverrider, FunctionSymbol(pcOverrider));

	//p + (p != 0) * adjust: a reference is never null, a pointer may be.
	sOffsetType = IRIntegerType(mcModule.GetPointerSize(0));
	if (pcFunction->mpsReturn->IsPointer())
	{
		sMoved = mpcFunction->Add(mpcBlock, IRO_Ne, IRType(IRTK_Pointer), sResult, mpcFunction->AddConstant(IRType(IRTK_Pointer), 0));
		sMoved = mpcFunction->Add(mpcBlock, IRO_ZExt, sOffsetType, sMoved);
		sMoved = mpcFunction->Add(mpcBlock, IRO_Mul, sOffsetType, sMoved, mpcFunction->AddInteger(sOffsetType, pcFunction->miCovariantAdjust));
	}
	else
	{
		sMoved = mpcFunction->AddInteger(sOffsetType, pcFunction->miCovariantAdjust);
	}
	sResult = mpcFunction->Add(mpcBlock, IRO_PtrAdd, IRType(IRTK_Pointer), sResult, sMoved);
	mpcBlock->Return(sReturn, sResult);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The body of a copy constructor or a copy assignment that is defaulted
// after its class (S::S(const S&) = default;), which other units, and what
// this one wrote before, call: its parameter's object is copied to this as
// the compiler copies one of a class that declares neither (its bytes, the
// parts that have copy functions of their own by them, and for a constructor
// its vtable pointers), and an assignment gives *this.  One that takes
// S&& moves the parts that have a move of their own.  A constructor's
// base version, of a class with virtual bases, copies the class's own part
// and leaves the virtual bases to the complete object's, its vtable
// pointers from the VTT entries it is given.  [class.copy] 12, 15, 28
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ImplicitCopy(CCPPFunction* pcFunction)
{
	CCPPClass*		pcClass;
	SCPP65IRPlace	sThis;
	SCPP65IRPlace	sFrom;
	SIRReference	sAddress;
	bool			bMove;

	pcClass = pcFunction->mpcClass;
	bMove = pcFunction->mapsParameters.GetPtr(0)->bRvalue;

	//Its one parameter, a reference, is the function's last.
	sAddress = mpcFunction->Add(mpcBlock, IRO_ArgAddress, IRType(IRTK_Pointer), mpcFunction->AddInteger(IRType(IRTK_I32), (int64)mpcFunction->masParameters.NumElements() - 1));
	sFrom.sAddress = mpcFunction->Load(mpcBlock, IRType(IRTK_Pointer), sAddress);
	sFrom.psType = pcClass->mpsType;
	sThis.sAddress = msThis;
	sThis.psType = pcClass->mpsType;
	if ((pcFunction->meKind == CPPFK_Constructor) && mbBaseVersion)
	{
		CopyPieces(msThis, sFrom.sAddress, pcClass->miOwnBytes);
		return CopyParts(pcClass, msThis, sFrom.sAddress, bMove, msVTT) && StoreVtables(msThis, pcClass, true);
	}
	if (pcFunction->meKind == CPPFK_Constructor)
	{
		return CopyPlace(&sThis, &sFrom, &sAddress) && CopyParts(pcClass, msThis, sAddress, bMove, IRNone()) && StoreVtables(msThis, pcClass);
	}
	if (!AssignParts(pcClass, msThis, sFrom.sAddress, bMove, true))
	{
		return false;
	}
	mpcBlock->Return(IRType(IRTK_Pointer), msThis);
	return Terminated();
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
	masPendingGotos.ReInit();
	masGotoLocals.ReInit();
	mcCleanups.ReInit();
	masTries.ReInit();
	mpcThrowExit = NULL;
	mpcUnexpected = NULL;
	mpcTerminate = NULL;
	mbUnwinding = false;
	muiLandingTop = MAX_SIZE;
	mbConstructorHandler = false;
	mbListLocal = false;
	mbRoutine = false;
	miConditional = 0;
	maszRuntime.ReInit();
	maszWeakRuntime.ReInit();
	mpcExit = NULL;
	mauiStringBytes.ReInit();
	mauiStringLengths.ReInit();
	muiFirstString = mpcUnit->GetData()->NumStrings();
	mcAddresses.Init();
	mcImplicitValues.Init();
	mcAnonymousAddresses.Init();
	masDebugVariables.Init();
	miVariableSlots = 0;
	mpcBaseObject = NULL;
	msVTT = IRNone();
	msVTTArgument = IRNone();
	mpcFunction = mcModule.AddFunction(szLabel, sReturn, uiAttributes);
	if (pcFunction)
	{
		MarkFunctionSymbol(pcFunction, mpcFunction->muiSymbol);
	}
	msThis = IRNone();
	msClosure = IRNone();
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
	if (mpcDiagnostics->HasWarning(mpcCPPFunction->mpcDefinition, "Comparison is always"))
	{
		return;
	}
	pszWarnings->RemoveLastCharacter();
	szMessage.Init("In this function, once the values are followed: ");
	szMessage.Append(pszWarnings);
	mpcDiagnostics->Warning(mpcCPPFunction->mpcDefinition, szMessage.Text());
	szMessage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The function being written's name as C++ writes it (CShape::Area), for a
// debugger; its label for code the compiler made without a function (a
// routine: a global's R_Initialise, written for a stand-in function).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::FunctionName(CChars* pszDest, const char* szLabel)
{
	CCPPTypeWriter	cWriter;

	pszDest->Init();
	if (mpcCPPFunction && !mbRoutine)
	{
		cWriter.Init(mpcUnit, CPPTW_Shown);
		cWriter.AppendFunction(mpcCPPFunction, pszDest);
	}
	else
	{
		pszDest->Append(szLabel);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The debug type the C++ function being written returns, for a debugger
// to show what it returned (.dfunction): -1 for void, or code that isn't
// a C++ function's, or a unit without a line table.
//
//////////////////////////////////////////////////////////////////////////
int32 CCPP65IRFrontEnd::ResultTypeIndex(void)
{
	if (!mpcCPPFunction || mbRoutine || !mpcUnit->HasLineTable() || !mpcCPPFunction->mpsReturn || mpcCPPFunction->mpsReturn->IsVoid())
	{
		return -1;
	}
	return mpcUnit->GetDebugWriter()->TypeIndex(mpcCPPFunction->mpsReturn);
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
	SAsm65FunctionProfile*	psProfiled;
	size					ui;
	size					uiStart;
	size					uiLength;
	uint32					uiFingerprint;

	if (mbConstants)
	{
		return FinishConstant(bResult);
	}

	//Instructions the optimiser adds have no source of their own.
	mcModule.muiSource = 0;
	bResult = bResult && !mbUnsupported;
	uiFingerprint = 0;
	if (bResult)
	{
		//IR that doesn't verify is the front end's (or a pass's) mistake: the function is an error.
		Finish();
		if (mpcUnit->HasLineTable() || InliningProfile())
		{
			uiFingerprint = Fingerprint(mpcFunction);
		}
		NameVariableSlots();
		mcModule.mbDebugMarkers = mpcUnit->GetOptions()->mbDebugMarkers;
		if (mpcUnit->GetOptions()->mbOptimise)
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
		if (mpcUnit->GetOptions()->mbPrintIR)
		{
			fprintf(stderr, "Function %d\n", (int)muiAttempts);
			PrintIR(mpcFunction);
		}
	}

	if (bResult)
	{
		szCode.Init();
		mpcFunction->PlainKeptAccesses();
		mcModule.Begin(CPPCP_BackEnd);
		bResult = GenerateCode(mpcFunction, szLabel, &szCode, &pcWritten);
		mcModule.End(CPPCP_BackEnd);
		if (bResult)
		{
			//The code waits (with a marker where it goes) for FinishUnit, which inlines across the unit.
			psWritten = masWritten.Add();
			psWritten->pcFunction = mpcFunction;
			psWritten->szLabel.Init(szLabel);
			FunctionName(&psWritten->szName, szLabel);
			psWritten->iResult = ResultTypeIndex();
			psWritten->uiPieces = 0;
			CCPP65Assembly::PutCode(mpcUnit->GetSpillStore(), mpcUnit->GetSpillUnit(), masWritten.NumElements() - 1, szCode.Text(), szCode.Length(), &psWritten->uiPieces);
			psWritten->pcBackend = pcWritten;
			psWritten->uiFingerprint = uiFingerprint;
			RecordVariables(psWritten);
			mpcUnit->GetCode()->Append(IR65_FRONT_END_MARKER);
			mpcUnit->GetCode()->Append((int)(masWritten.NumElements() - 1));
			mpcUnit->GetCode()->Append(IR65_FRONT_END_MARKER);
			for (ui = 0; ui < maszRuntime.NumElements(); ui++)
			{
				mpcUnit->GetRuntimeCalls()->AddIfUnique(*maszRuntime.Get(ui));
			}
			for (ui = 0; ui < maszWeakRuntime.NumElements(); ui++)
			{
				maszUnitWeak.AddIfUnique(*maszWeakRuntime.Get(ui));
			}
			uiStart = 0;
			for (ui = 0; ui < mauiStringLengths.NumElements(); ui++)
			{
				uiLength = *mauiStringLengths.Get(ui);
				mpcUnit->GetData()->AddString((char*)mauiStringBytes.GetData() + uiStart, uiLength);
				uiStart += uiLength;
			}
		}
		szCode.Kill();
	}

	mbUnsupported = !bResult;
	EndWriting();
	if (bResult)
	{
		mcModule.Count(CPPCC_Functions);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The function being written is done with: one that failed is taken out
// of the module.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::EndWriting(void)
{
	if (mbUnsupported && mpcFunction)
	{
		mcModule.RemoveFunction(mpcFunction);
	}
	mcAddresses.Kill();
	mcImplicitValues.Kill();
	mcAnonymousAddresses.Kill();
	masDebugVariables.Kill();
	mpcFunction = NULL;
	mpcBlock = NULL;
	mpcCPPFunction = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Ends a function written for a constant (mbConstants): it is kept as it
// was written, with its slots still memory, which is what the interpreter
// runs; nothing is optimised or generated and the unit is given nothing.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::FinishConstant(bool bResult)
{
	mcModule.muiSource = 0;
	bResult = bResult && !mbUnsupported;
	if (bResult)
	{
		Finish();
		bResult = VerifyIR(mpcFunction, __FILE__, __LINE__);
	}
	mpcConstant = bResult ? mpcFunction : NULL;
	mbUnsupported = !bResult;
	EndWriting();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The IR of a function (set up as Body's is) for the interpreter, or NULL
// if the front end can't write it.
//
//////////////////////////////////////////////////////////////////////////
CIRFunction* CCPP65IRFrontEnd::ConstantFunction(CCPPFunction* pcFunction)
{
	return Body(pcFunction) ? mpcConstant : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// An expression as a function of no arguments that returns its value as a
// psType, for the interpreter; NULL if the front end can't write it.  It
// is written where it is met: what it names is what is in scope there.
//
//////////////////////////////////////////////////////////////////////////
CIRFunction* CCPP65IRFrontEnd::ConstantExpression(const char* szLabel, CCPPSyntax* pcExpression, SCPP65Type* psType)
{
	CCPPFunction	cFunction;
	SCPP65IRValue	sValue;
	SIRType			sType;
	bool			bResult;

	if (!IsSupportedType(psType) || !psType->IsScalar())
	{
		return NULL;
	}
	memset(&cFunction, 0, sizeof(CCPPFunction));
	cFunction.meKind = CPPFK_Function;
	cFunction.mpsReturn = psType;
	sType = ReturnIRType(psType);
	BeginWriting(&cFunction, szLabel, sType, IRA_Local);
	mpcDiagnostics->BeginQuiet();
	bResult = ValueFor(pcExpression, psType, &sValue) && ConvertImplicit(&sValue, psType, pcExpression) && DestroyTemporaries(0);
	mpcDiagnostics->EndQuiet();
	if (bResult)
	{
		mpcBlock->Return(sType, ConvertIR(sValue.sValue, IRTypeOf(psType), sType, psType->bSigned && (psType->eKind == CPP65TK_Integer)));
	}
	return FinishWriting(bResult, szLabel) ? mpcConstant : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A variable's initialiser as a function that makes its value where its
// one argument points, as its start up routine would at its symbol, for
// the interpreter; NULL if the front end can't write it.
//
//////////////////////////////////////////////////////////////////////////
CIRFunction* CCPP65IRFrontEnd::ConstantInitialiser(const char* szLabel, SCPP65Type* psType, CCPPSyntax* pcInitialiser)
{
	CCPPFunction	cFunction;
	SCPP65IRPlace	sPlace;
	bool			bResult;

	memset(&cFunction, 0, sizeof(CCPPFunction));
	cFunction.meKind = CPPFK_Function;
	cFunction.mpsReturn = &mpcTypes->msVoid;
	BeginWriting(&cFunction, szLabel, IRType(IRTK_None), IRA_Local);
	mbRoutine = true;
	sPlace.sAddress = mpcFunction->AddParameter(IRType(IRTK_Pointer));
	sPlace.psType = psType;
	mpcDiagnostics->BeginQuiet();
	bResult = Construct(&sPlace, NULL, pcInitialiser) && DestroyTemporaries(0);
	mpcDiagnostics->EndQuiet();
	if (bResult)
	{
		mpcBlock->Return();
	}
	return FinishWriting(bResult, szLabel) ? mpcConstant : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIRModule* CCPP65IRFrontEnd::GetModule(void)
{
	return &mcModule;
}


//////////////////////////////////////////////////////////////////////////
//
// A global's R_Initialise (constructs it from pcInitialiser) or, with bFinalise, its R_Finalise (destroys it), as a
// function of its own named szLabel that takes and returns nothing.  For a target the generator doesn't write code
// for.  False if the front end can't write it.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::GlobalRoutine(const char* szLabel, const char* szGlobal, SCPP65Type* psType, CCPPSyntax* pcInitialiser, bool bFinalise, const char* szExitNode, const char* szFinaliser, bool bThread, bool bLocal)
{
	CCPPFunction	cFunction;
	SCPP65IRPlace	sPlace;
	bool			bResult;

	memset(&cFunction, 0, sizeof(CCPPFunction));
	cFunction.meKind = CPPFK_Function;
	cFunction.mpsReturn = &mpcTypes->msVoid;
	BeginWriting(&cFunction, szLabel, IRType(IRTK_None), bLocal ? IRA_Local : 0);
	mbRoutine = true;
	mpcDiagnostics->BeginQuiet();
	sPlace.sAddress = mcModule.Symbol(szGlobal);
	sPlace.psType = psType;
	if (bFinalise)
	{
		bResult = Destroy(sPlace.sAddress, psType);
	}
	else
	{
		bResult = Construct(&sPlace, NULL, pcInitialiser) && DestroyTemporaries(0);
		if (bResult && szExitNode)
		{
			//Made: it is destroyed from an exit list.
			PushExitNode(szExitNode, szFinaliser, bThread);
		}
	}
	mpcDiagnostics->EndQuiet();
	return FinishWriting(bResult, szLabel);
}


//////////////////////////////////////////////////////////////////////////
//
// An object has been made: its node (szNode, two pointers: the next node,
// then the function) is pushed on __ExitList, or __ThreadExitList for a
// thread_local, with szFinaliser, which destroys it.  The lists are run,
// last pushed first, when the program ends (R_RunExitLists, or Libc65's
// __RunAtExit, whose atexit pushes on __ExitList too), so objects are
// destroyed in the reverse of the order they were made, with the atexit
// functions between them.  [basic.start.term] 1, 3
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::PushExitNode(const char* szNode, const char* szFinaliser, bool bThread)
{
	SIRReference	sHead;
	SIRReference	sNode;

	sHead = mcModule.Symbol(bThread ? "__ThreadExitList" : "__ExitList");
	sNode = mcModule.Symbol(szNode);
	mpcFunction->Store(mpcBlock, IRType(IRTK_Pointer), mpcFunction->Load(mpcBlock, IRType(IRTK_Pointer), sHead), sNode);
	mpcFunction->Store(mpcBlock, IRType(IRTK_Pointer), mcModule.Symbol(szFinaliser), mpcFunction->Add(mpcBlock, IRO_PtrAdd, IRType(IRTK_Pointer), sNode, Offset(mcModule.GetPointerSize(0))));
	mpcFunction->Store(mpcBlock, IRType(IRTK_Pointer), sNode, sHead);
	mpcUnit->GetGlobals()->UseExitLists();
}


//////////////////////////////////////////////////////////////////////////
//
// R_RunExitLists: the thread's exit list, then the program's, each node
// taken off before its function is called (which may push more).  The
// start up calls it when Libc65's __RunAtExit, which does the same, isn't
// linked.  [basic.start.term] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ExitListsRoutine(const char* szLabel)
{
	CCPPFunction	cFunction;
	SIRReference	sHead;
	SIRReference	sNode;
	SIRReference	sTest;
	CIRBlock*		pcLoop;
	CIRBlock*		pcCall;
	CIRBlock*		pcDone;
	int				i;

	memset(&cFunction, 0, sizeof(CCPPFunction));
	cFunction.meKind = CPPFK_Function;
	cFunction.mpsReturn = &mpcTypes->msVoid;
	BeginWriting(&cFunction, szLabel, IRType(IRTK_None), 0);
	mbRoutine = true;
	for (i = 0; i < 2; i++)
	{
		sHead = mcModule.Symbol((i == 0) ? "__ThreadExitList" : "__ExitList");
		pcLoop = NewBlock();
		pcCall = NewBlock();
		pcDone = NewBlock();
		JumpTo(pcLoop);
		StartBlock(pcLoop);
		sNode = mpcFunction->Load(mpcBlock, IRType(IRTK_Pointer), sHead);
		sTest = mpcFunction->Add(mpcBlock, IRO_Eq, IRType(IRTK_Pointer), sNode, mpcFunction->AddConstant(IRType(IRTK_Pointer), 0));
		mpcBlock->Branch(sTest, pcDone, pcCall);
		StartBlock(pcCall);
		mpcFunction->Store(mpcBlock, IRType(IRTK_Pointer), mpcFunction->Load(mpcBlock, IRType(IRTK_Pointer), sNode), sHead);
		mpcFunction->Call(mpcBlock, IRType(IRTK_None), mpcFunction->Load(mpcBlock, IRType(IRTK_Pointer), mpcFunction->Add(mpcBlock, IRO_PtrAdd, IRType(IRTK_Pointer), sNode, Offset(mcModule.GetPointerSize(0)))));
		JumpTo(pcLoop);
		StartBlock(pcDone);
	}
	return FinishWriting(true, szLabel);
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
	mpcUnit->GetStatistics()->Count(CPPCC_UnsupportedExits);
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
		return IRType(psType->IsMemberFunctionPointer() ? IRTK_I64 : IRTK_I32);
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

	if (!mpcUnit->HasNoFloats() || mbFloatReported || mbConstants)
	{
		return;
	}
	mbFloatReported = true;
	iToken = (mcModule.muiSource > 0) ? (int)mcModule.muiSource - 1 : -1;
	if ((iToken < 0) && mpcCPPFunction && mpcCPPFunction->mpcDefinition)
	{
		iToken = mpcCPPFunction->mpcDefinition->miFirstToken;
	}
	mpcDiagnostics->ReportErrorAt(iToken, "Floating point is used here, and this build has none (CCPP65Builder::SetNoFloats: the kernal has no floats)");
}


//////////////////////////////////////////////////////////////////////////
//
// On the 65C64 a result narrower than 4 bytes comes back extended to 32 bits
// in A, so the IR returns an i32.
//
//////////////////////////////////////////////////////////////////////////
SIRType CCPP65IRFrontEnd::ReturnIRType(SCPP65Type* psType)
{
	if (mpcUnit->GetTarget()->WidensResults() && psType->IsScalar() && !psType->IsPointer() && (psType->iBytes < 4))
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
		return mpcUnit->GetClasses()->CompleteClass(psElement->pcClass, NULL);
	}
	return IsSupportedType(psType);
}


//////////////////////////////////////////////////////////////////////////
//
// The address a reference of type psReference bound to psPlace holds: a
// derived class's object is referred to as its base, and an object of any
// other class as what its conversion function gives (ConvertedAddress).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ReferenceAddress(SCPP65IRPlace* psPlace, SCPP65Type* psReference, SIRReference* psAddress)
{
	SCPP65IRValue	sPointer;
	SCPP65IRValue	sValue;

	if (psPlace->psType->IsClass() && !(psReference->psBase->IsClass() && mpcUnit->GetClasses()->IsDerived(psPlace->psType->pcClass, psReference->psBase->pcClass, NULL)))
	{
		return ConvertedAddress(psPlace, psReference, psAddress);
	}
	if (psPlace->IsBitField())
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
// The place a reference of type psReference is bound to: the expression's,
// or for a reference to a function the overload of the name of the
// function's type.  [over.over] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ReferredPlace(CCPPSyntax* pcExpression, SCPP65Type* psReference, SCPP65IRPlace* psPlace)
{
	CCPPSyntaxIdExpression*	pcName;
	SCPP65IRValue			sAddress;

	pcName = psReference->psBase->IsFunction() ? mpcUnit->GetBinder()->FunctionNameOf(pcExpression) : NULL;
	if (!pcName || !pcName->mpcName->mpcSymbol)
	{
		return Place(pcExpression, psPlace);
	}
	if (!FunctionAddress(pcName, psReference->psBase, &sAddress))
	{
		return false;
	}
	psPlace->sAddress = sAddress.sValue;
	psPlace->psType = sAddress.psType->psBase;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The address a reference of type psReference holds, bound to an object
// (psPlace) of a class that isn't what it refers to: the place the class's
// conversion function to such a reference returns (operator T&).
// [dcl.init.ref] 5
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ConvertedAddress(SCPP65IRPlace* psPlace, SCPP65Type* psReference, SIRReference* psAddress)
{
	CCPPBoundConversion	cBound;
	SCPP65IRValue		sResult;
	SCPP65IRPlace		sConverted;

	mpcUnit->GetBinder()->BindObjectConversion(psPlace->psType->pcClass, psReference, false, NULL, &cBound);
	if ((cBound.meHow != CPPBV_Function) || !cBound.mpcFunction->mpsReturn->IsReference() || cBound.mpcFunction->mpsReturn->psBase->Equals(psPlace->psType))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (!Call(cBound.mpcFunction, FunctionSymbol(cBound.mpcFunction), NULL, &mapcNone, &sResult, NULL, false, cBound.mpcClass, true, psPlace->sAddress))
	{
		return false;
	}
	sConverted.sAddress = sResult.sValue;
	sConverted.psType = sResult.psType->psBase;
	return ReferenceAddress(&sConverted, psReference, psAddress);
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

	//A member function's this comes first (in X), then a base version's VTT entries or the address a function that
	//returns an object makes it at (the first argument on the stack).
	if (mpcFunction->muiAttributes & IRA_ThisCall)
	{
		msThis = mpcFunction->AddParameter(IRType(IRTK_Pointer));
	}
	if (mbBaseVersion)
	{
		msVTT = mpcFunction->AddParameter(IRType(IRTK_Pointer));
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
			if (!mpcUnit->GetClasses()->CompleteClass(psType->pcClass, NULL))
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
	for (ui = 0; ui < mpcUnit->GetLocals()->NumElements(); ui++)
	{
		pcVariable = mpcUnit->GetLocals()->GetPtr(ui);
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
			DebugParameter(pcVariable, *asByAddress.Get(uiParameter), true);
			continue;
		}
		sAddress = mpcFunction->Add(mpcBlock, IRO_ArgAddress, IRType(IRTK_Pointer), mpcFunction->AddInteger(IRType(IRTK_I32), *aiFirst.Get(uiParameter)));
		mcAddresses.Put((int64)(size)pcVariable, &sAddress);
		if (!DebugParameter(pcVariable, sAddress, pcVariable->mpsType->IsClass() || mpcCPPFunction->mbVariadic))
		{
			aiFirst.Kill();
			asByAddress.Kill();
			return Unsupported(__FILE__, __LINE__);
		}
		if (pcVariable->mpsType->IsClass())
		{
			sPlace.sAddress = sAddress;
			sPlace.psType = pcVariable->mpsType;
			AddLiving(&sPlace);
		}
	}
	aiFirst.Kill();
	asByAddress.Kill();
	ClosureThis();
	if (Debuggable() && !msThis.IsNone())
	{
		AddDebugVariable(NULL, VariableSlot(4, 4), 0, false);
		mpcFunction->Store(mpcBlock, IRType(IRTK_Pointer), msThis, masDebugVariables.Get(0)->sSlot);
	}
	else if (!msThis.IsNone())
	{
		//Optimised, this is only its value: a debug value marker says where it is (NameVariableSlots).
		AddDebugVariable(NULL, IRNone(), 0, false);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// In code for a debugger (Debuggable), a parameter is a kept slot of its
// own, its value copied in from where it was passed (sAddress, which the
// code then forgets): the place it was passed in may be shared once it is
// read.  An object, passed by address or in the bytes its caller pushed,
// and a variadic function's parameters (va_start finds the arguments
// after them from where they were passed) stay where they are
// (bByAddress): the kept slot holds the address.
//
// Built for speed, a parameter isn't copied: it is described where it was
// passed (sAddress, its argaddr), which is its own while the argaddr is
// (a back end shares no parameter's place in a function with one), and
// isn't kept once the optimiser makes the parameter a value.  An object
// passed by address has no place of its own.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::DebugParameter(CCPPVariable* pcVariable, SIRReference sAddress, bool bByAddress)
{
	SIRReference	sSlot;
	SIRType			sType;
	size			ui;
	bool			bPassedByAddress;

	if (!Debuggable())
	{
		//The address of an object passed by address is a parameter itself; any other is the parameter's argaddr.
		bPassedByAddress = false;
		for (ui = 0; ui < mpcFunction->masParameters.NumElements(); ui++)
		{
			bPassedByAddress |= mpcFunction->masParameters.Get(ui)->Equals(sAddress);
		}
		AddDebugVariable(pcVariable, bPassedByAddress ? IRNone() : sAddress, 0, false);
		return true;
	}
	if (bByAddress)
	{
		sSlot = VariableSlot(4, 4);
		mpcFunction->Store(mpcBlock, IRType(IRTK_Pointer), sAddress, sSlot);
		AddDebugVariable(pcVariable, sSlot, 0, true);
		return true;
	}
	if (!IsSupportedType(pcVariable->mpsType))
	{
		return false;
	}
	sType = IRTypeOf(pcVariable->mpsType);
	sSlot = VariableSlot((uint32)pcVariable->mpsType->iBytes, (pcVariable->mpsType->iBytes >= 4) ? 4 : (uint32)pcVariable->mpsType->iBytes);
	mpcFunction->Store(mpcBlock, sType, mpcFunction->Load(mpcBlock, sType, sAddress), sSlot);
	mcAddresses.Put((int64)(size)pcVariable, &sSlot);
	AddDebugVariable(pcVariable, sSlot, 0, false);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// In a lambda's operator(), this is the closure, which its captures are
// read from (msClosure); this in its body is the enclosing object's, kept
// in the closure if it captures it.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::ClosureThis(void)
{
	SCPPLambda*			psLambda;
	SCPPLambdaCapture*	psCapture;

	psLambda = mpcUnit->GetClosures()->BodyLambda(mpcCPPFunction);
	if (!psLambda || msThis.IsNone())
	{
		return;
	}
	msClosure = msThis;
	msThis = IRNone();
	psCapture = mpcUnit->GetClosures()->FindLambdaCapture(psLambda, NULL);
	if (psCapture)
	{
		msThis = mpcFunction->Load(mpcBlock, IRType(IRTK_Pointer), AddressAt(msClosure, psLambda->pcClass->macFields.Get(psCapture->uiField)->iOffset));
	}
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
	return psType->IsClass() && !psType->pcClass->IsTriviallyCopyable();
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
	mpcBackend->Init(&mcModule, mpcUnit->GetLabelCounter(), mpcUnit->GetBackendScratch());
	mpcBackend->SetOptions(mpcUnit->GetOptions());
	mpcBackend->SetSourceMarkers(mpcUnit->HasListing() || mpcUnit->HasLineTable());
	bResult = mpcBackend->GenerateFunction(pcFunction, szLabel, pszCode, &cErrors);
	if (bResult)
	{
		*ppcWritten = mpcBackend->TakeWritten();
		paszRuntime = mpcBackend->GetRuntime();
		for (ui = 0; ui < paszRuntime->NumElements(); ui++)
		{
			mpcUnit->GetRuntimeCalls()->AddIfUnique(paszRuntime->GetText(ui));
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
// The labels of the functions the unit calls by name and tests for an
// exception: none of them may be assembly, which leaves the flag the test
// reads as it falls (CCPP65Builder::CheckTested).
//
//////////////////////////////////////////////////////////////////////////
CArrayChars* CCPP65IRFrontEnd::GetTestedCallees(void)
{
	return &maszTested;
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
// The back ends write a place marker (IR_PLACE_MARKER) where a variable's
// place changes (CIRBackend::DebugPlaces).  With bRanges each becomes a
// label (IR_DEBUG_LABEL and a number, one for the markers before a line),
// and paszRanges gets per variable slot the .drange lines its places make:
// from the label where it got a place to the label where it lost it.
// Otherwise they go.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::DebugRanges(CChars* pszCode, bool bRanges, CArrayChars* paszRanges)
{
	CChars			szOut;
	CArrayChars		aszPlaces;		//Per slot: its place now, empty for none...
	CArrayChars		aszFrom;		//...and the label it is there from.
	CChars			szLabel;
	const char*		szText;
	const char*		szEnd;
	const char*		szPlace;
	size			uiMarker;
	size			uiSlot;

	if (!pszCode->Contains(IR_PLACE_MARKER))
	{
		return;
	}

	uiMarker = strlen(IR_PLACE_MARKER);
	szOut.Init();
	aszPlaces.Init();
	aszFrom.Init();
	szLabel.Init();
	for (szText = pszCode->Text(); *szText != '\0'; szText = (*szEnd == '\n') ? szEnd + 1 : szEnd)
	{
		szEnd = strchr(szText, '\n');
		if (!szEnd)
		{
			szEnd = szText + strlen(szText);
		}
		if (strncmp(szText, IR_PLACE_MARKER, uiMarker) != 0)
		{
			szOut.AppendSubString(szText, (*szEnd == '\n') ? szEnd + 1 : szEnd);
			szLabel.Clear();
			continue;
		}
		if (!bRanges)
		{
			continue;
		}

		uiSlot = (size)strtol(szText + uiMarker, (char**)&szPlace, 10);
		szPlace++;
		while (aszPlaces.NumElements() <= uiSlot)
		{
			aszPlaces.Add("");
			aszFrom.Add("");
			paszRanges->Add("");
		}
		if (aszPlaces.Get(uiSlot)->Equals(szPlace, (size)(szEnd - szPlace)) || (aszPlaces.Get(uiSlot)->Empty() && (strncmp(szPlace, "none", 4) == 0)))
		{
			continue;
		}

		//The markers before one line share its label.
		if (szLabel.Empty())
		{
			DebugLabel(&szLabel, &szOut);
		}
		if (!aszPlaces.Get(uiSlot)->Empty())
		{
			paszRanges->Get(uiSlot)->Append("\t\t.drange ")->Append(*aszFrom.Get(uiSlot))->Append(", ")->Append(szLabel)->Append(", ")->Append(*aszPlaces.Get(uiSlot))->AppendNewLine();
		}
		if (strncmp(szPlace, "none", 4) == 0)
		{
			aszPlaces.Get(uiSlot)->Clear();
		}
		else
		{
			aszPlaces.Get(uiSlot)->Clear();
			aszPlaces.Get(uiSlot)->AppendSubString(szPlace, szEnd);
			aszFrom.Get(uiSlot)->Set(szLabel.Text());
		}
	}
	pszCode->Set(szOut.Text());
	szLabel.Kill();
	aszFrom.Kill();
	aszPlaces.Kill();
	szOut.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The back ends (and CIRPrinter) write a marker line (IR_SOURCE_MARKER and
// the instructions' location: the source token + 1, or an inline location
// of pcFunction, CIRFunction::SourceToken) where the instructions' source
// changes.  With bInlines (the function is described for a debugger) the
// code of the calls inlined into it is told too (InlineSpans,
// DescribeInlines).  Where it
// is another line than the last's, with bComments (a listing) it becomes a
// comment with the file, line and text of the line, and with bLines a
// .line directive with the file's full name (the linker's line table);
// otherwise they go.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::SourceComments(CChars* pszCode, CIRFunction* pcFunction, bool bComments, bool bLines, bool bInlines)
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
	uint32				uiSource;
	SCPP65IRInlineSpans	sSpans;

	bInlines = bInlines && pcFunction && (pcFunction->masInlines.NumElements() > 0);
	if (!bInlines && !pszCode->Contains(IR_SOURCE_MARKER))
	{
		return;
	}
	if (bInlines)
	{
		sSpans.aiOpen.Init();
		sSpans.aszFrom.Init();
		sSpans.aszSpans.Init();
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
		uiSource = (uint32)strtoul(szText + uiMarker, NULL, 10);
		if (bInlines)
		{
			InlineSpans(&sSpans, pcFunction, uiSource, &szOut);
		}
		uiSource = pcFunction ? pcFunction->SourceToken(uiSource) : uiSource;
		if ((!bComments && !bLines) || (uiSource == 0) || (uiSource & IR_INLINE_LOCATION) || !SourceLine((int)uiSource - 1, &pcFile, &iLine))
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
			szOut.Append("\t\t.line ");
			CCPP65DebugWriter::AppendString(&szOut, pcFile->FullName());
			szOut.Append(", ");
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
	if (bInlines)
	{
		DescribeInlines(&sSpans, pcFunction, &szOut);
		sSpans.aszSpans.Kill();
		sSpans.aszFrom.Kill();
		sSpans.aiOpen.Kill();
	}
	pszCode->Kill();
	pszCode->Init(&szOut);
	szOut.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A source marker at uiLocation: where the inlines the code is in change,
// a label (DebugLabel) ends the spans of those it left and starts those
// of the ones it entered.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::InlineSpans(SCPP65IRInlineSpans* psSpans, CIRFunction* pcFunction, uint32 uiLocation, CChars* pszOut)
{
	CArrayInt32			aiChain;		//The inlines at uiLocation, innermost first.
	SIRInlineLocation*	psLocation;
	CChars				szLabel;
	size				uiCommon;
	size				ui;
	int32				iInline;

	aiChain.Init();
	for (psLocation = pcFunction->GetInlineLocation(uiLocation); psLocation && (aiChain.NumElements() <= pcFunction->masInlines.NumElements()); psLocation = pcFunction->GetInlineLocation(pcFunction->masInlines.Get(psLocation->uiInline)->uiCall))
	{
		iInline = (int32)psLocation->uiInline;
		aiChain.Add(iInline);
	}
	for (uiCommon = 0; (uiCommon < psSpans->aiOpen.NumElements()) && (uiCommon < aiChain.NumElements()); uiCommon++)
	{
		if (psSpans->aiOpen.GetValue(uiCommon) != aiChain.GetValue(aiChain.NumElements() - 1 - uiCommon))
		{
			break;
		}
	}
	if ((uiCommon == psSpans->aiOpen.NumElements()) && (uiCommon == aiChain.NumElements()))
	{
		aiChain.Kill();
		return;
	}

	szLabel.Init();
	DebugLabel(&szLabel, pszOut);
	while (psSpans->aszSpans.NumElements() < pcFunction->masInlines.NumElements())
	{
		psSpans->aszSpans.Add("");
	}
	for (ui = psSpans->aiOpen.NumElements(); ui > uiCommon; ui--)
	{
		psSpans->aszSpans.Get((size)psSpans->aiOpen.GetValue(ui - 1))->Append("\t\t.dinspan ")->Append(psSpans->aszFrom.GetText(ui - 1))->Append(", ")->Append(szLabel)->AppendNewLine();
		psSpans->aiOpen.RemoveTail();
		psSpans->aszFrom.RemoveTail();
	}
	for (ui = uiCommon; ui < aiChain.NumElements(); ui++)
	{
		psSpans->aiOpen.Add(aiChain.GetValue(aiChain.NumElements() - 1 - ui));
		psSpans->aszFrom.Add(szLabel.Text());
	}
	szLabel.Kill();
	aiChain.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// After the function's code: the inlines still open end, then a .dinline
// for each of the function's inlines in order (their variables name them
// by index), each with its spans: its callee's C++ name (the function the
// unit wrote; another unit's by its label) and its call's file and line.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::DescribeInlines(SCPP65IRInlineSpans* psSpans, CIRFunction* pcFunction, CChars* pszOut)
{
	SIRInline*			psInline;
	SIRInlineLocation*	psCall;
	SCPP65IRWritten*	psCallee;
	CSourceFile*		pcFile;
	const char*			szName;
	size				ui;
	int					iLine;
	uint32				uiToken;

	InlineSpans(psSpans, pcFunction, 0, pszOut);
	for (ui = 0; ui < pcFunction->masInlines.NumElements(); ui++)
	{
		psInline = pcFunction->masInlines.Get(ui);
		psCall = pcFunction->GetInlineLocation(psInline->uiCall);
		szName = mcModule.GetSymbolName(psInline->uiCallee);
		psCallee = FindWritten(szName);
		szName = psCallee ? psCallee->szName.Text() : szName;
		pszOut->Append("\t\t.dinline ")->Append(psCall ? (int32)psCall->uiInline : -1)->Append(", ");
		CCPP65DebugWriter::AppendString(pszOut, szName);
		pszOut->Append(", ");
		iLine = -1;
		uiToken = pcFunction->SourceToken(psInline->uiCall);
		if ((uiToken != 0) && SourceLine((int)uiToken - 1, &pcFile, &iLine))
		{
			CCPP65DebugWriter::AppendString(pszOut, pcFile->FullName());
		}
		else
		{
			CCPP65DebugWriter::AppendString(pszOut, "");
		}
		pszOut->Append(", ")->Append(iLine + 1)->AppendNewLine();
		if (ui < psSpans->aszSpans.NumElements())
		{
			pszOut->Append(*psSpans->aszSpans.Get(ui));
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A new debug label (IR_DEBUG_LABEL and the unit's next number) into
// pszLabel, and its line into pszOut.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::DebugLabel(CChars* pszLabel, CChars* pszOut)
{
	pszLabel->Set(IR_DEBUG_LABEL);
	pszLabel->Append(miDebugLabels);
	miDebugLabels++;
	pszOut->Append(*pszLabel)->Append(':')->AppendNewLine();
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

	if ((iToken < 0) || ((size)iToken >= mpcUnit->GetTokeniser()->NumTokens()))
	{
		return false;
	}
	psSource = mpcUnit->GetTokeniser()->GetSource(mpcUnit->GetTokeniser()->Get(iToken));
	if (!psSource)
	{
		return false;
	}
	*ppcFile = psSource->pcFile;
	*piLine = psSource->iLine;
	pcSource = mpcUnit->GetTokeniser()->mpcSource;
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
	int				iAlignas;
	int				iFrameAlign;
	SIRType			sOffsetType;
	SIRReference	sPadding;

	mcAnonymousAddresses.Kill();
	mcAnonymousAddresses.Init();
	for (ui = 0; ui < mpcUnit->GetLocals()->NumElements(); ui++)
	{
		pcVariable = mpcUnit->GetLocals()->GetPtr(ui);
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
				sAddress = VariableSlot((uint32)pcVariable->mpcAnonymous->mpsType->iBytes, 4);
				mcAnonymousAddresses.Put((int64)(size)pcVariable->mpcAnonymous, &sAddress);
				psAnonymousAddress = mcAnonymousAddresses.Get((int64)(size)pcVariable->mpcAnonymous);
			}
			AddDebugVariable(pcVariable, *psAnonymousAddress, pcVariable->miOffset, false);
			sAddress = AddressAt(*psAnonymousAddress, pcVariable->miOffset);
			mcAddresses.Put((int64)(size)pcVariable, &sAddress);
			continue;
		}
		iAlign = ((pcVariable->mpsType->iBytes >= 4) || pcVariable->mpsType->IsArray() || pcVariable->mpsType->IsClass()) ? 4 : pcVariable->mpsType->iBytes;
		mpcUnit->GetClasses()->VariableAlignment(pcVariable, &iAlignas);
		iFrameAlign = mpcUnit->GetTarget()->GetFrameAlignment();
		if (iAlignas > iFrameAlign)
		{
			//alignas asks more than a place in the frame is: a bigger slot, and the first place in it that is a multiple
			//of the alignment.  [dcl.align] 1
			//Where in it is only known as the code runs: no place a debugger can read.
			AddDebugVariable(pcVariable, IRNone(), 0, false);
			sAddress = VariableSlot((uint32)(pcVariable->mpsType->iBytes + iAlignas - iFrameAlign), (uint32)iAlign);
			sOffsetType = IRIntegerType(mcModule.GetPointerSize(0));
			sPadding = mpcFunction->Add(mpcBlock, IRO_Neg, sOffsetType, mpcFunction->Add(mpcBlock, IRO_PtrToI, sOffsetType, sAddress));
			sPadding = mpcFunction->Add(mpcBlock, IRO_And, sOffsetType, sPadding, mpcFunction->AddInteger(sOffsetType, iAlignas - 1));
			sAddress = mpcFunction->Add(mpcBlock, IRO_PtrAdd, IRType(IRTK_Pointer), sAddress, sPadding);
		}
		else
		{
			sAddress = VariableSlot((uint32)pcVariable->mpsType->iBytes, (uint32)iAlign);
			AddDebugVariable(pcVariable, sAddress, 0, false);
		}
		mcAddresses.Put((int64)(size)pcVariable, &sAddress);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the code is for a debugger (CCPPCompileOptions::mbDebuggable).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Debuggable(void)
{
	return mpcUnit->GetOptions()->mbDebuggable;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a debugger is told of the function's variables (.dvariable):
// when the unit has a line table, Debug or Release (Debugger65C64Plan.txt
// A3).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::DescribesVariables(void)
{
	return mpcUnit->HasLineTable();
}


//////////////////////////////////////////////////////////////////////////
//
// A local's slot.  In code for a debugger it is kept (CIRFunction::Slot):
// never promoted or split.  In Release the optimiser may promote or split
// it, and the debugger says the variable isn't kept.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CCPP65IRFrontEnd::VariableSlot(uint32 uiSize, uint32 uiAlign)
{
	return mpcFunction->Slot(mpcBlock, IRType(IRTK_Pointer), uiSize, uiAlign, NULL, Debuggable());
}


//////////////////////////////////////////////////////////////////////////
//
// The slot of each variable a debugger is told of named dbg<n> (its
// iSlot), which the back end says where it put (CIRBackend::
// VariableSlotName) while it is still the variable's alone.  Optimised
// code's this has no slot: its iSlot is the next, and a debug value marker
// at the function's start says it is msThis (DebuggerFPlan.txt F1).  Named
// after the function's fingerprint is taken, so the IR a profile matches
// is the same with a line table and without.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::NameVariableSlots(void)
{
	SCPP65IRDebugVariable*	psVariable;
	SCPP65IRDebugVariable*	psEarlier;
	char					szName[32];
	size					ui;
	size					uj;

	for (ui = 0; ui < masDebugVariables.NumElements(); ui++)
	{
		psVariable = masDebugVariables.Get(ui);
		if (psVariable->sSlot.IsNone())
		{
			if (!psVariable->pcVariable && !msThis.IsNone() && mpcUnit->GetOptions()->mbDebugMarkers && (mpcFunction->NumBlocks() > 0))
			{
				psVariable->iSlot = miVariableSlots;
				miVariableSlots++;
				mpcFunction->GetBlock(0)->AddDebugValue(0, (uint32)psVariable->iSlot, msThis);
			}
			continue;
		}

		//The members of an anonymous union share its slot.
		for (uj = 0; uj < ui; uj++)
		{
			psEarlier = masDebugVariables.Get(uj);
			if (psEarlier->sSlot.Equals(psVariable->sSlot))
			{
				psVariable->iSlot = psEarlier->iSlot;
				break;
			}
		}
		if (uj == ui)
		{
			psVariable->iSlot = miVariableSlots;
			snprintf(szName, sizeof(szName), "dbg%d", (int)miVariableSlots);
			miVariableSlots++;
			mpcFunction->NameTemporary(psVariable->sSlot, szName);
		}
	}
	mpcFunction->muiDebugSlots = (uint32)miVariableSlots;
}


//////////////////////////////////////////////////////////////////////////
//
// A variable a debugger is told of, in code for one.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::AddDebugVariable(CCPPVariable* pcVariable, SIRReference sSlot, int32 iOffset, bool bIndirect)
{
	SCPP65IRDebugVariable	sVariable;

	if (!DescribesVariables())
	{
		return;
	}
	sVariable.pcVariable = pcVariable;
	sVariable.sSlot = sSlot;
	sVariable.iSlot = -1;
	sVariable.iOffset = iOffset;
	sVariable.bIndirect = bIndirect;

	//this (found after the parameters) first, as a debugger shows it.
	if (pcVariable)
	{
		masDebugVariables.Add(&sVariable);
	}
	else
	{
		masDebugVariables.InsertAt(&sVariable, 0);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What a debugger is told of the function written (DescribesVariables):
// each of its variables (this first), its type and the lines it is in
// scope from and to, kept with it (psWritten) to describe once the unit's
// code is finished (DescribeVariables); and its static locals, at their
// labels.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::RecordVariables(SCPP65IRWritten* psWritten)
{
	SCPP65IRDebugVariable*		psVariable;
	SCPP65IRDescribedVariable*	psDescribed;
	CCPPVariable*				pcStatic;
	CCPPTypeWriter				cWriter;
	CChars						szLabel;
	size						ui;

	psWritten->asVariables.Init();
	psWritten->szStatics.Init();
	if (!DescribesVariables())
	{
		return;
	}
	cWriter.Init(mpcUnit, CPPTW_Shown);
	for (ui = 0; ui < masDebugVariables.NumElements(); ui++)
	{
		psVariable = masDebugVariables.Get(ui);
		psDescribed = psWritten->asVariables.Add();
		psDescribed->szName.Init();
		if (psVariable->pcVariable)
		{
			cWriter.AppendIdentifier(psVariable->pcVariable->mpcSymbol->mlliName, &psDescribed->szName);
			psDescribed->psType = psVariable->pcVariable->mpsType;
		}
		else
		{
			psDescribed->szName.Append("this");
			psDescribed->psType = mpcTypes->Pointer(mpcCPPFunction->mpcClass->mpsType);
		}
		psDescribed->iSlot = psVariable->iSlot;
		psDescribed->iOffset = psVariable->iOffset;
		psDescribed->bIndirect = psVariable->bIndirect;
		psDescribed->bParameter = !psVariable->pcVariable || (psVariable->pcVariable->meStorage == CPPVS_Parameter);
		VariableLines(psVariable, &psDescribed->iFirstLine, &psDescribed->iLastLine);
	}

	for (ui = 0; ui < mpcUnit->GetLocals()->NumElements(); ui++)
	{
		pcStatic = mpcUnit->GetLocals()->GetPtr(ui);
		if (pcStatic->meStorage == CPPVS_Static)
		{
			szLabel.Init();
			mpcUnit->GetMangler()->AppendVariableLabel(&szLabel, pcStatic);
			mpcUnit->GetDebugWriter()->AppendGlobal(&psWritten->szStatics, pcStatic, szLabel.Text(), true);
			szLabel.Kill();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A copy of the written function (CIRInterprocedural's, for a constant)
// has its variables, in the slots it copied with their names.  Its static
// locals are psWritten's, described there.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::CopyVariables(SCPP65IRWritten* psCopy, SCPP65IRWritten* psWritten)
{
	SCPP65IRDescribedVariable*	psVariable;
	size						ui;

	psCopy->asVariables.Init();
	psCopy->szStatics.Init();
	for (ui = 0; ui < psWritten->asVariables.NumElements(); ui++)
	{
		psVariable = psCopy->asVariables.Add(psWritten->asVariables.Get(ui));
		psVariable->szName.Init(psWritten->asVariables.Get(ui)->szName);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// .dvariable for each of the written function's variables (RecordVariables)
// and for those of the calls inlined into it (DebuggerFPlan.txt F2-2: the
// callee's variables, of the inline), then its static locals.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::DescribeVariables(CChars* pszCode, SCPP65IRWritten* psWritten, CArrayChars* paszRanges)
{
	CIRFunction*				pcFunction;
	SIRInlineVariable*			psInlineVariable;
	SCPP65IRWritten*			psCallee;
	SCPP65IRDescribedVariable*	psVariable;
	size						ui;
	size						uj;
	size						uiInline;

	for (ui = 0; ui < psWritten->asVariables.NumElements(); ui++)
	{
		psVariable = psWritten->asVariables.Get(ui);
		DescribeVariable(pszCode, psWritten, psVariable, psVariable->iSlot, -1, paszRanges);
	}
	//Each inline's in its callee's order.
	pcFunction = psWritten->pcFunction;
	for (uiInline = 0; pcFunction && (uiInline < pcFunction->masInlines.NumElements()); uiInline++)
	{
		psCallee = FindWritten(mcModule.GetSymbolName(pcFunction->masInlines.Get(uiInline)->uiCallee));
		for (uj = 0; psCallee && (uj < psCallee->asVariables.NumElements()); uj++)
		{
			psVariable = psCallee->asVariables.Get(uj);
			for (ui = 0; ui < pcFunction->masInlineVariables.NumElements(); ui++)
			{
				psInlineVariable = pcFunction->masInlineVariables.Get(ui);
				if ((psInlineVariable->uiInline == uiInline) && (psVariable->iSlot == (int32)psInlineVariable->uiCalleeSlot))
				{
					DescribeVariable(pszCode, psWritten, psVariable, (int32)psInlineVariable->uiSlot, (int32)uiInline, paszRanges);
				}
			}
		}
	}
	pszCode->Append(psWritten->szStatics);
}


//////////////////////////////////////////////////////////////////////////
//
// .dvariable for psVariable, its slot the written function's debug slot
// iSlot (of its inline iInline, -1 for the function's own): where the back
// end last put the slot (psWritten's pcBackend); or, for a slot promoted,
// the places its values were in (paszRanges, per slot, DebugRanges):
// ranges, its .drange lines after it.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::DescribeVariable(CChars* pszCode, SCPP65IRWritten* psWritten, SCPP65IRDescribedVariable* psVariable, int32 iSlot, int32 iInline, CArrayChars* paszRanges)
{
	char		szSlot[32];
	const char*	szWhere;
	CChars*		pszRanges;
	int32		iPlace;

	snprintf(szSlot, sizeof(szSlot), "dbg%d", (int)iSlot);
	pszRanges = ((iSlot >= 0) && ((size)iSlot < paszRanges->NumElements())) ? paszRanges->Get((size)iSlot) : NULL;
	iPlace = 0;
	if ((iSlot >= 0) && psWritten->pcBackend && psWritten->pcBackend->FindSlot(szSlot, &iPlace))
	{
		szWhere = psVariable->bIndirect ? "indirect" : "frame";
		iPlace += psVariable->iOffset;
	}
	else if (pszRanges && !pszRanges->Empty() && !psVariable->bIndirect && (psVariable->iOffset == 0))
	{
		szWhere = "ranges";
	}
	else
	{
		szWhere = "none";
		pszRanges = NULL;
	}
	mpcUnit->GetDebugWriter()->AppendVariable(pszCode, psVariable->szName.Text(), psVariable->psType, psVariable->bParameter, szWhere, iPlace, psVariable->iFirstLine, psVariable->iLastLine, iInline);
	if (pszRanges)
	{
		pszCode->Append(*pszRanges);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The unit's written function with the label szLabel, NULL if none (a
// function of another unit).
//
//////////////////////////////////////////////////////////////////////////
SCPP65IRWritten* CCPP65IRFrontEnd::FindWritten(const char* szLabel)
{
	SCPP65IRWritten*	psWritten;
	size				ui;

	for (ui = 0; ui < masWritten.NumElements(); ui++)
	{
		psWritten = masWritten.Get(ui);
		if (psWritten->szLabel.Equals(szLabel))
		{
			return psWritten;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The lines a variable is in scope from and to: a parameter (and this)
// the function's; a local from its declaration to the end of the block,
// or the statement (a for's), that it is declared in.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::VariableLines(SCPP65IRDebugVariable* psVariable, int* piFirst, int* piLast)
{
	CCPPSyntax*		pcNode;
	ECPPSyntax		eKind;

	pcNode = mpcCPPFunction ? (CCPPSyntax*)mpcCPPFunction->mpcDefinition : NULL;
	if (psVariable->pcVariable && (psVariable->pcVariable->meStorage != CPPVS_Parameter) && psVariable->pcVariable->mpcSymbol->mpcDeclaration)
	{
		*piFirst = TokenLine(psVariable->pcVariable->mpcSymbol->mpcDeclaration->miFirstToken);
		for (pcNode = psVariable->pcVariable->mpcSymbol->mpcDeclaration->mpcParent; pcNode; pcNode = pcNode->mpcParent)
		{
			eKind = pcNode->GetKind();
			if ((eKind == CPPS_CompoundStatement) || (eKind == CPPS_ForStatement) || (eKind == CPPS_RangeForStatement) || (eKind == CPPS_IfStatement) ||
				(eKind == CPPS_SwitchStatement) || (eKind == CPPS_WhileStatement) || (eKind == CPPS_CatchHandler) || (eKind == CPPS_FunctionDefinition))
			{
				break;
			}
		}
		*piLast = pcNode ? TokenLine(pcNode->miLastToken) : *piFirst;
	}
	else
	{
		*piFirst = pcNode ? TokenLine(pcNode->miFirstToken) : 0;
		*piLast = pcNode ? TokenLine(pcNode->miLastToken) : 0;
	}
	if (*piLast < *piFirst)
	{
		*piLast = *piFirst;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The source line (1 based) a token is on, where a macro was used for one
// it made; 0 if it has none.
//
//////////////////////////////////////////////////////////////////////////
int CCPP65IRFrontEnd::TokenLine(int iToken)
{
	CSourceFile*	pcFile;
	int				iLine;

	return SourceLine(iToken, &pcFile, &iLine) ? iLine + 1 : 0;
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
	SourceComments(&szText, pcFunction, true, false);
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
	mpcUnit->GetTarget()->ConfigureOptimiser(pcOptimiser);
	pcOptimiser->SetLibraryInt(IRTypeOf(&mpcTypes->msInt));
	pcOptimiser->SetLibrarySize(IRTypeOf(mpcTypes->Size()));
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
// it with the function's error (CCPPUnit::AppendUnsupported).
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
// How far into an object of pcDerived its base psBase is; 0 if psBase isn't
// a base of it (or either is NULL), CPP_VIRTUAL_PATH if it is through a
// virtual base (BaseAddress finds it).
//
//////////////////////////////////////////////////////////////////////////
int CCPP65IRFrontEnd::BaseOffset(CCPPClass* pcDerived, CCPPClass* pcBase)
{
	int	iOffset;

	if (!pcDerived || !pcBase || (pcDerived == pcBase) || !mpcUnit->GetClasses()->CompleteClass(pcDerived, NULL) || !mpcUnit->GetClasses()->IsDerived(pcDerived, pcBase, &iOffset))
	{
		return 0;
	}
	return iOffset;
}
