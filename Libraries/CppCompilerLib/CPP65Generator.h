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
#include "CPPVariable.h"
#include "CPPFunction.h"
#include "CPPTemplate.h"
#include "CPPClass.h"
#include "CPPClassLayout.h"
#include "CPPMangler.h"
#include "IRImports.h"
#include "Assembler65Lib/Asm65Profile.h"
#include "CPPTarget.h"
#include "CPPErrors.h"
#include "Config.h"
#include "CPP65Assembly.h"
#include "CPP65TypeChecker.h"
#include "CPPCompileStatistics.h"
#include "CPPCompileOptions.h"


class CCTString;
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


//An error in a class's own declaration (a bit-field's width), found when it is laid out: kept apart from mcErrors,
//which quiet trials and the IR front end truncate, and reported when the unit ends (ReportClassErrors).
struct SCPPClassError
{
	int				iToken;
	const char*		szMessage;
};


typedef CArrayTemplate<SCPPClassError>	CArrayCPPClassErrors;


//GCC's builtin functions the compiler does (CCPP65Generator::FindBuiltin), for the type checker and the IR front end.
enum ECPPBuiltin
{
	CPPB_Unreachable,		//unreachable and trap.
	CPPB_Pause,			//ia32_pause: nothing.
	CPPB_Expect,
	CPPB_Memcpy,
	CPPB_Alloca,			//An error: frames are fixed in size.
	CPPB_BCDAdd,			//The W65C816's decimal mode.
	CPPB_BCDSubtract,
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


typedef CArrayTemplatePtr<CArrayCPP65Types>	CArrayCPPPacks;
typedef CArrayTemplatePtr<CCPPSymbol>		CArrayCPPSymbols;


//A remembered TypeOf: the type of an expression with some template arguments.
struct SCPPTypeOf
{
	CCPPTemplateArguments*		pcBindings;
	CCPPFunction*		pcFunction;
	SCPP65Type*			psType;
};


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
	CCPPMangler				mcMangler;			//Makes the labels.
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
	CAsm65Profile*			mpcFrameProfile;	//Measured functions, for the W65C816's aligned frames (SetFrameProfile), or NULL.
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
	void					SetFrameProfile(CAsm65Profile* pcProfile);
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
	void					RequireFunction(CCPPFunction* pcFunction);
	bool					Emit(CChars* pszLabel);
	bool					Declarations(CArraySyntax* papcDeclarations);
	bool					IsScopeDeclaration(CCPPSyntax* pcDeclaration);
	bool					IsOutsideMemberTemplate(CCPPSyntax* pcDeclaration);
	bool					GlobalDeclaration(CCPPSyntaxSimpleDeclaration* pcDeclaration);
	bool					GlobalInitialiser(SCPP65Type* psType, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode, bool* pbInitialise);
	void					AppendData(SCPP65Type* psType, int64 lliValue);
	void					AppendZeros(int iBytes);
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
	CCPPClass*			GetComponentClass(CCPPSyntaxNameComponent* pcComponent, CCPPSymbol* pcSymbol, CCPPSyntax* pcNode, CCPPClass* pcContext = NULL);
	CCPPClass*			FindBaseInstance(CCPPClass* pcClass, CCPPTemplate* pcTemplate);
	CCPPClass*			QualifierClass(CCPPClass* pcClass, CCPPSyntaxNameComponent* pcComponent, CCPPSyntax* pcNode);
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
	bool					ClassMember(CCPPClass* pcClass, CCPPSyntax* pcMember);
	bool					ClassBitField(CCPPClass* pcClass, CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxInitDeclarator* pcInit);
	bool					ClassError(int iToken, const char* szMessage);
	void					ReportClassErrors(void);
	void					EnsureVtable(CCPPClass* pcClass);
	void					EnsureTypeInfo(CCPPClass* pcClass);
	void					AppendTypeInfoBases(CCPPClass* pcClass, int iOffset);
	void					AppendVtableHeader(CCPPClass* pcClass, int iVtablePointer);
	void					AppendVirtualCallThunk(CChars* psz, CCPPFunction* pcFunction);
	void					AppendThunkLabel(CChars* pszLabel, bool bComdat);
	void					AppendVtableEntry(CCPPFunction* pcFunction, int iAdjust);
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
	int						CompareQualifiers(CCPPFunction* pcFirst, CCPPFunction* pcSecond, CArraySyntax* papcArguments, int iConstObject);
	bool					IsDerived(CCPPClass* pcClass, CCPPClass* pcBase, int* piOffset);
	int64					GetFunctionName(CCPPSyntaxNameComponent* pcComponent);
	int64					GetOperatorName(ECppTokens eToken, bool bArray);
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
