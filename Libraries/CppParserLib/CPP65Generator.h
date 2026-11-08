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
#define CPP65_MAX_ASSOCIATED_DEPTH	16		//How deep a type's bases and template arguments are followed for associated namespaces.
#define CPP65_INLINE_MEMCPY			64		//memcpy of at most this many bytes, a constant, is copied in place.
#define CPP65_W65C816_ASSEMBLY		1		//1: a W65C816 build links X816.asm in place of the parts of X.cpp it replaces (they are left out with #ifndef __W65C816_ASSEMBLY__); 0: the C++ (SetW65C816Assembly changes it at run time).
#define CPP65_CONVERSION_NAME		(-1 - 2 * (int64)NUM_CPP_TOKENS)	//The name of every conversion function (operator T); they differ by their result.
#define CPP65_BACKEND_SCRATCH_CHUNK	(256 KB)	//The W65C816 back end's arena (CCPP65Generator::GetBackendScratch) grows by chunks this big...
#define CPP65_BACKEND_SCRATCH_KEPT	(1 MB)		//...and keeps this much of them from one function to the next.
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayUint8.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/ScratchPadAllocator.h"
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
#include "CPP65TypeChecker.h"


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
	int				iOffset;		//Parameters: the index in the function's parameters.  Static locals: the number (the label's S<n>_).  Members of an anonymous union or struct (psAnonymous): the offset in it.
	SCPP65Class*	psClass;		//The class of a static data member.
	CCPPSyntax*		pcInClassInitialiser;	//static const int k = 5; in the class: what its definition has if it has none.
	bool			bInternal;		//A static global, or one in a namespace without a name: the unit's own.
	bool			bCLinkage;		//Declared in extern "C": its label is only its name.
	bool			bDefined;		//A global's data has been written.
	bool			bUsed;
	bool			bRestrict;		//A global or parameter pointer declared __restrict.
	SCPP65Class*	psAnonymous;	//The anonymous union or struct declared in a function that this local is a member of.
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
	CCPPScope*						pcNamespace;	//The namespace of a function that is not a member, NULL for the global one.
	ECPP65FunctionKind				eKind;
	bool							bStatic;
	bool							bVirtual;
	bool							bPure;
	int								iSlot;			//A virtual function's place in the vtable.
	bool							bCLinkage;		//Declared in extern "C": its label is only its name.
	bool							bExplicit;		//An explicit constructor or conversion function: not used for implicit conversions.
	bool							bConstThis;		//A const member function: an overload of the same function without const.
	bool							bImplicit;		//A constructor or destructor the generator wrote.
	SCPP65Function*					psInherited;	//The base's constructor an inherited constructor calls (using Base::Base).
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	CCPPSyntaxDeclarator*			pcDeclarator;	//The first declaration's declarator (a template's, for an instance).
	SCPP65Bindings*					psBindings;		//The template arguments the definition is generated with.
	bool							bInline;		//Generated when used.
	bool							bQueued;
	bool							bGenerated;
	bool							bCalled;
	SCPP65Template*					psTemplate;		//The function template this is an instance of.
	CChars							mszArguments;	//The instance's template arguments, mangled for its label.
	SCPP65Function*					psNextOfName;	//The next function added with the same lliName (see FirstFunctionOfName).

	size	RequiredParameters(void);
};


typedef CArrayTemplatePtr<SCPP65Function>	CArrayCPP65FunctionPtrs;


//The functions of a name, in the order they were added: psFirst then each psNextOfName.
struct SCPP65FunctionsOfName
{
	SCPP65Function*	psFirst;
	SCPP65Function*	psLast;
};


typedef CMapLongTemplate<SCPP65FunctionsOfName>	CMapCPP65FunctionsOfNames;


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
	CPP65CR_Qualification,	//An exact match but for const or volatile added: worse than the same type.  [over.ics.rank] 3.2.6
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
	bool			bMutable;			//Declared mutable: not const in a const object.
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


//A vtable (CCPP65Generator::VtableEntries): before each part (the class's own, then each secondary vtable) a header of
//two words, the distance from the part's vtable pointer to the object's start and the class's type info, then the
//part's functions.  A vtable pointer points at its part's first function, and the label (_Class__2) at the first part's.
//The type info (_Class__3) is an entry per subobject, its class's type info and its offset, and a zero.
#define CPP65_VTABLE_ENTRY_BYTES		4
#define CPP65_VTABLE_HEADER_ENTRIES		2
#define CPP65_VTABLE_TOP_OFFSET			(-CPP65_VTABLE_HEADER_ENTRIES * CPP65_VTABLE_ENTRY_BYTES)	//From a vtable pointer: the distance to the object's start.
#define CPP65_VTABLE_TYPE_INFO_OFFSET	(-CPP65_VTABLE_ENTRY_BYTES)			//From a vtable pointer: the class's type info.
#define CPP65_TYPE_INFO_ENTRY_BYTES		(2 * CPP65_VTABLE_ENTRY_BYTES)
#define CPP65_TYPE_INFO_OFFSET_OFFSET	CPP65_VTABLE_ENTRY_BYTES				//In a type info entry: the subobject's offset.
#define CPP65_VTABLE_LABEL				"2"
#define CPP65_TYPE_INFO_LABEL			"3"


//The W65C816's thunks, which work a word at a time.  A this adjusting thunk (AppendAdjustThunk816) has its caller's
//stack: this, the first argument and so pushed last, is above the long return address.  A virtual call thunk
//(AppendVirtualCallThunk816) saves D and pushes a word twice, and D then points at the stack: from D, what follows.
//The function's address - 1 is put at CPP65_816_VCALL_TARGET, over the saved D, for rtl to jump to.
#define CPP65_816_WORD_BYTES			2
#define CPP65_816_ADJUST_THIS			4	//From S: this, in the adjusting thunk.
#define CPP65_816_VCALL_VTABLE			1	//The vtable pointer (a low word and a high word).
#define CPP65_816_VCALL_TARGET			4	//The function's address - 1: its low word...
#define CPP65_816_VCALL_CALLER_D		5	//The caller's D.
#define CPP65_816_VCALL_TARGET_BANK		6	//...and its bank.
#define CPP65_816_VCALL_THIS			10	//this, past the caller's D and the long return address.


enum ECPP65VtableEntry
{
	CPP65VE_Header,		//A part's header: iVtablePointer is where the part's vtable pointer is in the object.
	CPP65VE_Function,	//psFunction, through a thunk that adds iAdjust to this if it isn't 0.
};


struct SCPP65VtableEntry
{
	ECPP65VtableEntry	eKind;
	int					iVtablePointer;
	SCPP65Function*		psFunction;
	int					iAdjust;
};


typedef CArrayTemplate<SCPP65VtableEntry>	CArrayCPP65VtableEntries;


//GCC's builtin functions the compiler does (CCPP65Generator::FindBuiltin), for the type checker and the IR front end.
enum ECPP65Builtin
{
	CPP65B_Unreachable,		//unreachable and trap.
	CPP65B_Pause,			//ia32_pause: nothing.
	CPP65B_Expect,
	CPP65B_Memcpy,
	CPP65B_Alloca,			//An error: frames are fixed in size.
	CPP65B_BCDAdd,			//The W65C816's decimal mode.
	CPP65B_BCDSubtract,
	CPP65B_Ctz,
	CPP65B_Clz,
	CPP65B_Popcount,
};


//What ctz, clz and popcount count the bits of: unsigned int, unsigned long (l) or unsigned long long (ll).
enum ECPP65BuiltinOperand
{
	CPP65BO_None,
	CPP65BO_UnsignedInt,
	CPP65BO_UnsignedLong,
	CPP65BO_UnsignedLongLong,
};


struct SCPP65Builtin
{
	const char*				szName;			//Without __builtin_.
	ECPP65Builtin			eBuiltin;
	int						iArguments;
	ECPP65BuiltinOperand	eOperand;
	const char*				szW65C816Only;	//The error on another target, NULL if the builtin is every target's.
};


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
	CArrayCPP65FunctionPtrs		mapsMembers;		//Its functions, in the order they were added.
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
	bool						bTypeInfo;			//The type info (for dynamic_cast) has been written.
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
	CCPPScope*						pcNamespace;	//The namespace it was declared in, NULL for the global one: of an instance with no symbol (an operator).
	CChars							mszArguments;
	CArrayCPP65FunctionPtrs			mapsInstances;	//A function template's instances.
};


typedef CArrayTemplatePtr<SCPP65Template>	CArrayCPP65Templates;
typedef CArrayTemplatePtr<CArrayCPP65Types>	CArrayCPP65Packs;
typedef CArrayTemplatePtr<CCPPSymbol>		CArrayCPP65Symbols;


typedef CArrayTemplatePtr<SCPP65Class>		CArrayCPP65Classes;


//A remembered TypeOf: the type of an expression with some template arguments.
struct SCPP65TypeOf
{
	SCPP65Bindings*		psBindings;
	SCPP65Function*		psFunction;
	SCPP65Type*			psType;
};


typedef CArrayTemplatePtr<SCPP65Variable>	CArrayCPP65Variables;
typedef CMapLongTemplate<SCPP65TypeOf>		CMapCPP65TypeOfs;


typedef CArrayTemplate<CChars>				CArrayCPP65Strings;


#define CPP65_BUILTIN_PREFIX			"__builtin_"
#define CPP65_BUILTIN_PREFIX_LENGTH		10


class CCPP65IRFrontEnd;
class CCTInt;
class CCPPSemanticPass;


//Generates 65C64 or W65C816 assembly (Assembler65Lib syntax) from a parsed translation unit: the declarations,
//types, globals and start up here, and every function body as IR (CPP65IRFrontEnd, IR.txt) generated by the
//target's IR back end.  The calling convention and the parts of C++ that are supported are in CPP65Generator.txt.
//
//Check only works out the types of the unit's expressions, for CCPPSemanticPass: no code is generated and it keeps
//going past what it can't type (CPP65GeneratorCheck.cpp).
class CCPP65Generator
{
friend class CCPP65CheckChildren;
friend class CCPP65IRFrontEnd;
friend class CCPP65TypeChecker;
protected:
	CCPPSyntaxTree*			mpcTree;
	CCPPTokeniser*			mpcTokeniser;
	CCPP65Types				mcTypes;
	SCPPTarget*				mpsTarget;

	CChars					mszCode;
	CChars					mszData;
	CChars					mszThunks;			//Adjust this, then jump to a virtual function.
	CArrayChars				maszRuntime;		//The runtime routines' labels the unit calls: linked (soft float), or written into the unit (AppendRuntime).
	CCPPErrors				mcErrors;
	CCPPErrors				mcWarnings;			//Each at most once (by token); they don't fail the unit.

	CArrayCPP65Variables	mapsGlobals;
	CArrayCPP65FunctionPtrs	mapsFunctions;
	CMapCPP65FunctionsOfNames	mcFunctionsOfNames;	//mapsFunctions by name (FirstFunctionOfName).
	CArrayCPP65Classes		mapsClasses;
	CArrayCPP65Templates	mapsTemplates;
	CArrayCPP65BindingsPtrs	mapsAllBindings;
	CArrayCPP65Packs		mapapsPacks;		//The packs' types (NewPack).
	CArrayCPP65Symbols		mapcPackSymbols;	//The symbols made for function parameter packs' elements.
	CCPPSymbol*				mpcPackParameter;	//The function parameter pack of the function being generated, or NULL.
	CArrayCPP65Symbols		mapcPackElements;	//Its elements' symbols.
	CArrayCPP65FunctionPtrs	mapsQueue;			//Functions to generate, first queued first...
	size					muiQueueNext;		//...from this one: those before it have been.
	CArraySyntax			mapcFailed;			//Declarators of functions whose declarations were not supported.
	SCPP65Bindings*			mpsBindings;		//The template arguments of what is being generated or laid out.
	CMapCPP65TypeOfs		mcTypeOfs;			//TypeOf's answers, by expression.
	CArrayChars				maszEmitted;		//Labels of the functions, vtables, thunks and runtime routines written.
	int						miQuiet;			//Errors are not reported, only noted in mbQuietFailed.
	int						miWarningChecks;	//Above 0 while a warning check works out a type (TypeOf can fold the same constant again).
	int						miConstantTypes;	//Above 0 while Constant works out a type (ConstantTypeOf), for the same reason.
	bool					mbWarnNarrowing;	//Implicit conversions of non constant integers to a narrower type are warned about.
	bool					mbQuietFailed;
	CArrayCPP65Strings		maszStrings;
	int						miStartUps;			//R_Initialise and R_Finalise routines written.
	bool						mbDataBss;			//The last BeginData was bss: the global's start up routines associate with bss.label.
	bool					mbPureVirtual;

	CCPPScope*				mpcDeclaringNamespace;	//The namespace Declarations is in, NULL for the global one: of what has no symbol (an operator).
	CArrayScopes			mapcUnnamedNamespaces;	//Their places are the numbers in the labels of what they declare.
	CArrayCPP65Variables	mapsLocals;
	SCPP65Function*			mpsFunction;
	int						miStatics;			//Static locals in the unit.
	bool					mbStaticLocal;		//A static local's data is being written.
	bool					mbLinkageExtern;	//In extern "C" without braces: declarations are extern.
	bool					mbCLinkage;			//In extern "C": functions declared have C linkage.
	bool					mbThis;				//The function being written is a member function with this (not a global's or static local's initialiser).
	int						miLabels;
	CCPP65IRFrontEnd*		mpcIRFrontEnd;		//Writes every function body as IR.
	CCPP65TypeChecker		mcTypeChecker;		//Works out expressions' types (TypeOf).
	CIRImports*				mpcImports;			//Other units' functions the IR inliner may use (SetImports), NULL for none.
	size					muiImportsTurn;		//The unit's turn with them (CIRImports::BeginTurn).
	bool					mbMissedImports;	//The unit must be generated again (MissedImports).
	CAsm65816Profile*			mpcFrameProfile;	//Measured functions, for the W65C816's aligned frames (SetFrameProfile), or NULL.
	bool					mbListing;			//The code has comments with the C++ source lines it is for (SetListing).
	bool					mbLineTable;		//The code has .line and .fingerprint directives, for the linker's line table (SetLineTable).
	bool					mbNoFloats;			//Floating point in the code it writes is an error (SetNoFloats): a kernal's units.
	bool					mbMathTables;		//The W65C816's MathMult is in memory (SetMathTables).
	int64					mlliCompileTime;	//When the unit was compiled (seconds since 1970), for .fingerprint.
	CSpillStore*			mpcSpillStore;		//Where the functions' code waits until the unit is assembled (SetSpillStore),
	uint32					muiSpillUnit;		//by the unit's number.
	CMemorySpillStore		mcMemorySpillStore;	//The spill store when none is set.
	CScratchPadAllocator	mcBackendScratch;	//The W65C816 back end's working arrays and strings, for one function at a time (CIR816Backend::SetScratch).
	CArrayLargeTemplate<uint16>	mauiCodePieces;	//Per function written by the IR front end: how many pieces its code is in.
	CCPPSemanticPass*		mpcSemanticPass;	//Check only: what names resolved to goes here.  NULL when generating.

public:
	void					Init(CCPPSyntaxTree* pcTree, ECPPTarget eTarget = CPPT_65C64);
	void					Kill(void);
	static void				SetDefaultWarnNarrowing(bool bWarnNarrowing);

	bool					Generate(CChars* pszDest);
	bool					Generate(CCPP65Assembly* pcDest);
	bool					GenerateProgram(CChars* pszDest);
	static void				AppendStartUp(CChars* pszDest, ECPPTarget eTarget = CPPT_65C64);
	static void				AppendStartUp816(CChars* pszDest);
	static void				AddDefines(CConfig* pcConfig, ECPPTarget eTarget = CPPT_65C64);
	static void				SetW65C816Assembly(bool bAssembly);
	static bool				IsW65C816Assembly(void);
	CCPPErrors*				GetErrors(void);
	CCPPErrors*				GetWarnings(void);
	int						NumErrors(void);
	void					GetExternals(CArrayChars* paszLabels);
	void					SetImports(CIRImports* pcImports, size uiTurn);
	bool					MissedImports(void);
	bool					GenerateBeforeTurn(void);
	bool					GenerateAfterTurn(CCPP65Assembly* pcDest);
	CArrayChars*			GetNotImported(void);
	void					SetFrameProfile(CAsm65816Profile* pcProfile);
	void					SetListing(bool bListing);
	void					SetLineTable(bool bLineTable, int64 lliCompileTime);
	void					SetNoFloats(bool bNoFloats);
	void					SetMathTables(bool bMathTables);
	void					SetSpillStore(CSpillStore* pcStore, uint32 uiUnit = 0);
	CScratchPadAllocator*	GetBackendScratch(void);
	void					GetMissingCallees(CArrayChars* paszNames);
	bool					Check(CCPPSemanticPass* pcPass);

protected:
	//Declarations, functions and statements.  CPP65Generator.cpp
	bool					Error(CCPPSyntax* pcNode, const char* szMessage);
	void					Warning(CCPPSyntax* pcNode, const char* szMessage);

	bool					ErrorAt(int iToken, const char* szMessage);
	void					AppendMacroUse(SPreprocessedToken* psSource, CChars* pszMessage);
	void					BeginQuiet(void);
	bool					EndQuiet(void);
	CChars*					Line(const char* szText);
	void					AppendName(CChars* psz, int64 lliName);
	int64					GetName(CCPPSyntaxDeclarator* pcDeclarator);

	SCPP65Function*			DeclareFunction(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, bool bDefinition, SCPP65Class* psClass, bool bPure, SCPP65Template* psTemplate = NULL, CChars* pszArguments = NULL);
	SCPP65Function*			AddFunction(SCPP65Class* psClass, int64 lliName, ECPP65FunctionKind eKind, SCPP65Type* psReturn);
	SCPP65Function*			FirstFunctionOfName(int64 lliName);
	bool					IsOverloaded(SCPP65Function* psFunction);
	bool					SameParameters(SCPP65Function* psFunction, SCPP65Function* psOther);
	void					RequireFunction(SCPP65Function* psFunction);
	bool					Emit(CChars* pszLabel);
	bool					Declarations(CArraySyntax* papcDeclarations);
	bool					IsScopeDeclaration(CCPPSyntax* pcDeclaration);
	bool					IsOutsideMemberTemplate(CCPPSyntax* pcDeclaration);
	bool					GlobalDeclaration(CCPPSyntaxSimpleDeclaration* pcDeclaration);
	bool					GlobalInitialiser(SCPP65Type* psType, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode, bool* pbInitialise);
	void					AppendData(SCPP65Type* psType, int64 lliValue);
	void					AppendZeros(int iBytes);
	void					AppendGlobalLabel(CChars* psz, SCPP65Variable* psVariable);
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
	bool					GenerateQueued(void);
	bool					GenerateDefinition(SCPP65Function* psFunction);
	void					AddParameters(SCPP65Function* psFunction);
	void					BeginFunction(SCPP65Function* psFunction);
	void					EndFunction(SCPP65Function* psFunction);
	void					AppendUnsupported(CChars* psz);
	bool					AllocateLocals(CCPPSyntax* pcNode);
	bool					StaticLocal(CCPPSyntaxInitDeclarator* pcInit, CCPPSyntaxDeclSpecifiers* pcSpecifiers);
	bool					AnonymousLocal(CCPPSyntaxClassSpecifier* pcSpecifier);
	bool					StaticAssert(CCPPSyntaxStaticAssert* pcAssert);
	bool					IsCLinkage(CCPPSyntaxLinkageSpecification* pcLinkage);
	void					AppendVariableLabel(CChars* psz, SCPP65Variable* psVariable);
	void					AppendStrings(CChars* pszDest);
	void					AppendRuntime(void);
	void					AppendRuntime816(void);
	void					AppendDivideSigned(const char* szName, int iBytes);
	void					BeginRuntime(const char* szName);
	void					AppendStartUpEntry(const char* szTable, CChars* pszLabel, CChars* pszData);
	bool					GlobalStartUpRoutines(SCPP65Type* psType, CCPPSyntax* pcInitialiser, CChars* pszData, bool bConstruct, bool bDestroy);
	void					AppendSection(CChars* psz, const char* szKind, CChars* pszName, const char* szOptions);
	bool					HasUsedAttribute(CCPPSyntaxDeclarator* pcDeclarator);
	bool					IsRestrictPointer(CCPPSyntaxDeclarator* pcDeclarator);
	SCPP65Type*				QualifiedPointer(SCPP65Type* psType, CCPPSyntaxPointerOperator* pcPointer);
	SCPP65Type*				ParameterType(SCPP65Type* psType);
	SCPP65Type*				ParameterVariableType(SCPP65Type* psType, CCPPSyntaxParameter* pcParameter);
	void					BeginData(CChars* pszLabel, int iAlignment, bool bGlobal, const char* szOptions, bool bBss = false);
	bool					IsBss(CCPPSyntax* pcInitialiser, SCPP65Type* psType);
	void					AppendSpace(int iBytes);
	int						DataAlignment(SCPP65Type* psType, bool bClass);
	void					AppendGlobal(CChars* psz, CChars* pszLabel);
	bool					IsUnitFunction(SCPP65Function* psFunction);

	void					ReportTypeOf(CCPPSyntax* pcExpression);
	bool					ReportError(CCPPSyntax* pcNode, const char* szMessage);
	bool					ReportErrorAt(int iToken, const char* szMessage);
	bool					HasWarning(CCPPSyntax* pcNode, const char* szText);

	//Classes, templates, virtual functions, overloads and labels.  CPP65GeneratorClasses.cpp, CPP65GeneratorTemplates.cpp,
	//CPP65GeneratorVtables.cpp, CPP65GeneratorOverloads.cpp and CPP65GeneratorLabels.cpp
	SCPP65Class*			NewClass(CCPPSymbol* pcSymbol, int64 lliName);
	SCPP65Class*			GetClass(CCPPSymbol* pcSymbol, CCPPSyntax* pcNode);
	SCPP65Class*			GetUnnamedClass(CCPPSyntaxClassSpecifier* pcSpecifier);
	SCPP65Class*			FindClass(int64 lliName, CCPPScope* pcLookup);
	CCPPScope*				ComponentLookup(CCPPSyntaxNameComponent* pcComponent);
	static bool				IsInstanceNamed(SCPP65Class* psClass, CCPPSymbol* pcSymbol);
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
	SCPP65Template*			GetTemplateByName(int64 lliName, ECPPSyntax eDefinition, CCPPScope* pcLookup);
	SCPP65Template*			GetComponentTemplate(CCPPSyntaxNameComponent* pcComponent);
	SCPP65Bindings*			BindArguments(CCPPSyntaxTemplateDeclaration* pcTemplate, CArraySyntax* papcArguments, CArrayCPP65Types* papsDeduced, CChars* pszArguments, CCPPSyntax* pcNode, CArrayInt64* palliDeduced = NULL);
	SCPP65Binding*			FindBinding(CCPPSymbol* pcParameter);
	SCPP65Class*			InstantiateClass(SCPP65Template* psTemplate, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	SCPP65Class*			UndefinedInstance(CCPPSymbol* pcSymbol, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	SCPP65Function*			InstantiateFunction(SCPP65Template* psTemplate, CArraySyntax* papcExplicit, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	SCPP65Template*			NewTemplate(CCPPSymbol* pcSymbol, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntax* pcDefinition, bool bMember);
	bool					MemberTemplate(SCPP65Class* psClass, CCPPSyntaxTemplateDeclaration* pcTemplate);
	SCPP65Template*			FindMemberTemplate(SCPP65Class* psClass, int64 lliName, size uiArguments, int iConstObject = -1);
	SCPP65Template*			ChooseMemberTemplate(SCPP65Class* psClass, int64 lliName, CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcExplicit, int iConstObject);
	void					GetMemberTemplates(SCPP65Class* psClass, int64 lliName, size uiArguments, CArrayCPP65Templates* papsCandidates);
	SCPP65Template*			GetCallTemplate(CCPPSymbol* pcSymbol, int64 lliName, CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcExplicit);
	void					AddCallTemplates(CCPPSymbol* pcSymbol, CCPPSyntaxCallExpression* pcCall, CArrayCPP65Templates* papsCandidates);
	SCPP65Template*			ChooseTemplate(CArrayCPP65Templates* papsCandidates, int iConstObject, CArraySyntax* papcExplicit, CCPPSyntaxCallExpression* pcCall);
	bool					TemplateTakes(SCPP65Template* psTemplate, CArraySyntax* papcExplicit, CArraySyntax* papcArguments, CCPPSyntax* pcNode, int* aiRanks);
	bool					TakesArguments(CCPPSyntaxFunctionSuffix* pcSuffix, size uiArguments);
	bool					IsConstSuffix(CCPPSyntaxFunctionSuffix* pcSuffix);
	bool					DefineMemberTemplate(SCPP65Template* psTemplate, CCPPSyntax* pcNode);
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
	void					DeduceArguments(SCPP65Template* psTemplate, CArrayCPP65Types* papsArguments, CArraySyntax* papcArguments, CArrayCPP65Types* papsDeduced, CArrayInt64* palliDeduced);
	void					DeduceParameters(SCPP65Template* psTemplate, CCPPSyntaxFunctionSuffix* pcSuffix, CArrayCPP65Types* papsArguments, CArraySyntax* papcArguments, CArrayCPP65Types* papsDeduced, CArrayInt64* palliDeduced);
	bool					DeduceParameter(SCPP65Template* psTemplate, CCPPSyntaxParameter* pcParameter, CArrayCPP65Types* papsArguments, CArraySyntax* papcArguments, size i, CArrayCPP65Types* papsDeduced, CArrayInt64* palliDeduced);
	bool					IsForwardingReference(CCPPSyntaxParameter* pcParameter);
	SCPP65Type*				ForwardedType(SCPP65Type* psArgument, CCPPSyntax* pcArgument);
	void					DeduceMemberPointer(SCPP65Template* psTemplate, CCPPSyntaxDeclarator* pcDeclarator, SCPP65Type* psArgument, CArrayCPP65Types* papsDeduced, CArrayInt64* palliDeduced);
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
	void					EnsureTypeInfo(SCPP65Class* psClass);
	void					AppendTypeInfoBases(SCPP65Class* psClass, int iOffset);
	void					AppendVtableHeader(SCPP65Class* psClass, int iVtablePointer);
	int						VtablePointerOffset(SCPP65Class* psClass);
	void					AppendVirtualCallThunk(CChars* psz, SCPP65Function* psFunction);
	void					AppendThunkLabel(CChars* pszLabel, bool bComdat);
	void					AppendVtableEntry(SCPP65Function* psFunction, int iAdjust);
	void					VtableEntries(SCPP65Class* psClass, CArrayCPP65VtableEntries* pasEntries);
	void					AppendVtableEntryLabel(CChars* psz, SCPP65Function* psFunction, int iAdjust);
	SCPP65Field*			FindField(SCPP65Class* psClass, int64 lliName, int* piOffset);
	SCPP65Field*			FindFieldSymbol(SCPP65Class* psClass, CCPPSymbol* pcSymbol, int* piOffset);
	SCPP65Variable*			FindStatic(SCPP65Class* psClass, int64 lliName);
	void					ClassUsing(SCPP65Class* psClass, CCPPSyntaxUsingDeclaration* pcUsing);
	void					InheritConstructors(SCPP65Class* psClass);
	bool					IsOverloadOf(SCPP65Class* psClass, SCPP65Function* psFunction, int64 lliName);
	SCPP65Function*			FindFunction(SCPP65Class* psClass, int64 lliName, ECPP65FunctionKind eKind, CArraySyntax* papcArguments, bool bBases, CCPPSyntax* pcNode, int iConstObject = -1, CArrayScopes* papcNamespaces = NULL);
	SCPP65Function*			FindNamedFunction(CCPPSyntaxName* pcName, int64 lliName, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	SCPP65Function*			FindAddressedFunction(CCPPSyntaxName* pcName, int64 lliName, SCPP65Type* psTarget, CCPPSyntax* pcNode, bool* pbOverloaded);
	SCPP65Function*			FindAssociatedFunction(CCPPSyntaxName* pcName, int64 lliName, CCPPSyntaxCallExpression* pcCall);
	void					GetOverloadNamespaces(CCPPSyntaxName* pcName, int64 lliName, CArrayScopes* papcNamespaces);
	void					GetCallNamespaces(CCPPSyntaxName* pcName, int64 lliName, CArraySyntax* papcArguments, CArrayScopes* papcNamespaces);
	void					AddArgumentNamespaces(CArraySyntax* papcArguments, CArrayScopes* papcNamespaces);
	void					AddAssociatedNamespaces(SCPP65Type* psType, CArrayScopes* papcNamespaces, int iDepth);
	void					AddNamespaceOf(CCPPScope* pcScope, CArrayScopes* papcNamespaces);
	static void				NormaliseNamespaces(CArrayScopes* papcNamespaces);
	bool					IsConstObject(CCPPSyntax* pcExpression);
	bool					IsConstPointer(CCPPSyntax* pcExpression);
	bool					IsConstThis(void);
	bool					RankArguments(SCPP65Function* psFunction, SCPP65Type** apsArguments, CArraySyntax* papcArguments, int* aiRanks);
	int						ConversionRank(SCPP65Type* psParameter, SCPP65Type* psArgument, CCPPSyntax* pcArgument, bool bUserDefined = true);
	bool					HasConvertingConstructor(SCPP65Class* psClass, SCPP65Type* psArgument, CCPPSyntax* pcArgument);
	int						CompareRanks(int* aiFirst, int* aiSecond, size uiArguments);
	int						QualificationRank(SCPP65Type* psTo, SCPP65Type* psFrom, bool bTopLevel);
	static bool				AddsQualifiers(SCPP65Type* psTo, SCPP65Type* psFrom);
	static bool				TakesObject(SCPP65Function* psFunction, int iConstObject);
	int						CompareQualifiers(SCPP65Function* psFirst, SCPP65Function* psSecond, CArraySyntax* papcArguments, int iConstObject);
	bool					IsDerived(SCPP65Class* psClass, SCPP65Class* psBase, int* piOffset);
	bool					NeedsConstructor(SCPP65Class* psClass);
	bool					NeedsDestructor(SCPP65Class* psClass);
	void					AppendFunctionLabel(CChars* psz, SCPP65Function* psFunction);
	void					AppendClassLabel(CChars* psz, SCPP65Class* psClass, const char* szPart);
	void					AppendVtableLabel(CChars* psz, SCPP65Class* psClass);
	void					AppendTypeInfoLabel(CChars* psz, SCPP65Class* psClass);
	void					AppendClassName(CChars* psz, SCPP65Class* psClass);
	void					AppendScopeNames(CChars* psz, CCPPScope* pcScope);
	static CCPPScope*		NamespaceOf(CCPPScope* pcScope);
	void					AppendFunctionName(CChars* psz, int64 lliName);
	void					AppendMangledType(CChars* psz, SCPP65Type* psType);
	int64					GetFunctionName(CCPPSyntaxNameComponent* pcComponent);
	int64					GetOperatorName(ECppTokens eToken, bool bArray);
	const char*				GetOperatorLabel(int64 lliName);
	void					SetDefinition(SCPP65Function* psFunction, CCPPSyntaxFunctionDefinition* pcDefinition);
	SCPP65Type*				ArrowType(CCPPSyntax* pcObject, CCPPSyntax* pcNode);
	SCPP65Function*			FindConversion(SCPP65Class* psClass, SCPP65Type* psTo, bool bExplicit, CCPPSyntax* pcNode);
	int64					GetComponentName(CCPPSyntaxNameComponent* pcComponent);

	void					AppendAdjustThunk816(int iAdjust);
	void					AppendVirtualCallThunk816(SCPP65Function* psFunction);

	void					NoFunctionMatches(SCPP65Class* psClass, int64 lliName, ECPP65FunctionKind eKind, SCPP65Type** apsArguments, size uiArguments, CCPPSyntax* pcNode);

	//Literals, constants, warnings and TypeOf.  CPP65GeneratorExpressions.cpp
	SCPP65Type*				IntegerLiteralType(CCTInt* pcInt);

	CCPPSymbol*				NewPackElement(CCPPSymbol* pcPack);
	bool					HasPackExpansion(CArraySyntax* papcArguments);
	CCPPSyntax*				ExpandPackArguments(CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments);
	void					RestoreArguments(CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments);

	bool					IsBuiltin(int64 lliName);
	const SCPP65Builtin*	FindBuiltin(int64 lliName);
	SCPP65Type*				BuiltinOperandType(const SCPP65Builtin* psBuiltin);
	bool					IsIntrinsic(int64 lliName);
	char*					BuiltinName(int64 lliName, int* piLength);
	bool					QuietConstant(CCPPSyntax* pcExpression, int64* plliValue);
	bool					IsInlineMemcpy(int64 lliName, CArraySyntax* papcArguments);

	bool					IsAssignmentOperator(ECppTokens eOperator);

	int						AddString(char* szBytes, size uiLength);
	bool					FunctionNameText(int64 lliName, CChars* pszText);
	bool					IsFunctionName(CCPPSyntaxIdExpression* pcId);
	SCPP65Type*				Literal(CCPPSyntaxLiteralExpression* pcLiteral);
	CCPPSyntaxIdExpression*	FunctionNameFor(CCPPSyntax* pcExpression, SCPP65Type* psTo);
	CCPPSyntaxIdExpression*	FunctionNameOf(CCPPSyntax* pcExpression);
	bool					HasOverloadOfType(CCPPSyntaxIdExpression* pcFunction, SCPP65Type* psPointer);
	SCPP65Function*			FindStaticFunction(SCPP65Class* psClass, int64 lliName, SCPP65Type* psTarget, bool* pbOverloaded);
	bool					IsMemberPointerOperator(CCPPSyntax* pcExpression);
	bool					HasMember(SCPP65Class* psClass, int64 lliName);
	bool					HasOperator(SCPP65Class* psClass, int64 lliName, size uiArguments);
	SCPP65Function*			FindOperator(SCPP65Class* psClass, int64 lliName, CArraySyntax* papcArguments, CCPPSyntax* pcNode, int iConstObject);
	CCPPSymbol*				QualifiedEnumerator(CCPPSyntaxName* pcName, CCPPSyntax* pcNode, SCPP65Class** ppsClass);
	CCPPSymbol*				QualifiedMember(CCPPSyntaxName* pcName, ECPPSymbolKind eKind, CCPPSyntax* pcNode, SCPP65Class** ppsClass);
	CCPPSymbol*				ClassMember(SCPP65Class* psClass, int64 lliName, ECPPSymbolKind eKind, SCPP65Class** ppsClass);
	void					ReportFailedDeclaration(int64 lliName);
	SCPP65Type*				TypeOf(CCPPSyntax* pcExpression);
	SCPP65Type*				ValueTypeOf(CCPPSyntax* pcExpression);
	bool					CheckInitialiser(CCPPSyntax* pcValue, SCPP65Type* psTo);

	SCPP65Type*				PlaceTypeOf(CCPPSyntax* pcExpression);

	bool					Supported(SCPP65Type* psType, CCPPSyntax* pcNode);


	bool					Constant(CCPPSyntax* pcExpression, int64* plliValue);
	int64					ConstantOfType(SCPP65Type* psType, int64 lliValue);
	SCPP65Type*				ConstantTypeOf(CCPPSyntax* pcExpression);
	bool					BinaryConstant(CCPPSyntaxBinaryExpression* pcBinary, int64 lliLeft, int64 lliRight, int64* plliValue);
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
	CCPPSyntax*				SkipParentheses(CCPPSyntax* pcExpression);
	bool					DecodeString(CCPPSyntax* pcString, CArrayUint8* pauiBytes);
	bool					DecodeStringToken(CCPPSyntax* pcString, CCTString* pcToken, CArrayUint8* pauiBytes);

	//Objects and copies.  CPP65GeneratorObjects.cpp
	bool					HasCopyConstructor(SCPP65Class* psClass);
	SCPP65Function*			GetCopyConstructor(SCPP65Class* psClass);
	bool					IsTriviallyCopyable(SCPP65Class* psClass);
	bool					IsObject(CCPPSyntax* pcExpression);
	SCPP65Type*				CastClassType(CCPPSyntax* pcExpression);
	SCPP65Class*			ObjectClass(CCPPSyntax* pcExpression);
	bool					IsMadeObject(CCPPSyntax* pcExpression, SCPP65Class* psClass);
	SCPP65Type*				PlaceOf(CCPPSyntax* pcExpression);
	bool					BindsToTemporary(SCPP65Type* psReference, CCPPSyntax* pcArgument);
	bool					MakesObject(CCPPSyntax* pcExpression);
	CCPPScope*				CurrentLookupScope(void);
	SCPP65Function*			FindFreeOperator(int64 lliName, CArraySyntax* papcArguments, CCPPSyntax* pcNode);

	//Check only.  CPP65GeneratorCheck.cpp
	bool					CheckDefinitions(void);
	void					CheckClasses(CArraySyntax* papcDeclarations);
	void					CheckFunction(SCPP65Function* psFunction);
	void					CheckNode(CCPPSyntax* pcNode);
	void					CheckChildren(CCPPSyntax* pcNode);
	void					CheckExpression(CCPPSyntax* pcExpression);
	void					CheckLocal(CCPPSyntaxInitDeclarator* pcInit);
	bool					IsCheckedExpression(CCPPSyntax* pcNode);
	void					Resolved(CCPPSyntaxName* pcName, SCPP65Function* psFunction);
	void					Resolved(CCPPSyntaxName* pcName, CCPPSymbol* pcSymbol);

	//Floating point.  CPP65GeneratorFloat.cpp
	int64					FloatBits(SCPP65Type* psType, double dValue);
	bool					FloatConstant(CCPPSyntax* pcExpression, double* pdValue);
	bool					FloatBuiltin(CCPPSyntax* pcExpression, double* pdValue, SCPP65Type** ppsType);
	bool					FloatValue(CCPPSyntax* pcExpression, double* pdValue, SCPP65Type** ppsType);
};


#endif // __C_P_P_65_GENERATOR_H__
