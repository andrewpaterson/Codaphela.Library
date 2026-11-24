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
#ifndef __C_P_P_65_I_R_FRONT_END_H__
#define __C_P_P_65_I_R_FRONT_END_H__


//Libc65's routines that throw what the language does (exception.cpp), for a program built with exceptions.
#define CPP65_THROW_BAD_CAST_LABEL		"___ThrowBadCast"
#define CPP65_THROW_BAD_TYPEID_LABEL	"___ThrowBadTypeid"
#define CPP65_THROW_BAD_LENGTH			"___ThrowBadArrayNewLength"
#include "BaseLib/MapLongTemplate.h"
#include "BaseLib/HashMapPrimitiveTemplate.h"
#include "BaseLib/ArrayUint8.h"
#include "IRModule.h"
#include "IRBackend.h"
#include "IRInliner.h"
#include "IRInterprocedural.h"
#include "IRPipeline.h"
#include "CPPSyntaxes.h"
#include "CPP65Type.h"
#include "CPP65IRCleanups.h"
#include "CPPFunction.h"
#include "CPP65DebugWriter.h"


class CSourceFile;
class CIROptimiser;


class CCPPUnit;
class CCPPDiagnostics;
class CCPPBoundCall;
class CCPPBoundName;
class CCPPBoundConversion;
class CCPPFunction;
class CCPPVariable;
class CCPPExceptionSpec;
struct SCPPVtableEntry;


//A C++ value: an IR value and its C++ type (never a reference).
struct SCPP65IRValue
{
	SIRReference	sValue;
	SCPP65Type*		psType;
};


//(SCPP65IRPlace, where a C++ lvalue is, is CPP65IRCleanups.h's.)


//A try block being written: how many entries the cleanup stack had when it began (an exception from the block undoes
//those above), and the block that tries its handlers.
struct SCPP65IRTry
{
	size		uiBase;
	CIRBlock*	pcDispatch;
};


//A temporary that has its place (CCPP65IRFrontEnd::Temporary) and is still to be made: it is registered for destroying
//when its construction has finished (TemporaryMade), never before, so that what leaves in the middle of making it
//doesn't destroy it, and so that temporaries are destroyed in the reverse of the order they were finished in
//([class.temporary] 3).  bDestroy: it has a destructor to run.  One made in an operand that may not be worked out has a
//flag (sFlag), set when it is made.
struct SCPP65IRMade
{
	SCPP65IRPlace	sPlace;
	bool			bDestroy;
	bool			bConditional;
	SIRReference	sFlag;
};


//Where break and continue go (NULL for continue in a switch), and how many objects were living there
//(CCPP65IRCleanups::NumLocals): those after them are destroyed on the way.
struct SCPP65IRJump
{
	CIRBlock*	pcBreak;
	CIRBlock*	pcContinue;
	size		uiLiving;			//The locals a break leaves living...
	size		uiContinueLiving;	//...and a continue: a for's condition variable lives through its step.
};


//A case or default of the switch being written, and its block.
struct SCPP65IRCase
{
	CCPPSyntaxCaseStatement*	pcCase;
	CIRBlock*					pcBlock;
};


//A goto label and its block.
struct SCPP65IRLabel
{
	int64		lliName;
	CIRBlock*	pcBlock;
	bool		bPlaced;		//The label's statement has been written...
	size		uiLiving;		//...with this many locals living there, masGotoLocals' from uiFirstLocal.
	size		uiFirstLocal;
};


//A goto written before its label: it jumps to pcBlock, which destroys what the goto leaves the scope of once the label
//is reached and so known (PlaceGotos).  The locals living at the goto are masGotoLocals' from uiFirst, uiLiving of them.
struct SCPP65IRPendingGoto
{
	int64		lliName;
	CIRBlock*	pcBlock;
	size		uiFirst;
	size		uiLiving;
	CCPPSyntax*	pcGoto;
};


//A variable a debugger is told of (DescribesVariables, Debug or Release): the slot it is in (VariableSlot), at iOffset
//(a member of an anonymous union), or that holds its address (bIndirect: an object passed by address, or in the bytes
//its caller pushed).  The slot is named for its number (NameVariableSlots: dbg<iSlot>), which the back end says where
//it put (CIRWrittenFunction::FindSlot).  pcVariable NULL is this; sSlot IRNone (iSlot -1) a variable that has no place
//a debugger can read.
struct SCPP65IRDebugVariable
{
	CCPPVariable*	pcVariable;
	SIRReference	sSlot;
	int32			iSlot;
	int32			iOffset;
	bool			bIndirect;
};


typedef CArrayTemplate<SCPP65IRDebugVariable>	CArrayCPP65IRDebugVariables;


//A variable a debugger is told of, kept with its function (SCPP65IRWritten) until the unit's code is finished, when it
//is described (.dvariable, DescribeVariables) from what the back end last kept: a function written again once inlined
//is described as it was written last.  Its place is then dbg<iSlot>'s (CIRWrittenFunction::FindSlot) plus iOffset.
struct SCPP65IRDescribedVariable
{
	CChars			szName;
	SCPP65Type*		psType;
	int32			iSlot;			//-1: no place a debugger can read.
	int32			iOffset;
	int				iFirstLine;
	int				iLastLine;
	bool			bParameter;
	bool			bIndirect;
};


typedef CArrayTemplate<SCPP65IRDescribedVariable>	CArrayCPP65IRDescribedVariables;


//A function written as IR, waiting for FinishUnit: its label and the code Body generated.
struct SCPP65IRWritten
{
	CIRFunction*	pcFunction;		//NULL once it failed after inlining.
	CChars			szLabel;
	CChars			szName;			//As C++ writes it (CShape::Area), for a debugger (.dfunction),
	int32			iResult;		//with the debug type it returns (CCPP65DebugWriter::TypeIndex), -1 for void.
	uint16			uiPieces;		//How many pieces its code is in, in the generator's spill store (CCPP65Assembly::PutCode).
	CIRWrittenFunction*	pcBackend;	//What the back end kept about it, for CIRBackend::FinishFunction, or NULL.
	uint32			uiFingerprint;	//A hash of its IR before it was optimised (for a line table).
	CArrayCPP65IRDescribedVariables	asVariables;	//For a debugger (DescribesVariables): its variables, this first,
	CChars			szStatics;		//and its static locals' .dglobal lines.
};


//A loop round an index (BeginCountedLoop to EndCountedLoop): its test's block, which holds the index's phi, and the
//block after it.
struct SCPP65IRCountedLoop
{
	CIRBlock*		pcLoop;
	CIRBlock*		pcDone;
	SIRReference	sIndex;
};


#define IR65_FRONT_END_MARKER			"\x01"	//Around a written function's index in the generator's code.
#define IR65_ZERO_LONG_BYTES			4		//ZeroBytes zeroes a long at a time...
#define IR65_ZERO_LONGS					4		//...in a loop for more than this many of them...
#define IR65_ZERO_ELEMENT_BYTES			(IR65_ZERO_LONG_BYTES * IR65_ZERO_LONGS)	//...and elements of an array past as many bytes use it.
#define IR65_INLINE_ROUNDS				4		//Calls are inlined into a function in at most this many rounds (optimising between them).


typedef CHashMapLongTemplate<SIRReference>	CMapCPP65IRAddresses;
typedef CHashMapLongTemplate<SCPP65IRValue>	CMapCPP65IRValues;
typedef CArrayTemplate<SCPP65IRJump>		CArrayCPP65IRJumps;
typedef CArrayTemplate<SCPP65IRCase>		CArrayCPP65IRCases;
typedef CArrayTemplate<SCPP65IRLabel>		CArrayCPP65IRLabels;
typedef CArrayTemplate<SCPP65IRPendingGoto>	CArrayCPP65IRPendingGotos;
typedef CArrayTemplate<SCPP65IRWritten>		CArrayCPP65IRWritten;


uint16	CPPArithmeticOpcode(ECppTokens eOperator, bool bSigned);


//Writes every function body as IR and generates it with the target's back end (CIR65Backend or CIRW65C816Backend).
//The generator works out everything about the C++ (types with CCPP65TypeChecker, names, overloads, layouts) and
//sets up the function; Body then writes the function as IR if it supports everything in it, or returns false
//(having changed nothing) and the function is an error (CCPPUnit::GenerateDefinition).  Locals are slots,
//read and written with load and store; parameters are where their caller pushed them (argaddr).  What is
//supported is in IR.txt (Front end).
class CCPP65IRFrontEnd : public CIRInlineProfile, public CIRUnitFunctions
{
protected:
	CIRFunction*			mpcProfiledCaller;	//The caller CallCount last looked at...
	bool					mbProfiledCaller;	//...and whether the profile has it, as it is now (its fingerprint).
	CCPPUnit*		mpcUnit;
	CCPPDiagnostics*	mpcDiagnostics;	//Where its errors go: the unit's.
	CCPP65Types*			mpcTypes;

	CIRModule				mcModule;			//The unit's functions written as IR.
	CIRPipeline				mcPipeline;			//The optimiser's passes and their order, as the target builds them (CCPPTarget::BuildPipeline).
	CIRBackend*				mpcBackend;			//The target's back end (CCPPTarget::NewBackend), which writes their code; NULL for a target nothing is generated for.
	CArrayCPP65IRWritten	masWritten;			//The same, in the order written.
	bool					mbCodeGiven;		//The written functions' code is the unit's assembly's (FinishUnit), which takes it out of the store.
	CIRFunction*			mpcFunction;
	CIRBlock*				mpcBlock;
	CCPPFunction*			mpcCPPFunction;
	bool					mbBaseVersion;		//The base version of a constructor or destructor is being written: its class's virtual bases are left alone.
	CCPPClass*				mpcBaseObject;		//The class whose base version the next constructor or destructor call is (a base subobject's), or NULL.
	SIRReference			msThis;				//A member function's this, IRNone in others; in a lambda's body the enclosing object's, if it captures it.
	SIRReference			msClosure;			//A lambda's operator(): the closure it is called on, IRNone in others.
	SIRReference			msResult;			//Where a function that returns an object makes it, IRNone in others.
	SIRReference			msElide;			//Where the next call that returns an object makes it, instead of a temporary.
	SIRReference			msVTT;				//A base version's VTT entries (its hidden parameter after this), IRNone in others.
	SIRReference			msVTTArgument;		//The VTT entries the next call (of a base version) is given, or IRNone.
	CCPPSyntax*				mpcValue;			//The expression Value last started on (where a conversion's error is).
	CMapCPP65IRAddresses	mcAddresses;		//Each local's and parameter's address, by its CCPPVariable.
	CMapCPP65IRValues		mcImplicitValues;	//The values of the implicit arguments of the calls being written (CCPPUnit::ImplicitValue).
	CMapCPP65IRAddresses	mcAnonymousAddresses;	//Each anonymous union or struct local's slot, by its class.
	CArrayCPP65IRDebugVariables	masDebugVariables;	//The function's variables a debugger is told of (DescribesVariables), in their order.
	int32					miVariableSlots;	//The function's variables' slots so far (NameVariableSlots names them).
	int32					miDebugLabels;		//The unit's debug labels so far (DebugRanges).
	CArrayCPP65IRJumps		masJumps;
	CArrayCPP65IRCases		masCases;
	CArrayCPP65IRLabels		masLabels;
	CArrayCPP65IRPendingGotos	masPendingGotos;
	CArrayCPP65IRCleanups	masGotoLocals;
	CArrayTemplate<SCPP65IRTry>	masTries;		//The try blocks being written, the outermost first.
	CIRBlock*				mpcThrowExit;		//The function's block that ends in throw, once something needs it (WayOut).
	CIRBlock*				mpcUnexpected;		//Its block that checks an exception against a throw(A, B) list, once something needs it (UnexpectedBlock).
	CIRBlock*				mpcTerminate;		//Its block that calls std::terminate, once something needs it.
	bool					mbUnwinding;		//A cleanup is being written for an exception on its way: a call that throws there is std::terminate.
	size					muiLandingTop;		//While a normal way out destroys the cleanup entry at this index, an exception goes to the ones below it (MAX_SIZE: all of them).
	bool					mbListLocal;		//A local std::initializer_list is being made: its array lives as long as the local does (InitializerList).
	bool					mbConstructorHandler;	//A handler of a constructor's function-try-block is being written: it can't return.
	bool					mbRoutine;			//The function is one the compiler writes for itself (a global's initialiser or finaliser): no exception leaves it.
	CCPP65IRCleanups		mcCleanups;			//What needs destroying: the objects of the scopes being written and the temporaries of the full expression.
	int						miConditional;		//Inside an operand that may not be worked out (of &&, || or ?:).
	CIRBlock*				mpcExit;			//A destructor's last block (it destroys the fields and bases); NULL in others.
	CArraySyntax			mapcNone;			//No arguments.
	CArrayChars				maszRuntime;		//Runtime routines called, given to the generator if the function is written.
	CArrayChars				maszWeakRuntime;	//...and those called only if the program has them (CallRuntime's bWeak).
	CArrayChars				maszUnitWeak;		//The unit's written functions' maszWeakRuntime: declared .weak by FinishUnit unless the unit needs them.
	CArrayChars				maszTested;			//The functions called by name whose calls are tested for an exception (GetTestedCallees).
	CArrayChars				maszMissing;		//Named callees with no body in the unit or the imports (GetMissingCallees).
	CArrayChars				maszNotImported;	//Every name the inliner looked for in the imports and didn't find (CIRImports::Missed).
	CArrayUint8				mauiStringBytes;	//String literals, given to the generator if the function is written.
	CArrayTemplate<size>	mauiStringLengths;
	size					muiFirstString;		//The generator's number for the first of them.
	bool					mbUnsupported;		//Something the front end can't write yet: the function is an error.
	bool					mbConstants;		//It writes the IR of constant expressions (CCPPConstants): no code is generated and the unit is told nothing.
	CIRFunction*			mpcConstant;		//The last function FinishConstant kept.
	bool					mbFloatReported;	//The function's floating point (CCPPUnit::SetNoFloats) has been reported.
	const char*				mszUnsupportedFile;	//The front end's file (no directory) and line that last left a function to the generator.
	int						miUnsupportedLine;
	char					mszUnsupported[1024];	//The verifier's errors when that was IR that didn't verify.
	size					muiAttempts;		//Bodies asked for; only the first CCPPCompileOptions::muiFunctionLimit may be written.

public:
	void			Init(CCPPUnit* pcUnit, bool bConstants = false);
	void			Kill(void);
	void			SetStatistics(CCPPCompileStatistics* pcStatistics);

	bool			Body(CCPPFunction* pcFunction);
	bool			WriteBody(CCPPFunction* pcFunction);
	CIRFunction*	ConstantFunction(CCPPFunction* pcFunction);
	CIRFunction*	ConstantExpression(const char* szLabel, CCPPSyntax* pcExpression, SCPP65Type* psType);
	CIRFunction*	ConstantInitialiser(const char* szLabel, SCPP65Type* psType, CCPPSyntax* pcInitialiser);
	CIRModule*		GetModule(void);
	bool			ThrownDestroyerRoutine(const char* szLabel, SCPP65Type* psType);
	bool			GlobalRoutine(const char* szLabel, const char* szGlobal, SCPP65Type* psType, CCPPSyntax* pcInitialiser, bool bFinalise, const char* szExitNode = NULL, const char* szFinaliser = NULL, bool bThread = false, bool bLocal = true);
	bool			ExitListsRoutine(const char* szLabel);
	bool			DynamicCastRoutine(const char* szLabel);
	void			PushExitNode(const char* szNode, const char* szFinaliser, bool bThread);
	void			InlineUnit(void);
	bool			FinishUnit(void);
	CArrayChars*	GetNotImported(void);
	size			NumUnitFunctions(void);
	CIRFunction*	GetUnitFunction(size uiFunction);
	void			InitOptimiser(CIROptimiser* pcOptimiser);
	void			AddSpecialised(size uiFunction, CIRFunction* pcCopy);
	bool			RemoveSpecialised(size uiFunction);
	void			SetUnsupported(const char* szFile, int iLine, const char* szReason);
	void			AppendUnsupported(CChars* psz);
	void			ClearUnsupported(void);
	void			SourceComments(CChars* pszCode, bool bComments, bool bLines);
	void			DebugRanges(CChars* pszCode, bool bRanges, CArrayChars* paszRanges);
	uint32			Fingerprint(CIRFunction* pcFunction);
	uint64			CallCount(CIRFunction* pcCaller, SIRInstruction* psCall);
	uint64			InlinedEntries(CAsm65Profile* pcProfile, const char* szLabel, SIRInstruction* psCall);
	uint64			ProfileCycles(void);
	CAsm65Profile*	InliningProfile(void);
	bool			SourceLine(int iToken, CSourceFile** ppcFile, int* piLine);
	CArrayChars*	GetMissingCallees(void);
	CArrayChars*	GetTestedCallees(void);

protected:
	void			BeginWriting(CCPPFunction* pcFunction, const char* szLabel, SIRType sReturn, uint32 uiAttributes);
	bool			FinishWriting(bool bResult, const char* szLabel);
	void			FunctionName(CChars* pszDest, const char* szLabel);
	int32			ResultTypeIndex(void);
	bool			FinishConstant(bool bResult);
	void			EndWriting(void);
	void			OptimiserWarnings(CChars* pszWarnings);
	bool			GenerateCode(CIRFunction* pcFunction, const char* szLabel, CChars* pszCode, CIRWrittenFunction** ppcWritten);
	void			FreeWritten(SCPP65IRWritten* psWritten);
	bool			Unsupported(const char* szFile, int iLine);
	bool			BitFieldAddress(const char* szFile, int iLine);
	SIRType			IRTypeOf(SCPP65Type* psType);
	void			NoFloats(void);
	SIRType			ReturnIRType(SCPP65Type* psType);
	bool			IsSupportedType(SCPP65Type* psType);
	SCPP65Type*		ElementType(SCPP65Type* psType);
	bool			IsPlaceType(SCPP65Type* psType);
	bool			IsLocalType(SCPP65Type* psType);
	bool			ReferenceAddress(SCPP65IRPlace* psPlace, SCPP65Type* psReference, SIRReference* psAddress);
	bool			ReferredPlace(CCPPSyntax* pcExpression, SCPP65Type* psReference, SCPP65IRPlace* psPlace);
	void			ArrowResult(SCPP65IRValue* psResult);
	bool			ConvertedAddress(SCPP65IRPlace* psPlace, SCPP65Type* psReference, SIRReference* psAddress);
	CIRBlock*		NewBlock(void);
	void			StartBlock(CIRBlock* pcBlock);
	void			JumpTo(CIRBlock* pcBlock);
	bool			Terminated(void);
	SIRReference	Integer(SCPP65Type* psType, int64 lliValue);
	SIRReference	Offset(int iBytes);
	SIRReference	AddressAt(SIRReference sAddress, int iOffset);
	bool			Parameters(void);
	bool			DebugParameter(CCPPVariable* pcVariable, SIRReference sAddress, bool bByAddress);
	void			ClosureThis(void);
	bool			LambdaObject(SCPP65IRPlace* psPlace, CCPPSyntaxLambdaExpression* pcSyntax);
	bool			ReturnAddressOf(CCPPFunction* pcFunction);
	bool			DeletingDestructor(CCPPClass* pcClass);
	void			AddPieces(int iBytes);
	int				PieceCount(int iBytes);
	int				PieceBytes(int iLeft);
	bool			Locals(void);
	bool			Debuggable(void);
	bool			DescribesVariables(void);
	SIRReference	VariableSlot(uint32 uiSize, uint32 uiAlign);
	void			NameVariableSlots(void);
	void			AddDebugVariable(CCPPVariable* pcVariable, SIRReference sSlot, int32 iOffset, bool bIndirect);
	void			RecordVariables(SCPP65IRWritten* psWritten);
	void			CopyVariables(SCPP65IRWritten* psCopy, SCPP65IRWritten* psWritten);
	void			DescribeVariables(CChars* pszCode, SCPP65IRWritten* psWritten, CArrayChars* paszRanges);
	void			VariableLines(SCPP65IRDebugVariable* psVariable, int* piFirst, int* piLast);
	int				TokenLine(int iToken);
	void			Finish(void);
	bool			Statement(CCPPSyntax* pcStatement);
	bool			StatementOf(CCPPSyntax* pcStatement);
	bool			CompoundStatement(CCPPSyntaxCompoundStatement* pcCompound);
	bool			JumpStatement(CCPPSyntax* pcStatement, CIRBlock* pcBreak, CIRBlock* pcContinue, size uiBreakLiving = (size)-1, size uiContinueLiving = (size)-1);
	bool			ScopedStatement(CCPPSyntax* pcStatement);
	void			AddLiving(SCPP65IRPlace* psPlace);
	void			ExtendTemporary(SIRReference sReferred);
	bool			LeaveScope(size uiLiving);
	bool			DestroyLiving(size uiLiving);
	bool			FullExpression(CCPPSyntax* pcExpression, SCPP65IRValue* psValue);
	bool			DestroyTemporaries(size uiTemporaries);
	void			MarkFullExpression(void);
	void			MarkSigned(SCPP65Type* psType);
	bool			DestroyTemporaryPlaces(size uiFrom);
	bool			DestroyTemporary(SCPP65IRCleanup* psTemporary);
	bool			CallMayThrow(CCPPFunction* pcFunction);
	void			TestThrown(CCPPFunction* pcFunction, SIRReference sCallee);
	CIRBlock*		Landing(void);
	CIRBlock*		LandingBelow(void);
	CIRBlock*		UndoBlock(size uiIndex);
	bool			UndoEntry(SCPP65IRCleanup* psEntry);
	bool			UndoEntryBelow(SCPP65IRCleanup* psEntry, size uiIndex);
	bool			UndoPart(SCPP65IRCleanup* psPart);
	bool			UndoElements(SCPP65IRCleanup* psPart);
	bool			UndoDeallocate(SCPP65IRCleanup* psPart);
	bool			FunctionTerminates(void);
	bool			IsWeak(CCPPFunction* pcFunction);
	CIRBlock*		ThrowExit(void);
	CIRBlock*		UnexpectedBlock(void);
	void			MatchListed(CCPPExceptionSpec* pcSpec, CIRBlock* pcAllowed);
	void			TestedRoutine(const char* szLabel, CIRBlock* pcThrown, CIRBlock* pcReturned);
	void			ThrowRoutine(const char* szLabel);
	bool			IsNullable(CCPPFunction* pcAllocation);
	bool			PartsWanted(void);
	bool			NeedsDestroying(SCPP65Type* psType);
	void			PushDestroyPart(SIRReference sAddress, SCPP65Type* psType, CCPPClass* pcBaseObject, size uiAt = MAX_SIZE);
	bool			MemberInitialised(size uiAt, size uiTemporaries, SIRReference sAddress, SCPP65Type* psType, CCPPClass* pcBaseObject);
	void			PushAddressPart(ECPP65IRPart ePart, SIRReference sAddress);
	void			DestructorStart(void);
	bool			Deallocation(CCPPSyntaxNewExpression* pcNew, CCPPFunction* pcAllocation, SCPP65Type* psType, CArraySyntax* papcArguments, SCPP65IRValue* psBytes, SCPP65IRCleanup* psFree);
	CCPPFunction*	PlacementDeallocation(CCPPFunction* pcAllocation, bool bArray);
	CIRBlock*		WayOut(void);
	CIRBlock*		TerminateBlock(void);
	SIRReference	CallNoThrow(const char* szLabel, SIRType sResult, SIRType* pasTypes, SIRReference* pasArguments, int iArguments);
	SCPP65Type*		ThrownType(SCPP65Type* psType);
	SIRReference	TypeInfoSymbol(SCPP65Type* psType);
	bool			ThrowExpression(CCPPSyntaxThrowExpression* pcThrow, SCPP65IRValue* psValue);
	bool			TryStatement(CCPPSyntaxTryStatement* pcTry);
	bool			FunctionTryBlock(CCPPFunction* pcFunction, CCPPSyntaxTryStatement* pcTry);
	bool			TryHandlers(CCPPSyntaxTryStatement* pcTry, CIRBlock* pcDispatch, CIRBlock* pcAfter, bool bRethrow);
	bool			Handler(CCPPSyntaxCatchHandler* pcHandler, SIRReference sMatched, bool bRethrow);
	bool			HandlerParameter(CCPPSyntaxCatchHandler* pcHandler, SIRReference sMatched);
	bool			CopyFromAddress(SCPP65IRPlace* psPlace, SIRReference sSource);
	bool			CopyArray(SCPP65IRPlace* psPlace, SIRReference sSource);
	bool			ImplicitCopy(CCPPFunction* pcFunction);
	bool			DeclarationStatement(CCPPSyntaxDeclarationStatement* pcStatement);
	bool			Declaration(CCPPSyntaxSimpleDeclaration* pcDeclaration, CCPPVariable** ppcLast);
	bool			GuardedStaticLocal(CCPPVariable* pcVariable, CCPPSyntax* pcInitialiser);
	bool			LocalInitialiser(SCPP65IRPlace* psPlace, CCPPSyntax* pcInitialiser, bool bInFrame = false);
	bool			ArrayInitialiser(SCPP65IRPlace* psPlace, CCPPSyntax* pcInitialiser);
	bool			ArrayElements(SCPP65IRPlace* psPlace, CArraySyntax* papcElements, size* puiNext);
	void			ZeroElements(SIRReference sAddress, int iOffset, SCPP65Type* psElement, int iCount);
	bool			IfStatement(CCPPSyntaxIfStatement* pcIf);
	bool			WhileStatement(CCPPSyntaxWhileStatement* pcWhile);
	bool			DoStatement(CCPPSyntaxDoStatement* pcDo);
	bool			ForStatement(CCPPSyntaxForStatement* pcFor);
	bool			RangeForStatement(CCPPSyntaxRangeForStatement* pcFor);
	bool			VariablePlace(CCPPSymbol* pcSymbol, SCPP65IRPlace* psPlace);
	bool			RangeIterator(CCPPSymbol* pcSymbol, CCPPFunction* pcFunction, bool bMember, CCPPSyntax* pcRange, bool bEnd);
	bool			RangeForVariable(CCPPSyntaxRangeForStatement* pcFor, CCPPSyntax* pcElement);
	bool			SwitchStatement(CCPPSyntaxSwitchStatement* pcSwitch);
	bool			FindCases(CCPPSyntax* pcNode, CIRBlock** ppcDefault);
	bool			CaseStatement(CCPPSyntaxCaseStatement* pcCase);
	bool			ReturnStatement(CCPPSyntaxReturnStatement* pcReturn);
	bool			BreakStatement(CCPPSyntax* pcStatement, bool bContinue);
	bool			GotoStatement(CCPPSyntaxGotoStatement* pcGoto);
	bool			AsmStatement(CCPPSyntax* pcStatement);
	bool			LabelStatement(CCPPSyntaxLabelStatement* pcLabel);
	SCPP65IRLabel*	GotoLabel(int iToken);
	bool			PlaceGotos(SCPP65IRLabel* psLabel);
	bool			GotoKeeps(SCPP65IRCleanup* pasLocals, size uiLiving, CCPPSyntax* pcGoto);
	bool			Expression(CCPPSyntax* pcExpression, SCPP65IRValue* psValue);
	bool			Value(CCPPSyntax* pcExpression, SCPP65IRValue* psValue);
	bool			ValueFor(CCPPSyntax* pcExpression, SCPP65Type* psTo, SCPP65IRValue* psValue);
	bool			ContextualValue(CCPPSyntax* pcExpression, ECPPContextualTypes eWanted, SCPP65IRValue* psValue);
	bool			HasOperatorFunction(CCPPSyntaxBinaryExpression* pcBinary);
	bool			Operands(CCPPSyntaxBinaryExpression* pcBinary, SCPP65IRValue* psLeft, SCPP65IRValue* psRight);
	bool			ReadReferred(SCPP65IRValue* psValue);
	bool			Load(SCPP65IRPlace* psPlace, SCPP65IRValue* psValue);
	void			Store(SCPP65IRPlace* psPlace, SCPP65IRValue* psValue);
	SIRReference	BitFieldValue(SCPP65Type* psType, int iOffset, int iWidth, SIRReference sUnit);
	void			BitFieldResult(SCPP65IRPlace* psPlace, SCPP65IRValue* psValue);
	SIRReference	FieldValue(SCPP65Type* psType, SCPPField* psBitField, bool bStored, SIRReference sUnit);
	SIRReference	UnitWithField(SCPP65IRPlace* psPlace, SCPPField* psBitField, SIRReference sValue);
	SIRReference	Either(SCPP65IRPlace* psPlace, SIRReference sTrue, SIRReference sFalse);
	bool			ConstantValue(CCPPSyntax* pcExpression, SCPP65IRValue* psValue);
	bool			StringLiteral(CCPPSyntax* pcString, SCPP65IRValue* psValue);
	bool			ScalarConstruct(CCPPSyntaxConstructExpression* pcConstruct, SCPP65IRValue* psValue);
	bool			MemberEnumerator(CCPPSyntaxMemberExpression* pcMember, SCPP65IRValue* psValue);
	bool			FunctionName(CCPPSyntaxIdExpression* pcId, SCPP65IRValue* psValue);
	bool			FunctionAddress(CCPPSyntaxIdExpression* pcId, SCPP65Type* psTarget, SCPP65IRValue* psValue);
	bool			MemberAddress(CCPPSyntaxIdExpression* pcId, SCPP65Type* psTarget, SCPP65IRValue* psValue, bool* pbMember);
	bool			OverloadAddress(CCPPSyntax* pcExpression, SCPP65Type* psTo, SCPP65IRValue* psValue, bool* pbAddress);
	bool			MemberPointerObject(CCPPSyntaxBinaryExpression* pcBinary, SIRReference* psObject, SCPP65Type** ppsMember);
	bool			MemberPointerCall(CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments, CCPPSyntaxBinaryExpression* pcBinary, SCPP65IRValue* psValue);
	bool			MemberPointerPlace(CCPPSyntaxBinaryExpression* pcBinary, SCPP65IRPlace* psPlace);
	bool			ImplicitArgument(CCPPSyntaxImplicitArgument* pcArgument, SCPP65IRValue* psValue);
	bool			LiteralCall(CCPPSyntax* pcLiteral, SCPP65IRValue* psValue);
	void			StringData(char* pcBytes, size uiLength, SCPP65IRValue* psValue);
	bool			Place(CCPPSyntax* pcExpression, SCPP65IRPlace* psPlace);
	bool			IsReferenceCast(CCPPSyntax* pcExpression);
	bool			ReferenceCastPlace(CCPPSyntax* pcCast, SCPP65Type* psReference, SCPP65IRPlace* psPlace);
	bool			DynamicCastPlace(CCPPSyntax* pcCast, SCPP65Type* psReference, SCPP65IRPlace* psPlace);
	bool			TypeidPlace(CCPPSyntaxTypeidExpression* pcTypeid, SCPP65IRPlace* psPlace);
	bool			TypeInfoCall(CCPPFunction* pcFunction, SIRReference sObject, CArrayIRReferences* pasArguments, SCPP65IRValue* psValue);
	bool			VariablePlace(CCPPVariable* pcVariable, SCPP65IRPlace* psPlace);
	bool			UnaryExpression(CCPPSyntaxUnaryExpression* pcUnary, SCPP65IRValue* psValue);
	bool			IncrementExpression(CCPPSyntaxUnaryExpression* pcUnary, SCPP65IRValue* psValue, SCPP65IRPlace* psStored = NULL);
	bool			BinaryExpression(CCPPSyntaxBinaryExpression* pcBinary, SCPP65IRValue* psValue);
	bool			Assignment(CCPPSyntaxBinaryExpression* pcBinary, SCPP65IRValue* psValue, SCPP65IRPlace* psStored = NULL);
	bool			AssignedObject(CCPPSyntax* pcRight, SCPP65Type* psClass, SCPP65IRValue* psValue);
	bool			Operand(CCPPSyntax* pcExpression, SCPP65IRValue* psValue);
	bool			Operate(ECppTokens eOperator, SCPP65IRValue* psLeft, SCPP65IRValue* psRight, SCPP65IRValue* psResult);
	bool			Compare(ECppTokens eOperator, SCPP65IRValue* psLeft, SCPP65IRValue* psRight, SIRReference* psCondition);
	bool			ConditionalExpression(CCPPSyntaxConditionalExpression* pcConditional, SCPP65IRValue* psValue);
	bool			ConditionalPlace(CCPPSyntaxConditionalExpression* pcConditional, SCPP65IRPlace* psPlace);
	bool			ObjectConditional(CCPPSyntaxConditionalExpression* pcConditional, SCPP65IRPlace* psInto, SCPP65IRValue* psValue);
	bool			IsObjectCall(CCPPSyntax* pcExpression);
	bool			IsObjectOf(CCPPSyntax* pcExpression, CCPPClass* pcClass);
	bool			InheritedConstructorCall(SCPP65IRPlace* psBase);
	bool			CovariantThunk(CCPPFunction* pcFunction);
	bool			ConditionValue(CCPPSyntax* pcExpression, SCPP65IRValue* psValue);
	bool			Condition(CCPPSyntax* pcExpression, CIRBlock* pcTrue, CIRBlock* pcFalse);
	bool			FullCondition(CCPPSyntax* pcExpression, CIRBlock* pcTrue, CIRBlock* pcFalse);
	bool			Truth(SCPP65IRValue* psValue, SIRReference* psCondition);
	bool			IsObjectValue(SCPP65IRValue* psValue);
	bool			ConvertObject(SCPP65IRValue* psValue, SCPP65Type* psTo, bool bExplicit = true);
	bool			ConvertObjectBy(SCPP65IRValue* psValue, CCPPBoundConversion* pcBound, SCPP65Type* psTo);
	bool			CallExpression(CCPPSyntaxCallExpression* pcCall, SCPP65IRValue* psValue);
	bool			MemberCall(CCPPBoundCall* pcBound, SCPP65IRValue* psValue);
	bool			CallThroughPointer(CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments, CCPPSyntax* pcCallee, SCPP65IRValue* psValue);
	bool			GatherArguments(CArraySyntax* papcArguments, CArrayCPP65Types* papsParameters, CArraySyntax* papcDefaults, CCPPTemplateArguments* pcDefaultBindings, bool bVariadic, SCPP65Type* psReturn,
									SIRReference* psMade, SCPP65IRMade* psReturned, CArrayTemplate<SCPP65IRValue>* pasArguments, CArrayIRReferences* pasLoaded, CArrayTemplate<int32>* paiLoaded);
	bool			ParameterArguments(CArraySyntax* papcArguments, CArrayCPP65Types* papsParameters, size uiFrom, size uiTo, bool bVariadic, CArrayTemplate<SCPP65IRValue>* pasArguments,
									   CArrayIRReferences* pasLoaded, CArrayTemplate<int32>* paiLoaded);
	bool			Call(CCPPFunction* pcFunction, SIRReference sCallee, SCPP65Type* psFunctionType, CArraySyntax* papcArguments, SCPP65IRValue* psValue, CCPPSyntax* pcObject = NULL, bool bArrow = false, CCPPClass* pcObjectClass = NULL, bool bVirtual = false, SIRReference sGivenObject = IRNone(), bool bObjectLast = false);
	bool			ArgumentPieces(CArraySyntax* papcArguments, CArrayCPP65Types* papsParameters, CArraySyntax* papcDefaults, CCPPTemplateArguments* pcDefaultBindings, bool bVariadic, SCPP65Type* psReturn, SIRReference* psMade,
								   SCPP65IRMade* psReturned, CArrayIRReferences* pasPieces);
	void			CallResult(SCPP65Type* psReturn, SIRReference sMade, SIRReference sResult, SCPP65IRValue* psValue);
	bool			ThisObject(CCPPFunction* pcFunction, CCPPSyntax* pcObject, bool bArrow, CCPPClass* pcObjectClass, bool bVirtual, SIRReference sGivenObject, SIRReference* psObject, SIRReference* psCallee);
	bool			CastExpression(CCPPSyntax* pcCast, CCPPSyntaxTypeId* pcTypeId, CCPPSyntax* pcOperand, SCPP65IRValue* psValue);
	bool			IsReinterpretCast(CCPPSyntax* pcCast);
	bool			DerivedAddress(SIRReference* psAddress, CCPPClass* pcFrom, CCPPClass* pcTo, bool bNull, CCPPSyntax* pcCast);
	bool			DynamicCastExpression(CCPPSyntaxNamedCastExpression* pcCast, SCPP65IRValue* psValue);
	SIRReference	DynamicCast(SIRReference sAddress, CCPPClass* pcFrom, CCPPClass* pcTo);
	SIRReference	TypeInfoSymbol(CCPPClass* pcClass);
	SIRReference	WholeObject(SIRReference sAddress, CCPPClass* pcFrom);
	SIRReference	ObjectTop(SIRReference sVtablePointer, SIRReference* psVtable);
	bool			Convert(SCPP65IRValue* psValue, SCPP65Type* psTo);
	bool			ConvertImplicit(SCPP65IRValue* psValue, SCPP65Type* psTo, CCPPSyntax* pcNode);
	bool			CountTable(bool bLeading, SCPP65IRValue* psArgument, SCPP65Type* psType, SCPP65IRValue* psValue);
	void			NarrowCount(bool bLeading, SIRReference* psValue, SIRReference* psCount, int* piBits, SIRType sInt);
	bool			Builtin(CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments, int64 lliName, SCPP65IRValue* psValue);
	bool			BuiltinMemcpy(CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments, SCPP65IRValue* psValue);
	bool			BuiltinBCD(CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments, bool bSubtract, SCPP65IRValue* psValue);
	bool			CallWithArguments(CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments, SCPP65Type** apsParameters, int iCount, const char* szSymbol, SCPP65Type* psResult, SCPP65IRValue* psValue);
	bool			VerifyIR(CIRFunction* pcFunction, const char* szFile, int iLine);
	SIRReference	ConvertIR(SIRReference sValue, SIRType sFrom, SIRType sTo, bool bSigned);
	SIRReference	MovedUnlessNull(SIRType sType, SIRReference sValue, int64 lliOffset);
	SIRReference	BaseAddress(SIRReference sAddress, int iOffset);
	SIRReference	BaseAddress(SIRReference sAddress, CCPPClass* pcFrom, CCPPClass* pcTo, bool bNull);
	SIRReference	VirtualBaseAddress(SIRReference sAddress, CCPPClass* pcClass, CCPPClass* pcBase);
	bool			ObjectAddress(CCPPSyntax* pcObject, bool bArrow, SIRReference* psAddress);
	bool			InlineMemcpy(CArraySyntax* papcArguments, SCPP65Type* psResult, SCPP65IRValue* psValue);
	bool			ConvertMemberPointer(SCPP65IRValue* psValue, SCPP65Type* psTo);
	SCPP65Type*		ArrowClass(CCPPSyntax* pcObject, SCPP65Type* psType);
	bool			MemberPlace(CCPPSyntaxMemberExpression* pcMember, SCPP65IRPlace* psPlace);
	bool			FieldAt(SIRReference sObject, SCPPField* psField, int iOffset, SCPP65IRPlace* psPlace);
	bool			BoundFieldAt(SIRReference sObject, CCPPBoundName* pcBound, SCPP65IRPlace* psPlace);
	bool			DestructorCall(CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments, CCPPClass* pcClass, CCPPSyntax* pcObject, bool bArrow, bool bVirtual, SCPP65IRValue* psValue);
	bool			OperatorCall(CCPPSyntax* pcObject, ECppTokens eOperator, CCPPSyntax* pcArgument, SCPP65IRValue* psValue);
	bool			BoundOperatorCall(CCPPBoundCall* pcBound, ECppTokens eOperator, SCPP65IRValue* psValue);
	bool			OperatorExpression(ECppTokens eOperator, CCPPSyntax* pcLeft, CCPPSyntax* pcRight, SCPP65IRValue* psValue);
	bool			IsObjectSubscript(CCPPSyntaxSubscriptExpression* pcSubscript);
	bool			ArrayValue(CCPPSyntax* pcArray, SCPP65IRValue* psPointer, SCPP65Type** ppsArray);
	bool			NarrowSubscript(SCPP65Type* psArray, SCPP65IRValue* psPointer, SCPP65IRValue* psIndex, SCPP65IRValue* psAddress);
	void			PrintIR(CIRFunction* pcFunction);
	void			PrintFunction(CIRFunction* pcFunction, CChars* pszText, bool bSources);
	bool			ConstructorStart(CCPPSyntaxFunctionDefinition* pcFunction);
	bool			DelegatedConstructor(CCPPClass* pcClass, CCPPSyntaxMemberInitializer* pcMember);
	bool			ConstructBase(CArraySyntax* papcInitialisers, CCPPClass* pcBase, SIRReference sAddress);
	bool			DestructorEnd(void);
	void			TakeDestroyPart(SCPP65Type* psType);
	bool			StoreVtables(SIRReference sObject, CCPPClass* pcClass, bool bConstruction = false);
	SIRReference	VTTOf(CCPPClass* pcClass);
	SIRReference	CompleteVTT(CCPPClass* pcClass);
	bool			DestroyBase(SIRReference sAddress, CCPPClass* pcBase);
	void			DescribeVtable(CArrayTemplate<SCPPVtableEntry>* pasEntries, CCPPClass* pcClass, uint32 uiSymbol);
	bool			Construct(SCPP65IRPlace* psPlace, CArraySyntax* papcArguments, CCPPSyntax* pcInitialiser);
	bool			InitializerList(SCPP65IRPlace* psPlace, SCPP65Type* psElement, CArraySyntax* papcElements);
	bool			ConvertToObject(SCPP65IRPlace* psPlace, CCPPFunction* pcConversion, CCPPSyntax* pcSource);
	bool			ConstructClass(SCPP65IRPlace* psPlace, CCPPClass* pcClass, CArraySyntax* papcArguments);
	bool			Destroy(SIRReference sAddress, SCPP65Type* psType);
	SIRReference	Phi(CIRBlock* pcBlock, SIRType sType, CIRBlock* pcFrom1, SIRReference sValue1, CIRBlock* pcFrom2, SIRReference sValue2);
	SIRReference	LoopPhi(CIRBlock* pcLoop, SIRType sType, CIRBlock* pcEntry, SIRReference sStart);
	void			ClosePhi(CIRBlock* pcLoop, SIRReference sPhi, CIRBlock* pcLatch, SIRReference sNext);
	SIRReference	BeginCountedLoop(SCPP65IRCountedLoop* psLoop, SIRType sType, SIRReference sStart, uint16 uiTest, SIRReference sLimit);
	void			EndCountedLoop(SCPP65IRCountedLoop* psLoop, SIRReference sNext);
	SIRReference	FunctionSymbol(CCPPFunction* pcFunction, bool bBaseVersion = false);
	void			MarkFunctionSymbol(CCPPFunction* pcFunction, uint32 uiSymbol);
	bool			FunctionTemplateAddress(CCPPSyntaxIdExpression* pcId, SCPP65IRValue* psValue);
	bool			CallObject(CCPPFunction* pcFunction, SIRReference sObject, CCPPClass* pcObjectClass, CArraySyntax* papcArguments, bool bVirtual);
	SIRReference	CallRuntime(const char* szName, SIRType sResult, SIRType sArgument, SIRReference sValue, bool bWeak = false);
	bool			NewExpression(CCPPSyntaxNewExpression* pcNew, SCPP65IRValue* psValue);
	bool			Allocate(CCPPSyntaxNewExpression* pcNew, SCPP65Type* psType, SCPP65IRValue* psBytes, SIRReference* psMemory, SCPP65IRCleanup* psFree = NULL, CCPPFunction** ppcAllocation = NULL);
	bool			CallWithValue(CCPPFunction* pcFunction, CArraySyntax* papcArguments, CCPPSyntax* pcImplicit, SCPP65IRValue* psImplicit, SCPP65IRValue* psResult);
	bool			ValueInitialise(SCPP65IRPlace* psPlace);
	bool			NewArray(CCPPSyntaxNewExpression* pcNew, SCPP65Type* psType, SCPP65IRValue* psValue);
	bool			DeleteArray(CCPPSyntaxDeleteExpression* pcDelete, SCPP65IRValue* psPointer, CCPPClass* pcClass);
	bool			DeleteExpression(CCPPSyntaxDeleteExpression* pcDelete, SCPP65IRValue* psValue);
	bool			Deallocate(CCPPSyntax* pcFor, CCPPFunction* pcDeallocation, bool bArray, SCPP65IRValue* psBytes, SIRReference sFree);
	SCPP65IRValue	ByteCount(int64 lliBytes);
	bool			ObjectValue(CCPPSyntax* pcExpression, SCPP65IRValue* psValue);
	bool			Temporary(SCPP65Type* psType, SCPP65IRPlace* psPlace, SCPP65IRMade* psMade);
	void			TemporaryMade(SCPP65IRMade* psMade);
	bool			ValueTemporary(CCPPSyntax* pcValue, SCPP65Type* psType, SCPP65IRPlace* psPlace);
	bool			CopyConstruct(SCPP65IRPlace* psPlace, CCPPSyntax* pcSource);
	bool			CopyObject(SCPP65IRPlace* psPlace, CCPPSyntax* pcSource);
	bool			CopyBytes(SCPP65IRPlace* psPlace, SCPP65IRValue* psSource);
	bool			CopyPlace(SCPP65IRPlace* psPlace, SCPP65IRPlace* psSource, SIRReference* psAddress);
	bool			CopyMembers(SCPP65IRPlace* psPlace, CCPPSyntax* pcSource);
	bool			CopyDeleted(CCPPSyntax* pcNode);
	bool			AssignmentDeleted(CCPPSyntax* pcNode);
	bool			CopyParts(CCPPClass* pcClass, SIRReference sDestination, SIRReference sSource, bool bMove, SIRReference sVTT);
	bool			CopyPart(CCPPClass* pcClass, SIRReference sDestination, SIRReference sSource, bool bMove, SIRReference sVTT);
	SIRReference	BaseVTT(CCPPClass* pcClass, SIRReference sVTT, CCPPClass* pcBase);
	bool			AssignMembers(SCPP65IRPlace* psPlace, SCPP65IRValue* psSource, CCPPSyntax* pcSource);
	bool			AssignParts(CCPPClass* pcClass, SIRReference sDestination, SIRReference sSource, bool bMove, bool bVirtualBases);
	bool			AssignPart(CCPPClass* pcClass, SIRReference sDestination, SIRReference sSource, bool bMove, bool bVirtualBases);
	int				BaseOffset(CCPPClass* pcDerived, CCPPClass* pcBase);
	void			CopyPieces(SIRReference sDest, SIRReference sSource, int iBytes);
	void			ZeroBytes(SIRReference sAddress, int iBytes);
	bool			IsPassedByAddress(SCPP65Type* psType);
};


#endif // __C_P_P_65_I_R_FRONT_END_H__
