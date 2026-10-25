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
#ifndef __C_P_P_65_I_R_FRONT_END_H__
#define __C_P_P_65_I_R_FRONT_END_H__
#include "BaseLib/MapLongTemplate.h"
#include "BaseLib/ArrayUint8.h"
#include "IRModule.h"
#include "CPPSyntaxes.h"
#include "CPP65Type.h"


class CCPP65Generator;
struct SCPP65Function;
struct SCPP65Variable;


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
	CChars			szCode;
};


#define IR65_FRONT_END_MARKER			"\x01"	//Around a written function's index in the generator's code.
#define IR65_INLINE_INSTRUCTIONS		16		//The most instructions a function inlined may have...
#define IR65_INLINE_LOOP_INSTRUCTIONS	32		//...or where the call is in a loop...
#define IR65_INLINE_CALLS				24		//...and the most calls inlined into one function at a time...
#define IR65_INLINE_ROUNDS				4		//...in at most this many rounds (optimising between them).


typedef CMapLongTemplate<SIRReference>		CMapCPP65IRAddresses;
typedef CArrayTemplate<SCPP65IRJump>		CArrayCPP65IRJumps;
typedef CArrayTemplate<SCPP65IRCase>		CArrayCPP65IRCases;
typedef CArrayTemplate<SCPP65IRLabel>		CArrayCPP65IRLabels;
typedef CArrayTemplate<SCPP65IRPlace>		CArrayCPP65IRPlaces;
typedef CArrayTemplate<SCPP65IRWritten>		CArrayCPP65IRWritten;


//Writes function bodies as IR for the 65C64 generator and generates them with CIR65Backend (IR.txt step 2).  The
//generator works out everything about the C++ (types, names, overloads, layouts) and sets up the function as it
//does for its own code; Body then writes the function as IR if it supports everything in it, or returns false
//(having changed nothing) so the generator writes it itself.  Locals are slots, read and written with load and
//store; parameters are where their caller pushed them (argaddr).  What is supported is in IR.txt (Front end).
class CCPP65IRFrontEnd
{
protected:
	CCPP65Generator*		mpcGenerator;
	CCPP65Types*			mpcTypes;

	CIRModule				mcModule;			//The unit's functions written as IR.
	CArrayCPP65IRWritten	masWritten;			//The same, in the order written.
	CIRFunction*			mpcFunction;
	CIRBlock*				mpcBlock;
	SCPP65Function*			mpsFunction;
	SIRReference			msThis;				//A member function's this, IRNone in others.
	SIRReference			msResult;			//Where a function that returns an object makes it, IRNone in others.
	SIRReference			msElide;			//Where the next call that returns an object makes it, instead of a temporary.
	CMapCPP65IRAddresses	mcAddresses;		//Each local's and parameter's address, by its SCPP65Variable.
	CArrayCPP65IRJumps		masJumps;
	CArrayCPP65IRCases		masCases;
	CArrayCPP65IRLabels		masLabels;
	CArrayCPP65IRPlaces		masLiving;			//The objects that need destroying in the scopes being written, oldest first.
	CArrayCPP65IRPlaces		masTemporaries;		//The temporaries to destroy at the end of the full expression, oldest first.
	int						miConditional;		//Inside an operand that may not be worked out (of &&, || or ?:).
	CIRBlock*				mpcExit;			//A destructor's last block (it destroys the fields and bases); NULL in others.
	CArraySyntax			mapcNone;			//No arguments.
	CArrayChars				maszRuntime;		//Runtime routines called, given to the generator if the function is written.
	CArrayUint8				mauiStringBytes;	//String literals, given to the generator if the function is written.
	CArrayTemplate<size>	mauiStringLengths;
	size					muiFirstString;		//The generator's number for the first of them.
	bool					mbUnsupported;		//Something the front end can't write yet: the generator writes the function.

public:
	static size		suiWritten;				//Functions written as IR, and left to the generator, by every front end.
	static size		suiLeft;
	static int		siUnsupportedLine;		//The line of CPP65IRFrontEnd.cpp that last left a function to the generator.
	static char		sszUnsupported[1024];	//The verifier's errors when that was IR that didn't verify.
	static size		sauiUnsupported[8192];	//How many functions each line of CPP65IRFrontEnd.cpp left to the generator.
	static bool		sbPrint;				//Print each function's IR to stderr, for debugging.
	static bool		sbOptimise;				//Run the mid level passes (CIROptimiser).
	static bool		sbInline;				//Inline small functions across the unit (CIRInliner).
	static size		suiAttempts;			//Bodies asked for; only the first suiLimit may be written (to bisect a
	static size		suiLimit;				//wrong one).

	void			Init(CCPP65Generator* pcGenerator);
	void			Kill(void);

	bool			Body(SCPP65Function* psFunction);
	bool			GlobalRoutine(const char* szLabel, const char* szGlobal, SCPP65Type* psType, CCPPSyntax* pcInitialiser, bool bFinalise);
	void			FinishUnit(void);

protected:
	void			BeginWriting(SCPP65Function* psFunction, const char* szLabel, SIRType sReturn, uint32 uiAttributes);
	bool			FinishWriting(bool bResult, const char* szLabel);
	bool			GenerateCode(CIRFunction* pcFunction, const char* szLabel, CChars* pszCode);
	bool			Unsupported(int iLine);
	SIRType			IRTypeOf(SCPP65Type* psType);
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
	bool			DestroyTemporaries(size uiTemporaries);
	bool			DestroyPlaces(CArrayCPP65IRPlaces* pasPlaces, size uiFrom);
	bool			DeclarationStatement(CCPPSyntaxDeclarationStatement* pcStatement);
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
	bool			BreakStatement(bool bContinue);
	bool			GotoStatement(CCPPSyntaxGotoStatement* pcGoto);
	bool			LabelStatement(CCPPSyntaxLabelStatement* pcLabel);
	CIRBlock*		GotoBlock(int iToken);
	bool			Expression(CCPPSyntax* pcExpression, SCPP65IRValue* psValue);
	bool			Value(CCPPSyntax* pcExpression, SCPP65IRValue* psValue);
	bool			Load(SCPP65IRPlace* psPlace, SCPP65IRValue* psValue);
	void			Store(SCPP65IRPlace* psPlace, SCPP65IRValue* psValue);
	bool			ConstantValue(CCPPSyntax* pcExpression, SCPP65IRValue* psValue);
	bool			StringLiteral(CCPPSyntax* pcString, SCPP65IRValue* psValue);
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
	bool			Assignment(CCPPSyntaxBinaryExpression* pcBinary, SCPP65IRValue* psValue);
	bool			Operate(ECppTokens eOperator, SCPP65IRValue* psLeft, SCPP65IRValue* psRight, SCPP65IRValue* psResult);
	bool			Compare(ECppTokens eOperator, SCPP65IRValue* psLeft, SCPP65IRValue* psRight, SIRReference* psCondition);
	bool			ConditionalExpression(CCPPSyntaxConditionalExpression* pcConditional, SCPP65IRValue* psValue);
	bool			ConditionValue(CCPPSyntax* pcExpression, SCPP65IRValue* psValue);
	bool			Condition(CCPPSyntax* pcExpression, CIRBlock* pcTrue, CIRBlock* pcFalse);
	bool			Truth(SCPP65IRValue* psValue, SIRReference* psCondition);
	bool			CallExpression(CCPPSyntaxCallExpression* pcCall, SCPP65IRValue* psValue);
	bool			CallThroughPointer(CCPPSyntaxCallExpression* pcCall, CCPPSyntax* pcCallee, SCPP65IRValue* psValue);
	bool			ExpandPackCall(CCPPSyntaxCallExpression* pcCall, SCPP65IRValue* psValue);
	bool			Call(SCPP65Function* psFunction, SIRReference sCallee, SCPP65Type* psFunctionType, CArraySyntax* papcArguments, SCPP65IRValue* psValue, CCPPSyntax* pcObject = NULL, bool bArrow = false, SCPP65Class* psObjectClass = NULL, bool bVirtual = false, SIRReference sGivenObject = IRNone());
	bool			CastExpression(CCPPSyntaxTypeId* pcTypeId, CCPPSyntax* pcOperand, SCPP65IRValue* psValue);
	bool			Convert(SCPP65IRValue* psValue, SCPP65Type* psTo);
	bool			Builtin(CCPPSyntaxCallExpression* pcCall, int64 lliName, SCPP65IRValue* psValue);
	SIRReference	ConvertIR(SIRReference sValue, SIRType sFrom, SIRType sTo, bool bSigned);
	SIRReference	BaseAddress(SIRReference sAddress, int iOffset);
	bool			ObjectAddress(CCPPSyntax* pcObject, bool bArrow, SIRReference* psAddress);
	bool			InlineMemcpy(CArraySyntax* papcArguments, SCPP65Type* psResult, SCPP65IRValue* psValue);
	bool			ConvertMemberPointer(SCPP65IRValue* psValue, SCPP65Type* psTo);
	SCPP65Type*		ArrowClass(CCPPSyntax* pcObject, SCPP65Type* psType);
	bool			MemberPlace(CCPPSyntaxMemberExpression* pcMember, SCPP65IRPlace* psPlace);
	bool			FieldPlace(SIRReference sObject, SCPP65Class* psClass, int64 lliName, CCPPSymbol* pcSymbol, SCPP65IRPlace* psPlace);
	bool			MemberCall(CCPPSyntaxCallExpression* pcCall, CCPPSyntax* pcCallee, SCPP65IRValue* psValue);
	bool			OperatorCall(CCPPSyntax* pcObject, ECppTokens eOperator, CCPPSyntax* pcArgument, SCPP65IRValue* psValue);
	bool			OperatorExpression(ECppTokens eOperator, CCPPSyntax* pcLeft, CCPPSyntax* pcRight, SCPP65IRValue* psValue);
	bool			IsObjectSubscript(CCPPSyntaxSubscriptExpression* pcSubscript);
	void			PrintIR(CIRFunction* pcFunction);
	bool			ConstructorStart(CCPPSyntaxFunctionDefinition* pcFunction);
	bool			DestructorEnd(void);
	bool			StoreVtables(SIRReference sObject, SCPP65Class* psClass);
	void			DescribeVtable(SCPP65Class* psClass, uint32 uiSymbol);
	void			AddVtableEntry(CIRGlobal* pcVtable, SCPP65Function* psFunction, int iAdjust);
	bool			Construct(SCPP65IRPlace* psPlace, CArraySyntax* papcArguments, CCPPSyntax* pcInitialiser);
	bool			ConstructClass(SCPP65IRPlace* psPlace, SCPP65Class* psClass, CArraySyntax* papcArguments);
	bool			Destroy(SIRReference sAddress, SCPP65Type* psType);
	SIRReference	Phi(CIRBlock* pcBlock, SIRType sType, CIRBlock* pcFrom1, SIRReference sValue1, CIRBlock* pcFrom2, SIRReference sValue2);
	SIRReference	FunctionSymbol(SCPP65Function* psFunction);
	bool			FunctionTemplateAddress(CCPPSyntaxIdExpression* pcId, SCPP65IRValue* psValue);
	bool			CallObject(SCPP65Function* psFunction, SIRReference sObject, SCPP65Class* psObjectClass, CArraySyntax* papcArguments, bool bVirtual);
	SIRReference	CallRuntime(const char* szName, SIRType sResult, SIRType sArgument, SIRReference sValue);
	bool			NewExpression(CCPPSyntaxNewExpression* pcNew, SCPP65IRValue* psValue);
	bool			NewArray(CCPPSyntaxNewExpression* pcNew, SCPP65Type* psType, SCPP65IRValue* psValue);
	bool			DeleteArray(SCPP65IRValue* psPointer, SCPP65Class* psClass);
	bool			DeleteExpression(CCPPSyntaxDeleteExpression* pcDelete, SCPP65IRValue* psValue);
	bool			ObjectValue(CCPPSyntax* pcExpression, SCPP65IRValue* psValue);
	bool			Temporary(SCPP65Type* psType, SCPP65IRPlace* psPlace, bool bDestroy);
	bool			CopyConstruct(SCPP65IRPlace* psPlace, CCPPSyntax* pcSource);
	bool			CopyObject(SCPP65IRPlace* psPlace, CCPPSyntax* pcSource);
	bool			CopyBytes(SCPP65IRPlace* psPlace, SCPP65IRValue* psSource);
	bool			CopyMembers(SCPP65IRPlace* psPlace, CCPPSyntax* pcSource);
	void			CopyParts(SCPP65Class* psClass, SIRReference sDestination, SIRReference sSource);
	void			CopyPart(SCPP65Class* psClass, SIRReference sDestination, SIRReference sSource);
	SCPP65Type*		FunctionType(SCPP65Function* psFunction);
	int				BaseOffset(SCPP65Class* psDerived, SCPP65Class* psBase);
	void			CopyPieces(SIRReference sDest, SIRReference sSource, int iBytes);
	void			ZeroBytes(SIRReference sAddress, int iBytes);
};


#endif // __C_P_P_65_I_R_FRONT_END_H__
