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
#define CPP65_HEAP_ADDRESS			0x80000		//Libc65's malloc heap (stdlib.cpp HEAP_START); a program ends below it.
#define CPP65_TEMPORARY_BYTES		0x100
#define CPP65_INLINE_MEMCPY			64		//memcpy of at most this many bytes, a constant, is copied in place.
#define CPP65_INLINE_ZERO			64		//Value initialised objects up to this size are zeroed in place, bigger ones with memset.
#define CPP65_CONVERSION_NAME		(-1 - 2 * (int64)NUM_CPP_TOKENS)	//The name of every conversion function (operator T); they differ by their result.
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayUint8.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/ArrayInt.h"
#include "BaseLib/ArrayInt64.h"
#include "BaseLib/MapLongTemplate.h"
#include "CPPSyntaxes.h"
#include "CPPSymbol.h"
#include "CPP65Type.h"
#include "IRImports.h"
#include "Assembler65Lib/Asm65816Profile.h"
#include "CPPTarget.h"
#include "CPPErrors.h"
#include "Config.h"
#include "CPP65Assembly.h"


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
	bool			bConst;			//The type argument was const (types do not record const).
	CArrayTemplatePtr<SCPP65Type>*	papsPack;	//A parameter pack's types.
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
	bool			bRestrict;		//A global or parameter pointer declared __restrict.
	SCPP65Class*	psAnonymous;	//The anonymous union or struct declared in a function that this local is a member of.
	int				iAnonymous;		//The anonymous object's offset in the frame (the member's is iOffset).
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
	uint32							uiConstReferences;	//A bit for each of the first 32 parameters that is a const reference (types do not record const).
	bool							bVariadic;
	bool							bDefined;
	SCPP65Class*					psClass;		//NULL for a function that is not a member.
	ECPP65FunctionKind				eKind;
	bool							bStatic;
	bool							bVirtual;
	bool							bPure;
	int								iSlot;			//A virtual function's place in the vtable.
	int								iOverload;		//Other functions with the same name before this one.
	bool							bCLinkage;		//Declared in extern "C": its label is only its name.
	bool							bExplicit;		//An explicit constructor or conversion function: not used for implicit conversions.
	bool							bConstThis;		//A const member function: an overload of the same function without const.
	bool							bImplicit;		//A constructor or destructor the generator wrote.
	SCPP65Function*					psInherited;	//The base's constructor an inherited constructor calls (using Base::Base).
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


//How well an argument converts to a parameter, best last.
enum ECPP65ConversionRank
{
	CPP65CR_None,
	CPP65CR_Ellipsis,
	CPP65CR_UserDefined,		//A conversion function whose result then needs a conversion.
	CPP65CR_UserDefinedExact,	//A converting constructor, or a conversion function whose result matches (or promotes).
	CPP65CR_Boolean,		//A pointer to bool, worse than the other conversions.
	CPP65CR_Conversion,
	CPP65CR_Promotion,
	CPP65CR_Exact,
};


//A non-static data member.  Offsets include the base class's.
struct SCPP65Field
{
	CCPPSymbol*		pcSymbol;
	int64			lliName;
	SCPP65Type*		psType;
	int				iOffset;
	int				iAlign;				//Its alignas, 0 for none.
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


//using Base::f; in a class: Base's functions named f are overloads of the class's own.
struct SCPP65Using
{
	SCPP65Class*	psBase;
	int64			lliName;
};


typedef CArrayTemplate<SCPP65Using>		CArrayCPP65Usings;


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
	CArrayCPP65Usings			macUsings;
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
	SCPP65Class*				psInheritedBase;	//The base whose constructors are inherited (using Base::Base), or NULL.
	SCPP65Function*				psDestructor;
	CCPPSyntaxClassSpecifier*	pcSpecifier;		//The definition, NULL until it is seen.
	SCPP65Template*				psTemplate;			//The class template this is an instance of.
	SCPP65Bindings*				psBindings;
	CChars						mszArguments;		//The instance's template arguments, mangled for its label.
};


//A class or function template, a member function of a class template defined outside the class, or a member
//template.  A member template is kept for each class (or class template instance) it is in (psClass).  One
//defined outside its class is kept once, with psClass NULL, and found by the class's name (pcSymbol) and its
//own name (lliName); pcClassDeclaration is the class template's parameters if the class is a template.  An
//explicit or partial specialisation has the template it specialises as psPrimary (a template is its own
//primary otherwise) and its template arguments in pcPattern.  An outside member function's class template is
//psOwner, the primary or a partial specialisation.
struct SCPP65Template
{
	CCPPSymbol*						pcSymbol;		//The template's name; for an outside member, the class's.
	CCPPSyntaxTemplateDeclaration*	pcDeclaration;
	CCPPSyntax*						pcDefinition;	//A class specifier or a function definition.
	bool							bMember;
	bool							bMemberTemplate;
	SCPP65Class*					psClass;
	int64							lliName;
	CCPPSyntaxTemplateDeclaration*	pcClassDeclaration;
	CCPPSyntaxDeclarator*			pcDeclarator;	//A member template's declarator in its class.
	bool							bStatic;
	SCPP65Template*					psPrimary;
	CCPPSyntaxNameComponent*		pcPattern;
	SCPP65Template*					psOwner;
	bool							bArguments;		//mszArguments is worked out (a function specialisation's).
	CChars							mszArguments;
};


typedef CArrayTemplatePtr<SCPP65Template>	CArrayCPP65Templates;
typedef CArrayTemplatePtr<CArrayCPP65Types>	CArrayCPP65Packs;
typedef CArrayTemplatePtr<CCPPSymbol>		CArrayCPP65Symbols;


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
	CPP65RK_Temporary,		//[offset], a pseudo register (W is 0).
};


//The right operand of a binary operator: in B, a constant or a pseudo register, used directly.
struct SCPP65Right
{
	SCPP65Type*		psType;
	ECPP65RightKind	eKind;
	int64			lliValue;	//The constant, or the pseudo register's offset from W.
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


//A temporary object's place in the frame.  One made only when a side of && or || or a ?: operand is worked out has a
//guard in the frame too, set when it is made, so it is only destroyed then.
struct SCPP65Slot
{
	int		iOffset;
	int		iGuard;		//-1 for none.
};


typedef CMapLongTemplate<SCPP65Slot>		CMapCPP65Slots;


//A temporary to destroy at the end of its full expression.
struct SCPP65Pending
{
	int				iOffset;
	SCPP65Type*		psType;
	int				iGuard;		//-1 for none.
};


typedef CArrayTemplate<SCPP65Pending>		CArrayCPP65Pendings;
typedef CArrayTemplate<CChars>				CArrayCPP65Strings;


#define CPP65_BUILTIN_PREFIX			"__builtin_"
#define CPP65_BUILTIN_PREFIX_LENGTH		10


class CCPP65IRFrontEnd;
class CCTInt;


//Generates 65C64 or W65C816 assembly (Assembler65Lib syntax) from a parsed translation unit: the declarations,
//types, globals and start up here, and every function body as IR (CPP65IRFrontEnd, IR.txt) generated by the
//target's IR back end.  The calling convention and the parts of C++ that are supported are in CPP65Generator.txt.
class CCPP65Generator
{
friend class CCPP65IRFrontEnd;
protected:
	CCPPSyntaxTree*			mpcTree;
	CCPPTokeniser*			mpcTokeniser;
	CCPP65Types				mcTypes;
	SCPPTarget*				mpsTarget;

	CChars					mszCode;
	CChars					mszData;
	CChars					mszThunks;			//Adjust this, then jump to a virtual function.
	CArrayChars				maszRuntime;		//The soft float runtime's labels the unit calls.
	CChars					mszBody;
	CChars*					mpszOut;
	CCPPErrors				mcErrors;
	CCPPErrors				mcWarnings;			//Each at most once (by token); they don't fail the unit.

	CArrayCPP65Variables	mapsGlobals;
	CArrayCPP65FunctionPtrs	mapsFunctions;
	CArrayCPP65Classes		mapsClasses;
	CArrayCPP65Templates	mapsTemplates;
	CArrayCPP65BindingsPtrs	mapsAllBindings;
	CArrayCPP65Packs		mapapsPacks;		//The packs' types (NewPack).
	CArrayCPP65Symbols		mapcPackSymbols;	//The symbols made for function parameter packs' elements.
	CCPPSymbol*				mpcPackParameter;	//The function parameter pack of the function being generated, or NULL.
	CArrayCPP65Symbols		mapcPackElements;	//Its elements' symbols.
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
	CArrayChars				maszEmitted;		//Labels of the functions, vtables, thunks and runtime routines written.
	int						miQuiet;			//Errors are not reported, only noted in mbQuietFailed.
	int						miWarningChecks;	//Above 0 while a warning check works out a type (TypeOf can fold the same constant again).
	bool					mbWarnNarrowing;	//Implicit conversions of non constant integers to a narrower type are warned about.
	bool					mbKeepTypeErrors;	//DiscardedType keeps its errors (ReportTypeOf).
	bool					mbQuietFailed;
	CArrayCPP65Strings		maszStrings;
	CChars					mszInitialise;		//Works out the global being defined (its R_Initialise).
	int						miStartUps;			//R_Initialise and R_Finalise routines written.
	bool					mbPureVirtual;

	CArrayCPP65Variables	mapsLocals;
	SCPP65Function*			mpsFunction;
	int						miFrameBytes;
	int						miStatics;			//Static locals in the unit.
	bool					mbStaticLocal;		//A static local's data is being written.
	bool					mbLinkageExtern;	//In extern "C" without braces: declarations are extern.
	bool					mbCLinkage;			//In extern "C": functions declared have C linkage.
	int						miDepth;			//Bytes pushed on S since the frame was made.
	int						miTemporaries;		//Bytes of pseudo registers ([offset]) in use.
	int						miThisOffset;		//this's place in the frame of a member function, or -1.
	bool					mbThisInX;
	int						miLabels;
	bool					mbSignedDivide;
	bool					mbSignedDivide64;
	CCPP65IRFrontEnd*		mpcIRFrontEnd;		//Writes every function body as IR.
	CIRImports*				mpcImports;			//Other units' functions the IR inliner may use (SetImports), NULL for none.
	CAsm65816Profile*			mpcFrameProfile;	//Measured functions, for the W65C816's aligned frames (SetFrameProfile), or NULL.
	bool					mbListing;			//The code has comments with the C++ source lines it is for (SetListing).
	bool					mbLineTable;		//The code has .line and .fingerprint directives, for the linker's line table (SetLineTable).
	int64					mlliCompileTime;	//When the unit was compiled (seconds since 1970), for .fingerprint.
	CSpillStore*			mpcSpillStore;		//Where the functions' code waits until the unit is assembled (SetSpillStore).
	CMemorySpillStore		mcMemorySpillStore;	//The spill store when none is set.
	CArrayLargeTemplate<uint16>	mauiCodePieces;	//Per function written by the IR front end: how many pieces its code is in.

public:
	void					Init(CCPPSyntaxTree* pcTree, ECPPTarget eTarget = CPPT_65C64);
	void					Kill(void);
	void					SetWarnNarrowing(bool bWarnNarrowing);
	static void				SetDefaultWarnNarrowing(bool bWarnNarrowing);
	static bool				GetDefaultWarnNarrowing(void);

	bool					Generate(CChars* pszDest);
	bool					Generate(CCPP65Assembly* pcDest);
	bool					GenerateProgram(CChars* pszDest);
	static void				AppendStartUp(CChars* pszDest, ECPPTarget eTarget = CPPT_65C64);
	static void				AppendStartUp816(CChars* pszDest);
	static void				AddDefines(CConfig* pcConfig, ECPPTarget eTarget = CPPT_65C64);
	CCPPErrors*				GetErrors(void);
	CCPPErrors*				GetWarnings(void);
	int						NumErrors(void);
	void					GetExternals(CArrayChars* paszLabels);
	void					SetImports(CIRImports* pcImports);
	void					SetFrameProfile(CAsm65816Profile* pcProfile);
	void					SetListing(bool bListing);
	void					SetLineTable(bool bLineTable, int64 lliCompileTime);
	void					SetSpillStore(CSpillStore* pcStore);
	CSpillStore*			GetSpillStore(void);
	void					GetMissingCallees(CArrayChars* paszNames);

protected:
	//Declarations, functions and statements.  CPP65Generator.cpp
	bool					Error(CCPPSyntax* pcNode, const char* szMessage);
	void					Warning(CCPPSyntax* pcNode, const char* szMessage);

	bool					ErrorAt(int iToken, const char* szMessage);
	void					AppendMacroUse(SPreprocessedToken* psSource, CChars* pszMessage);
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
	CCPPSyntax*				StringInitialiser(CCPPSyntax* pcInitialiser, SCPP65Type* psElement);
	SCPP65Type*				GetBaseType(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntax* pcNode);
	SCPP65Type*				GetTypeIdType(CCPPSyntaxTypeId* pcTypeId);
	SCPP65Type*				GetSymbolType(CCPPSymbol* pcSymbol, CCPPSyntax* pcNode);

	bool					Function(CCPPSyntaxFunctionDefinition* pcFunction, SCPP65Class* psClass);
	void					GenerateFunction(SCPP65Function* psFunction);
	bool					GenerateDefinition(SCPP65Function* psFunction);
	void					BeginFunction(SCPP65Function* psFunction);
	void					EndFunction(SCPP65Function* psFunction);
	void					AppendUnsupported(CChars* psz);
	bool					AllocateLocals(CCPPSyntax* pcNode);
	bool					StaticLocal(CCPPSyntaxInitDeclarator* pcInit, CCPPSyntaxDeclSpecifiers* pcSpecifiers);
	bool					AnonymousLocal(CCPPSyntaxClassSpecifier* pcSpecifier);
	bool					StaticAssert(CCPPSyntaxStaticAssert* pcAssert);
	bool					IsCLinkage(CCPPSyntaxLinkageSpecification* pcLinkage);
	void					AppendPlaceLabel(CChars* psz, SCPP65Place* psPlace);
	bool					LocalInitialiser(SCPP65Variable* psVariable, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode);
	bool					LocalArrayInitialiser(SCPP65Place* psPlace, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode);
	bool					LocalArrayElements(SCPP65Place* psPlace, CArraySyntax* papcElements, size* puiNext, CCPPSyntax* pcNode);
	void					ZeroArrayElements(SCPP65Place* psPlace, SCPP65Type* psElement, int iCount);
	bool					ConditionDeclaration(CCPPSyntax* pcCondition);
	void					AppendStrings(CChars* pszDest);
	void					AppendRuntime(void);
	void					AppendRuntime816(void);
	void					AppendDivideSigned(const char* szName, int iBytes);
	void					BeginRuntime(const char* szName);
	void					AppendStartUpRoutine(const char* szTable, const char* szName, CChars* pszBody, CChars* pszData);
	void					AppendStartUpEntry(const char* szTable, CChars* pszLabel, CChars* pszData);
	bool					GlobalStartUpRoutines(SCPP65Type* psType, CCPPSyntax* pcInitialiser, CChars* pszData, bool bConstruct, bool bDestroy);
	void					AppendSection(CChars* psz, const char* szKind, CChars* pszName, const char* szOptions);
	bool					HasUsedAttribute(CCPPSyntaxDeclarator* pcDeclarator);
	bool					IsRestrictPointer(CCPPSyntaxDeclarator* pcDeclarator);
	bool					IsVolatilePointer(CCPPSyntaxPointerOperator* pcPointer);
	void					BeginData(CChars* pszLabel, int iAlignment, bool bGlobal, const char* szOptions);
	int						DataAlignment(SCPP65Type* psType, bool bClass);
	void					AppendGlobal(CChars* psz, CChars* pszLabel);
	bool					IsUnitFunction(SCPP65Function* psFunction);

	void					ReportTypeOf(CCPPSyntax* pcExpression);
	bool					ReportError(CCPPSyntax* pcNode, const char* szMessage);
	bool					HasWarning(CCPPSyntax* pcNode, const char* szText);

	//Classes and templates.  CPP65GeneratorClasses.cpp
	SCPP65Class*			NewClass(CCPPSymbol* pcSymbol, int64 lliName);
	SCPP65Class*			GetClass(CCPPSymbol* pcSymbol, CCPPSyntax* pcNode);
	SCPP65Class*			GetUnnamedClass(CCPPSyntaxClassSpecifier* pcSpecifier);
	SCPP65Class*			FindClass(int64 lliName);
	SCPP65Class*			GetComponentClass(CCPPSyntaxNameComponent* pcComponent, CCPPSymbol* pcSymbol, CCPPSyntax* pcNode, SCPP65Class* psContext = NULL);
	SCPP65Class*			FindBaseInstance(SCPP65Class* psClass, SCPP65Template* psTemplate);
	SCPP65Class*			QualifierClass(SCPP65Class* psClass, CCPPSyntaxNameComponent* pcComponent, CCPPSyntax* pcNode);
	int						TypeAlignment(SCPP65Type* psType);
	int						PackAlignment(int iAlign, int iPack);
	int						AlignasAlignment(CArraySyntax* papcSpecifiers);
	SCPP65Class*			GetNameClass(CCPPSyntaxName* pcName, CCPPSyntax* pcNode);
	bool					TemplateDeclaration(CCPPSyntaxTemplateDeclaration* pcTemplate);
	bool					ExplicitInstantiation(CCPPSyntax* pcDeclaration);
	SCPP65Template*			GetTemplate(CCPPSymbol* pcSymbol);
	SCPP65Template*			GetTemplateByName(int64 lliName);
	SCPP65Bindings*			BindArguments(CCPPSyntaxTemplateDeclaration* pcTemplate, CArraySyntax* papcArguments, CArrayCPP65Types* papsDeduced, CChars* pszArguments, CCPPSyntax* pcNode, CArrayInt64* palliDeduced = NULL);
	SCPP65Binding*			FindBinding(CCPPSymbol* pcParameter);
	SCPP65Class*			InstantiateClass(SCPP65Template* psTemplate, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	SCPP65Class*			UndefinedInstance(CCPPSymbol* pcSymbol, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	SCPP65Function*			InstantiateFunction(SCPP65Template* psTemplate, CArraySyntax* papcExplicit, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	SCPP65Template*			NewTemplate(CCPPSymbol* pcSymbol, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntax* pcDefinition, bool bMember);
	bool					MemberTemplate(SCPP65Class* psClass, CCPPSyntaxTemplateDeclaration* pcTemplate);
	SCPP65Template*			FindMemberTemplate(SCPP65Class* psClass, int64 lliName, size uiArguments, int iConstObject = -1);
	SCPP65Template*			GetCallTemplate(CCPPSymbol* pcSymbol, size uiArguments);
	bool					TakesArguments(CCPPSyntaxFunctionSuffix* pcSuffix, size uiArguments);
	bool					IsConstSuffix(CCPPSyntaxFunctionSuffix* pcSuffix);
	bool					DefineMemberTemplate(SCPP65Template* psTemplate, CCPPSyntax* pcNode);
	size					NumParameters(CCPPSyntaxFunctionSuffix* pcSuffix);
	SCPP65Bindings*			RenameBindings(SCPP65Class* psClass, CCPPSyntaxTemplateDeclaration* pcDeclaration);
	SCPP65Bindings*			NewBindings(void);
	CArrayCPP65Types*		NewPack(void);
	SCPP65Binding*			PackBinding(CCPPSyntaxParameter* pcParameter);

	SCPP65Template*			Specialisation(CCPPSymbol* pcSymbol, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntax* pcDefinition, CCPPSyntaxNameComponent* pcPattern);
	SCPP65Template*			OutsideMemberOwner(SCPP65Template* psPrimary, CCPPSyntaxNameComponent* pcClass, CCPPSyntaxTemplateDeclaration* pcDeclaration);
	SCPP65Template*			FindClassSpecialisation(SCPP65Template* psPrimary, SCPP65Bindings* psArguments, SCPP65Bindings** ppsBindings, CCPPSyntax* pcNode);
	bool					MatchPattern(SCPP65Template* psSpecialisation, CArraySyntax* papcPattern, SCPP65Bindings* psArguments, SCPP65Bindings* psDeduced, int* piScore);
	bool					MatchType(SCPP65Template* psSpecialisation, CCPPSyntaxTypeId* pcPattern, SCPP65Binding* psArgument, SCPP65Bindings* psDeduced, int* piScore);
	SCPP65Binding*			DeducedBinding(SCPP65Bindings* psDeduced, CCPPSymbol* pcParameter);
	void					DeduceArguments(SCPP65Template* psTemplate, CArrayCPP65Types* papsArguments, CArrayCPP65Types* papsDeduced, CArrayInt64* palliDeduced);
	void					DeduceMemberPointerClass(SCPP65Template* psTemplate, CCPPSyntaxDeclarator* pcDeclarator, SCPP65Type* psArgument, CArrayCPP65Types* papsDeduced);
	void					DeduceFromInstance(SCPP65Template* psTemplate, CCPPSyntaxNameComponent* pcComponent, SCPP65Type* psArgument, CArrayCPP65Types* papsDeduced, CArrayInt64* palliDeduced);
	SCPP65Class*			FindInstanceBase(SCPP65Class* psClass, CCPPSymbol* pcTemplate);
	CCPPSymbol*				GetArgumentSymbol(CCPPSyntax* pcArgument);
	SCPP65Template*			FindFunctionSpecialisation(SCPP65Template* psPrimary, CChars* pszArguments, CCPPSyntax* pcNode);
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
	bool					IsAbstract(SCPP65Class* psClass);
	void					EnsureVtable(SCPP65Class* psClass);
	void					AppendVirtualCallThunk(CChars* psz, SCPP65Function* psFunction);
	void					AppendVtableEntry(SCPP65Function* psFunction, int iAdjust);
	void					StoreVtables(SCPP65Class* psClass, const char* szValue, const char* szObject);
	SCPP65Field*			FindField(SCPP65Class* psClass, int64 lliName, int* piOffset);
	SCPP65Field*			FindFieldSymbol(SCPP65Class* psClass, CCPPSymbol* pcSymbol, int* piOffset);
	SCPP65Variable*			FindStatic(SCPP65Class* psClass, int64 lliName);
	void					ClassUsing(SCPP65Class* psClass, CCPPSyntaxUsingDeclaration* pcUsing);
	void					InheritConstructors(SCPP65Class* psClass);
	bool					IsOverloadOf(SCPP65Class* psClass, SCPP65Function* psFunction, int64 lliName);
	SCPP65Function*			FindFunction(SCPP65Class* psClass, int64 lliName, ECPP65FunctionKind eKind, CArraySyntax* papcArguments, bool bBases, CCPPSyntax* pcNode, int iConstObject = -1);
	bool					IsConstObject(CCPPSyntax* pcExpression);
	bool					IsConstPointer(CCPPSyntax* pcExpression);
	bool					IsConstDeclared(CCPPSymbol* pcSymbol, bool bPointer);
	bool					IsConstThis(void);
	bool					RankArguments(SCPP65Function* psFunction, SCPP65Type** apsArguments, CArraySyntax* papcArguments, int* aiRanks);
	int						ConversionRank(SCPP65Type* psParameter, SCPP65Type* psArgument, CCPPSyntax* pcArgument, bool bUserDefined = true);
	bool					HasConvertingConstructor(SCPP65Class* psClass, SCPP65Type* psArgument, CCPPSyntax* pcArgument);
	int						CompareRanks(int* aiFirst, int* aiSecond, size uiArguments);
	int						CompareConstReferences(SCPP65Function* psFirst, SCPP65Function* psSecond, CArraySyntax* papcArguments, int iConstObject);
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
	bool					Construct(SCPP65Place* psPlace, CArraySyntax* papcArguments, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode);
	void					ZeroObject(SCPP65Place* psPlace, int iBytes);
	bool					ConstructReference(SCPP65Place* psPlace, CArraySyntax* papcArguments, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode);
	bool					ConstructClass(SCPP65Place* psPlace, SCPP65Class* psClass, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	void					Call(SCPP65Function* psFunction, bool bVirtual, int iBytes);
	bool					CallMember(SCPP65Function* psFunction, SCPP65Place* psObject, CArraySyntax* papcArguments, bool bVirtual, CCPPSyntax* pcNode);
	SCPP65Type*				MemberCall(CCPPSyntaxCallExpression* pcCall, CCPPSyntax* pcCallee);
	SCPP65Type*				CallMemberFunction(CCPPSyntaxCallExpression* pcCall, CCPPSyntax* pcCallee, CCPPSyntax* pcObject, SCPP65Class* psClass, SCPP65Function* psFunction, bool bVirtual);
	SCPP65Type*				OperatorCall(CCPPSyntax* pcObject, ECppTokens eOperator, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	SCPP65Type*				ArrowType(CCPPSyntax* pcObject, CCPPSyntax* pcNode);
	SCPP65Function*			FindConversion(SCPP65Class* psClass, SCPP65Type* psTo, bool bExplicit, CCPPSyntax* pcNode);
	SCPP65Type*				ValueFor(CCPPSyntax* pcExpression, SCPP65Type* psTo, bool bExplicit);
	SCPP65Type*				ObjectCall(CCPPSyntax* pcObject, SCPP65Class* psClass, SCPP65Function* psFunction, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	SCPP65Type*				ArrowPointer(CCPPSyntax* pcObject, CCPPSyntax* pcNode);
	int						PushArguments(SCPP65Function* psFunction, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	int						PushParameters(CArrayCPP65Types* papsParameters, CArraySyntax* papcDefaults, bool bVariadic, CArraySyntax* papcArguments, CCPPSyntax* pcNode, SCPP65Type* psReturn = NULL);
	bool					CopyObject(SCPP65Place* psDestination, CCPPSyntax* pcSource, CCPPSyntax* pcNode);
	bool					MemberPlace(CCPPSyntaxMemberExpression* pcMember, SCPP65Place* psPlace);
	void					EnsureThis(void);
	void					ChangeX(void);
	int64					GetComponentName(CCPPSyntaxNameComponent* pcComponent);

	void					AppendAdjustThunk816(int iAdjust);
	void					AppendVirtualCallThunk816(SCPP65Function* psFunction);

	void					NoFunctionMatches(SCPP65Class* psClass, int64 lliName, ECPP65FunctionKind eKind, SCPP65Type** apsArguments, size uiArguments, CCPPSyntax* pcNode);

	//Expressions.  CPP65GeneratorExpressions.cpp
	SCPP65Type*				IntegerLiteralType(CCTInt* pcInt);

	CCPPSymbol*				NewPackElement(CCPPSymbol* pcPack);
	bool					HasPackExpansion(CArraySyntax* papcArguments);
	SCPP65Type*				ExpandPackCall(CCPPSyntaxCallExpression* pcCall);
	CCPPSyntax*				ExpandPackArguments(CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments);
	void					RestoreArguments(CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments);

	SCPP65Type*				Expression(CCPPSyntax* pcExpression);
	bool					IsBuiltin(int64 lliName);
	char*					BuiltinName(int64 lliName, int* piLength);
	bool					QuietConstant(CCPPSyntax* pcExpression, int64* plliValue);
	bool					IsInlineMemcpy(int64 lliName, CArraySyntax* papcArguments);
	SCPP65Type*				InlineMemcpy(CArraySyntax* papcArguments, CCPPSyntax* pcNode);

	bool					IsAssignmentOperator(ECppTokens eOperator);

	void					StringData(char* szBytes, size uiLength);
	int						AddString(char* szBytes, size uiLength);
	bool					FunctionNameText(int64 lliName, CChars* pszText);
	bool					IsFunctionName(CCPPSyntaxIdExpression* pcId);
	SCPP65Type*				Builtin(CCPPSyntaxCallExpression* pcCall, int64 lliName);
	SCPP65Type*				Evaluate(CCPPSyntax* pcExpression);
	SCPP65Type*				Value(CCPPSyntax* pcExpression);
	SCPP65Type*				Literal(CCPPSyntaxLiteralExpression* pcLiteral);
	SCPP65Type*				SimpleValue(CCPPSyntax* pcExpression, const char* szRegister);
	SCPP65Type*				StringLiteral(CCPPSyntaxStringExpression* pcString);
	SCPP65Type*				IdExpression(CCPPSyntaxIdExpression* pcId);
	SCPP65Type*				FunctionAddress(CCPPSyntax* pcNode, int64 lliName, SCPP65Type* psTarget = NULL);
	SCPP65Function*			FindStaticFunction(SCPP65Class* psClass, int64 lliName, SCPP65Type* psTarget, bool* pbOverloaded);
	SCPP65Type*				UnaryExpression(CCPPSyntaxUnaryExpression* pcUnary);
	SCPP65Type*				IncrementExpression(CCPPSyntaxUnaryExpression* pcUnary);
	SCPP65Type*				BinaryExpression(CCPPSyntaxBinaryExpression* pcBinary);
	SCPP65Type*				Assignment(CCPPSyntaxBinaryExpression* pcBinary);
	SCPP65Type*				ConditionalExpression(CCPPSyntaxConditionalExpression* pcConditional);
	SCPP65Type*				ObjectConditional(CCPPSyntaxConditionalExpression* pcConditional, SCPP65Type* psTrue, SCPP65Type* psFalse);
	SCPP65Type*				ConditionValue(CCPPSyntax* pcExpression);
	SCPP65Type*				CallExpression(CCPPSyntaxCallExpression* pcCall);
	bool					MemberAddress(CCPPSyntaxIdExpression* pcId, SCPP65Type** ppsType);
	SCPP65Type*				FunctionTemplateAddress(CCPPSyntaxIdExpression* pcId);
	bool					MemberFunctionAddress(SCPP65Class* psClass, int64 lliName, SCPP65Type** ppsType, CCPPSyntax* pcNode);
	bool					IsMemberPointerOperator(CCPPSyntax* pcExpression);
	SCPP65Type*				MemberPointerObject(CCPPSyntaxBinaryExpression* pcBinary);
	bool					MemberPointerPlace(CCPPSyntaxBinaryExpression* pcBinary, SCPP65Place* psPlace);
	SCPP65Type*				MemberPointerCall(CCPPSyntaxCallExpression* pcCall, CCPPSyntaxBinaryExpression* pcBinary);
	SCPP65Type*				PointerCall(CCPPSyntaxCallExpression* pcCall, SCPP65Type* psFunctionType);
	bool					HasMember(SCPP65Class* psClass, int64 lliName);
	void					ReportFailedDeclaration(int64 lliName);
	SCPP65Type*				ThisExpression(CCPPSyntax* pcThis);
	SCPP65Type*				NewExpression(CCPPSyntaxNewExpression* pcNew);
	SCPP65Type*				NewArray(CCPPSyntaxNewExpression* pcNew, SCPP65Type* psType);
	SCPP65Type*				DeleteExpression(CCPPSyntaxDeleteExpression* pcDelete);
	bool					ConstructAtA(SCPP65Type* psType, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode);
	void					AddressPlace(SCPP65Place* psPlace, SCPP65Type* psType);
	SCPP65Type*				CastExpression(CCPPSyntaxTypeId* pcTypeId, CCPPSyntax* pcOperand, CCPPSyntax* pcNode);
	SCPP65Type*				SizeofExpression(CCPPSyntax* pcSizeof);
	SCPP65Type*				TypeOf(CCPPSyntax* pcExpression);

	SCPP65Type*				PlaceTypeOf(CCPPSyntax* pcExpression);
	SCPP65Type*				DiscardedType(CCPPSyntax* pcExpression, bool bPlace);
	SCPP65Type*				Operands(CCPPSyntax* pcLeft, CCPPSyntax* pcRight, SCPP65Right* psRight);
	SCPP65Type*				Operate(ECppTokens eOperator, SCPP65Type* psLeft, SCPP65Right* psRight, CCPPSyntax* pcNode);
	SCPP65Type*				OperateWide(ECppTokens eOperator, SCPP65Type* psLeft, SCPP65Right* psRight, SCPP65Type* psType, CCPPSyntax* pcNode);

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
	void					ConstantOverflow(CCPPSyntax* pcExpression, int64 lliValue, bool bUnsignedWraps);
	void					ConversionOverflow(CCPPSyntax* pcExpression, SCPP65Type* psTo);
	void					WideningArithmetic(CCPPSyntax* pcExpression, SCPP65Type* psFrom, SCPP65Type* psTo);
	CCPPSyntaxBinaryExpression*	NarrowMultiply(CCPPSyntax* pcExpression, SCPP65Type* psType);
	void					ShiftCount(CCPPSyntaxBinaryExpression* pcBinary, int iBits);
	void					ComparisonRange(CCPPSyntaxBinaryExpression* pcBinary);
	void					ComparisonAtLimit(CCPPSyntaxBinaryExpression* pcBinary, CCPPSyntax* pcConstant, SCPP65Type* psVariable, int64 lliValue, int64 lliMin, int64 lliMax);
	void					LoopCounterWidth(CCPPSyntaxForStatement* pcFor);
	void					LoopCounters(CCPPSyntax* pcIncrement, CArrayVoidPtr* papcCounters);
	void					LoopCounterComparisons(CCPPSyntax* pcCondition, CArrayVoidPtr* papcCounters);
	CCPPSymbol*				LoopCounterSymbol(CCPPSyntax* pcExpression);
	bool					TypeTrait(CCPPSyntaxTypeTraitExpression* pcTrait, int64* plliValue);
	bool					EnumeratorValue(CCPPSymbol* pcSymbol, int64* plliValue);
	SCPP65Type*				EnumType(CCPPSymbol* pcEnum, CCPPSyntaxEnumSpecifier* pcSpecifier);
	SCPP65Type*				EnumeratorType(CCPPSymbol* pcSymbol);
	bool					IsSimple(CCPPSyntax* pcExpression);
	bool					ContainsCall(CCPPSyntax* pcNode);
	CCPPSyntax*				SkipParentheses(CCPPSyntax* pcExpression);
	bool					DecodeString(CCPPSyntax* pcString, CArrayUint8* pauiBytes);
	bool					DecodeStringToken(CCPPSyntax* pcString, CCTString* pcToken, CArrayUint8* pauiBytes);

	//Objects, temporaries and copies.  CPP65GeneratorObjects.cpp
	void					AllocateTemporaries(CCPPSyntax* pcNode, bool bConditional = false);
	void					ClearTemporaryGuards(CCPPSyntax* pcNode);
	bool					TemporaryAddress(CCPPSyntax* pcNode, SCPP65Type* psType);
	bool					HasCopyConstructor(SCPP65Class* psClass);
	SCPP65Function*			GetCopyConstructor(SCPP65Class* psClass);
	bool					CopyMembers(SCPP65Place* psPlace, CCPPSyntax* pcSource, CCPPSyntax* pcNode);

	void					CopyParts(SCPP65Class* psClass, int iOffset, int iDestinationDepth, int iSourceDepth);
	void					CopyPart(SCPP65Class* psClass, int iOffset, int iDestinationDepth, int iSourceDepth);
	bool					IsTriviallyCopyable(SCPP65Class* psClass);
	bool					CopyConstruct(SCPP65Place* psPlace, CCPPSyntax* pcSource, CCPPSyntax* pcNode);
	void					LoadResultAddress(void);
	SCPP65Type*				TemporaryObject(SCPP65Type* psType, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode);
	bool					IsObject(CCPPSyntax* pcExpression);

	SCPP65Type*				CastClassType(CCPPSyntax* pcExpression);
	SCPP65Class*			ObjectClass(CCPPSyntax* pcExpression);
	bool					IsMadeObject(CCPPSyntax* pcExpression, SCPP65Class* psClass);
	bool					MakesObject(CCPPSyntax* pcExpression);
	bool					ConstructFromMade(SCPP65Place* psPlace, CCPPSyntax* pcInitialiser, ECPP65Elide eElide, int iDepth, CCPPSyntax* pcNode);
	SCPP65Type*				OperatorExpression(ECppTokens eOperator, CCPPSyntax* pcLeft, CCPPSyntax* pcRight, CCPPSyntax* pcNode);
	SCPP65Function*			FindFreeOperator(int64 lliName, CArraySyntax* papcArguments, CCPPSyntax* pcNode);

	//Floating point.  CPP65GeneratorFloat.cpp
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
	bool					FloatBuiltin(CCPPSyntax* pcExpression, double* pdValue, SCPP65Type** ppsType);
	bool					FloatValue(CCPPSyntax* pcExpression, double* pdValue, SCPP65Type** ppsType);
	bool					UsesFloat(CCPPSyntax* pcNode);
};


#endif // __C_P_P_65_GENERATOR_H__
