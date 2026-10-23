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
#include "BaseLib/ArrayChars.h"
#include "BaseLib/ArrayInt.h"
#include "BaseLib/MapLongTemplate.h"
#include "CPPSyntaxes.h"
#include "CPPSymbol.h"
#include "CPP65Type.h"
#include "CPPErrors.h"
#include "Config.h"


enum ECPP65Storage
{
	CPP65S_Local,
	CPP65S_Parameter,
	CPP65S_Global,
	CPP65S_Static,		//A static local: data, numbered in the unit by iOffset.
};


struct SCPP65Class;
struct SCPP65Function;
class CCTString;
struct SCPP65Template;


//A template parameter's value in an instantiation: a type, or a constant for a non-type parameter.
struct SCPP65Binding
{
	CCPPSymbol*		pcParameter;
	SCPP65Type*		psType;
	int64			lliValue;
};


typedef CArrayTemplate<SCPP65Binding>	CArrayCPP65Bindings;


//The template arguments of an instantiation.  psOuter is the class instance a member template is in.  psClass
//is the instance the bindings belong to, for the class's own name used without arguments inside it.
struct SCPP65Bindings
{
	CArrayCPP65Bindings		macBindings;
	SCPP65Bindings*			psOuter;
	SCPP65Class*			psClass;
};


typedef CArrayTemplatePtr<SCPP65Bindings>	CArrayCPP65BindingsPtrs;


//A variable the generator has given storage.
struct SCPP65Variable
{
	CCPPSymbol*		pcSymbol;
	SCPP65Type*		psType;
	ECPP65Storage	eStorage;
	int				iOffset;		//Locals: bytes from the bottom of the frame.  Parameters: bytes after the first argument's.
	SCPP65Class*	psClass;		//The class of a static data member.
	bool			bInternal;		//A static global: the unit's own.
	bool			bDefined;		//A global's data has been written.
	bool			bUsed;
};


enum ECPP65FunctionKind
{
	CPP65FK_Function,
	CPP65FK_Constructor,
	CPP65FK_Destructor,
};


//A function's signature, from its definition or a declaration, and its definition.  Inline, in class and
//template functions are generated when they are first called; others when their definition is reached.
struct SCPP65Function
{
	int64							lliName;		//An identifier, or an operator (see GetOperatorName).
	SCPP65Type*						psReturn;
	CArrayCPP65Types				mapsParameters;
	CArraySyntax					mapcDefaults;	//Each parameter's default argument, or NULL.
	bool							bVariadic;
	bool							bDefined;
	SCPP65Class*					psClass;		//NULL for a function that is not a member.
	ECPP65FunctionKind				eKind;
	bool							bStatic;
	bool							bVirtual;
	bool							bPure;
	int								iSlot;			//A virtual function's place in the vtable.
	int								iOverload;		//Other functions with the same name before this one.
	bool							bImplicit;		//A constructor or destructor the generator wrote.
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	SCPP65Bindings*					psBindings;		//The template arguments the definition is generated with.
	bool							bInline;		//Generated when used.
	bool							bQueued;
	bool							bGenerated;
	bool							bCalled;
	SCPP65Template*					psTemplate;		//The function template this is an instance of.
	CChars							mszArguments;	//The instance's template arguments, mangled for its label.
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


//A base class and where it is in the derived class.
struct SCPP65Base
{
	SCPP65Class*	psClass;
	int				iOffset;
};


typedef CArrayTemplate<SCPP65Base>		CArrayCPP65Bases;


//A secondary vtable: the one for the polymorphic base subobject at iOffset that does not share the class's
//vtable pointer.  It is laid out like psOwner's vtable.  A function whose object is not that subobject is
//called through a thunk that adds maiAdjust's entry to this first.
struct SCPP65Vtable
{
	int							iOffset;
	SCPP65Class*				psOwner;
	CArrayCPP65FunctionPtrs		mapsFunctions;
	CArrayInt					maiAdjust;
};


typedef CArrayTemplate<SCPP65Vtable>	CArrayCPP65Vtables;


//A class's layout: the bases in order (an empty base takes no space), then the vtable pointer if no base has
//one, then the fields.  A class is laid out when it is first needed whole.  psBase is the first base.
struct SCPP65Class
{
	CCPPSymbol*					pcSymbol;
	int64						lliName;
	SCPP65Type*					psType;
	SCPP65Class*				psBase;
	CArrayCPP65Bases			macBases;
	CArrayCPP65Fields			macFields;
	CArrayCPP65FunctionPtrs		mapsVirtuals;		//The vtable.
	int							iVtableOffset;		//-1 if the class has no vtable of its own.
	CArrayCPP65Vtables			macVtables;			//The secondary vtables, written after the vtable.
	bool						bComplete;
	bool						bLayingOut;
	bool						bBases;				//macBases is filled in (their offsets only once it is complete).
	bool						bEmpty;				//No fields, no vtable pointer and empty bases.
	int							iAlign;
	bool						bUnion;
	bool						bVtable;			//The vtable has been written.
	bool						bConstructors;		//The class declares a constructor.
	SCPP65Function*				psDestructor;
	CCPPSyntaxClassSpecifier*	pcSpecifier;		//The definition, NULL until it is seen.
	SCPP65Template*				psTemplate;			//The class template this is an instance of.
	SCPP65Bindings*				psBindings;
	CChars						mszArguments;		//The instance's template arguments, mangled for its label.
};


//A class or function template, or a member function of a class template defined outside the class.
struct SCPP65Template
{
	CCPPSymbol*						pcSymbol;		//The template's name; for an outside member, the class template's.
	CCPPSyntaxTemplateDeclaration*	pcDeclaration;
	CCPPSyntax*						pcDefinition;	//A class specifier or a function definition.
	bool							bMember;
};


typedef CArrayTemplatePtr<SCPP65Template>	CArrayCPP65Templates;


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
	int					iString;	//Global: the string literal's number, or a static local's when lliName is its name.
	SCPP65Class*		psClass;	//Global: the class of a static data member.
	bool				bInternal;	//Global: a static global, labelled with the unit.
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


//A goto label in the function and the assembly label it was given.
struct SCPP65GotoLabel
{
	int64	lliName;
	int		iLabel;
};


typedef CArrayTemplate<SCPP65GotoLabel>	CArrayCPP65GotoLabels;


//A case or default label and the assembly label it was given.
struct SCPP65Case
{
	CCPPSyntaxCaseStatement*	pcCase;
	int							iLabel;
};


//A remembered TypeOf: the type of an expression with some template arguments.
struct SCPP65TypeOf
{
	SCPP65Bindings*		psBindings;
	SCPP65Function*		psFunction;
	SCPP65Type*			psType;
};


typedef CArrayTemplatePtr<SCPP65Variable>	CArrayCPP65Variables;
typedef CMapLongTemplate<SCPP65TypeOf>		CMapCPP65TypeOfs;


//Where a call's result is made instead of a temporary: a place, the function's own result, or a place on the
//stack (an argument).
enum ECPP65Elide
{
	CPP65E_None,
	CPP65E_Place,
	CPP65E_Result,
	CPP65E_Stack,
};


//A temporary object's place in the frame.
struct SCPP65Slot
{
	int		iOffset;
};


typedef CMapLongTemplate<SCPP65Slot>		CMapCPP65Slots;


//A temporary to destroy at the end of its full expression.
struct SCPP65Pending
{
	int				iOffset;
	SCPP65Type*		psType;
};


typedef CArrayTemplate<SCPP65Pending>		CArrayCPP65Pendings;
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
	CChars					mszThunks;			//Adjust this, then jump to a virtual function.
	CArrayChars				maszRuntime;		//The soft float runtime's labels the unit calls.
	CChars					mszBody;
	CChars*					mpszOut;
	CCPPErrors				mcErrors;

	CArrayCPP65Variables	mapsGlobals;
	CArrayCPP65FunctionPtrs	mapsFunctions;
	CArrayCPP65Classes		mapsClasses;
	CArrayCPP65Templates	mapsTemplates;
	CArrayCPP65BindingsPtrs	mapsAllBindings;
	CArrayCPP65FunctionPtrs	mapsQueue;			//Functions to generate.
	CArraySyntax			mapcFailed;			//Declarators of functions whose declarations were not supported.
	SCPP65Bindings*			mpsBindings;		//The template arguments of what is being generated or laid out.
	CMapCPP65TypeOfs		mcTypeOfs;			//TypeOf's answers, by expression.
	CMapCPP65Slots			mcTemporarySlots;	//The function's temporary objects' places, by expression.
	CArrayCPP65Pendings		macPendingTemporaries;
	int						miTypeOf;			//TypeOf is working out an expression (its code is thrown away).
	CCPPSyntax*				mpcElide;			//The call whose result is made where msElide is, not in a temporary.
	SCPP65Place				msElide;
	ECPP65Elide				meElide;
	int						miElideDepth;		//CPP65E_Stack: miDepth just after the object's place was pushed.
	ECPP65Elide				meNextElide;		//How Construct reaches an Address place's object, set by its caller.
	int						miNextElideDepth;
	int						miDataTemporaries;
	CChars					mszUnit;			//Put in front of this translation unit's own labels.
	CArrayChars				maszOwnEmitted;
	CArrayChars*			mpaszEmitted;		//Labels of functions, vtables and runtime routines any unit has written.
	int						miQuiet;			//Errors are not reported, only noted in mbQuietFailed.
	bool					mbQuietFailed;
	CArrayCPP65Strings		maszStrings;
	CChars					mszInitialise;		//Constructs the global objects (R_Initialise).
	CChars					mszFinalise;		//Destroys them (R_Finalise).
	bool					mbPureVirtual;

	CArrayCPP65Variables	mapsLocals;
	CArrayCPP65Variables	mapsDestructors;	//Local objects to destroy, last made last.
	CArrayCPP65Jumps		macJumps;
	CArrayCPP65Cases		macCases;
	CArrayCPP65GotoLabels	macGotoLabels;
	SCPP65Function*			mpsFunction;
	int						miFrameBytes;
	int						miStatics;			//Static locals in the unit.
	bool					mbStaticLocal;		//A static local's data is being written.
	bool					mbFramePointer;		//The function saves V and points it at its frame.
	int						miDepth;			//Bytes pushed on S since the frame was made.
	int						miTemporaries;		//Bytes of pseudo registers ([offset + W]) in use.
	int						miReturnLabel;
	int						miThisOffset;		//this's place in the frame of a member function, or -1.
	bool					mbThisInX;
	int						miLabels;
	bool					mbSignedDivide;
	bool					mbSignedModulus;
	bool					mbSignedDivide64;

public:
	void					Init(CCPPSyntaxTree* pcTree);
	void					Kill(void);

	bool					Generate(CChars* pszDest);
	bool					GenerateProgram(CChars* pszDest);
	void					SetUnit(const char* szUnit, CArrayChars* paszEmitted);
	void					GetStartUp(CArrayChars* paszInitialise, CArrayChars* paszFinalise);
	static void				AppendStartUp(CChars* pszDest, CArrayChars* paszInitialise, CArrayChars* paszFinalise);
	static void				AddDefines(CConfig* pcConfig);
	CCPPErrors*				GetErrors(void);
	int						NumErrors(void);
	void					GetExternals(CArrayChars* paszLabels);

protected:
	//Declarations, functions and statements.  CPP65Generator.cpp
	bool					Error(CCPPSyntax* pcNode, const char* szMessage);
	bool					ErrorAt(int iToken, const char* szMessage);
	void					BeginQuiet(void);
	bool					EndQuiet(void);
	CChars*					Line(const char* szText);
	void					Label(int iLabel);
	void					AppendLabel(CChars* psz, int iLabel);
	void					AppendName(CChars* psz, int64 lliName);
	int						NewLabel(void);
	int64					GetName(CCPPSyntaxDeclarator* pcDeclarator);

	SCPP65Function*			DeclareFunction(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, bool bDefinition, SCPP65Class* psClass, bool bPure, SCPP65Template* psTemplate = NULL, CChars* pszArguments = NULL);
	SCPP65Function*			AddFunction(SCPP65Class* psClass, int64 lliName, ECPP65FunctionKind eKind, SCPP65Type* psReturn);
	bool					SameParameters(SCPP65Function* psFunction, SCPP65Function* psOther);
	void					RequireFunction(SCPP65Function* psFunction);
	bool					Emit(CChars* pszLabel);
	bool					Declarations(CArraySyntax* papcDeclarations);
	bool					GlobalDeclaration(CCPPSyntaxSimpleDeclaration* pcDeclaration);
	bool					GlobalInitialiser(SCPP65Type* psType, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode, SCPP65Place* psPlace);
	void					AppendData(SCPP65Type* psType, int64 lliValue);
	void					AppendZeros(int iBytes);
	void					AppendGlobalLabel(CChars* psz, SCPP65Class* psClass, int64 lliName);
	SCPP65Variable*			AddVariable(CArrayCPP65Variables* papsVariables, CCPPSymbol* pcSymbol, SCPP65Type* psType, ECPP65Storage eStorage, int iOffset);
	void					FreeVariables(CArrayCPP65Variables* papsVariables);
	SCPP65Variable*			GetVariable(CCPPSymbol* pcSymbol);
	SCPP65Type*				GetType(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, CCPPSyntax* pcNode, CCPPSyntax* pcInitialiser = NULL);
	SCPP65Type*				GetDeclaratorType(SCPP65Type* psType, CCPPSyntaxDeclarator* pcDeclarator, CCPPSyntax* pcNode, CCPPSyntax* pcInitialiser);
	SCPP65Type*				GetFunctionType(SCPP65Type* psReturn, CCPPSyntaxFunctionSuffix* pcSuffix, CCPPSyntax* pcNode);
	int						GetInitialiserCount(CCPPSyntax* pcInitialiser);
	SCPP65Type*				GetBaseType(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntax* pcNode);
	SCPP65Type*				GetTypeIdType(CCPPSyntaxTypeId* pcTypeId);
	SCPP65Type*				GetSymbolType(CCPPSymbol* pcSymbol, CCPPSyntax* pcNode);

	bool					Function(CCPPSyntaxFunctionDefinition* pcFunction, SCPP65Class* psClass);
	void					GenerateFunction(SCPP65Function* psFunction);
	bool					GenerateDefinition(SCPP65Function* psFunction);
	void					BeginFunction(SCPP65Function* psFunction);
	void					EndFunction(SCPP65Function* psFunction);
	bool					AllocateLocals(CCPPSyntax* pcNode);
	bool					StaticLocal(CCPPSyntaxInitDeclarator* pcInit, CCPPSyntaxDeclSpecifiers* pcSpecifiers);
	bool					GotoStatement(CCPPSyntaxGotoStatement* pcGoto);
	bool					LabelStatement(CCPPSyntaxLabelStatement* pcLabel);
	int						GotoLabel(int iToken);
	void					AppendPlaceLabel(CChars* psz, SCPP65Place* psPlace);
	bool					Statement(CCPPSyntax* pcStatement);
	bool					StatementOf(CCPPSyntax* pcStatement);
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
	void					AppendDivideSigned(const char* szName, int iBytes);

	//Classes and templates.  CPP65GeneratorClasses.cpp
	SCPP65Class*			NewClass(CCPPSymbol* pcSymbol, int64 lliName);
	SCPP65Class*			GetClass(CCPPSymbol* pcSymbol, CCPPSyntax* pcNode);
	SCPP65Class*			GetUnnamedClass(CCPPSyntaxClassSpecifier* pcSpecifier);
	SCPP65Class*			FindClass(int64 lliName);
	SCPP65Class*			GetComponentClass(CCPPSyntaxNameComponent* pcComponent, CCPPSymbol* pcSymbol, CCPPSyntax* pcNode);
	SCPP65Class*			GetNameClass(CCPPSyntaxName* pcName, CCPPSyntax* pcNode);
	bool					TemplateDeclaration(CCPPSyntaxTemplateDeclaration* pcTemplate);
	bool					ExplicitInstantiation(CCPPSyntax* pcDeclaration);
	SCPP65Template*			GetTemplate(CCPPSymbol* pcSymbol);
	SCPP65Template*			GetTemplateByName(int64 lliName);
	SCPP65Bindings*			BindArguments(CCPPSyntaxTemplateDeclaration* pcTemplate, CArraySyntax* papcArguments, CArrayCPP65Types* papsDeduced, CChars* pszArguments, CCPPSyntax* pcNode);
	SCPP65Binding*			FindBinding(CCPPSymbol* pcParameter);
	SCPP65Class*			InstantiateClass(SCPP65Template* psTemplate, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	SCPP65Function*			InstantiateFunction(SCPP65Template* psTemplate, CArraySyntax* papcExplicit, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	bool					CompleteClass(SCPP65Class* psClass, CCPPSyntax* pcNode);
	bool					ResolveBases(SCPP65Class* psClass, CCPPSyntax* pcNode);
	bool					LayoutClass(SCPP65Class* psClass, CCPPSyntax* pcNode);
	bool					ClassMember(SCPP65Class* psClass, CCPPSyntax* pcMember);
	bool					LayoutVirtuals(SCPP65Class* psClass, CCPPSyntax* pcNode);
	bool					Overrides(SCPP65Function* psVirtual, SCPP65Function* psFunction);
	bool					OverridesBase(SCPP65Class* psClass, SCPP65Function* psFunction);
	void					LayoutSecondaryVtables(SCPP65Class* psClass);
	void					AddSecondaryVtable(SCPP65Class* psClass, int iOffset, SCPP65Class* psOwner, CArrayCPP65FunctionPtrs* papsFunctions, CArrayInt* paiAdjust);
	bool					IsPolymorphic(SCPP65Class* psClass);
	void					EnsureVtable(SCPP65Class* psClass);
	void					AppendVtableEntry(SCPP65Function* psFunction, int iAdjust);
	void					StoreVtables(SCPP65Class* psClass, const char* szValue, const char* szObject);
	SCPP65Field*			FindField(SCPP65Class* psClass, int64 lliName, int* piOffset);
	SCPP65Field*			FindFieldSymbol(SCPP65Class* psClass, CCPPSymbol* pcSymbol, int* piOffset);
	SCPP65Variable*			FindStatic(SCPP65Class* psClass, int64 lliName);
	SCPP65Function*			FindFunction(SCPP65Class* psClass, int64 lliName, ECPP65FunctionKind eKind, CArraySyntax* papcArguments, bool bBases, CCPPSyntax* pcNode);
	bool					IsDerived(SCPP65Class* psClass, SCPP65Class* psBase, int* piOffset);
	bool					NeedsConstructor(SCPP65Class* psClass);
	bool					NeedsDestructor(SCPP65Class* psClass);
	void					AppendFunctionLabel(CChars* psz, SCPP65Function* psFunction);
	void					AppendClassLabel(CChars* psz, SCPP65Class* psClass, const char* szPart);
	void					AppendClassName(CChars* psz, SCPP65Class* psClass);
	void					AppendFunctionName(CChars* psz, int64 lliName);
	void					AppendMangledType(CChars* psz, SCPP65Type* psType);
	int64					GetFunctionName(CCPPSyntaxNameComponent* pcComponent);
	int64					GetOperatorName(ECppTokens eToken, bool bArray);
	const char*				GetOperatorLabel(int64 lliName);
	void					SetDefinition(SCPP65Function* psFunction, CCPPSyntaxFunctionDefinition* pcDefinition);
	bool					ConstructorBody(SCPP65Function* psFunction, CCPPSyntaxFunctionDefinition* pcFunction);
	bool					DestructorBody(SCPP65Function* psFunction);
	bool					Construct(SCPP65Place* psPlace, CArraySyntax* papcArguments, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode);
	bool					ConstructReference(SCPP65Place* psPlace, CArraySyntax* papcArguments, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode);
	bool					ConstructClass(SCPP65Place* psPlace, SCPP65Class* psClass, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	bool					Destroy(SCPP65Place* psPlace);
	void					Call(SCPP65Function* psFunction, bool bVirtual, int iBytes);
	bool					CallMember(SCPP65Function* psFunction, SCPP65Place* psObject, CArraySyntax* papcArguments, bool bVirtual, CCPPSyntax* pcNode);
	SCPP65Type*				MemberCall(CCPPSyntaxCallExpression* pcCall, CCPPSyntax* pcCallee);
	SCPP65Type*				OperatorCall(CCPPSyntax* pcObject, ECppTokens eOperator, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	int						PushArguments(SCPP65Function* psFunction, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	int						PushParameters(CArrayCPP65Types* papsParameters, CArraySyntax* papcDefaults, bool bVariadic, CArraySyntax* papcArguments, CCPPSyntax* pcNode, SCPP65Type* psReturn = NULL);
	bool					CopyObject(SCPP65Place* psDestination, CCPPSyntax* pcSource, CCPPSyntax* pcNode);
	bool					MemberPlace(CCPPSyntaxMemberExpression* pcMember, SCPP65Place* psPlace);
	bool					SetVtable(SCPP65Place* psPlace, SCPP65Class* psClass);
	void					EnsureThis(void);
	void					ChangeX(void);
	int64					GetComponentName(CCPPSyntaxNameComponent* pcComponent);

	//Expressions.  CPP65GeneratorExpressions.cpp
	SCPP65Type*				Expression(CCPPSyntax* pcExpression);
	bool					IsBuiltin(int64 lliName);
	void					AllocateTemporaries(CCPPSyntax* pcNode);
	bool					TemporaryAddress(CCPPSyntax* pcNode, SCPP65Type* psType);
	void					DestroyTemporaries(bool bKeep, bool bWide);
	bool					HasCopyConstructor(SCPP65Class* psClass);
	bool					IsTriviallyCopyable(SCPP65Class* psClass);
	bool					CopyConstruct(SCPP65Place* psPlace, CCPPSyntax* pcSource, CCPPSyntax* pcNode);
	void					LoadResultAddress(void);
	bool					ReturnObject(CCPPSyntax* pcValue, CCPPSyntax* pcNode);
	SCPP65Type*				TemporaryObject(SCPP65Type* psType, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode);
	bool					IsObject(CCPPSyntax* pcExpression);
	bool					IsMadeObject(CCPPSyntax* pcExpression, SCPP65Class* psClass);
	bool					MakesObject(CCPPSyntax* pcExpression);
	bool					ConstructFromMade(SCPP65Place* psPlace, CCPPSyntax* pcInitialiser, ECPP65Elide eElide, int iDepth, CCPPSyntax* pcNode);
	SCPP65Type*				OperatorExpression(ECppTokens eOperator, CCPPSyntax* pcLeft, CCPPSyntax* pcRight, CCPPSyntax* pcNode);
	SCPP65Function*			FindFreeOperator(int64 lliName, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	void					StringData(char* szBytes, size uiLength);
	bool					FunctionNameText(int64 lliName, CChars* pszText);
	bool					IsFunctionName(CCPPSyntaxIdExpression* pcId);
	SCPP65Type*				Builtin(CCPPSyntaxCallExpression* pcCall, int64 lliName);
	SCPP65Type*				Evaluate(CCPPSyntax* pcExpression);
	SCPP65Type*				Value(CCPPSyntax* pcExpression);
	SCPP65Type*				Literal(CCPPSyntaxLiteralExpression* pcLiteral);
	SCPP65Type*				SimpleValue(CCPPSyntax* pcExpression, const char* szRegister);
	SCPP65Type*				StringLiteral(CCPPSyntaxStringExpression* pcString);
	SCPP65Type*				IdExpression(CCPPSyntaxIdExpression* pcId);
	SCPP65Type*				FunctionAddress(CCPPSyntax* pcNode, int64 lliName);
	SCPP65Type*				UnaryExpression(CCPPSyntaxUnaryExpression* pcUnary);
	SCPP65Type*				IncrementExpression(CCPPSyntaxUnaryExpression* pcUnary);
	SCPP65Type*				BinaryExpression(CCPPSyntaxBinaryExpression* pcBinary);
	SCPP65Type*				Assignment(CCPPSyntaxBinaryExpression* pcBinary);
	SCPP65Type*				ConditionalExpression(CCPPSyntaxConditionalExpression* pcConditional);
	SCPP65Type*				ConditionValue(CCPPSyntax* pcExpression);
	SCPP65Type*				CallExpression(CCPPSyntaxCallExpression* pcCall);
	SCPP65Type*				PointerCall(CCPPSyntaxCallExpression* pcCall, SCPP65Type* psFunctionType);
	bool					HasMember(SCPP65Class* psClass, int64 lliName);
	void					ReportFailedDeclaration(int64 lliName);
	SCPP65Type*				ThisExpression(CCPPSyntax* pcThis);
	SCPP65Type*				NewExpression(CCPPSyntaxNewExpression* pcNew);
	SCPP65Type*				CastExpression(CCPPSyntaxTypeId* pcTypeId, CCPPSyntax* pcOperand, CCPPSyntax* pcNode);
	SCPP65Type*				SizeofExpression(CCPPSyntax* pcSizeof);
	SCPP65Type*				TypeOf(CCPPSyntax* pcExpression);
	SCPP65Type*				Operands(CCPPSyntax* pcLeft, CCPPSyntax* pcRight, SCPP65Right* psRight);
	SCPP65Type*				Operate(ECppTokens eOperator, SCPP65Type* psLeft, SCPP65Right* psRight, CCPPSyntax* pcNode);
	SCPP65Type*				OperateWide(ECppTokens eOperator, SCPP65Type* psLeft, SCPP65Right* psRight, SCPP65Type* psType, CCPPSyntax* pcNode);
	SCPP65Type*				FloatArithmetic(SCPP65Type* psLeft, SCPP65Type* psRight);
	void					CallRuntime(const char* szName, int iBytes);
	void					MoveBToA(SCPP65Type* psType);
	int						PushFloatOperands(SCPP65Type* psLeft, SCPP65Right* psRight, SCPP65Type* psType, CCPPSyntax* pcNode);
	SCPP65Type*				OperateFloat(ECppTokens eOperator, SCPP65Type* psLeft, SCPP65Right* psRight, CCPPSyntax* pcNode);
	bool					CompareFloat(ECppTokens eOperator, SCPP65Type* psLeft, SCPP65Right* psRight, bool bJumpIf, int iLabel, CCPPSyntax* pcNode);
	bool					ConvertFloat(SCPP65Type* psFrom, SCPP65Type* psTo, CCPPSyntax* pcNode);
	SCPP65Type*				IncrementFloat(CCPPSyntaxUnaryExpression* pcUnary, SCPP65Place* psPlace, const char* szAddress);
	int64					FloatBits(SCPP65Type* psType, double dValue);
	bool					FloatConstant(CCPPSyntax* pcExpression, double* pdValue);
	bool					FloatValue(CCPPSyntax* pcExpression, double* pdValue, SCPP65Type** ppsType);
	bool					UsesFloat(CCPPSyntax* pcNode);
	void					MoveAToB(SCPP65Type* psType);
	void					LoadConstant(const char* szRegister, SCPP65Type* psType, int64 lliValue);
	bool					Compare(ECppTokens eOperator, SCPP65Type* psLeft, SCPP65Right* psRight, bool bJumpIf, int iLabel, CCPPSyntax* pcNode);
	void					RightInB(SCPP65Right* psRight);
	CChars*					AppendRight(CChars* psz, SCPP65Right* psRight);
	bool					Condition(CCPPSyntax* pcExpression, bool bJumpIf, int iLabel);
	bool					Convert(SCPP65Type* psFrom, SCPP65Type* psTo, CCPPSyntax* pcNode);
	bool					Supported(SCPP65Type* psType, CCPPSyntax* pcNode);
	void					Truncate(const char* szRegister, int iBytes, bool bSigned);
	void					Scale(const char* szRegister, int iBytes);

	bool					Place(CCPPSyntax* pcExpression, SCPP65Place* psPlace);
	bool					VariablePlace(SCPP65Variable* psVariable, SCPP65Place* psPlace);
	void					DerefPlace(SCPP65Place* psPlace);
	bool					ResultPlace(SCPP65Type* psType, SCPP65Place* psPlace, CCPPSyntax* pcNode);
	void					AppendPlace(CChars* psz, SCPP65Place* psPlace, const char* szAddress);
	void					Load(SCPP65Place* psPlace, const char* szRegister, const char* szAddress);
	void					Store(SCPP65Place* psPlace, const char* szRegister, const char* szAddress);
	void					LoadAddress(SCPP65Place* psPlace);
	void					Push(const char* szRegister);
	void					PushValue(SCPP65Type* psType);
	void					PushWide(const char* szRegister, bool bWide);
	void					PullWide(const char* szRegister, bool bWide);
	void					Widen(const char* szRegister, bool bSigned);
	void					CompareZero(SCPP65Type* psType);
	void					Multiply(const char* szInstruction);
	void					Pull(const char* szRegister);

	bool					Constant(CCPPSyntax* pcExpression, int64* plliValue);
	bool					TypeTrait(CCPPSyntaxTypeTraitExpression* pcTrait, int64* plliValue);
	bool					EnumeratorValue(CCPPSymbol* pcSymbol, int64* plliValue);
	bool					IsSimple(CCPPSyntax* pcExpression);
	bool					ContainsCall(CCPPSyntax* pcNode);
	CCPPSyntax*				SkipParentheses(CCPPSyntax* pcExpression);
	bool					DecodeString(CCPPSyntax* pcString, CArrayUint8* pauiBytes);
	bool					DecodeStringToken(CCPPSyntax* pcString, CCTString* pcToken, CArrayUint8* pauiBytes);
};


#endif // __C_P_P_65_GENERATOR_H__
