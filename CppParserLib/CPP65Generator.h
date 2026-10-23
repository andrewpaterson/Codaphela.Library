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
#ifndef __C_P_P_65_GENERATOR_H__
#define __C_P_P_65_GENERATOR_H__
//The memory map (the user, 2026-10-23): 0000:0000 to 0000:FFFF are pseudo registers and
//scratch memory, programs start at 0001:0000 and the stack works down from 001F:FFFC.
#define CPP65_PROGRAM_ADDRESS		0x10000
#define CPP65_STACK_TOP				0x1FFFFF
#define CPP65_TEMPORARY_BYTES		0x100
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayUint8.h"
#include "CPPSyntaxes.h"
#include "CPPSymbol.h"
#include "CPP65Type.h"
#include "CPPErrors.h"


enum ECPP65Storage
{
	CPP65S_Local,
	CPP65S_Parameter,
	CPP65S_Global,
};


struct SCPP65Class;
struct SCPP65Function;


//A variable the generator has given storage.
struct SCPP65Variable
{
	CCPPSymbol*		pcSymbol;
	SCPP65Type*		psType;
	ECPP65Storage	eStorage;
	int				iOffset;		//Locals: bytes from the bottom of the frame.  Parameters: bytes after the first argument's.
	SCPP65Class*	psClass;		//The class of a static data member.
	bool			bDefined;		//A global's data has been written.
};


enum ECPP65FunctionKind
{
	CPP65FK_Function,
	CPP65FK_Constructor,
	CPP65FK_Destructor,
};


//A function's signature, from its definition or a declaration.
struct SCPP65Function
{
	int64				lliName;
	SCPP65Type*			psReturn;
	CArrayCPP65Types	mapsParameters;
	bool				bDefined;
	SCPP65Class*		psClass;		//NULL for a function that is not a member.
	ECPP65FunctionKind	eKind;
	bool				bStatic;
	bool				bVirtual;
	bool				bPure;
	int					iSlot;			//A virtual function's place in the vtable.
	int					iOverload;		//Other functions with the same name before this one.
	bool				bImplicit;		//A constructor or destructor the generator wrote.
};


typedef CArrayTemplatePtr<SCPP65Function>	CArrayCPP65FunctionPtrs;


//A non-static data member.  Offsets include the base class's.
struct SCPP65Field
{
	CCPPSymbol*		pcSymbol;
	int64			lliName;
	SCPP65Type*		psType;
	int				iOffset;
	CCPPSyntax*		pcInitialiser;		//The default member initialiser, or NULL.
};


typedef CArrayTemplate<SCPP65Field>		CArrayCPP65Fields;


//A class's layout: the base class first, then the vtable pointer if the class is the first polymorphic one,
//then the fields.
struct SCPP65Class
{
	CCPPSymbol*					pcSymbol;
	int64						lliName;
	SCPP65Type*					psType;
	SCPP65Class*				psBase;
	CArrayCPP65Fields			macFields;
	CArrayCPP65FunctionPtrs		mapsVirtuals;		//The vtable.
	int							iVtableOffset;		//-1 if the class is not polymorphic.
	bool						bComplete;
	bool						bConstructors;		//The class declares a constructor.
	SCPP65Function*				psDestructor;
};


typedef CArrayTemplatePtr<SCPP65Class>		CArrayCPP65Classes;


enum ECPP65PlaceKind
{
	CPP65PK_Frame,		//[offset + V].
	CPP65PK_Global,		//[label + offset].
	CPP65PK_Address,	//The address is in A.
	CPP65PK_This,		//[offset + X], a member of this.
};


//Where an lvalue is.
struct SCPP65Place
{
	ECPP65PlaceKind		eKind;
	SCPP65Type*			psType;
	int					iOffset;	//Frame: bytes above V + 1.  Global: bytes after the label.  This and Address: bytes after the object.
	int64				lliName;	//Global: the variable's name.  -1 for a string literal.
	int					iString;	//Global: the string literal's number.
	SCPP65Class*		psClass;	//Global: the class of a static data member.
};


enum ECPP65RightKind
{
	CPP65RK_B,
	CPP65RK_Constant,
	CPP65RK_Temporary,		//[offset + W], a pseudo register.
};


//The right operand of a binary operator: in B, a constant or a pseudo register, used directly.
struct SCPP65Right
{
	SCPP65Type*		psType;
	ECPP65RightKind	eKind;
	int64			lliValue;	//The constant, or the pseudo register's offset from W.
};


//The labels break and continue go to.  iContinue is -1 in a switch.  iDestructors is the number of objects
//to destroy that were made before the loop or switch.
struct SCPP65Jump
{
	int		iBreak;
	int		iContinue;
	int		iDestructors;
};


//A case or default label and the assembly label it was given.
struct SCPP65Case
{
	CCPPSyntaxCaseStatement*	pcCase;
	int							iLabel;
};


typedef CArrayTemplatePtr<SCPP65Variable>	CArrayCPP65Variables;
typedef CArrayTemplate<SCPP65Jump>			CArrayCPP65Jumps;
typedef CArrayTemplate<SCPP65Case>			CArrayCPP65Cases;
typedef CArrayTemplate<CChars>				CArrayCPP65Strings;


//Generates 65C64 assembly (Assembler65Lib syntax) from a parsed translation unit.  The calling convention and
//the parts of C++ that are supported are in CPP65Generator.txt.
class CCPP65Generator
{
protected:
	CCPPSyntaxTree*			mpcTree;
	CCPPTokeniser*			mpcTokeniser;
	CCPP65Types				mcTypes;

	CChars					mszCode;
	CChars					mszData;
	CChars					mszBody;
	CChars*					mpszOut;
	CCPPErrors				mcErrors;

	CArrayCPP65Variables	mapsGlobals;
	CArrayCPP65FunctionPtrs	mapsFunctions;
	CArrayCPP65Classes		mapsClasses;
	CArrayCPP65Strings		maszStrings;
	CChars					mszInitialise;		//Constructs the global objects (R_Initialise).
	CChars					mszFinalise;		//Destroys them (R_Finalise).
	bool					mbPureVirtual;

	CArrayCPP65Variables	mapsLocals;
	CArrayCPP65Variables	mapsDestructors;	//Local objects to destroy, last made last.
	CArrayCPP65Jumps		macJumps;
	CArrayCPP65Cases		macCases;
	SCPP65Function*			mpsFunction;
	int						miFrameBytes;
	bool					mbFramePointer;		//The function saves V and points it at its frame.
	int						miDepth;			//Bytes pushed on S since the frame was made.
	int						miTemporaries;		//Bytes of pseudo registers ([offset + W]) in use.
	int						miReturnLabel;
	int						miThisOffset;		//this's place in the frame of a member function, or -1.
	bool					mbThisInX;
	int						miLabels;
	bool					mbSignedDivide;
	bool					mbSignedModulus;

public:
	void					Init(CCPPSyntaxTree* pcTree);
	void					Kill(void);

	bool					Generate(CChars* pszDest);
	bool					GenerateProgram(CChars* pszDest);
	CCPPErrors*				GetErrors(void);
	int						NumErrors(void);

protected:
	//Declarations, functions and statements.  CPP65Generator.cpp
	bool					Error(CCPPSyntax* pcNode, const char* szMessage);
	bool					ErrorAt(int iToken, const char* szMessage);
	CChars*					Line(const char* szText);
	void					Label(int iLabel);
	void					AppendLabel(CChars* psz, int iLabel);
	void					AppendName(CChars* psz, int64 lliName);
	int						NewLabel(void);
	int64					GetName(CCPPSyntaxDeclarator* pcDeclarator);

	SCPP65Function*			DeclareFunction(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, bool bDefinition, SCPP65Class* psClass, bool bPure);
	SCPP65Function*			AddFunction(SCPP65Class* psClass, int64 lliName, ECPP65FunctionKind eKind, SCPP65Type* psReturn);
	bool					SameParameters(SCPP65Function* psFunction, SCPP65Function* psOther);
	bool					Declarations(CArraySyntax* papcDeclarations);
	bool					GlobalDeclaration(CCPPSyntaxSimpleDeclaration* pcDeclaration);
	bool					GlobalInitialiser(SCPP65Type* psType, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode);
	void					AppendData(SCPP65Type* psType, int64 lliValue);
	void					AppendZeros(int iBytes);
	void					AppendGlobalLabel(CChars* psz, SCPP65Class* psClass, int64 lliName);
	SCPP65Variable*			AddVariable(CArrayCPP65Variables* papsVariables, CCPPSymbol* pcSymbol, SCPP65Type* psType, ECPP65Storage eStorage, int iOffset);
	void					FreeVariables(CArrayCPP65Variables* papsVariables);
	SCPP65Variable*			GetVariable(CCPPSymbol* pcSymbol);
	SCPP65Type*				GetType(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, CCPPSyntax* pcNode, CCPPSyntax* pcInitialiser = NULL);
	int						GetInitialiserCount(CCPPSyntax* pcInitialiser);
	SCPP65Type*				GetBaseType(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntax* pcNode);
	SCPP65Type*				GetTypeIdType(CCPPSyntaxTypeId* pcTypeId);
	SCPP65Type*				GetSymbolType(CCPPSymbol* pcSymbol, CCPPSyntax* pcNode);

	bool					Function(CCPPSyntaxFunctionDefinition* pcFunction, SCPP65Class* psClass);
	void					BeginFunction(SCPP65Function* psFunction);
	void					EndFunction(SCPP65Function* psFunction);
	bool					AllocateLocals(CCPPSyntax* pcNode);
	bool					Statement(CCPPSyntax* pcStatement);
	bool					CompoundStatement(CCPPSyntaxCompoundStatement* pcCompound);
	bool					DeclarationStatement(CCPPSyntaxDeclarationStatement* pcStatement);
	bool					LocalInitialiser(SCPP65Variable* psVariable, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode);
	bool					IfStatement(CCPPSyntaxIfStatement* pcIf);
	bool					WhileStatement(CCPPSyntaxWhileStatement* pcWhile);
	bool					DoStatement(CCPPSyntaxDoStatement* pcDo);
	bool					ForStatement(CCPPSyntaxForStatement* pcFor);
	bool					SwitchStatement(CCPPSyntaxSwitchStatement* pcSwitch);
	bool					FindCases(CCPPSyntax* pcNode, int* piDefault);
	bool					CaseStatement(CCPPSyntaxCaseStatement* pcCase);
	bool					ReturnStatement(CCPPSyntaxReturnStatement* pcReturn);
	bool					BreakStatement(CCPPSyntax* pcStatement, bool bContinue);
	void					DestroyLocals(int iFirst);
	SCPP65Jump*				PushJump(int iBreak, int iContinue);
	bool					ConditionDeclaration(CCPPSyntax* pcCondition);
	void					AppendStrings(CChars* pszDest);
	void					AppendRuntime(void);

	//Classes.  CPP65GeneratorClasses.cpp
	SCPP65Class*			GetClass(CCPPSymbol* pcSymbol, CCPPSyntax* pcNode);
	SCPP65Class*			FindClass(int64 lliName);
	SCPP65Class*			DefineClass(CCPPSyntaxClassSpecifier* pcSpecifier);
	bool					ClassMember(SCPP65Class* psClass, CCPPSyntax* pcMember, int* piBytes, CArraySyntax* papcBodies);
	bool					LayoutVirtuals(SCPP65Class* psClass, CCPPSyntax* pcNode);
	void					AppendVtable(SCPP65Class* psClass);
	SCPP65Field*			FindField(SCPP65Class* psClass, int64 lliName);
	SCPP65Field*			FindFieldSymbol(SCPP65Class* psClass, CCPPSymbol* pcSymbol);
	SCPP65Variable*			FindStatic(SCPP65Class* psClass, int64 lliName);
	SCPP65Function*			FindFunction(SCPP65Class* psClass, int64 lliName, ECPP65FunctionKind eKind, CArraySyntax* papcArguments, bool bBases, CCPPSyntax* pcNode);
	bool					IsDerived(SCPP65Class* psClass, SCPP65Class* psBase);
	bool					NeedsConstructor(SCPP65Class* psClass);
	bool					NeedsDestructor(SCPP65Class* psClass);
	void					AppendFunctionLabel(CChars* psz, SCPP65Function* psFunction);
	void					AppendClassLabel(CChars* psz, SCPP65Class* psClass, const char* szPart);
	bool					ImplicitMembers(SCPP65Class* psClass);
	bool					ConstructorBody(SCPP65Function* psFunction, CCPPSyntaxFunctionDefinition* pcFunction);
	bool					DestructorBody(SCPP65Function* psFunction);
	bool					Construct(SCPP65Place* psPlace, CArraySyntax* papcArguments, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode);
	bool					ConstructClass(SCPP65Place* psPlace, SCPP65Class* psClass, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	bool					Destroy(SCPP65Place* psPlace);
	void					Call(SCPP65Function* psFunction, bool bVirtual, int iBytes);
	bool					CallMember(SCPP65Function* psFunction, SCPP65Place* psObject, CArraySyntax* papcArguments, bool bVirtual, CCPPSyntax* pcNode);
	SCPP65Type*				MemberCall(CCPPSyntaxCallExpression* pcCall, CCPPSyntax* pcCallee);
	int						PushArguments(SCPP65Function* psFunction, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	bool					CopyObject(SCPP65Place* psDestination, CCPPSyntax* pcSource, CCPPSyntax* pcNode);
	bool					MemberPlace(CCPPSyntaxMemberExpression* pcMember, SCPP65Place* psPlace);
	bool					SetVtable(SCPP65Place* psPlace, SCPP65Class* psClass);
	void					EnsureThis(void);
	void					ChangeX(void);
	int64					GetComponentName(CCPPSyntaxNameComponent* pcComponent);

	//Expressions.  CPP65GeneratorExpressions.cpp
	SCPP65Type*				Expression(CCPPSyntax* pcExpression);
	SCPP65Type*				Value(CCPPSyntax* pcExpression);
	SCPP65Type*				Literal(CCPPSyntaxLiteralExpression* pcLiteral);
	SCPP65Type*				SimpleValue(CCPPSyntax* pcExpression, const char* szRegister);
	SCPP65Type*				StringLiteral(CCPPSyntaxStringExpression* pcString);
	SCPP65Type*				IdExpression(CCPPSyntaxIdExpression* pcId);
	SCPP65Type*				UnaryExpression(CCPPSyntaxUnaryExpression* pcUnary);
	SCPP65Type*				IncrementExpression(CCPPSyntaxUnaryExpression* pcUnary);
	SCPP65Type*				BinaryExpression(CCPPSyntaxBinaryExpression* pcBinary);
	SCPP65Type*				Assignment(CCPPSyntaxBinaryExpression* pcBinary);
	SCPP65Type*				ConditionalExpression(CCPPSyntaxConditionalExpression* pcConditional);
	SCPP65Type*				ConditionValue(CCPPSyntax* pcExpression);
	SCPP65Type*				CallExpression(CCPPSyntaxCallExpression* pcCall);
	SCPP65Type*				ThisExpression(CCPPSyntax* pcThis);
	SCPP65Type*				CastExpression(CCPPSyntaxTypeId* pcTypeId, CCPPSyntax* pcOperand, CCPPSyntax* pcNode);
	SCPP65Type*				SizeofExpression(CCPPSyntaxSizeofExpression* pcSizeof);
	SCPP65Type*				TypeOf(CCPPSyntax* pcExpression);
	SCPP65Type*				Operands(CCPPSyntax* pcLeft, CCPPSyntax* pcRight, SCPP65Right* psRight);
	SCPP65Type*				Operate(ECppTokens eOperator, SCPP65Type* psLeft, SCPP65Right* psRight, CCPPSyntax* pcNode);
	bool					Compare(ECppTokens eOperator, SCPP65Type* psLeft, SCPP65Right* psRight, bool bJumpIf, int iLabel, CCPPSyntax* pcNode);
	void					RightInB(SCPP65Right* psRight);
	CChars*					AppendRight(CChars* psz, SCPP65Right* psRight);
	bool					Condition(CCPPSyntax* pcExpression, bool bJumpIf, int iLabel);
	bool					Convert(SCPP65Type* psFrom, SCPP65Type* psTo, CCPPSyntax* pcNode);
	void					Truncate(const char* szRegister, int iBytes, bool bSigned);
	void					Scale(const char* szRegister, int iBytes);

	bool					Place(CCPPSyntax* pcExpression, SCPP65Place* psPlace);
	bool					VariablePlace(SCPP65Variable* psVariable, SCPP65Place* psPlace);
	void					AppendPlace(CChars* psz, SCPP65Place* psPlace, const char* szAddress);
	void					Load(SCPP65Place* psPlace, const char* szRegister, const char* szAddress);
	void					Store(SCPP65Place* psPlace, const char* szRegister, const char* szAddress);
	void					LoadAddress(SCPP65Place* psPlace);
	void					Push(const char* szRegister);
	void					PushValue(SCPP65Type* psType);
	void					Multiply(const char* szInstruction);
	void					Pull(const char* szRegister);

	bool					Constant(CCPPSyntax* pcExpression, int64* plliValue);
	bool					EnumeratorValue(CCPPSymbol* pcSymbol, int64* plliValue);
	bool					IsSimple(CCPPSyntax* pcExpression);
	bool					ContainsCall(CCPPSyntax* pcNode);
	CCPPSyntax*				SkipParentheses(CCPPSyntax* pcExpression);
	bool					DecodeString(CCPPSyntax* pcString, CArrayUint8* pauiBytes);
};


#endif // __C_P_P_65_GENERATOR_H__
