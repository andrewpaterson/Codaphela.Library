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
#ifndef __C_P_P_65_GENERATOR_H__
#define __C_P_P_65_GENERATOR_H__
//The memory map (the user, 2026-10-23): 0000:0000 to 0000:FFFF are pseudo registers and
//scratch memory, programs start at 0001:0000 and the stack works down from 001F:FFFC.
#define CPP65_PROGRAM_ADDRESS		0x10000
#define CPP65_STACK_TOP				0x1FFFFF
#define CPP65_HEAP_ADDRESS			0x80000		//Libc65's malloc heap (stdlib.cpp HEAP_START); a program ends below it.
#define CPP65_MAX_ASSOCIATED_DEPTH	16		//How deep a type's bases and template arguments are followed for associated namespaces.
#define CPP65_INLINE_MEMCPY			64		//memcpy of at most this many bytes, a constant, is copied in place...
#define CPP65_INLINE_MEMCPY_SIZE	8		//...or, preferring size (CCPPCompileOptions::mePrefer), this many: about what the call's own code is.
#define CPP65_CONVERSION_NAME		(-1 - 2 * (int64)NUM_CPP_TOKENS)	//The name of every conversion function (operator T); they differ by their result.
#define CPP65_BACKEND_SCRATCH_CHUNK	(256 KB)	//The back end's arena (CCPP65Generator::GetBackendScratch) grows by chunks this big.
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
#include "Assembler65Lib/AsmW65C816Profile.h"
#include "CPPTarget.h"
#include "CPPErrors.h"
#include "Config.h"
#include "CPP65Assembly.h"
#include "CPP65TypeChecker.h"
#include "CPPCompileStatistics.h"
#include "CPPCompileOptions.h"


enum ECPPStorage
{
	CPPVS_Local,
	CPPVS_Parameter,
	CPPVS_Global,
	CPPVS_Static,		//A static local: data, numbered in the unit by iOffset.
};


class CCPPClass;
class CCPPFunction;
class CCTString;
class CCPPTemplate;


//A template parameter's value in an instantiation: a type, or a constant for a non-type parameter.
struct SCPPBinding
{
	CCPPSymbol*		pcParameter;
	SCPP65Type*		psType;
	int64			lliValue;
	CArrayTemplatePtr<SCPP65Type>*	papsPack;	//A parameter pack's types.
};


typedef CArrayTemplate<SCPPBinding>	CArrayCPPBindings;


//The template arguments of an instantiation.  pcOuter is the class instance a member template is in.  pcClass
//is the instance the bindings belong to, for the class's own name used without arguments inside it.
class CCPPTemplateArguments
{
public:
	CArrayCPPBindings		macBindings;
	CCPPTemplateArguments*			mpcOuter;
	CCPPClass*			mpcClass;
};


typedef CArrayTemplatePtr<CCPPTemplateArguments>	CArrayCPPTemplateArguments;


//A variable the generator has given storage.
class CCPPVariable
{
public:
	CCPPSymbol*		mpcSymbol;
	SCPP65Type*		mpsType;
	ECPPStorage	meStorage;
	int				miOffset;		//Parameters: the index in the function's parameters.  Static locals: the number (the label's S<n>_).  Members of an anonymous union or struct (pcAnonymous): the offset in it.
	CCPPClass*	mpcClass;		//The class of a static data member.
	CCPPSyntax*		mpcInClassInitialiser;	//static const int k = 5; in the class: what its definition has if it has none.
	bool			mbInternal;		//A static global, or one in a namespace without a name: the unit's own.
	bool			mbCLinkage;		//Declared in extern "C": its label is only its name.
	bool			mbDefined;		//A global's data has been written.
	bool			mbUsed;
	bool			mbRestrict;		//A global or parameter pointer declared __restrict.
	CCPPClass*	mpcAnonymous;	//The anonymous union or struct declared in a function that this local is a member of.
};


enum ECPPFunctionKind
{
	CPPFK_Function,
	CPPFK_Constructor,
	CPPFK_Destructor,
};


//A function's signature, from its definition or a declaration, and its definition.  Inline, in class and
//template functions are generated when they are first called; others when their definition is reached.
class CCPPFunction
{
public:
	int64							mlliName;		//An identifier, or an operator (see GetOperatorName).
	SCPP65Type*						mpsReturn;
	CArrayCPP65Types				mapsParameters;
	CArraySyntax					mapcDefaults;	//Each parameter's default argument, or NULL.
	bool							mbVariadic;
	bool							mbDefined;
	CCPPClass*					mpcClass;		//NULL for a function that is not a member.
	CCPPScope*						mpcNamespace;	//The namespace of a function that is not a member, NULL for the global one.
	ECPPFunctionKind				meKind;
	bool							mbStatic;
	bool							mbVirtual;
	bool							mbPure;
	int								miSlot;			//A virtual function's place in the vtable.
	bool							mbCLinkage;		//Declared in extern "C": its label is only its name.
	bool							mbExplicit;		//An explicit constructor or conversion function: not used for implicit conversions.
	bool							mbConstThis;		//A const member function: an overload of the same function without const.
	bool							mbImplicit;		//A constructor or destructor the generator wrote.
	CCPPFunction*					mpcInherited;	//The base's constructor an inherited constructor calls (using Base::Base).
	CCPPSyntaxFunctionDefinition*	mpcDefinition;
	CCPPSyntaxDeclarator*			mpcDeclarator;	//The first declaration's declarator (a template's, for an instance).
	CCPPTemplateArguments*					mpcBindings;		//The template arguments the definition is generated with.
	bool							mbInline;		//Generated when used.
	bool							mbQueued;
	bool							mbGenerated;
	bool							mbCalled;
	CCPPTemplate*					mpcTemplate;		//The function template this is an instance of.
	CChars							mszArguments;	//The instance's template arguments, mangled for its label.
	CCPPFunction*					mpcNextOfName;	//The next function added with the same lliName (see FirstFunctionOfName).

	size	RequiredParameters(void);
};


typedef CArrayTemplatePtr<CCPPFunction>	CArrayCPPFunctions;


//The functions of a name, in the order they were added: pcFirst then each pcNextOfName.
struct SCPPFunctionsOfName
{
	CCPPFunction*	pcFirst;
	CCPPFunction*	pcLast;
};


typedef CMapLongTemplate<SCPPFunctionsOfName>	CMapCPPFunctionsOfNames;


//How well an argument converts to a parameter, best last.
enum ECPPConversionRank
{
	CPPCR_None,
	CPPCR_Ellipsis,
	CPPCR_UserDefined,		//A conversion function whose result then needs a conversion.
	CPPCR_UserDefinedExact,	//A converting constructor, or a conversion function whose result matches (or promotes).
	CPPCR_Boolean,		//A pointer to bool, worse than the other conversions.
	CPPCR_Conversion,
	CPPCR_Promotion,
	CPPCR_Qualification,	//An exact match but for const or volatile added: worse than the same type.  [over.ics.rank] 3.2.6
	CPPCR_Exact,
};


//A non-static data member.  Offsets include the base class's.
struct SCPPField
{
	CCPPSymbol*		pcSymbol;
	int64			lliName;
	SCPP65Type*		psType;
	int				iOffset;
	int				iAlign;				//Its alignas, 0 for none.
	CCPPSyntax*		pcInitialiser;		//The default member initialiser, or NULL.
	bool			bMutable;			//Declared mutable: not const in a const object.
	int				iBitWidth;			//A bit-field's width, -1 for a field that isn't one.  An unnamed bit-field (int : 3;) has no symbol.
	int				iBitOffset;			//A bit-field's first bit in the unit of its type at iOffset, 0 the lowest.

	bool			IsBitField(void) { return iBitWidth >= 0; }
	bool			IsUnnamedBitField(void) { return (iBitWidth >= 0) && (pcSymbol == NULL); }
};


typedef CArrayTemplate<SCPPField>		CArrayCPPFields;


//An error in a class's own declaration (a bit-field's width), found when it is laid out: kept apart from mcErrors,
//which quiet trials and the IR front end truncate, and reported when the unit ends (ReportClassErrors).
struct SCPPClassError
{
	int				iToken;
	const char*		szMessage;
};


typedef CArrayTemplate<SCPPClassError>	CArrayCPPClassErrors;


//A base class and where it is in the derived class.
struct SCPPBase
{
	CCPPClass*	pcClass;
	int				iOffset;
};


typedef CArrayTemplate<SCPPBase>		CArrayCPPBases;


//using Base::f; in a class: Base's functions named f are overloads of the class's own.
struct SCPPUsing
{
	CCPPClass*	pcBase;
	int64			lliName;
};


typedef CArrayTemplate<SCPPUsing>		CArrayCPPUsings;


//A secondary vtable: the one for the polymorphic base subobject at iOffset that does not share the class's
//vtable pointer.  It is laid out like pcOwner's vtable.  A function whose object is not that subobject is
//called through a thunk that adds maiAdjust's entry to this first.
struct SCPPSecondaryVtable
{
	int							iOffset;
	CCPPClass*				pcOwner;
	CArrayCPPFunctions		mapcFunctions;
	CArrayInt					maiAdjust;
};


typedef CArrayTemplate<SCPPSecondaryVtable>	CArrayCPPSecondaryVtables;


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


enum ECPPVtableEntry
{
	CPPVE_Header,		//A part's header: iVtablePointer is where the part's vtable pointer is in the object.
	CPPVE_Function,	//pcFunction, through a thunk that adds iAdjust to this if it isn't 0.
};


struct SCPPVtableEntry
{
	ECPPVtableEntry	eKind;
	int					iVtablePointer;
	CCPPFunction*		pcFunction;
	int					iAdjust;
};


typedef CArrayTemplate<SCPPVtableEntry>	CArrayCPPVtableEntries;


//GCC's builtin functions the compiler does (CCPP65Generator::FindBuiltin), for the type checker and the IR front end.
enum ECPPBuiltin
{
	CPPB_Unreachable,		//unreachable and trap.
	CPPB_Pause,			//ia32_pause: nothing.
	CPPB_Expect,
	CPPB_Memcpy,
	CPPB_Alloca,			//An error: frames are fixed in size.
	CPP65B_BCDAdd,			//The W65C816's decimal mode.
	CPP65B_BCDSubtract,
	CPPB_Ctz,
	CPPB_Clz,
	CPPB_Popcount,
};


//What ctz, clz and popcount count the bits of: unsigned int, unsigned long (l) or unsigned long long (ll).
enum ECPPBuiltinOperand
{
	CPPBO_None,
	CPPBO_UnsignedInt,
	CPPBO_UnsignedLong,
	CPPBO_UnsignedLongLong,
};


struct SCPPBuiltin
{
	const char*				szName;			//Without __builtin_.
	ECPPBuiltin			eBuiltin;
	int						iArguments;
	ECPPBuiltinOperand	eOperand;
	const char*				szDecimalModeOnly;	//The error on a target with no decimal mode (CCPPTarget::HasDecimalMode), NULL if the builtin is every target's.
};


//A class's layout: the bases in order (an empty base takes no space), then the vtable pointer if no base has
//one, then the fields.  A class is laid out when it is first needed whole.  pcBase is the first base.
class CCPPClass
{
public:
	CCPPSymbol*					mpcSymbol;
	int64						mlliName;
	SCPP65Type*					mpsType;
	CCPPClass*				mpcBase;
	CArrayCPPBases			macBases;
	CArrayCPPUsings			macUsings;
	CArrayCPPFields			macFields;
	CArrayCPPFunctions		mapcMembers;		//Its functions, in the order they were added.
	CArrayCPPFunctions		mapcVirtuals;		//The vtable.
	int							miVtableOffset;		//-1 if the class has no vtable of its own.
	CArrayCPPSecondaryVtables			macVtables;			//The secondary vtables, written after the vtable.
	bool						mbComplete;
	bool						mbLayingOut;
	bool						mbBases;				//macBases is filled in (their offsets only once it is complete).
	bool						mbEmpty;				//No fields, no vtable pointer and empty bases.
	int							miAlign;
	bool						mbUnion;
	bool						mbVtable;			//The vtable has been written.
	bool						mbTypeInfo;			//The type info (for dynamic_cast) has been written.
	bool						mbConstructors;		//The class declares a constructor.
	CCPPClass*				mpcInheritedBase;	//The base whose constructors are inherited (using Base::Base), or NULL.
	CCPPFunction*				mpcDestructor;
	CCPPSyntaxClassSpecifier*	mpcSpecifier;		//The definition, NULL until it is seen.
	CCPPTemplate*				mpcTemplate;			//The class template this is an instance of.
	CCPPTemplateArguments*				mpcBindings;
	CChars						mszArguments;		//The instance's template arguments, mangled for its label.
};


//A class or function template, a member function of a class template defined outside the class, or a member
//template.  A member template is kept for each class (or class template instance) it is in (pcClass).  One
//defined outside its class is kept once, with pcClass NULL, and found by the class's name (pcSymbol) and its
//own name (lliName); mpcClassDeclaration is the class template's parameters if the class is a template.  An
//explicit or partial specialisation has the template it specialises as pcPrimary (a template is its own
//primary otherwise) and its template arguments in pcPattern.  An outside member function's class template is
//pcOwner, the primary or a partial specialisation.
class CCPPTemplate
{
public:
	CCPPSymbol*						mpcSymbol;		//The template's name; for an outside member, the class's.
	CCPPSyntaxTemplateDeclaration*	mpcDeclaration;
	CCPPSyntax*						mpcDefinition;	//A class specifier or a function definition.
	bool							mbMember;
	bool							mbMemberTemplate;
	CCPPClass*					mpcClass;
	int64							mlliName;
	CCPPSyntaxTemplateDeclaration*	mpcClassDeclaration;
	CCPPSyntaxDeclarator*			mpcDeclarator;	//A member template's declarator in its class.
	bool							mbStatic;
	CCPPTemplate*					mpcPrimary;
	CCPPSyntaxNameComponent*		mpcPattern;
	CCPPTemplate*					mpcOwner;
	bool							mbArguments;		//mszArguments is worked out (a function specialisation's).
	CCPPScope*						mpcNamespace;	//The namespace it was declared in, NULL for the global one: of an instance with no symbol (an operator).
	CChars							mszArguments;
	CArrayCPPFunctions			mapcInstances;	//A function template's instances.
};


typedef CArrayTemplatePtr<CCPPTemplate>	CArrayCPPTemplates;
typedef CArrayTemplatePtr<CArrayCPP65Types>	CArrayCPPPacks;
typedef CArrayTemplatePtr<CCPPSymbol>		CArrayCPPSymbols;


typedef CArrayTemplatePtr<CCPPClass>		CArrayCPPClasses;


//A remembered TypeOf: the type of an expression with some template arguments.
struct SCPPTypeOf
{
	CCPPTemplateArguments*		pcBindings;
	CCPPFunction*		pcFunction;
	SCPP65Type*			psType;
};


typedef CArrayTemplatePtr<CCPPVariable>	CArrayCPPVariables;
typedef CMapLongTemplate<SCPPTypeOf>		CMapCPPTypeOfs;


typedef CArrayTemplate<CChars>				CArrayCPPStrings;


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
class CCPP65Generator : public CCPPRuntimeWriter
{
friend class CCPP65CheckChildren;
friend class CCPP65IRFrontEnd;
friend class CCPP65TypeChecker;
protected:
	CCPPSyntaxTree*			mpcTree;
	CCPPTokeniser*			mpcTokeniser;
	CCPP65Types				mcTypes;
	CCPPTarget*				mpcTarget;

	CChars					mszCode;
	CChars					mszData;
	CChars					mszThunks;			//Adjust this, then jump to a virtual function.
	CArrayChars				maszRuntime;		//The runtime routines' labels the unit calls: linked (soft float), or written into the unit (AppendRuntime).
	CCPPErrors				mcErrors;
	CArrayCPPClassErrors	masClassErrors;		//Errors laying out classes, reported by ReportClassErrors.
	CCPPErrors				mcWarnings;			//Each at most once (by token); they don't fail the unit.

	CArrayCPPVariables	mapcGlobals;
	CArrayCPPFunctions	mapcFunctions;
	CMapCPPFunctionsOfNames	mcFunctionsOfNames;	//mapcFunctions by name (FirstFunctionOfName).
	CArrayCPPClasses		mapcClasses;
	CArrayCPPTemplates	mapcTemplates;
	CArrayCPPTemplateArguments	mapcAllBindings;
	CArrayCPPPacks		mapapsPacks;		//The packs' types (NewPack).
	CArrayCPPSymbols		mapcPackSymbols;	//The symbols made for function parameter packs' elements.
	CCPPSymbol*				mpcPackParameter;	//The function parameter pack of the function being generated, or NULL.
	CArrayCPPSymbols		mapcPackElements;	//Its elements' symbols.
	CArrayCPPFunctions	mapcQueue;			//Functions to generate, first queued first...
	size					muiQueueNext;		//...from this one: those before it have been.
	CArraySyntax			mapcFailed;			//Declarators of functions whose declarations were not supported.
	CCPPTemplateArguments*			mpcBindings;		//The template arguments of what is being generated or laid out.
	CMapCPPTypeOfs		mcTypeOfs;			//TypeOf's answers, by expression.
	CArrayChars				maszEmitted;		//Labels of the functions, vtables, thunks and runtime routines written.
	int						miQuiet;			//Errors are not reported, only noted in mbQuietFailed.
	int						miWarningChecks;	//Above 0 while a warning check works out a type (TypeOf can fold the same constant again).
	CArraySyntax			mapcConstantTypes;	//The expressions whose types Constant is finding (ConstantTypeOf).
	bool					mbQuietFailed;
	CArrayCPPStrings		maszStrings;
	int						miStartUps;			//R_Initialise and R_Finalise routines written.
	bool						mbDataBss;			//The last BeginData was bss: the global's start up routines associate with bss.label.
	bool					mbPureVirtual;

	CCPPScope*				mpcDeclaringNamespace;	//The namespace Declarations is in, NULL for the global one: of what has no symbol (an operator).
	CArrayScopes			mapcUnnamedNamespaces;	//Their places are the numbers in the labels of what they declare.
	CArrayCPPVariables	mapcLocals;
	CCPPFunction*			mpcFunction;
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
	CAsmW65C816Profile*			mpcFrameProfile;	//Measured functions, for the W65C816's aligned frames (SetFrameProfile), or NULL.
	bool					mbListing;			//The code has comments with the C++ source lines it is for (SetListing).
	bool					mbLineTable;		//The code has .line and .fingerprint directives, for the linker's line table (SetLineTable).
	bool					mbNoFloats;			//Floating point in the code it writes is an error (SetNoFloats): a kernal's units.
	int64					mlliCompileTime;	//When the unit was compiled (seconds since 1970), for .fingerprint.
	CSpillStore*			mpcSpillStore;		//Where the functions' code waits until the unit is assembled (SetSpillStore),
	uint32					muiSpillUnit;		//by the unit's number.
	CMemorySpillStore		mcMemorySpillStore;	//The spill store when none is set.
	CScratchPadAllocator	mcBackendScratch;	//The W65C816 back end's working arrays and strings, for one function at a time (CIRW65C816Backend::SetScratch).
	CArrayLargeTemplate<uint16>	mauiCodePieces;	//Per function written by the IR front end: how many pieces its code is in.
	CCPPSemanticPass*		mpcSemanticPass;	//Check only: what names resolved to goes here.  NULL when generating.
	CCPPCompileStatistics*	mpcStatistics;		//What compiling the unit is counted and timed in (SetStatistics): the builder's for the unit...
	CCPPCompileStatistics	mcOwnStatistics;	//...or the generator's own when none is set.
	CCPPCompileOptions*		mpcOptions;			//How the unit is compiled where there is a choice (SetOptions): the builder's, or the defaults.

public:
	void					Init(CCPPSyntaxTree* pcTree, ECPPTarget eTarget = CPPT_65C64);
	void					Kill(void);

	bool					Generate(CChars* pszDest);
	bool					Generate(CCPP65Assembly* pcDest);
	bool					GenerateProgram(CChars* pszDest);
	CCPPErrors*				GetErrors(void);
	CCPPErrors*				GetWarnings(void);
	int						NumErrors(void);
	void					GetExternals(CArrayChars* paszLabels);
	void					SetImports(CIRImports* pcImports, size uiTurn);
	bool					MissedImports(void);
	bool					GenerateBeforeTurn(void);
	bool					GenerateAfterTurn(CCPP65Assembly* pcDest);
	CArrayChars*			GetNotImported(void);
	void					SetFrameProfile(CAsmW65C816Profile* pcProfile);
	void					SetListing(bool bListing);
	void					SetLineTable(bool bLineTable, int64 lliCompileTime);
	void					SetNoFloats(bool bNoFloats);
	void					SetSpillStore(CSpillStore* pcStore, uint32 uiUnit = 0);
	void					SetStatistics(CCPPCompileStatistics* pcStatistics);
	void					SetOptions(CCPPCompileOptions* pcOptions);
	CCPPCompileOptions*		GetOptions(void);
	CCPPCompileStatistics*	GetStatistics(void);
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

	CCPPFunction*			DeclareFunction(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, bool bDefinition, CCPPClass* pcClass, bool bPure, CCPPTemplate* pcTemplate = NULL, CChars* pszArguments = NULL);
	CCPPFunction*			AddFunction(CCPPClass* pcClass, int64 lliName, ECPPFunctionKind eKind, SCPP65Type* psReturn);
	CCPPFunction*			FirstFunctionOfName(int64 lliName);
	bool					IsOverloaded(CCPPFunction* pcFunction);
	bool					SameParameters(CCPPFunction* pcFunction, CCPPFunction* pcOther);
	void					RequireFunction(CCPPFunction* pcFunction);
	bool					Emit(CChars* pszLabel);
	bool					Declarations(CArraySyntax* papcDeclarations);
	bool					IsScopeDeclaration(CCPPSyntax* pcDeclaration);
	bool					IsOutsideMemberTemplate(CCPPSyntax* pcDeclaration);
	bool					GlobalDeclaration(CCPPSyntaxSimpleDeclaration* pcDeclaration);
	bool					GlobalInitialiser(SCPP65Type* psType, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode, bool* pbInitialise);
	void					AppendData(SCPP65Type* psType, int64 lliValue);
	void					AppendZeros(int iBytes);
	void					AppendGlobalLabel(CChars* psz, CCPPVariable* pcVariable);
	CCPPVariable*			AddVariable(CArrayCPPVariables* papcVariables, CCPPSymbol* pcSymbol, SCPP65Type* psType, ECPPStorage eStorage, int iOffset);
	void					FreeVariables(CArrayCPPVariables* papcVariables);
	CCPPVariable*			GetVariable(CCPPSymbol* pcSymbol);
	SCPP65Type*				GetType(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, CCPPSyntax* pcNode, CCPPSyntax* pcInitialiser = NULL);
	SCPP65Type*				GetDeclaratorType(SCPP65Type* psType, CCPPSyntaxDeclarator* pcDeclarator, CCPPSyntax* pcNode, CCPPSyntax* pcInitialiser, CCPPSyntaxFunctionSuffix* pcFunction = NULL);
	SCPP65Type*				GetFunctionType(SCPP65Type* psReturn, CCPPSyntaxFunctionSuffix* pcSuffix, CCPPSyntax* pcNode);
	int						GetInitialiserCount(CCPPSyntax* pcInitialiser);
	CCPPSyntax*				StringInitialiser(CCPPSyntax* pcInitialiser, SCPP65Type* psElement);
	SCPP65Type*				GetBaseType(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntax* pcNode);
	SCPP65Type*				GetTypeIdType(CCPPSyntaxTypeId* pcTypeId);
	SCPP65Type*				GetSymbolType(CCPPSymbol* pcSymbol, CCPPSyntax* pcNode);

	bool					Function(CCPPSyntaxFunctionDefinition* pcDefinition, CCPPClass* pcClass);
	void					GenerateFunction(CCPPFunction* pcFunction);
	bool					GenerateQueued(void);
	bool					GenerateDefinition(CCPPFunction* pcFunction);
	bool					WriteBody(CCPPFunction* pcFunction);
	void					AddParameters(CCPPFunction* pcFunction);
	void					BeginFunction(CCPPFunction* pcFunction);
	void					EndFunction(CCPPFunction* pcFunction);
	void					AppendUnsupported(CChars* psz);
	bool					AllocateLocals(CCPPSyntax* pcNode);
	bool					StaticLocal(CCPPSyntaxInitDeclarator* pcInit, CCPPSyntaxDeclSpecifiers* pcSpecifiers);
	bool					AnonymousLocal(CCPPSyntaxClassSpecifier* pcSpecifier);
	bool					StaticAssert(CCPPSyntaxStaticAssert* pcAssert);
	bool					IsCLinkage(CCPPSyntaxLinkageSpecification* pcLinkage);
	void					AppendVariableLabel(CChars* psz, CCPPVariable* pcVariable);
	void					AppendStrings(CChars* pszDest);
	void					AppendRuntime(void);
	CArrayChars*			GetRuntimeCalls(void) override;
	bool					BeginRuntime(const char* szName) override;
	CChars*					GetRuntimeCode(void) override;
	void					RuntimeError(const char* szMessage) override;
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
	bool					IsUnitFunction(CCPPFunction* pcFunction);

	void					ReportTypeOf(CCPPSyntax* pcExpression);
	bool					ReportError(CCPPSyntax* pcNode, const char* szMessage);
	bool					ReportErrorAt(int iToken, const char* szMessage);
	bool					HasWarning(CCPPSyntax* pcNode, const char* szText);

	//Classes, templates, virtual functions, overloads and labels.  CPP65GeneratorClasses.cpp, CPP65GeneratorTemplates.cpp,
	//CPP65GeneratorVtables.cpp, CPP65GeneratorOverloads.cpp and CPP65GeneratorLabels.cpp
	CCPPClass*			NewClass(CCPPSymbol* pcSymbol, int64 lliName);
	CCPPClass*			GetClass(CCPPSymbol* pcSymbol, CCPPSyntax* pcNode);
	CCPPClass*			GetUnnamedClass(CCPPSyntaxClassSpecifier* pcSpecifier);
	CCPPClass*			FindClass(int64 lliName, CCPPScope* pcLookup);
	CCPPScope*				ComponentLookup(CCPPSyntaxNameComponent* pcComponent);
	static bool				IsInstanceNamed(CCPPClass* pcClass, CCPPSymbol* pcSymbol);
	CCPPClass*			GetComponentClass(CCPPSyntaxNameComponent* pcComponent, CCPPSymbol* pcSymbol, CCPPSyntax* pcNode, CCPPClass* pcContext = NULL);
	CCPPClass*			FindBaseInstance(CCPPClass* pcClass, CCPPTemplate* pcTemplate);
	CCPPClass*			QualifierClass(CCPPClass* pcClass, CCPPSyntaxNameComponent* pcComponent, CCPPSyntax* pcNode);
	int						TypeAlignment(SCPP65Type* psType);
	int						PackAlignment(int iAlign, int iPack);
	int						AlignasAlignment(CArraySyntax* papcSpecifiers);
	CCPPClass*			GetNameClass(CCPPSyntaxName* pcName, CCPPSyntax* pcNode);
	bool					TemplateDeclaration(CCPPSyntaxTemplateDeclaration* pcTemplateDeclaration);
	bool					ExplicitInstantiation(CCPPSyntax* pcDeclaration);
	CCPPTemplate*			GetTemplate(CCPPSymbol* pcSymbol);
	CCPPTemplate*			GetTemplateByName(int64 lliName, ECPPSyntax eDefinition, CCPPScope* pcLookup);
	CCPPTemplate*			GetComponentTemplate(CCPPSyntaxNameComponent* pcComponent);
	CCPPTemplateArguments*			BindArguments(CCPPSyntaxTemplateDeclaration* pcTemplate, CArraySyntax* papcArguments, CArrayCPP65Types* papsDeduced, CChars* pszArguments, CCPPSyntax* pcNode, CArrayInt64* palliDeduced = NULL);
	SCPPBinding*			FindBinding(CCPPSymbol* pcParameter);
	CCPPClass*			InstantiateClass(CCPPTemplate* pcTemplate, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	CCPPClass*			UndefinedInstance(CCPPSymbol* pcSymbol, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	CCPPFunction*			InstantiateFunction(CCPPTemplate* pcTemplate, CArraySyntax* papcExplicit, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	CCPPTemplate*			NewTemplate(CCPPSymbol* pcSymbol, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntax* pcDefinition, bool bMember);
	bool					MemberTemplate(CCPPClass* pcClass, CCPPSyntaxTemplateDeclaration* pcTemplateDeclaration);
	CCPPTemplate*			FindMemberTemplate(CCPPClass* pcClass, int64 lliName, size uiArguments, int iConstObject = -1);
	CCPPTemplate*			ChooseMemberTemplate(CCPPClass* pcClass, int64 lliName, CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcExplicit, int iConstObject);
	void					GetMemberTemplates(CCPPClass* pcClass, int64 lliName, size uiArguments, CArrayCPPTemplates* papcCandidates);
	CCPPTemplate*			GetCallTemplate(CCPPSymbol* pcSymbol, int64 lliName, CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcExplicit);
	void					AddCallTemplates(CCPPSymbol* pcSymbol, CCPPSyntaxCallExpression* pcCall, CArrayCPPTemplates* papcCandidates);
	CCPPTemplate*			ChooseTemplate(CArrayCPPTemplates* papcCandidates, int iConstObject, CArraySyntax* papcExplicit, CCPPSyntaxCallExpression* pcCall);
	bool					TemplateTakes(CCPPTemplate* pcTemplate, CArraySyntax* papcExplicit, CArraySyntax* papcArguments, CCPPSyntax* pcNode, int* aiRanks);
	bool					TakesArguments(CCPPSyntaxFunctionSuffix* pcSuffix, size uiArguments);
	bool					IsConstSuffix(CCPPSyntaxFunctionSuffix* pcSuffix);
	bool					DefineMemberTemplate(CCPPTemplate* pcTemplate, CCPPSyntax* pcNode);
	CCPPTemplateArguments*			RenameBindings(CCPPClass* pcClass, CCPPSyntaxTemplateDeclaration* pcDeclaration);
	CCPPTemplateArguments*			NewBindings(void);
	CArrayCPP65Types*		NewPack(void);
	SCPPBinding*			PackBinding(CCPPSyntaxParameter* pcParameter);

	CCPPTemplate*			Specialisation(CCPPSymbol* pcSymbol, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntax* pcDefinition, CCPPSyntaxNameComponent* pcPattern);
	CCPPTemplate*			OutsideMemberOwner(CCPPTemplate* pcPrimary, CCPPSyntaxNameComponent* pcClass, CCPPSyntaxTemplateDeclaration* pcDeclaration);
	CCPPTemplate*			FindClassSpecialisation(CCPPTemplate* pcPrimary, CCPPTemplateArguments* pcArguments, CCPPTemplateArguments** ppcBindings, CCPPSyntax* pcNode);
	bool					MatchPattern(CCPPTemplate* pcSpecialisation, CArraySyntax* papcPattern, CCPPTemplateArguments* pcArguments, CCPPTemplateArguments* pcDeduced, int* piScore);
	bool					MatchType(CCPPTemplate* pcSpecialisation, CCPPSyntaxTypeId* pcPattern, SCPPBinding* psArgument, CCPPTemplateArguments* pcDeduced, int* piScore);
	SCPPBinding*			DeducedBinding(CCPPTemplateArguments* pcDeduced, CCPPSymbol* pcParameter);
	void					DeduceArguments(CCPPTemplate* pcTemplate, CArrayCPP65Types* papsArguments, CArraySyntax* papcArguments, CArrayCPP65Types* papsDeduced, CArrayInt64* palliDeduced);
	void					DeduceParameters(CCPPTemplate* pcTemplate, CCPPSyntaxFunctionSuffix* pcSuffix, CArrayCPP65Types* papsArguments, CArraySyntax* papcArguments, CArrayCPP65Types* papsDeduced, CArrayInt64* palliDeduced);
	bool					DeduceParameter(CCPPTemplate* pcTemplate, CCPPSyntaxParameter* pcParameter, CArrayCPP65Types* papsArguments, CArraySyntax* papcArguments, size i, CArrayCPP65Types* papsDeduced, CArrayInt64* palliDeduced);
	bool					IsForwardingReference(CCPPSyntaxParameter* pcParameter);
	SCPP65Type*				ForwardedType(SCPP65Type* psArgument, CCPPSyntax* pcArgument);
	void					DeduceMemberPointer(CCPPTemplate* pcTemplate, CCPPSyntaxDeclarator* pcDeclarator, SCPP65Type* psArgument, CArrayCPP65Types* papsDeduced, CArrayInt64* palliDeduced);
	void					DeduceFromInstance(CCPPTemplate* pcTemplate, CCPPSyntaxNameComponent* pcComponent, SCPP65Type* psArgument, CArrayCPP65Types* papsDeduced, CArrayInt64* palliDeduced);
	CCPPClass*			FindInstanceBase(CCPPClass* pcClass, CCPPSymbol* pcTemplate);
	CCPPSymbol*				GetArgumentSymbol(CCPPSyntax* pcArgument);
	CCPPTemplate*			FindFunctionSpecialisation(CCPPTemplate* pcPrimary, CChars* pszArguments, CCPPSyntax* pcNode);
	bool					CompleteClass(CCPPClass* pcClass, CCPPSyntax* pcNode);
	bool					ResolveBases(CCPPClass* pcClass, CCPPSyntax* pcNode);
	bool					LayoutClass(CCPPClass* pcClass, CCPPSyntax* pcNode);
	void					LayoutBitField(CCPPClass* pcClass, SCPPField* psField, int iPack, int* piBytes, int* piAlign, int* piUnitBytes, int* piUnitOffset, int* piUnitBits);
	bool					ClassMember(CCPPClass* pcClass, CCPPSyntax* pcMember);
	bool					ClassBitField(CCPPClass* pcClass, CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxInitDeclarator* pcInit);
	bool					ClassError(int iToken, const char* szMessage);
	void					ReportClassErrors(void);
	bool					LayoutVirtuals(CCPPClass* pcClass, CCPPSyntax* pcNode);
	bool					Overrides(CCPPFunction* pcVirtual, CCPPFunction* pcFunction);
	bool					OverridesBase(CCPPClass* pcClass, CCPPFunction* pcFunction);
	void					LayoutSecondaryVtables(CCPPClass* pcClass);
	void					AddSecondaryVtable(CCPPClass* pcClass, int iOffset, CCPPClass* pcOwner, CArrayCPPFunctions* papcFunctions, CArrayInt* paiAdjust);
	bool					IsPolymorphic(CCPPClass* pcClass);
	bool					IsAbstract(CCPPClass* pcClass);
	void					EnsureVtable(CCPPClass* pcClass);
	void					EnsureTypeInfo(CCPPClass* pcClass);
	void					AppendTypeInfoBases(CCPPClass* pcClass, int iOffset);
	void					AppendVtableHeader(CCPPClass* pcClass, int iVtablePointer);
	int						VtablePointerOffset(CCPPClass* pcClass);
	void					AppendVirtualCallThunk(CChars* psz, CCPPFunction* pcFunction);
	void					AppendThunkLabel(CChars* pszLabel, bool bComdat);
	void					AppendVtableEntry(CCPPFunction* pcFunction, int iAdjust);
	void					VtableEntries(CCPPClass* pcClass, CArrayCPPVtableEntries* pasEntries);
	void					AppendVtableEntryLabel(CChars* psz, CCPPFunction* pcFunction, int iAdjust);
	SCPPField*			FindField(CCPPClass* pcClass, int64 lliName, int* piOffset);
	size				NumAggregateFields(CCPPClass* pcClass);
	SCPPField*			FindFieldSymbol(CCPPClass* pcClass, CCPPSymbol* pcSymbol, int* piOffset);
	CCPPVariable*			FindStatic(CCPPClass* pcClass, int64 lliName);
	void					ClassUsing(CCPPClass* pcClass, CCPPSyntaxUsingDeclaration* pcUsing);
	void					InheritConstructors(CCPPClass* pcClass);
	bool					IsOverloadOf(CCPPClass* pcClass, CCPPFunction* pcFunction, int64 lliName);
	CCPPFunction*			FindFunction(CCPPClass* pcClass, int64 lliName, ECPPFunctionKind eKind, CArraySyntax* papcArguments, bool bBases, CCPPSyntax* pcNode, int iConstObject = -1, CArrayScopes* papcNamespaces = NULL);
	CCPPFunction*			FindNamedFunction(CCPPSyntaxName* pcName, int64 lliName, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	CCPPFunction*			FindAddressedFunction(CCPPSyntaxName* pcName, int64 lliName, SCPP65Type* psTarget, CCPPSyntax* pcNode, bool* pbOverloaded);
	CCPPFunction*			FindAssociatedFunction(CCPPSyntaxName* pcName, int64 lliName, CCPPSyntaxCallExpression* pcCall);
	void					GetOverloadNamespaces(CCPPSyntaxName* pcName, int64 lliName, CArrayScopes* papcNamespaces);
	void					GetCallNamespaces(CCPPSyntaxName* pcName, int64 lliName, CArraySyntax* papcArguments, CArrayScopes* papcNamespaces);
	void					AddArgumentNamespaces(CArraySyntax* papcArguments, CArrayScopes* papcNamespaces);
	void					AddAssociatedNamespaces(SCPP65Type* psType, CArrayScopes* papcNamespaces, int iDepth);
	void					AddNamespaceOf(CCPPScope* pcScope, CArrayScopes* papcNamespaces);
	static void				NormaliseNamespaces(CArrayScopes* papcNamespaces);
	bool					IsConstObject(CCPPSyntax* pcExpression);
	bool					IsConstPointer(CCPPSyntax* pcExpression);
	bool					IsConstThis(void);
	bool					RankArguments(CCPPFunction* pcFunction, SCPP65Type** apsArguments, CArraySyntax* papcArguments, int* aiRanks);
	int						ConversionRank(SCPP65Type* psParameter, SCPP65Type* psArgument, CCPPSyntax* pcArgument, bool bUserDefined = true);
	bool					HasConvertingConstructor(CCPPClass* pcClass, SCPP65Type* psArgument, CCPPSyntax* pcArgument);
	int						CompareRanks(int* aiFirst, int* aiSecond, size uiArguments);
	int						QualificationRank(SCPP65Type* psTo, SCPP65Type* psFrom, bool bTopLevel);
	static bool				AddsQualifiers(SCPP65Type* psTo, SCPP65Type* psFrom);
	static bool				TakesObject(CCPPFunction* pcFunction, int iConstObject);
	int						CompareQualifiers(CCPPFunction* pcFirst, CCPPFunction* pcSecond, CArraySyntax* papcArguments, int iConstObject);
	bool					IsDerived(CCPPClass* pcClass, CCPPClass* pcBase, int* piOffset);
	bool					NeedsConstructor(CCPPClass* pcClass);
	bool					NeedsDestructor(CCPPClass* pcClass);
	void					AppendFunctionLabel(CChars* psz, CCPPFunction* pcFunction);
	void					AppendClassLabel(CChars* psz, CCPPClass* pcClass, const char* szPart);
	void					AppendVtableLabel(CChars* psz, CCPPClass* pcClass);
	void					AppendTypeInfoLabel(CChars* psz, CCPPClass* pcClass);
	void					AppendClassName(CChars* psz, CCPPClass* pcClass);
	void					AppendScopeNames(CChars* psz, CCPPScope* pcScope);
	static CCPPScope*		NamespaceOf(CCPPScope* pcScope);
	void					AppendFunctionName(CChars* psz, int64 lliName);
	void					AppendMangledType(CChars* psz, SCPP65Type* psType);
	int64					GetFunctionName(CCPPSyntaxNameComponent* pcComponent);
	int64					GetOperatorName(ECppTokens eToken, bool bArray);
	const char*				GetOperatorLabel(int64 lliName);
	void					SetDefinition(CCPPFunction* pcFunction, CCPPSyntaxFunctionDefinition* pcDefinition);
	SCPP65Type*				ArrowType(CCPPSyntax* pcObject, CCPPSyntax* pcNode);
	CCPPFunction*			FindConversion(CCPPClass* pcClass, SCPP65Type* psTo, bool bExplicit, CCPPSyntax* pcNode);
	int64					GetComponentName(CCPPSyntaxNameComponent* pcComponent);

	void					NoFunctionMatches(CCPPClass* pcClass, int64 lliName, ECPPFunctionKind eKind, SCPP65Type** apsArguments, size uiArguments, CCPPSyntax* pcNode);

	//Literals, constants, warnings and TypeOf.  CPP65GeneratorExpressions.cpp
	SCPP65Type*				IntegerLiteralType(CCTInt* pcInt);

	CCPPSymbol*				NewPackElement(CCPPSymbol* pcPack);
	bool					HasPackExpansion(CArraySyntax* papcArguments);
	CCPPSyntax*				ExpandPackArguments(CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments);
	void					RestoreArguments(CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments);

	bool					IsBuiltin(int64 lliName);
	const SCPPBuiltin*	FindBuiltin(int64 lliName);
	SCPP65Type*				BuiltinOperandType(const SCPPBuiltin* psBuiltin);
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
	CCPPFunction*			FindStaticFunction(CCPPClass* pcClass, int64 lliName, SCPP65Type* psTarget, bool* pbOverloaded);
	bool					IsMemberPointerOperator(CCPPSyntax* pcExpression);
	bool					HasMember(CCPPClass* pcClass, int64 lliName);
	bool					HasOperator(CCPPClass* pcClass, int64 lliName, size uiArguments);
	CCPPFunction*			FindOperator(CCPPClass* pcClass, int64 lliName, CArraySyntax* papcArguments, CCPPSyntax* pcNode, int iConstObject);
	CCPPSymbol*				QualifiedEnumerator(CCPPSyntaxName* pcName, CCPPSyntax* pcNode, CCPPClass** ppcClass);
	CCPPSymbol*				QualifiedMember(CCPPSyntaxName* pcName, ECPPSymbolKind eKind, CCPPSyntax* pcNode, CCPPClass** ppcClass);
	CCPPSymbol*				ClassMember(CCPPClass* pcClass, int64 lliName, ECPPSymbolKind eKind, CCPPClass** ppcClass);
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
	bool					DecodeStringToken(CCPPSyntax* pcString, CCTString* pcToken, int iElementBytes, CArrayUint8* pauiBytes);
	int						StringElementBytes(CCPPSyntax* pcString);
	SCPP65Type*				StringElementType(CCPPSyntax* pcString);
	uint32					StringElement(CArrayUint8* pauiBytes, int iElementBytes, size uiElement);

	//Objects and copies.  CPP65GeneratorObjects.cpp
	bool					HasCopyConstructor(CCPPClass* pcClass);
	CCPPFunction*			GetCopyConstructor(CCPPClass* pcClass);
	bool					IsTriviallyCopyable(CCPPClass* pcClass);
	bool					IsObject(CCPPSyntax* pcExpression);
	bool					IsOverloadedComma(CCPPSyntaxBinaryExpression* pcBinary);
	SCPP65Type*				CastClassType(CCPPSyntax* pcExpression);
	CCPPClass*			ObjectClass(CCPPSyntax* pcExpression);
	bool					IsMadeObject(CCPPSyntax* pcExpression, CCPPClass* pcClass);
	SCPP65Type*				PlaceOf(CCPPSyntax* pcExpression);
	bool					BindsToTemporary(SCPP65Type* psReference, CCPPSyntax* pcArgument);
	bool					MakesObject(CCPPSyntax* pcExpression);
	CCPPScope*				CurrentLookupScope(void);
	CCPPFunction*			FindFreeOperator(int64 lliName, CArraySyntax* papcArguments, CCPPSyntax* pcNode);

	//Check only.  CPP65GeneratorCheck.cpp
	bool					CheckDefinitions(void);
	CArraySyntax*			ConstructElements(CCPPSyntaxConstructExpression* pcConstruct);
	void					DeclareBlockFunction(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator);
	void					CheckClasses(CArraySyntax* papcDeclarations);
	void					CheckFunction(CCPPFunction* pcFunction);
	void					CheckNode(CCPPSyntax* pcNode);
	void					CheckChildren(CCPPSyntax* pcNode);
	void					CheckExpression(CCPPSyntax* pcExpression);
	void					CheckLocal(CCPPSyntaxInitDeclarator* pcInit);
	bool					IsCheckedExpression(CCPPSyntax* pcNode);
	void					Resolved(CCPPSyntaxName* pcName, CCPPFunction* pcFunction);
	void					Resolved(CCPPSyntaxName* pcName, CCPPSymbol* pcSymbol);

	//Floating point.  CPP65GeneratorFloat.cpp
	int64					FloatBits(SCPP65Type* psType, double dValue);
	bool					FloatConstant(CCPPSyntax* pcExpression, double* pdValue);
	bool					FloatBuiltin(CCPPSyntax* pcExpression, double* pdValue, SCPP65Type** ppsType);
	bool					FloatValue(CCPPSyntax* pcExpression, double* pdValue, SCPP65Type** ppsType);
};


#endif // __C_P_P_65_GENERATOR_H__
