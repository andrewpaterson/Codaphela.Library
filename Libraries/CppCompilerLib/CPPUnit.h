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
#ifndef __C_P_P_UNIT_H__
#define __C_P_P_UNIT_H__
//The memory map (the user, 2026-10-23): 0000:0000 to 0000:FFFF are pseudo registers and
//scratch memory, programs start at 0001:0000 and the stack works down from 001F:FFFC.
#define CPP65_PROGRAM_ADDRESS		0x10000
#define CPP_EXCEPTIONS_LABEL		"R_Exceptions"	//Defined by a program built with exceptions (CCPPUnit::AppendRuntime).
#define CPP65_STACK_TOP				0x1FFFFF
#define CPP65_HEAP_ADDRESS			0x80000		//Libc65's malloc heap (stdlib.cpp HEAP_START); a program ends below it.
#define CPP65_MAX_ASSOCIATED_DEPTH	16		//How deep a type's bases and template arguments are followed for associated namespaces.
#define CPP65_INLINE_MEMCPY			64		//memcpy of at most this many bytes, a constant, is copied in place...
#define CPP65_INLINE_MEMCPY_SIZE	8		//...or, preferring size (CCPPCompileOptions::mePrefer), this many: about what the call's own code is.
#define CPP65_BACKEND_SCRATCH_CHUNK	(256 KB)	//The back end's arena (CCPPUnit::GetBackendScratch) grows by chunks this big.
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayUint8.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/ScratchPadAllocator.h"
#include "BaseLib/ArrayInt.h"
#include "BaseLib/ArrayInt64.h"
#include "BaseLib/MapLongTemplate.h"
#include "BaseLib/HashMapPrimitiveTemplate.h"
#include "CPPSyntaxes.h"
#include "CPPSymbol.h"
#include "CPP65Type.h"
#include "CPPVariable.h"
#include "CPPFunction.h"
#include "CPPTemplate.h"
#include "CPPClass.h"
#include "CPPClassLayout.h"
#include "CPPMangler.h"
#include "CPPContext.h"
#include "CPPDiagnostics.h"
#include "CPPUnitData.h"
#include "CPP65DebugWriter.h"
#include "CPPBinder.h"
#include "CPPDeclarators.h"
#include "CPPFunctionDeclarer.h"
#include "CPPGlobals.h"
#include "CPPClasses.h"
#include "CPPDeduction.h"
#include "IRImports.h"
#include "Assembler65Lib/Asm65Profile.h"
#include "CPPTarget.h"
#include "CPPErrors.h"
#include "Config.h"
#include "CPP65Assembly.h"
#include "CPP65TypeChecker.h"
#include "CPPCompileStatistics.h"
#include "CPPCompileOptions.h"
#include "CPPLambda.h"
#include "CPPTypeWriter.h"
#include "CPPConstantFolder.h"
#include "CPPClosures.h"
#include "CPPClassData.h"
#include "CPPStrings.h"
#include "CPPCheck.h"
#include "CPPUnitObserver.h"
#include "CPPWarnings.h"
#include "CPPBuiltins.h"
#include "CPPExceptions.h"
#include "CPPDesugarings.h"
#include "CPPInstantiator.h"


class CCTString;
class CCTUserLiteral;
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
	CPPCR_RvalueToConst,	//An object with no name bound to const T&: exact, but T&& takes it better.  [over.ics.rank] 3.2.3
	CPPCR_Exact,
};


typedef CArrayTemplatePtr<CArrayCPP65Types>	CArrayCPPPacks;
typedef CArrayTemplatePtr<CArrayInt64>		CArrayCPPValuePacks;


//A template parameter pack's element as a parameter of its own (CCPPUnit::PackElement), in the bindings it is bound
//in; or a function parameter pack's as a variable of its own (FunctionPackElement), in the function it is of.
struct SCPPPackElement
{
	CCPPTemplateArguments*	pcBindings;
	CCPPFunction*			pcFunction;
	CCPPSymbol*				pcPack;
	size					uiIndex;
	CCPPSymbol*				pcElement;
};


typedef CArrayTemplate<SCPPPackElement>	CArrayCPPPackElements;


//An argument list with its pack expansions expanded, for the bindings and function it was expanded in
//(CCPPUnit::ExpandArguments).  papcExpanded is NULL if it couldn't be.
struct SCPPExpandedArguments
{
	CArraySyntax*			papcArguments;
	CCPPTemplateArguments*	pcBindings;
	CCPPFunction*			pcIn;
	CArraySyntax*			papcExpanded;
};


typedef CArrayTemplate<SCPPExpandedArguments>	CArrayCPPExpandedArgumentLists;


//What a function has while it is entered (CCPPUnit::EnterFunction): its parameters and locals, and the function
//parameter packs it can expand.  The unit's first is no function's: what is asked of when none is entered.
struct SCPPFunctionScope
{
	CArrayCPPVariables	apcLocals;
	CCPPFunctionPacks	cFunctionPacks;
};


typedef CArrayTemplatePtr<SCPPFunctionScope>	CArrayCPPFunctionScopes;




#define CPP65_BUILTIN_PREFIX			"__builtin_"
#define CPP65_BUILTIN_PREFIX_LENGTH		10


class CCPP65IRFrontEnd;
class CCPPConstants;
class CCTInt;
class CCPPSemanticPass;


class CCPPTokenMarks;


//Generates 65C64 or W65C816 assembly (Assembler65Lib syntax) from a parsed translation unit: the declarations,
//types, globals and start up here, and every function body as IR (CPP65IRFrontEnd, IR.txt) generated by the
//target's IR back end.  The calling convention and the parts of C++ that are supported are in CPP65Generator.txt.
//
//Check only works out the types of the unit's expressions, for CCPPSemanticPass: no code is generated and it keeps
//going past what it can't type (CCPPCheck).
class CCPPUnit : public CCPPRuntimeWriter
{
friend class CCPPConstantFolder;
friend class CCPPClosures;
friend class CCPPClassData;
friend class CCPPStrings;
friend class CCPPCheck;
friend class CCPPWarnings;
friend class CCPPBuiltins;
friend class CCPPExceptions;
friend class CCPPDesugarings;
friend class CCPPInstantiator;
friend class CCPP65TypeChecker;
friend class CCPPBinder;
protected:
	CCPPInstantiator		mcInstantiator;		//The templates and their instances.
	CCPPDesugarings			mcDesugarings;		//What the compiler makes once for a node.
	CCPPExceptions			mcExceptions;		//Exception specifications.
	CCPPBuiltins			mcBuiltins;			//The builtins the compiler does.
	CCPPWarnings			mcWarnings;			//Warns of what a 16 bit int makes wrong.
	CCPPUnitObserver		mcNoObserver;		//Hears nothing: the observer when code is made.
	CCPPUnitObserver*		mpcObserver;		//Told what the unit works out: the semantic pass when it checks.
	CCPPCheck				mcCheck;			//The IDE's check of the unit as written.
	CCPPStrings				mcStrings;			//Decodes string literals.
	CCPPClassData			mcClassData;		//Writes classes' vtables, VTTs and type info.
	CCPPClosures			mcClosures;			//The lambdas' closures.
	CCPPConstantFolder		mcFolder;			//Works out constant expressions' values.
	CCPPSyntaxTree*			mpcTree;
	CCPPTokeniser*			mpcTokeniser;
	CCPP65Types				mcTypes;
	CCPPTarget*				mpcTarget;

	CChars					mszCode;
	CChars					mszThunks;			//Adjust this, then jump to a virtual function.
	CArrayChars				maszRuntime;		//The runtime routines' labels the unit calls: linked (soft float), or written into the unit (AppendRuntime).
	CCPPDiagnostics			mcDiagnostics;		//The errors, the warnings, and when nothing is said.

	CArrayCPPVariables	mapcGlobals;
	CArrayCPPFunctions	mapcFunctions;
	CMapCPPFunctionsOfNames	mcFunctionsOfNames;	//mapcFunctions by name (FirstFunctionOfName).
	CArrayCPPTemplates	mapcTemplates;
	CArrayCPPTemplateArguments	mapcAllBindings;
	CArrayCPPPacks		mapapsPacks;		//The packs' types (NewPack).
	CArrayCPPValuePacks	mapalliPacks;		//The non-type packs' values (NewValuePack).
	CArrayCPPSymbols		mapcPackSymbols;	//The symbols made for function and template parameter packs' elements.
	CArrayCPPPackElements	masPackElements;	//Template parameter packs' elements in each bindings (PackElement), and function ones' in each function.
	CArrayCPPExpandedArgumentLists	masExpandedArguments;	//Argument lists with their packs expanded (ExpandArguments).
	CArrayCPPSymbols		mapcHiddenSymbols;	//The symbols of the variables made for range-for statements.
	CHashMapLongTemplate<char>	mcExplicitInstances;	//The classes and functions explicitly instantiated (ExplicitlyInstantiatedOnce).
	CHashMapPrimitiveTemplate<CCPPSyntaxEnumerator*, int64>	mcEnumeratorValues;	//The values enumerators are given, folded once (GivenEnumeratorValue).
	CHashMapPrimitiveTemplate<CCPPSyntaxEnumerator*, SCPP65Type*>	mcEnumeratorBodyTypes;	//While EnumType works out an enum's type, each of its enumerators' so far: its value's.  [dcl.enum] 5
	CCPPTemplate*			mpcInitializerList;	//std::initializer_list, once it is found (InitializerListTemplate).
	CCPPClass*				mpcTypeInfo;		//std::type_info, once it is found (TypeInfoClass).
	CArrayCPPVariables		mapcVariableInstances;	//Variable templates' instances (VariableInstance), in mapcGlobals too.
	int						miSaysDeprecated;	//Whether the source says deprecated anywhere (SaysDeprecated): 1, 0, or -1 until it is asked.
	bool					mbValueArgumentRefused;	//A template argument's value wasn't one of its parameter's type (ValueArgumentFits), since it was cleared.
	CArrayCPPFunctions	mapcQueue;			//Functions to generate, first queued first...
	size					muiQueueNext;		//...from this one: those before it have been.
	CArrayChars				maszEmitted;		//Labels of the functions, vtables, thunks and runtime routines written.
	bool					mbPureVirtual;
	bool					mbBadCast;			//A dynamic_cast to a reference calls R_BadCast (UseBadCast).
	bool					mbRecursiveInit;	//A guarded static local calls R_RecursiveInit if it is reached again while being made.

	CCPPUnitData			mcData;				//What the unit has other than code: globals, the init and fini tables, strings.
	CCPP65DebugWriter		mcDebugWriter;		//With a line table: its debug directives' types and its globals.
	CCPPContext				mcContext;			//Where the compiler is: the template arguments, the namespace, the linkage, the function.
	CCPPBinder				mcBinder;			//What each call, name and member is, worked out in one place.
	CCPPDeclarators			mcDeclarators;
	CCPPFunctionDeclarer	mcFunctionDeclarer;	//Declares the functions.
	CCPPClasses				mcClasses;			//The classes: made, laid out and checked.
	CCPPGlobals				mcGlobals;			//The data of static storage, and what makes and destroys it.
	CCPPMangler				mcMangler;			//Makes the labels.
	CArrayCPPFunctionScopes	mapsFunctionScopes;	//The functions entered, the last the one being written (GetLocals, GetFunctionPacks).
	int						miLabels;
	CCPP65IRFrontEnd*		mpcIRFrontEnd;		//Writes every function body as IR.
	CCPPConstants*			mpcConstants;		//Works out the constants that take running the IR (constexpr).
	CCPP65TypeChecker		mcTypeChecker;		//Works out expressions' types (TypeOf).
	CIRImports*				mpcImports;			//Other units' functions the IR inliner may use (SetImports), NULL for none.
	size					muiImportsTurn;		//The unit's turn with them (CIRImports::BeginTurn).
	bool					mbMissedImports;	//The unit must be generated again (MissedImports).
	CAsm65Profile*			mpcFrameProfile;	//Measured functions, for the W65C816's aligned frames (SetFrameProfile), or NULL.
	bool					mbListing;			//The code has comments with the C++ source lines it is for (SetListing).
	bool					mbLineTable;		//The code has .line and .fingerprint directives, for the linker's line table (SetLineTable).
	bool					mbNoFloats;			//Floating point in the code it writes is an error (SetNoFloats): a kernal's units.
	bool					mbCompiledBefore;	//The same text was compiled before with no error (SetCompiledBefore): nothing is done only to report.
	int64					mlliCompileTime;	//When the unit was compiled (seconds since 1970), for .fingerprint.
	CSpillStore*			mpcSpillStore;		//Where the functions' code waits until the unit is assembled (SetSpillStore),
	uint32					muiSpillUnit;		//by the unit's number.
	CMemorySpillStore		mcMemorySpillStore;	//The spill store when none is set.
	CScratchPadAllocator	mcBackendScratch;	//The W65C816 back end's working arrays and strings, for one function at a time (CIRW65C816Backend::SetScratch).
	CArrayLargeTemplate<uint16>	mauiCodePieces;	//Per function written by the IR front end: how many pieces its code is in.
	CCPPSemanticPass*		mpcSemanticPass;	//Check only: what names resolved to goes here.  NULL when generating.
	bool					mbRemember;			//A kept check: the pass is told each expression's type and what changes its place too.
	CCPPCompileStatistics*	mpcStatistics;		//What compiling the unit is counted and timed in (SetStatistics): the builder's for the unit...
	CCPPCompileStatistics	mcOwnStatistics;	//...or the unit's own when none is set.
	CCPPCompileOptions*		mpcOptions;			//How the unit is compiled where there is a choice (SetOptions): the builder's, or the defaults.

public:
	//What the classes that work for the unit ask it for.
	CCPP65Types*			GetTypes(void);
	CCPPContext*			GetContext(void);
	CCPPDiagnostics*		GetDiagnostics(void);
	CCPPBinder*				GetBinder(void);
	CCPPConstants*			GetConstants(void) const;
	CCPPConstantFolder*		GetFolder(void);
	CCPPClosures*			GetClosures(void);
	CCPPClassData*			GetClassData(void);
	CCPPStrings*			GetStrings(void);
	CCPPCheck*				GetCheck(void);
	CCPPUnitObserver*		GetObserver(void);
	CCPPWarnings*			GetWarner(void);
	CCPPBuiltins*			GetBuiltins(void);
	CCPPExceptions*			GetExceptions(void);
	CCPPDesugarings*		GetDesugarings(void);
	CCPPInstantiator*		GetInstantiator(void);
	CCPPDeclarators*		GetDeclarators(void);
	CCPPFunctionDeclarer*	GetFunctionDeclarer(void);
	CCPPNames*				GetNames(void);
	CCPPGlobals*			GetGlobals(void);
	CCPPClasses*			GetClasses(void);
	CArrayCPPVariables*		GetGlobalVariables(void);
	CCPP65IRFrontEnd*		GetIRFrontEnd(void) const;
	CCPPMangler*			GetMangler(void);
	CCPPUnitData*			GetData(void);
	CCPP65DebugWriter*		GetDebugWriter(void);
	CCPP65TypeChecker*		GetTypeChecker(void);
	CChars*					GetCode(void);
	CArrayCPPTemplates*		GetTemplates(void);
	CArrayCPPVariables*		GetLocals(void);
	CCPPFunctionPacks*		GetFunctionPacks(void);
	CArrayLargeTemplate<uint16>*	GetCodePieces(void);
	int*					GetLabelCounter(void);
	CCPPTarget*				GetTarget(void) const;
	CIRImports*				GetImports(void) const;
	CCPPTokeniser*			GetTokeniser(void) const;
	CCPPSyntaxTree*			GetTree(void) const;
	CSpillStore*			GetSpillStore(void) const;
	CCPPSemanticPass*		GetSemanticPass(void) const;
	bool					RecordEnumCast(CCPPSyntax* pcNode, SCPP65Type* psFrom, SCPP65Type* psTo, uint8 uiKind);
	bool					IsRecordedEnum(SCPP65Type* psType);
	CAsm65Profile*			GetFrameProfile(void) const;
	uint32					GetSpillUnit(void) const;
	size					GetImportsTurn(void) const;
	bool					HasLineTable(void) const;
	bool					HasListing(void) const;
	bool					HasNoFloats(void) const;
	int64					GetCompileTime(void) const;

	void					Init(CCPPSyntaxTree* pcTree, ECPPTarget eTarget = CPPT_65C64);
	void					Kill(void);

	bool					Generate(CChars* pszDest);
	bool					Generate(CCPP65Assembly* pcDest);
	bool					GenerateProgram(CChars* pszDest);
	CCPPErrors*				GetErrors(void);
	CCPPErrors*				GetWarnings(void);
	int						NumErrors(void) const;
	void					GetExternals(CArrayChars* paszLabels);
	void					SetImports(CIRImports* pcImports, size uiTurn);
	bool					MissedImports(void);
	bool					GenerateBeforeTurn(void);
	void					CheckContext(void);
	bool					GenerateAfterTurn(CCPP65Assembly* pcDest);
	CArrayChars*			GetNotImported(void) const;
	void					SetFrameProfile(CAsm65Profile* pcProfile);
	void					SetListing(bool bListing);
	void					SetLayouts(bool bLayouts);
	void					SetLineTable(bool bLineTable, int64 lliCompileTime);
	void					SetNoFloats(bool bNoFloats);
	void					SetCompiledBefore(bool bCompiledBefore);
	bool					IsCompiledBefore(void) const;
	bool					IsCSource(void) const;
	void					SetSpillStore(CSpillStore* pcStore, uint32 uiUnit = 0);
	void					SetStatistics(CCPPCompileStatistics* pcStatistics);
	void					SetOptions(CCPPCompileOptions* pcOptions);
	CCPPCompileOptions*		GetOptions(void) const;
	CCPPCompileStatistics*	GetStatistics(void) const;
	CScratchPadAllocator*	GetBackendScratch(void);
	void					GetMissingCallees(CArrayChars* paszNames);
	void					GetTestedCallees(CArrayChars* paszNames);
	void					GetLayouts(CArrayChars* paszLines);

	//Declarations, functions and statements.  CPPUnit.cpp
	void					WarnDeprecated(CCPPSyntax* pcNode, const char* szWhat, int32 iAttribute);
	void					WarnDeprecatedSymbol(CCPPSymbol* pcSymbol, CCPPSyntax* pcNode);

	CChars*					Line(const char* szText);
	void					AppendName(CChars* psz, int64 lliName);
	int64					GetName(CCPPSyntaxDeclarator* pcDeclarator);

	void					EnterFunction(CCPPFunction* pcFunction);
	void					LeaveFunction(void);
	void					PushFunctionScope(void);
	void					PopFunctionScope(void);
	CCPPFunction*			AddFunction(CCPPClass* pcClass, int64 lliName, ECPPFunctionKind eKind, SCPP65Type* psReturn);
	bool					NeedsNoInitialiser(SCPP65Type* psType, CCPPSyntax* pcNode);
	bool					CheckDeclared(SCPP65Type* psType, CCPPSyntaxInitDeclarator* pcInit, CCPPSyntax* pcInitialiser);
	CCPPFunction*			FirstFunctionOfName(int64 lliName);
	void					RequireFunction(CCPPFunction* pcFunction);
	bool					Emit(CChars* pszLabel);
	bool					Declarations(CArraySyntax* papcDeclarations);
	bool					IsScopeDeclaration(CCPPSyntax* pcDeclaration);
	bool					IsOutsideMemberTemplate(CCPPSyntax* pcDeclaration);
	CCPPVariable*			AddVariable(CArrayCPPVariables* papcVariables, CCPPSymbol* pcSymbol, SCPP65Type* psType, ECPPStorage eStorage, int iOffset);
	void					FreeVariables(CArrayCPPVariables* papcVariables);
	CCPPVariable*			GetVariable(CCPPSymbol* pcSymbol);
	CCPPSyntax*				StringInitialiser(CCPPSyntax* pcInitialiser, SCPP65Type* psElement);

	bool					Function(CCPPSyntaxFunctionDefinition* pcDefinition, CCPPClass* pcClass);
	void					GenerateFunction(CCPPFunction* pcFunction);
	bool					GenerateQueued(void);
	bool					GenerateDefinition(CCPPFunction* pcFunction);
	bool					WriteBody(CCPPFunction* pcFunction);
	void					AddParameters(CCPPFunction* pcFunction, CCPPSyntaxDeclarator* pcDeclarator = NULL);
	void					AppendUnsupported(CChars* psz);
	bool					AllocateLocals(CCPPSyntax* pcNode);
	bool					CheckTypedefs(CCPPSyntaxSimpleDeclaration* pcDeclaration);
	bool					AnonymousLocal(CCPPSyntaxClassSpecifier* pcSpecifier);
	bool					StaticAssert(CCPPSyntaxStaticAssert* pcAssert);
	bool					ClassStaticAsserts(CCPPSyntaxClassSpecifier* pcSpecifier);
	bool					CheckEnumerators(CCPPSyntax* pcType);
	bool					IsCLinkage(CCPPSyntaxLinkageSpecification* pcLinkage);
	void					AppendRuntime(void);
	void					UseBadCast(void) { mbBadCast = true; }
	void					UseRecursiveInit(void) { mbRecursiveInit = true; }
	CArrayChars*			GetRuntimeCalls(void) override;
	bool					BeginRuntime(const char* szName) override;
	CChars*					GetRuntimeCode(void) override;
	void					RuntimeError(const char* szMessage) override;
	bool					HasUsedAttribute(CCPPSyntaxDeclarator* pcDeclarator);
	bool					HasAttribute(CCPPSyntaxDeclarator* pcDeclarator, const char* szName);
	int32					FindAttribute(CCPPSyntaxDeclarator* pcDeclarator, const char* szName);
	int32					FindAttribute(CArraySyntax* papcAttributes, const char* szName);
	int32					DeprecatedAttribute(CCPPSymbol* pcSymbol);
	bool					SaysDeprecated(void);
	bool					IsRestrictPointer(CCPPSyntaxDeclarator* pcDeclarator);
	SCPP65Type*				QualifiedPointer(SCPP65Type* psType, CCPPSyntaxPointerOperator* pcPointer);
	SCPP65Type*				ParameterVariableType(SCPP65Type* psType, CCPPSyntaxParameter* pcParameter);


	//Classes, templates, virtual functions, this and a class's bases, and the names of functions.  CPPUnitClasses.cpp,
	//CPPUnitTemplates.cpp, CPPUnitVtables.cpp, CPPUnitOverloads.cpp and CPPUnitLabels.cpp
	void					CheckEnumeratorFits(CCPPSyntaxEnumSpecifier* pcEnum, CCPPSyntaxEnumerator* pcEnumerator, int64 lliValue);
	bool					GivenEnumeratorValue(CCPPSyntaxEnumSpecifier* pcEnum, size uiEnumerator, int64* plliValue);
	bool					FoldEnumeratorValue(CCPPSyntaxEnumSpecifier* pcEnum, CCPPSyntaxEnumerator* pcEnumerator, int64* plliValue);
	static bool				IsOutsideTemplates(CCPPSyntax* pcNode);
	void					SetEnumeratorBodyType(CCPPSyntaxEnumSpecifier* pcSpecifier, size uiEnumerator, SCPP65Type* psFixed);
	void					ForgetEnumeratorBodyTypes(CCPPSyntaxEnumSpecifier* pcSpecifier);
	CArrayCPP65Types*		NewPack(void);
	CArrayInt64*			NewValuePack(void);
	void					FindPacks(CCPPSyntax* pcPattern, CArrayCPPBindingPtrs* papsPacks, CArrayCPPSymbols* papcFunctionPacks);
	bool					PackExpansionLength(CArrayCPPBindingPtrs* papsPacks, CArrayCPPSymbols* papcFunctionPacks, CCPPSyntax* pcNode, size* puiElements);
	SCPPFunctionPack*		FindFunctionPack(CCPPSymbol* pcSymbol);
	bool					ExpandPattern(CCPPSyntax* pcPattern, CCPPSyntax* pcParent, CArraySyntax* papcExpanded);
	CArraySyntax*			ExpandArguments(CArraySyntax* papcArguments);
	bool					IsInUnevaluatedOperand(CCPPSyntax* pcNode);
	CCPPSyntaxParameter*	DefaultArgumentOf(CCPPSyntax* pcNode);
	bool					IsDecltypeOperand(CCPPSyntax* pcNode);
	CArraySyntax*			ListElements(CCPPSyntax* pcList);
	CCPPSymbol*				PackElement(SCPPBinding* psPack, size k);
	bool					ExpandTemplateArguments(CArraySyntax* papcArguments, CArrayCPPExpandedArguments* pasExpanded);
	SCPP65Type*				ExpandedArgumentType(SCPPExpandedArgument* psArgument);
	bool					ExpandedArgumentValue(SCPPExpandedArgument* psArgument, int64* plliValue);
	bool					PackSize(CCPPSymbol* pcPack, int64* plliValue);
	SCPPBinding*			PackBinding(CCPPSyntaxParameter* pcParameter);

	bool					VariableTemplateValue(CCPPSyntaxIdExpression* pcId, int64* plliValue);
	bool					IsOnePublicBase(CCPPClass* pcClass, CCPPClass* pcBase, int* piOffset);
	bool					IsPublicBase(CCPPClass* pcClass, CCPPClass* pcBase);
	bool					IsConstObject(CCPPSyntax* pcExpression);
	CCPPFunction*			ThisFunction(void);
	bool					IsConstThis(void);
	bool					IsVolatileThis(void);
	SCPP65Type*				ThisType(CCPPClass* pcClass);
	bool					IsDerived(CCPPClass* pcClass, CCPPClass* pcBase, int* piOffset);
	size					CountBases(CCPPClass* pcClass, CCPPClass* pcBase);
	void					CollectVirtualBases(CCPPClass* pcClass, CArrayCPPClasses* papcVirtual);
	CArrayCPPBasePaths*		BasePaths(CCPPClass* pcClass);
	void					ResolveAllBases(CCPPClass* pcClass);
	size					CountFixedBases(CCPPClass* pcClass, CCPPClass* pcBase);
	SCPPVirtualBase*		VirtualBaseOf(CCPPClass* pcClass, CCPPClass* pcBase, int* piBelow);
	bool					CompleteOffset(CCPPClass* pcClass, CCPPClass* pcBase, int* piOffset);
	int64					GetFunctionName(CCPPSyntaxNameComponent* pcComponent);
	int64					GetOperatorName(ECppTokens eToken, bool bArray);
	int64					GetComponentName(CCPPSyntaxNameComponent* pcComponent);


	//Literals, constants, warnings and TypeOf.  CPPUnitExpressions.cpp
	SCPP65Type*				IntegerLiteralType(CCTInt* pcInt);

	CCPPSymbol*				FunctionPackElement(CCPPFunction* pcFunction, CCPPSymbol* pcPack, size k);
	bool					HasPackExpansion(CArraySyntax* papcArguments);
	CCPPSyntax*				ExpansionPattern(CCPPSyntax* pcNode);

	bool					QuietConstant(CCPPSyntax* pcExpression, int64* plliValue);


	bool					FunctionNameText(int64 lliName, CChars* pszText);
	bool					IsFunctionName(CCPPSyntaxIdExpression* pcId);
	SCPP65Type*				Literal(CCPPSyntaxLiteralExpression* pcLiteral);
	SCPP65Type*				CharacterTokenType(CCPPToken* pcToken);
	CCPPSyntaxIdExpression*	FunctionNameFor(CCPPSyntax* pcExpression, SCPP65Type* psTo);
	CCPPSyntaxIdExpression*	MemberFunctionNameFor(CCPPSyntax* pcExpression, SCPP65Type* psTo);
	bool					IsStringLiteral(CCPPSyntax* pcExpression);
	CCPPSyntaxFunctionSuffix*	PointedFunctionSuffix(CCPPSyntaxDeclarator* pcDeclarator);
	CCPPSyntax*				NewSize(CCPPSyntaxNewExpression* pcNew);
	SCPP65Type*				NewElementType(CCPPSyntaxNewExpression* pcNew);
	SCPP65Type*				NewTypeIdArray(CCPPSyntaxNewExpression* pcNew);
	bool					IsArrayNew(CCPPSyntaxNewExpression* pcNew);
	void					NewArguments(CCPPSyntaxNewExpression* pcNew, CArraySyntax* papcArguments);
	CCPPSyntaxIdExpression*	FunctionNameOf(CCPPSyntax* pcExpression);
	bool					IsMemberPointerOperator(CCPPSyntax* pcExpression);
	bool					NamesFunction(CCPPSyntax* pcExpression);
	bool					CheckInitialiser(CCPPSyntax* pcValue, SCPP65Type* psTo);
	bool					CheckBracedValue(CCPPSyntax* pcValue, SCPP65Type* psTo);


	bool					Supported(SCPP65Type* psType, CCPPSyntax* pcNode);


	bool					CheckConstexprLocal(CCPPVariable* pcVariable, CCPPSyntaxInitDeclarator* pcInit);
	bool					TypeTrait(CCPPSyntaxTypeTraitExpression* pcTrait, int64* plliValue);
	bool					TypeTraitValue(int iTrait, CArrayCPP65Types* papsTypes);
	bool					IsTriviallyCopyableType(SCPP65Type* psType);
	bool					IsStandardLayoutType(SCPP65Type* psType);
	bool					EnumeratorValue(CCPPSymbol* pcSymbol, int64* plliValue);
	SCPP65Type*				EnumType(CCPPSymbol* pcEnum, CCPPSyntaxEnumSpecifier* pcSpecifier);
	SCPP65Type*				EnumeratorType(CCPPSymbol* pcSymbol);
	CCPPSyntax*				SkipParentheses(CCPPSyntax* pcExpression);
	bool					IsNullPointerConstant(CCPPSyntax* pcExpression);
	SCPP65Type*				QualifiedFieldType(CCPPSyntax* pcExpression);

	//Objects and copies.  CPPUnitObjects.cpp
	CCPPSyntaxIdExpression*	MemberPointerName(CCPPSyntaxUnaryExpression* pcAddress);

	//Check only.  CPPUnitCheck.cpp
	CArraySyntax*			ConstructElements(CCPPSyntaxConstructExpression* pcConstruct);

	//Floating point.  CPPUnitFloat.cpp
};


#endif // __C_P_P_UNIT_H__
