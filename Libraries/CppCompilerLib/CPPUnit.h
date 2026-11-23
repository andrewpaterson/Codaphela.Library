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
#include "CPPBinder.h"
#include "CPPDeclarators.h"
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


//A function declared quietly that couldn't be (DeclareQuietly): its declarator, and what was said declaring it, which
//is said where it is called.
struct SCPPFailedDeclaration
{
	CCPPSyntaxDeclarator*	pcDeclarator;
	CCPPErrors*				pcErrors;		//The diagnostics' to free; NULL for nothing said.
};


typedef CArrayTemplate<SCPPFailedDeclaration>	CArrayCPPFailedDeclarations;


//A routine the unit writes once its functions are: a static local's R_Finalise (WriteExitRoutines) or the function
//that destroys a thrown object (WriteThrownDestroyers).
struct SCPPUnitRoutine
{
	CChars			szLabel;	//The static local's, for its R_Finalise; the destroyer's own.
	SCPP65Type*		psType;		//What it destroys.
	bool			bShared;	//A shared function's static local: its R_Finalise is a COMDAT.
};


typedef CArrayTemplate<SCPPUnitRoutine>	CArrayCPPUnitRoutines;


//What a definition written outside its class template defines (SCPPOutsideDefinition).
enum ECPPOutsideDefinition
{
	CPPOD_MemberSpecialisation,			//template<> R Class<Arguments>::f() { ... }, given to its instance (SpecialiseMembers).
	CPPOD_MemberTemplateSpecialisation,	//template<> template<> R Class<Arguments>::f<T>(...) { ... }, given to the instance's member template (AddMemberTemplateSpecialisations).
	CPPOD_MemberClass,					//template<class T> struct Class<T>::Inner { ... }; and template<class T> template<class U> struct Class<T>::Pair { ... }; (MemberClassDefinition).
	CPPOD_MemberVariableTemplate,		//template<class C> template<class T> T W<C>::value = ...;, each instance's member variable template's definition (DefineMemberVariableTemplate).
	CPPOD_MemberVariableSpecialisation,	//template<class C> template<class T> T W<C>::value<T*> = ...;, each instance's member variable template's partial specialisation.
	CPPOD_StaticMember,					//template<class T> int Class<T>::s = ...;, defined for each instance (DefineInstanceStatics).
};


//A definition written outside its class template, kept to be given to each instance: what it is, and the pieces of it
//its readers want, worked out once when it is seen (AddOutsideDefinition).
struct SCPPOutsideDefinition
{
	ECPPOutsideDefinition			eKind;
	CCPPSyntaxTemplateDeclaration*	pcDeclaration;	//The template declaration whose parameters name the class template's (the inner template<> of a member template's specialisation); NULL for a member's specialisation.
	CCPPSyntaxTemplateDeclaration*	pcMember;		//A member template's own template declaration, in pcDeclaration; else NULL.
	CCPPSyntaxFunctionDefinition*	pcFunction;		//The function defined, or NULL.
	CCPPSyntaxSimpleDeclaration*	pcSimple;		//The declaration of the class, variable or static member defined, or NULL.
	CCPPSyntaxClassSpecifier*		pcSpecifier;	//The member class defined, or NULL.
	CCPPSyntaxInitDeclarator*		pcInit;			//The static member's declarator, or NULL.
	CCPPSyntaxName*					pcName;			//The name defined (Class<T>::s, W<C>::value), or NULL for a class.
};


typedef CArrayTemplate<SCPPOutsideDefinition>	CArrayCPPOutsideDefinitions;


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
	CArrayCPPClasses		mapcClasses;
	CArrayCPPTemplates	mapcTemplates;
	CArrayCPPTemplateArguments	mapcAllBindings;
	CArrayCPPPacks		mapapsPacks;		//The packs' types (NewPack).
	CArrayCPPValuePacks	mapalliPacks;		//The non-type packs' values (NewValuePack).
	CArrayCPPSymbols		mapcPackSymbols;	//The symbols made for function and template parameter packs' elements.
	CArrayCPPPackElements	masPackElements;	//Template parameter packs' elements in each bindings (PackElement), and function ones' in each function.
	CArrayCPPExpandedArgumentLists	masExpandedArguments;	//Argument lists with their packs expanded (ExpandArguments).
	CArrayCPPSymbols		mapcHiddenSymbols;	//The symbols of the variables made for range-for statements.
	CMapLongTemplate<char>	mcExplicitInstances;	//The classes and functions explicitly instantiated (ExplicitlyInstantiatedOnce).
	CCPPTemplate*			mpcInitializerList;	//std::initializer_list, once it is found (InitializerListTemplate).
	CCPPClass*				mpcTypeInfo;		//std::type_info, once it is found (TypeInfoClass).
	CArrayCPPOutsideDefinitions	masOutsideDefinitions;	//The definitions written outside their class template, in the order seen, to give each instance.
	CArrayCPPVariables		mapcVariableInstances;	//Variable templates' instances (VariableInstance), in mapcGlobals too.
	int						miSaysDeprecated;	//Whether the source says deprecated anywhere (SaysDeprecated): 1, 0, or -1 until it is asked.
	bool					mbValueArgumentRefused;	//A template argument's value wasn't one of its parameter's type (ValueArgumentFits), since it was cleared.
	CArrayCPPFunctions	mapcQueue;			//Functions to generate, first queued first...
	size					muiQueueNext;		//...from this one: those before it have been.
	CArrayCPPFailedDeclarations	masFailed;	//Functions whose declarations were not supported, and why (ReportFailedDeclaration).
	CArrayChars				maszEmitted;		//Labels of the functions, vtables, thunks and runtime routines written.
	bool					mbPureVirtual;
	bool					mbBadCast;			//A dynamic_cast to a reference calls R_BadCast (UseBadCast).
	bool					mbRecursiveInit;	//A guarded static local calls R_RecursiveInit if it is reached again while being made.
	bool					mbExitLists;		//Something is pushed on an exit list: the unit writes R_RunExitLists (UseExitLists).
	bool					mbExitListsWritten;
	bool					mbDynamicCast;		//Something casts with dynamic_cast: the unit writes R_DynamicCast (UseDynamicCast).
	bool					mbDynamicCastWritten;
	CArrayCPPUnitRoutines	masThrownDestroyers;	//The functions that destroy thrown objects, still to write (ThrownDestroyer).
	CArrayCPPUnitRoutines	masExitFinalisers;	//The static locals destroyed at exit whose R_Finalise is still to write (WriteExitRoutines).

	CCPPUnitData			mcData;				//What the unit has other than code: globals, the init and fini tables, strings.
	CCPPContext				mcContext;			//Where the compiler is: the template arguments, the namespace, the linkage, the function.
	CCPPBinder				mcBinder;			//What each call, name and member is, worked out in one place.
	CCPPDeclarators			mcDeclarators;
	CCPPMangler				mcMangler;			//Makes the labels.
	CArrayCPPFunctionScopes	mapsFunctionScopes;	//The functions entered, the last the one being written (GetLocals, GetFunctionPacks).
	CArrayCPPVariables		mapcStaticLocals;	//Each static local as last defined, which a local class's member functions name (EnclosingStatic).
	int						miStatics;			//Static locals in the unit.
	int						miLabels;
	CCPP65IRFrontEnd*		mpcIRFrontEnd;		//Writes every function body as IR.
	CCPPConstants*			mpcConstants;		//Works out the constants that take running the IR (constexpr).
	CCPP65TypeChecker		mcTypeChecker;		//Works out expressions' types (TypeOf).
	CIRImports*				mpcImports;			//Other units' functions the IR inliner may use (SetImports), NULL for none.
	size					muiImportsTurn;		//The unit's turn with them (CIRImports::BeginTurn).
	bool					mbMissedImports;	//The unit must be generated again (MissedImports).
	CAsm65Profile*			mpcFrameProfile;	//Measured functions, for the W65C816's aligned frames (SetFrameProfile), or NULL.
	bool					mbListing;			//The code has comments with the C++ source lines it is for (SetListing).
	bool					mbLayouts;			//Each class laid out is written down (SetLayouts)...
	CArrayChars				maszLayouts;		//...here, a line each (GetLayouts).
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
	CCPPMangler*			GetMangler(void);
	CCPPUnitData*			GetData(void);
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

	CCPPFunction*			DeclareFunction(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, bool bDefinition, CCPPClass* pcClass, bool bPure, CCPPTemplate* pcTemplate = NULL, CChars* pszArguments = NULL);
	CCPPFunction*			DeclareSignature(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, bool bDefinition, CCPPClass* pcClass, bool bPure, CCPPTemplate* pcTemplate, CChars* pszArguments);
	SCPP65Type*				FunctionType(CCPPFunction* pcFunction);
	CCPPSyntaxExceptionSpecification*	WrittenSpecification(CCPPSyntaxFunctionSuffix* pcSuffix);
	bool					CheckOverrides(CCPPClass* pcClass);
	bool					HasVirtSpecifier(CCPPSyntaxFunctionSuffix* pcSuffix, int64 lliName);
	bool					IsAllocationFunction(int64 lliName);
	bool					IsDefinedWhereDeclared(CCPPSyntaxName* pcName);
	bool					ResolveReturn(CCPPFunction* pcFunction, CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator);
	SCPP65Type*				ReturnedType(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, CCPPSyntaxReturnStatement* pcReturn);
	void					FindReturns(CCPPSyntax* pcNode, CArraySyntax* papcReturns);
	SCPP65Type*				ReturnDeclaratorType(SCPP65Type* psReturn, CCPPSyntaxDeclarator* pcDeclarator);
	void					EnterFunction(CCPPFunction* pcFunction);
	void					LeaveFunction(void);
	void					PushFunctionScope(void);
	void					PopFunctionScope(void);
	CCPPFunction*			AddFunction(CCPPClass* pcClass, int64 lliName, ECPPFunctionKind eKind, SCPP65Type* psReturn);
	CCPPFunction*			AddImplicitFunction(CCPPClass* pcClass, int64 lliName, ECPPFunctionKind eKind, SCPP65Type* psReturn, bool bDefined);
	bool					CheckDefaultsFollow(CCPPFunction* pcFunction, CCPPSyntax* pcNode);
	bool					NeedsNoInitialiser(SCPP65Type* psType, CCPPSyntax* pcNode);
	bool					CheckDeclared(SCPP65Type* psType, CCPPSyntaxInitDeclarator* pcInit, CCPPSyntax* pcInitialiser);
	bool					CheckDefaultedOrDeleted(CCPPFunction* pcFunction, CCPPSyntaxFunctionDefinition* pcDefinition);
	bool					CheckAllocationFunction(int64 lliName, SCPP65Type* psReturn, CArrayCPP65Types* papsParameters, CCPPSyntax* pcNode);
	bool					IsAllocationName(CCPPSyntaxDeclarator* pcDeclarator);
	CCPPFunction*			FirstFunctionOfName(int64 lliName);
	void					RequireFunction(CCPPFunction* pcFunction);
	bool					Emit(CChars* pszLabel);
	bool					Declarations(CArraySyntax* papcDeclarations);
	bool					IsScopeDeclaration(CCPPSyntax* pcDeclaration);
	bool					IsOutsideMemberTemplate(CCPPSyntax* pcDeclaration);
	bool					GlobalDeclaration(CCPPSyntaxSimpleDeclaration* pcDeclaration, CCPPVariable* pcDefining = NULL);
	bool					GlobalInitialiser(SCPP65Type* psType, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode, bool* pbInitialise);
	CCPPVariable*			AddVariable(CArrayCPPVariables* papcVariables, CCPPSymbol* pcSymbol, SCPP65Type* psType, ECPPStorage eStorage, int iOffset);
	void					FreeVariables(CArrayCPPVariables* papcVariables);
	CCPPVariable*			GetVariable(CCPPSymbol* pcSymbol);
	void					CompleteDeclaredArrays(CCPPVariable* pcDefined);
	CCPPSyntax*				StringInitialiser(CCPPSyntax* pcInitialiser, SCPP65Type* psElement);

	bool					Function(CCPPSyntaxFunctionDefinition* pcDefinition, CCPPClass* pcClass);
	void					GenerateFunction(CCPPFunction* pcFunction);
	bool					GenerateQueued(void);
	bool					GenerateDefinition(CCPPFunction* pcFunction);
	bool					WriteBody(CCPPFunction* pcFunction);
	void					AddParameters(CCPPFunction* pcFunction, CCPPSyntaxDeclarator* pcDeclarator = NULL);
	void					AppendUnsupported(CChars* psz);
	bool					AllocateLocals(CCPPSyntax* pcNode);
	bool					HandlerLocal(CCPPSyntaxCatchHandler* pcHandler);
	bool					IsSharedFunction(CCPPFunction* pcFunction);
	CCPPVariable*			RedeclaredVariable(CCPPSymbol* pcSymbol);
	bool					AnonymousGlobal(CCPPSyntaxClassSpecifier* pcSpecifier);
	bool					CheckTypedefs(CCPPSyntaxSimpleDeclaration* pcDeclaration);
	CCPPVariable*			NamespaceVariable(CCPPSymbol* pcSymbol);
	bool					SameVariableType(SCPP65Type* psFirst, SCPP65Type* psSecond);
	bool					CheckBaseMemberName(CCPPSyntaxName* pcName, CCPPSyntax* pcNode);
	bool					ExternLocal(CCPPSyntaxInitDeclarator* pcInit, CCPPSyntaxDeclSpecifiers* pcSpecifiers);
	bool					StaticLocal(CCPPSyntaxInitDeclarator* pcInit, CCPPSyntaxDeclSpecifiers* pcSpecifiers);
	bool					AnonymousLocal(CCPPSyntaxClassSpecifier* pcSpecifier);
	bool					StaticAssert(CCPPSyntaxStaticAssert* pcAssert);
	bool					ClassStaticAsserts(CCPPSyntaxClassSpecifier* pcSpecifier);
	bool					CheckVirtSpecifiers(CCPPSyntaxClassSpecifier* pcSpecifier);
	bool					CheckEnumerators(CCPPSyntax* pcType);
	bool					WriteExitRoutines(void);
	bool					WriteThrownDestroyers(void);
	int						ExitNodeBytes(void);
	void					AddUnitRoutine(CArrayCPPUnitRoutines* pasRoutines, const char* szLabel, SCPP65Type* psType, bool bShared);
	void					KillUnitRoutines(CArrayCPPUnitRoutines* pasRoutines);
	bool					IsCLinkage(CCPPSyntaxLinkageSpecification* pcLinkage);
	void					AppendRuntime(void);
	void					UseBadCast(void) { mbBadCast = true; }
	void					UseRecursiveInit(void) { mbRecursiveInit = true; }
	void					UseExitLists(void) { mbExitLists = true; }
	void					UseDynamicCast(void) { mbDynamicCast = true; }
	bool					WriteDynamicCast(void);
	CArrayChars*			GetRuntimeCalls(void) override;
	bool					BeginRuntime(const char* szName) override;
	CChars*					GetRuntimeCode(void) override;
	void					RuntimeError(const char* szMessage) override;
	bool					StartUpRoutine(bool bFinalise, SCPP65Type* psType, CCPPSyntax* pcInitialiser, CChars* pszData, CChars* pszNode, CChars* pszFinaliser, bool bEntry, bool bThread);
	bool					GlobalStartUpRoutines(SCPP65Type* psType, CCPPSyntax* pcInitialiser, CChars* pszData, bool bConstruct, bool bDestroy, bool bThread);
	bool					BoundTemporary(SCPP65Type* psReference, CCPPSyntax* pcInitialiser, CChars* pszLabel, bool bGlobal, const char* szOptions, CChars* pszBound, int* piOffset, bool* pbResult);
	bool					HasUsedAttribute(CCPPSyntaxDeclarator* pcDeclarator);
	bool					HasAttribute(CCPPSyntaxDeclarator* pcDeclarator, const char* szName);
	int32					FindAttribute(CCPPSyntaxDeclarator* pcDeclarator, const char* szName);
	int32					FindAttribute(CArraySyntax* papcAttributes, const char* szName);
	int32					DeprecatedAttribute(CCPPSymbol* pcSymbol);
	bool					SaysDeprecated(void);
	bool					IsRestrictPointer(CCPPSyntaxDeclarator* pcDeclarator);
	SCPP65Type*				QualifiedPointer(SCPP65Type* psType, CCPPSyntaxPointerOperator* pcPointer);
	SCPP65Type*				ParameterType(SCPP65Type* psType);
	SCPP65Type*				ParameterVariableType(SCPP65Type* psType, CCPPSyntaxParameter* pcParameter);
	bool					IsBss(CCPPSyntax* pcInitialiser, SCPP65Type* psType);
	int						DataAlignment(SCPP65Type* psType, bool bClass);
	int						VariableDataAlignment(CCPPVariable* pcVariable, bool bClass);

	void					ReportTypeOf(CCPPSyntax* pcExpression);

	//Classes, templates, virtual functions, this and a class's bases, and the names of functions.  CPPUnitClasses.cpp,
	//CPPUnitTemplates.cpp, CPPUnitVtables.cpp, CPPUnitOverloads.cpp and CPPUnitLabels.cpp
	CCPPClass*			NewClass(CCPPSymbol* pcSymbol, int64 lliName);
	CCPPClass*			GetClass(CCPPSymbol* pcSymbol, CCPPSyntax* pcNode, CCPPClass* pcEnclosing = NULL);
	CCPPClass*				MemberScopeClass(CCPPSymbol* pcSymbol);
	CCPPClass*				EnclosingInstance(CCPPSymbol* pcSymbol);
	CCPPClass*			GetUnnamedClass(CCPPSyntaxClassSpecifier* pcSpecifier);
	CCPPClass*			FindClass(int64 lliName, CCPPScope* pcLookup);
	CCPPScope*				ComponentLookup(CCPPSyntaxNameComponent* pcComponent);
	CCPPClass*			GetComponentClass(CCPPSyntaxNameComponent* pcComponent, CCPPSymbol* pcSymbol, CCPPSyntax* pcNode, CCPPClass* pcContext = NULL);
	CCPPClass*			FindBaseInstance(CCPPClass* pcClass, CCPPTemplate* pcTemplate);
	CCPPClass*			QualifierClass(CCPPClass* pcClass, CCPPSyntaxNameComponent* pcComponent, CCPPSyntax* pcNode);
	int						AlignasAlignment(CArraySyntax* papcSpecifiers, bool bClass = false);
	int						ObjectAlignment(SCPP65Type* psType);
	int						VariableAlignment(CCPPVariable* pcVariable, int* piAlignas);
	bool					IsDefinedBefore(CCPPClass* pcClass, CCPPSyntax* pcNode);
	CCPPClass*			GetNameClass(CCPPSyntaxName* pcName, CCPPSyntax* pcNode);
	bool					IsFunctionByType(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator);
	void					CheckEnumeratorFits(CCPPSyntaxEnumSpecifier* pcEnum, CCPPSyntaxEnumerator* pcEnumerator, int64 lliValue);
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

	bool					CompleteClass(CCPPClass* pcClass, CCPPSyntax* pcNode);
	bool					FindSpecifier(CCPPClass* pcClass);
	bool					CompleteObjectClass(CCPPClass* pcClass, CCPPSyntax* pcNode);
	bool					ResolveBases(CCPPClass* pcClass, CCPPSyntax* pcNode);
	bool					AddBase(CCPPClass* pcClass, CCPPSyntaxBaseSpecifier* pcBaseSpecifier, CCPPSyntaxName* pcName);
	bool					AddPackBases(CCPPClass* pcClass, CCPPSyntaxBaseSpecifier* pcBaseSpecifier);
	bool					LayoutClass(CCPPClass* pcClass, CCPPSyntax* pcNode);
	void					AddLayout(CCPPClass* pcClass);
	void					CompleteMembers(CCPPClass* pcClass);
	void					PlaceMembers(CCPPClass* pcClass, int iPack, int iAlign);
	void					DefineOutsideMember(CCPPClass* pcClass, CCPPTemplate* pcMember);
	void					MemberSpecialisation(CCPPSyntaxFunctionDefinition* pcDefinition);
	void					AddOutsideDefinition(ECPPOutsideDefinition eKind, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntaxTemplateDeclaration* pcMember, CCPPSyntax* pcDefined);
	bool					VariableTemplateValue(CCPPSyntaxIdExpression* pcId, int64* plliValue);
	void					SpecialiseMembers(CCPPClass* pcClass);
	void					SpecialiseMember(CCPPClass* pcClass, CCPPSyntaxFunctionDefinition* pcDefinition);
	bool					QualifierIsInstance(CCPPSyntaxNameComponent* pcQualifier, CCPPClass* pcClass, CCPPSyntax* pcNode);
	bool					CheckOperatorSignature(CCPPSyntaxNameComponent* pcComponent, int64 lliName, CArrayCPP65Types* papsParameters, CArraySyntax* papcDefaults, CCPPClass* pcClass, CCPPSyntax* pcNode);
	bool					OperatorError(CCPPClass* pcClass, CCPPSyntax* pcNode, const char* szMessage);
	void					DefineOutsideMembers(CCPPTemplate* pcMember);
	bool					IsOutsideMemberOf(CCPPTemplate* pcMember, CCPPClass* pcClass);
	bool					ClassMember(CCPPClass* pcClass, CCPPSyntax* pcMember);
	bool					ClassBitField(CCPPClass* pcClass, CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxInitDeclarator* pcInit);
	bool					IsDeclaredAgainInClass(CCPPClass* pcClass, CCPPFunction* pcFunction, size uiMembers);
	bool					ReturnsCovariant(CCPPFunction* pcOverrider, CCPPFunction* pcOverridden);
	bool					InjectedNameReached(CCPPClass* pcClass, CCPPSyntaxDeclSpecifiers* pcSpecifiers, SCPP65Type* psType, CCPPSyntax* pcNode);
	bool					CheckAbstractSignature(CCPPFunction* pcFunction, CCPPSyntax* pcNode);
	bool					HasFieldNeedingInitialiser(CCPPClass* pcClass);
	bool					CheckMemberInitialisers(CCPPFunction* pcFunction);
	bool					IsOnePublicBase(CCPPClass* pcClass, CCPPClass* pcBase, int* piOffset);
	bool					IsPublicBase(CCPPClass* pcClass, CCPPClass* pcBase);
	CCPPVariable*			FindStatic(CCPPClass* pcClass, int64 lliName);
	bool					IsConstantMember(CCPPVariable* pcVariable);
	void					ClassUsing(CCPPClass* pcClass, CCPPSyntaxUsingDeclaration* pcUsing);
	void					InheritConstructors(CCPPClass* pcClass);
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
	SCPP65Type*				ArrowType(CCPPSyntax* pcObject, CCPPSyntax* pcNode);
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
	bool					IsWeakFunction(CCPPFunction* pcFunction);
	bool					IsAmbiguousName(CCPPSyntaxName* pcName);
	CCPPSyntax*				NewSize(CCPPSyntaxNewExpression* pcNew);
	SCPP65Type*				NewElementType(CCPPSyntaxNewExpression* pcNew);
	SCPP65Type*				NewTypeIdArray(CCPPSyntaxNewExpression* pcNew);
	bool					IsArrayNew(CCPPSyntaxNewExpression* pcNew);
	void					NewArguments(CCPPSyntaxNewExpression* pcNew, CArraySyntax* papcArguments);
	CCPPSyntaxIdExpression*	FunctionNameOf(CCPPSyntax* pcExpression);
	bool					IsMemberPointerOperator(CCPPSyntax* pcExpression);
	bool					HasMember(CCPPClass* pcClass, int64 lliName);
	bool					HasOperator(CCPPClass* pcClass, int64 lliName, size uiArguments);
	CCPPSymbol*				QualifiedEnumerator(CCPPSyntaxName* pcName, CCPPSyntax* pcNode, CCPPClass** ppcClass);
	CCPPSymbol*				DecltypeEnumerator(CCPPSyntaxName* pcName);
	SCPP65Type*				OperandType(CCPPSyntaxTypeId* pcType, CCPPSyntax* pcOperand);
	CCPPSymbol*				QualifiedMember(CCPPSyntaxName* pcName, ECPPSymbolKind eKind, CCPPSyntax* pcNode, CCPPClass** ppcClass);
	bool					IsAmbiguousInBases(CCPPClass* pcClass, int64 lliName);
	void					FindInBases(CCPPClass* pcClass, int64 lliName, CArrayCPPClasses* papcFound);
	CCPPClass*				QualifiersClass(CCPPSyntaxName* pcName, CCPPSyntax* pcNode);
	CCPPClass*				NameQualifierClass(CCPPSyntaxName* pcName, CCPPSyntax* pcNode);
	CCPPClass*				QualifiedClass(CCPPSyntaxName* pcName, CCPPSyntax* pcNode);
	CCPPClass*				MemberClass(CCPPClass* pcClass, CCPPSyntaxNameComponent* pcComponent, CCPPSyntax* pcNode);
	void					ReportFailedDeclaration(int64 lliName);
	CCPPFunction*			DeclareQuietly(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, bool bDefinition, CCPPClass* pcClass, bool bPure, CCPPErrors** ppcErrors = NULL);
	SCPP65Type*				TypeOf(CCPPSyntax* pcExpression);
	SCPP65Type*				DeclaredTypeOf(CCPPSyntax* pcExpression);
	SCPP65Type*				UnevaluatedTypeOf(CCPPSyntax* pcExpression);
	bool					NamesFunction(CCPPSyntax* pcExpression);
	SCPP65Type*				ValueTypeOf(CCPPSyntax* pcExpression);
	bool					CheckInitialiser(CCPPSyntax* pcValue, SCPP65Type* psTo);
	bool					CheckBracedValue(CCPPSyntax* pcValue, SCPP65Type* psTo);

	SCPP65Type*				PlaceTypeOf(CCPPSyntax* pcExpression);

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
	bool					IsObject(CCPPSyntax* pcExpression);
	bool					IsOperatorOperand(CCPPSyntax* pcExpression, ECppTokens eOperator);
	bool					IsOverloadedComma(CCPPSyntaxBinaryExpression* pcBinary);
	CCPPSyntax*				CommaResult(CCPPSyntax* pcExpression);
	bool					IsAggregate(SCPP65Type* psType);
	SCPP65Type*				AggregateMember(SCPP65Type* psType, size uiIndex);
	CCPPClass*				ConditionalClass(SCPP65Type* psTrue, SCPP65Type* psFalse);
	SCPP65Type*				ConditionalLvalueType(CCPPSyntaxConditionalExpression* pcConditional);
	SCPP65Type*				ConditionalXvalueType(CCPPSyntaxConditionalExpression* pcConditional);
	SCPP65Type*				ConditionalObjectType(CCPPSyntaxConditionalExpression* pcConditional, bool* pbAmbiguous);
	CCPPClass*				MemberObjectClass(CCPPSyntaxMemberExpression* pcMember);
	bool					BindMemberOf(CCPPSyntaxMemberExpression* pcMember, CCPPBoundName* pcBound);
	CCPPSymbol*				MemberEnumerator(CCPPSyntaxMemberExpression* pcMember);
	SCPPField*				BitFieldOf(CCPPSyntax* pcExpression);
	CCPPSyntaxIdExpression*	MemberPointerName(CCPPSyntaxUnaryExpression* pcAddress);
	SCPP65Type*				OperandType(SCPP65Type* psType, CCPPSyntax* pcExpression);
	SCPP65Type*				VariadicArgumentType(SCPP65Type* psType);
	SCPP65Type*				CalleePointerType(CCPPSyntax* pcCallee);
	SCPP65Type*				ConditionalType(SCPP65Type* psTrue, SCPP65Type* psFalse);
	SCPP65Type*				CompositePointerType(SCPP65Type* psLeft, SCPP65Type* psRight);
	SCPP65Type*				CompositePointee(SCPP65Type* psLeft, SCPP65Type* psRight, bool bFirstLevel);
	bool					IsStdName(int64 lliName, CCPPScope* pcScope, const char* szName);
	bool					IsNamed(int64 lliName, const char* szName);
	CCPPClass*				TypeInfoClass(void);
	SCPP65Type*				HandlerType(CCPPSyntaxCatchHandler* pcHandler);
	CCPPSyntax*				TryRegionOf(CCPPSyntax* pcNode, CCPPSyntax* pcStop);
	void					ThrownDestroyer(SCPP65Type* psType, CChars* pszLabel);
	bool					IsTypeInfoMember(CCPPFunction* pcFunction);
	SCPP65Type*				TypeidType(CCPPSyntaxTypeidExpression* pcTypeid, bool* pbDynamic);
	CCPPTemplate*			InitializerListTemplate(void);
	SCPP65Type*				InitializerListElement(SCPP65Type* psType);
	SCPP65Type*				InitializerListOf(SCPP65Type* psElement, CCPPSyntax* pcNode);
	SCPP65Type*				BracedListType(CCPPSyntax* pcList, CCPPSyntax* pcNode);
	bool					InitializerListLayout(CCPPClass* pcClass, bool* pbEnd);
	CCPPClass*			ObjectClass(CCPPSyntax* pcExpression);
	bool					IsMadeObject(CCPPSyntax* pcExpression, CCPPClass* pcClass);
	bool					IsRvalue(CCPPSyntax* pcExpression);
	bool					IsReturnedLocal(CCPPSyntaxIdExpression* pcId);
	bool					IsObjectLvalue(CCPPSyntax* pcExpression);
	bool					IsUncapturedConstant(CCPPSyntax* pcExpression);
	SCPP65Type*				PlaceOf(CCPPSyntax* pcExpression);
	bool					BindsToTemporary(SCPP65Type* psReference, CCPPSyntax* pcArgument);
	CCPPSyntax*				BoundInitialiser(SCPP65Type* psReference, CCPPSyntax* pcInitialiser);
	bool					MakesObject(CCPPSyntax* pcExpression);

	//Check only.  CPPUnitCheck.cpp
	CArraySyntax*			ConstructElements(CCPPSyntaxConstructExpression* pcConstruct);
	void					DeclareBlockFunction(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator);

	//Floating point.  CPPUnitFloat.cpp
};


#endif // __C_P_P_UNIT_H__
