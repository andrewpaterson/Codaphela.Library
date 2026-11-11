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
#include "BaseLib/MapLongTemplate.h"
#include "BaseLib/ArrayUint8.h"
#include "IRModule.h"
#include "IRBackend.h"
#include "IRInliner.h"
#include "IRInterprocedural.h"
#include "CPPSyntaxes.h"
#include "CPP65Type.h"


class CSourceFile;
class CIROptimiser;


class CCPP65Generator;
struct SCPP65Function;
struct SCPP65Variable;
struct SCPP65VtableEntry;


//A C++ value: an IR value and its C++ type (never a reference).
struct SCPP65IRValue
{
	SIRReference	sValue;
	SCPP65Type*		psType;
};


//Where a C++ lvalue is: its address (an IR pointer) and its type.
struct SCPP65IRPlace
{
	SIRReference	sAddress;
	SCPP65Type*		psType;
};


//A temporary to destroy at the end of its full expression.  One made in an operand that may not be worked out (of
//&&, || or ?:) has its place and an i8 flag (1 once it is made) in the entry block.
struct SCPP65IRTemporary
{
	SCPP65IRPlace	sPlace;
	bool			bConditional;
	SIRReference	sMade;
};


//Where break and continue go (NULL for continue in a switch), and how many objects were living there (see
//masLiving): those after them are destroyed on the way.
struct SCPP65IRJump
{
	CIRBlock*	pcBreak;
	CIRBlock*	pcContinue;
	size		uiLiving;
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
};


//A function written as IR, waiting for FinishUnit: its label and the code Body generated.
struct SCPP65IRWritten
{
	CIRFunction*	pcFunction;		//NULL once it failed after inlining.
	CChars			szLabel;
	uint16			uiPieces;		//How many pieces its code is in, in the generator's spill store (CCPP65Assembly::PutCode).
	CIRWrittenFunction*	pcBackend;	//What the back end kept about it, for CIRBackend::FinishFunction, or NULL.
	uint32			uiFingerprint;	//A hash of its IR before it was optimised (for a line table).
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


typedef CMapLongTemplate<SIRReference>		CMapCPP65IRAddresses;
typedef CArrayTemplate<SCPP65IRJump>		CArrayCPP65IRJumps;
typedef CArrayTemplate<SCPP65IRCase>		CArrayCPP65IRCases;
typedef CArrayTemplate<SCPP65IRLabel>		CArrayCPP65IRLabels;
typedef CArrayTemplate<SCPP65IRPlace>		CArrayCPP65IRPlaces;
typedef CArrayTemplate<SCPP65IRTemporary>	CArrayCPP65IRTemporaries;
typedef CArrayTemplate<SCPP65IRWritten>		CArrayCPP65IRWritten;


//Writes every function body as IR and generates it with the target's back end (CIR65Backend or CIR816Backend).
//The generator works out everything about the C++ (types with CCPP65TypeChecker, names, overloads, layouts) and
//sets up the function; Body then writes the function as IR if it supports everything in it, or returns false
//(having changed nothing) and the function is an error (CCPP65Generator::GenerateDefinition).  Locals are slots,
//read and written with load and store; parameters are where their caller pushed them (argaddr).  What is
//supported is in IR.txt (Front end).
class CCPP65IRFrontEnd : public CIRInlineProfile, public CIRUnitFunctions
{
protected:
	CIRFunction*			mpcProfiledCaller;	//The caller CallCount last looked at...
	bool					mbProfiledCaller;	//...and whether the profile has it, as it is now (its fingerprint).
	CCPP65Generator*		mpcGenerator;
	CCPP65Types*			mpcTypes;

	CIRModule				mcModule;			//The unit's functions written as IR.
	CIRBackend*				mpcBackend;			//The target's back end (CCPPTarget::NewBackend), which writes their code; NULL for a target nothing is generated for.
	CArrayCPP65IRWritten	masWritten;			//The same, in the order written.
	bool					mbCodeGiven;		//The written functions' code is the unit's assembly's (FinishUnit), which takes it out of the store.
	CIRFunction*			mpcFunction;
	CIRBlock*				mpcBlock;
	SCPP65Function*			mpsFunction;
	SIRReference			msThis;				//A member function's this, IRNone in others.
	SIRReference			msResult;			//Where a function that returns an object makes it, IRNone in others.
	SIRReference			msElide;			//Where the next call that returns an object makes it, instead of a temporary.
	CCPPSyntax*				mpcValue;			//The expression Value last started on (where a conversion's error is).
	CMapCPP65IRAddresses	mcAddresses;		//Each local's and parameter's address, by its SCPP65Variable.
	CMapCPP65IRAddresses	mcAnonymousAddresses;	//Each anonymous union or struct local's slot, by its class.
	CArrayCPP65IRJumps		masJumps;
	CArrayCPP65IRCases		masCases;
	CArrayCPP65IRLabels		masLabels;
	CArrayCPP65IRPlaces		masLiving;			//The objects that need destroying in the scopes being written, oldest first.
	CArrayCPP65IRTemporaries	masTemporaries;		//The temporaries to destroy at the end of the full expression, oldest first.
	int						miConditional;		//Inside an operand that may not be worked out (of &&, || or ?:).
	CIRBlock*				mpcExit;			//A destructor's last block (it destroys the fields and bases); NULL in others.
	CArraySyntax			mapcNone;			//No arguments.
	CArrayChars				maszRuntime;		//Runtime routines called, given to the generator if the function is written.
	CArrayChars				maszMissing;		//Named callees with no body in the unit or the imports (GetMissingCallees).
	CArrayChars				maszNotImported;	//Every name the inliner looked for in the imports and didn't find (CIRImports::Missed).
	CArrayUint8				mauiStringBytes;	//String literals, given to the generator if the function is written.
	CArrayTemplate<size>	mauiStringLengths;
	size					muiFirstString;		//The generator's number for the first of them.
	bool					mbUnsupported;		//Something the front end can't write yet: the function is an error.
	bool					mbFloatReported;	//The function's floating point (CCPP65Generator::SetNoFloats) has been reported.
	const char*				mszUnsupportedFile;	//The front end's file (no directory) and line that last left a function to the generator.
	int						miUnsupportedLine;
	char					mszUnsupported[1024];	//The verifier's errors when that was IR that didn't verify.
	size					muiAttempts;		//Bodies asked for; only the first CCPPCompileOptions::muiFunctionLimit may be written.

public:
	void			Init(CCPP65Generator* pcGenerator);
	void			Kill(void);
	void			SetStatistics(CCPPCompileStatistics* pcStatistics);

	bool			Body(SCPP65Function* psFunction);
	bool			GlobalRoutine(const char* szLabel, const char* szGlobal, SCPP65Type* psType, CCPPSyntax* pcInitialiser, bool bFinalise);
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
	uint32			Fingerprint(CIRFunction* pcFunction);
	uint64			CallCount(CIRFunction* pcCaller, SIRInstruction* psCall);
	uint64			InlinedEntries(CAsm65816Profile* pcProfile, const char* szLabel, SIRInstruction* psCall);
	uint64			ProfileCycles(void);
	CAsm65816Profile*	InliningProfile(void);
	bool			SourceLine(int iToken, CSourceFile** ppcFile, int* piLine);
	CArrayChars*	GetMissingCallees(void);

protected:
	void			BeginWriting(SCPP65Function* psFunction, const char* szLabel, SIRType sReturn, uint32 uiAttributes);
	bool			FinishWriting(bool bResult, const char* szLabel);
	void			OptimiserWarnings(CChars* pszWarnings);
	bool			GenerateCode(CIRFunction* pcFunction, const char* szLabel, CChars* pszCode, CIRWrittenFunction** ppcWritten);
	void			FreeWritten(SCPP65IRWritten* psWritten);
	bool			Unsupported(const char* szFile, int iLine);
	SIRType			IRTypeOf(SCPP65Type* psType);
	void			NoFloats(void);
	SIRType			ReturnIRType(SCPP65Type* psType);
	bool			IsSupportedType(SCPP65Type* psType);
	SCPP65Type*		ElementType(SCPP65Type* psType);
	bool			IsPlaceType(SCPP65Type* psType);
	bool			IsLocalType(SCPP65Type* psType);
	bool			ReferenceAddress(SCPP65IRPlace* psPlace, SCPP65Type* psReference, SIRReference* psAddress);
	CIRBlock*		NewBlock(void);
	void			StartBlock(CIRBlock* pcBlock);
	void			JumpTo(CIRBlock* pcBlock);
	bool			Terminated(void);
	SIRReference	Integer(SCPP65Type* psType, int64 lliValue);
	SIRReference	Offset(int iBytes);
	SIRReference	AddressAt(SIRReference sAddress, int iOffset);
	bool			Parameters(void);
	void			AddPieces(int iBytes);
	int				PieceCount(int iBytes);
	int				PieceBytes(int iLeft);
	bool			Locals(void);
	void			Finish(void);
	bool			Statement(CCPPSyntax* pcStatement);
	bool			StatementOf(CCPPSyntax* pcStatement);
	bool			CompoundStatement(CCPPSyntaxCompoundStatement* pcCompound);
	bool			JumpStatement(CCPPSyntax* pcStatement, CIRBlock* pcBreak, CIRBlock* pcContinue);
	bool			ScopedStatement(CCPPSyntax* pcStatement);
	void			AddLiving(SCPP65IRPlace* psPlace);
	bool			LeaveScope(size uiLiving);
	bool			DestroyLiving(size uiLiving);
	bool			FullExpression(CCPPSyntax* pcExpression, SCPP65IRValue* psValue);
	bool			DestroyTemporaries(size uiTemporaries);
	bool			DestroyTemporaryPlaces(size uiFrom);
	bool			DestroyPlaces(CArrayCPP65IRPlaces* pasPlaces, size uiFrom);
	bool			DeclarationStatement(CCPPSyntaxDeclarationStatement* pcStatement);
	bool			Declaration(CCPPSyntaxSimpleDeclaration* pcDeclaration, SCPP65Variable** ppsLast);
	bool			LocalInitialiser(SCPP65IRPlace* psPlace, CCPPSyntax* pcInitialiser);
	bool			ArrayInitialiser(SCPP65IRPlace* psPlace, CCPPSyntax* pcInitialiser);
	bool			ArrayElements(SCPP65IRPlace* psPlace, CArraySyntax* papcElements, size* puiNext);
	void			ZeroElements(SIRReference sAddress, int iOffset, SCPP65Type* psElement, int iCount);
	bool			IfStatement(CCPPSyntaxIfStatement* pcIf);
	bool			WhileStatement(CCPPSyntaxWhileStatement* pcWhile);
	bool			DoStatement(CCPPSyntaxDoStatement* pcDo);
	bool			ForStatement(CCPPSyntaxForStatement* pcFor);
	bool			SwitchStatement(CCPPSyntaxSwitchStatement* pcSwitch);
	bool			FindCases(CCPPSyntax* pcNode, CIRBlock** ppcDefault);
	bool			CaseStatement(CCPPSyntaxCaseStatement* pcCase);
	bool			ReturnStatement(CCPPSyntaxReturnStatement* pcReturn);
	bool			BreakStatement(CCPPSyntax* pcStatement, bool bContinue);
	bool			GotoStatement(CCPPSyntaxGotoStatement* pcGoto);
	bool			LabelStatement(CCPPSyntaxLabelStatement* pcLabel);
	CIRBlock*		GotoBlock(int iToken);
	bool			Expression(CCPPSyntax* pcExpression, SCPP65IRValue* psValue);
	bool			Value(CCPPSyntax* pcExpression, SCPP65IRValue* psValue);
	bool			ValueFor(CCPPSyntax* pcExpression, SCPP65Type* psTo, SCPP65IRValue* psValue);
	bool			Load(SCPP65IRPlace* psPlace, SCPP65IRValue* psValue);
	void			Store(SCPP65IRPlace* psPlace, SCPP65IRValue* psValue);
	bool			ConstantValue(CCPPSyntax* pcExpression, SCPP65IRValue* psValue);
	bool			StringLiteral(CCPPSyntax* pcString, SCPP65IRValue* psValue);
	bool			ScalarConstruct(CCPPSyntaxConstructExpression* pcConstruct, SCPP65IRValue* psValue);
	bool			FunctionName(CCPPSyntaxIdExpression* pcId, SCPP65IRValue* psValue);
	bool			FunctionAddress(CCPPSyntaxIdExpression* pcId, SCPP65Type* psTarget, SCPP65IRValue* psValue);
	bool			MemberAddress(CCPPSyntaxIdExpression* pcId, SCPP65IRValue* psValue, bool* pbMember);
	bool			MemberFunction(SCPP65Class* psClass, int64 lliName, SCPP65Function** ppsFound, SCPP65Class** ppsClass);
	bool			MemberPointerObject(CCPPSyntaxBinaryExpression* pcBinary, SIRReference* psObject, SCPP65Type** ppsMember);
	bool			MemberPointerCall(CCPPSyntaxCallExpression* pcCall, CCPPSyntaxBinaryExpression* pcBinary, SCPP65IRValue* psValue);
	bool			MemberPointerPlace(CCPPSyntaxBinaryExpression* pcBinary, SCPP65IRPlace* psPlace);
	void			StringData(char* pcBytes, size uiLength, SCPP65IRValue* psValue);
	bool			Place(CCPPSyntax* pcExpression, SCPP65IRPlace* psPlace);
	bool			VariablePlace(SCPP65Variable* psVariable, SCPP65IRPlace* psPlace);
	bool			UnaryExpression(CCPPSyntaxUnaryExpression* pcUnary, SCPP65IRValue* psValue);
	bool			IncrementExpression(CCPPSyntaxUnaryExpression* pcUnary, SCPP65IRValue* psValue);
	bool			BinaryExpression(CCPPSyntaxBinaryExpression* pcBinary, SCPP65IRValue* psValue);
	bool			Assignment(CCPPSyntaxBinaryExpression* pcBinary, SCPP65IRValue* psValue, SCPP65IRPlace* psStored = NULL);
	bool			Operate(ECppTokens eOperator, SCPP65IRValue* psLeft, SCPP65IRValue* psRight, SCPP65IRValue* psResult);
	bool			IsClassPointerTo(SCPP65Type* psFrom, SCPP65Type* psTo);
	bool			Compare(ECppTokens eOperator, SCPP65IRValue* psLeft, SCPP65IRValue* psRight, SIRReference* psCondition);
	bool			ConditionalExpression(CCPPSyntaxConditionalExpression* pcConditional, SCPP65IRValue* psValue);
	bool			ObjectConditional(CCPPSyntaxConditionalExpression* pcConditional, SCPP65IRPlace* psInto, SCPP65IRValue* psValue);
	bool			IsObjectLvalue(CCPPSyntax* pcExpression);
	bool			IsObjectCall(CCPPSyntax* pcExpression);
	bool			IsObjectExpression(CCPPSyntax* pcExpression);
	bool			InheritedConstructorCall(SCPP65IRPlace* psBase);
	bool			ConditionValue(CCPPSyntax* pcExpression, SCPP65IRValue* psValue);
	bool			Condition(CCPPSyntax* pcExpression, CIRBlock* pcTrue, CIRBlock* pcFalse);
	bool			FullCondition(CCPPSyntax* pcExpression, CIRBlock* pcTrue, CIRBlock* pcFalse);
	bool			Truth(SCPP65IRValue* psValue, SIRReference* psCondition);
	bool			IsObjectValue(SCPP65IRValue* psValue);
	bool			ConvertObject(SCPP65IRValue* psValue, SCPP65Type* psTo);
	bool			CallExpression(CCPPSyntaxCallExpression* pcCall, SCPP65IRValue* psValue);
	bool			CallThroughPointer(CCPPSyntaxCallExpression* pcCall, CCPPSyntax* pcCallee, SCPP65IRValue* psValue);
	bool			ExpandPackCall(CCPPSyntaxCallExpression* pcCall, SCPP65IRValue* psValue);
	bool			GatherArguments(CArraySyntax* papcArguments, CArrayCPP65Types* papsParameters, CArraySyntax* papcDefaults, bool bVariadic, SCPP65Type* psReturn, SIRReference* psMade, CArrayTemplate<SCPP65IRValue>* pasArguments, CArrayIRReferences* pasLoaded, CArrayTemplate<int32>* paiLoaded);
	bool			Call(SCPP65Function* psFunction, SIRReference sCallee, SCPP65Type* psFunctionType, CArraySyntax* papcArguments, SCPP65IRValue* psValue, CCPPSyntax* pcObject = NULL, bool bArrow = false, SCPP65Class* psObjectClass = NULL, bool bVirtual = false, SIRReference sGivenObject = IRNone(), bool bObjectLast = false);
	bool			ArgumentPieces(CArraySyntax* papcArguments, CArrayCPP65Types* papsParameters, CArraySyntax* papcDefaults, bool bVariadic, SCPP65Type* psReturn, SIRReference* psMade, CArrayIRReferences* pasPieces);
	void			CallResult(SCPP65Type* psReturn, SIRReference sMade, SIRReference sResult, SCPP65IRValue* psValue);
	bool			ThisObject(SCPP65Function* psFunction, CCPPSyntax* pcObject, bool bArrow, SCPP65Class* psObjectClass, bool bVirtual, SIRReference sGivenObject, SIRReference* psObject, SIRReference* psCallee);
	bool			CastExpression(CCPPSyntaxTypeId* pcTypeId, CCPPSyntax* pcOperand, SCPP65IRValue* psValue);
	bool			DynamicCastExpression(CCPPSyntaxNamedCastExpression* pcCast, SCPP65IRValue* psValue);
	SIRReference	DynamicCast(SIRReference sAddress, SCPP65Class* psFrom, SCPP65Class* psTo);
	bool			Convert(SCPP65IRValue* psValue, SCPP65Type* psTo);
	bool			CountTable(bool bLeading, SCPP65IRValue* psArgument, SCPP65Type* psType, SCPP65IRValue* psValue);
	void			NarrowCount(bool bLeading, SIRReference* psValue, SIRReference* psCount, int* piBits, SIRType sInt);
	bool			Builtin(CCPPSyntaxCallExpression* pcCall, int64 lliName, SCPP65IRValue* psValue);
	bool			BuiltinMemcpy(CCPPSyntaxCallExpression* pcCall, SCPP65IRValue* psValue);
	bool			BuiltinBCD(CCPPSyntaxCallExpression* pcCall, bool bSubtract, SCPP65IRValue* psValue);
	bool			CallWithArguments(CCPPSyntaxCallExpression* pcCall, SCPP65Type** apsParameters, int iCount, const char* szSymbol, SCPP65Type* psResult, SCPP65IRValue* psValue);
	bool			VerifyIR(CIRFunction* pcFunction, const char* szFile, int iLine);
	SIRReference	ConvertIR(SIRReference sValue, SIRType sFrom, SIRType sTo, bool bSigned);
	SIRReference	BaseAddress(SIRReference sAddress, int iOffset);
	bool			ObjectAddress(CCPPSyntax* pcObject, bool bArrow, SIRReference* psAddress);
	bool			InlineMemcpy(CArraySyntax* papcArguments, SCPP65Type* psResult, SCPP65IRValue* psValue);
	bool			ConvertMemberPointer(SCPP65IRValue* psValue, SCPP65Type* psTo);
	SCPP65Type*		ArrowClass(CCPPSyntax* pcObject, SCPP65Type* psType);
	bool			MemberPlace(CCPPSyntaxMemberExpression* pcMember, SCPP65IRPlace* psPlace);
	bool			FieldPlace(SIRReference sObject, SCPP65Class* psClass, int64 lliName, CCPPSymbol* pcSymbol, SCPP65IRPlace* psPlace);
	bool			MemberCall(CCPPSyntaxCallExpression* pcCall, CCPPSyntax* pcCallee, SCPP65IRValue* psValue);
	bool			DestructorCall(CCPPSyntaxCallExpression* pcCall, SCPP65Class* psClass, CCPPSyntax* pcObject, bool bArrow, bool bVirtual, SCPP65IRValue* psValue);
	bool			OperatorCall(CCPPSyntax* pcObject, ECppTokens eOperator, CCPPSyntax* pcArgument, SCPP65IRValue* psValue);
	bool			OperatorExpression(ECppTokens eOperator, CCPPSyntax* pcLeft, CCPPSyntax* pcRight, SCPP65IRValue* psValue);
	bool			IsObjectSubscript(CCPPSyntaxSubscriptExpression* pcSubscript);
	bool			ArrayValue(CCPPSyntax* pcArray, SCPP65IRValue* psPointer, SCPP65Type** ppsArray);
	bool			NarrowSubscript(SCPP65Type* psArray, SCPP65IRValue* psPointer, SCPP65IRValue* psIndex, SCPP65IRValue* psAddress);
	void			PrintIR(CIRFunction* pcFunction);
	void			PrintFunction(CIRFunction* pcFunction, CChars* pszText, bool bSources);
	bool			ConstructorStart(CCPPSyntaxFunctionDefinition* pcFunction);
	bool			DestructorEnd(void);
	bool			StoreVtables(SIRReference sObject, SCPP65Class* psClass);
	void			DescribeVtable(CArrayTemplate<SCPP65VtableEntry>* pasEntries, SCPP65Class* psClass, uint32 uiSymbol);
	bool			Construct(SCPP65IRPlace* psPlace, CArraySyntax* papcArguments, CCPPSyntax* pcInitialiser);
	bool			ConstructClass(SCPP65IRPlace* psPlace, SCPP65Class* psClass, CArraySyntax* papcArguments);
	bool			Destroy(SIRReference sAddress, SCPP65Type* psType);
	SIRReference	Phi(CIRBlock* pcBlock, SIRType sType, CIRBlock* pcFrom1, SIRReference sValue1, CIRBlock* pcFrom2, SIRReference sValue2);
	SIRReference	LoopPhi(CIRBlock* pcLoop, SIRType sType, CIRBlock* pcEntry, SIRReference sStart);
	void			ClosePhi(CIRBlock* pcLoop, SIRReference sPhi, CIRBlock* pcLatch, SIRReference sNext);
	SIRReference	BeginCountedLoop(SCPP65IRCountedLoop* psLoop, SIRType sType, SIRReference sStart, uint16 uiTest, SIRReference sLimit);
	void			EndCountedLoop(SCPP65IRCountedLoop* psLoop, SIRReference sNext);
	SIRReference	FunctionSymbol(SCPP65Function* psFunction);
	void			MarkFunctionSymbol(SCPP65Function* psFunction, uint32 uiSymbol);
	bool			FunctionTemplateAddress(CCPPSyntaxIdExpression* pcId, SCPP65IRValue* psValue);
	bool			CallObject(SCPP65Function* psFunction, SIRReference sObject, SCPP65Class* psObjectClass, CArraySyntax* papcArguments, bool bVirtual);
	SIRReference	CallRuntime(const char* szName, SIRType sResult, SIRType sArgument, SIRReference sValue);
	bool			NewExpression(CCPPSyntaxNewExpression* pcNew, SCPP65IRValue* psValue);
	bool			NewArray(CCPPSyntaxNewExpression* pcNew, SCPP65Type* psType, SCPP65IRValue* psValue);
	bool			DeleteArray(SCPP65IRValue* psPointer, SCPP65Class* psClass);
	bool			DeleteExpression(CCPPSyntaxDeleteExpression* pcDelete, SCPP65IRValue* psValue);
	bool			ObjectValue(CCPPSyntax* pcExpression, SCPP65IRValue* psValue);
	bool			Temporary(SCPP65Type* psType, SCPP65IRPlace* psPlace, bool bDestroy);
	bool			ValueTemporary(CCPPSyntax* pcValue, SCPP65Type* psType, SCPP65IRPlace* psPlace);
	bool			CopyConstruct(SCPP65IRPlace* psPlace, CCPPSyntax* pcSource);
	bool			CopyObject(SCPP65IRPlace* psPlace, CCPPSyntax* pcSource);
	bool			CopyBytes(SCPP65IRPlace* psPlace, SCPP65IRValue* psSource);
	bool			CopyPlace(SCPP65IRPlace* psPlace, SCPP65IRPlace* psSource, SIRReference* psAddress);
	bool			CopyMembers(SCPP65IRPlace* psPlace, CCPPSyntax* pcSource);
	bool			CopyParts(SCPP65Class* psClass, SIRReference sDestination, SIRReference sSource);
	bool			CopyPart(SCPP65Class* psClass, SIRReference sDestination, SIRReference sSource);
	SCPP65Type*		FunctionType(SCPP65Function* psFunction);
	int				BaseOffset(SCPP65Class* psDerived, SCPP65Class* psBase);
	void			CopyPieces(SIRReference sDest, SIRReference sSource, int iBytes);
	void			ZeroBytes(SIRReference sAddress, int iBytes);
	bool			IsPassedByAddress(SCPP65Type* psType);
};


#endif // __C_P_P_65_I_R_FRONT_END_H__
