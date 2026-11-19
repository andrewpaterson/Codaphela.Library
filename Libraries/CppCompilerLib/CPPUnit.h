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


//An error in a class's own declaration (a bit-field's width), found when it is laid out: kept apart from mcErrors,
//which quiet trials and the IR front end truncate, and reported when the unit ends (ReportClassErrors).
struct SCPPClassError
{
	int				iToken;
	const char*		szMessage;
};


typedef CArrayTemplate<SCPPClassError>	CArrayCPPClassErrors;


//GCC's builtin functions the compiler does (CCPPUnit::FindBuiltin), for the type checker and the IR front end.
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
typedef CArrayTemplatePtr<CArrayInt64>		CArrayCPPValuePacks;


//A template parameter pack's element as a parameter of its own (CCPPUnit::PackElement), in the bindings it is bound in.
struct SCPPPackElement
{
	CCPPTemplateArguments*	pcBindings;
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


//A remembered TypeOf: the type of an expression with some template arguments.
struct SCPPTypeOf
{
	CCPPTemplateArguments*		pcBindings;
	CCPPFunction*		pcFunction;
	SCPP65Type*			psType;
	SCPP65Type*			psDeclared;		//psType, or the reference to it the expression is declared to give (f() of T& f(), a cast to T&&).
};


typedef CMapLongTemplate<SCPPTypeOf>		CMapCPPTypeOfs;


//What a range-for statement is made of (CCPPUnit::GetRangeFor): the variables the compiler makes for it, and the
//expressions on them.
struct SCPPRangeFor
{
	CCPPSymbol*					pcRange;		//auto&& __range = range.
	CCPPSymbol*					pcBegin;		//auto __begin = begin.
	CCPPSymbol*					pcEnd;			//auto __end = end.
	CCPPSyntaxIdExpression*		pcRangeOfBegin;	//__range, as what begin is of.
	CCPPSyntaxIdExpression*		pcRangeOfEnd;	//__range, as what end is of.
	CCPPSyntaxBinaryExpression*	pcCondition;	//__begin != __end.
	CCPPSyntaxUnaryExpression*	pcIncrement;	//++__begin.
	CCPPSyntaxUnaryExpression*	pcElement;		//*__begin.
};


typedef CMapLongTemplate<SCPPRangeFor>	CMapCPPRangeFors;


//The call a user-defined literal is (CCPPUnit::GetLiteralCall): the literal operator of its suffix, and its arguments
//(implicit ones, holding the literal) in an initialiser of their own.
struct SCPPLiteralCall
{
	CCPPFunction*							pcFunction;		//NULL if no literal operator takes the literal.
	CCPPSyntaxParenthesisedInitializer*		pcArguments;
};


typedef CMapLongTemplate<SCPPLiteralCall>	CMapCPPLiteralCalls;


//The function being written, put aside while another's parameters and locals are made (CCPPUnit::EnterFunctionScope).
struct SCPPFunctionScope
{
	CCPPFunction*		pcFunction;
	CCPPSymbol*			pcPackParameter;
	CArrayCPPVariables	apcLocals;
	CArrayCPPSymbols	apcPackElements;
};




#define CPP65_BUILTIN_PREFIX			"__builtin_"
#define CPP65_BUILTIN_PREFIX_LENGTH		10


class CCPP65IRFrontEnd;
class CCPPConstants;
class CCTInt;
class CCPPSemanticPass;


//Generates 65C64 or W65C816 assembly (Assembler65Lib syntax) from a parsed translation unit: the declarations,
//types, globals and start up here, and every function body as IR (CPP65IRFrontEnd, IR.txt) generated by the
//target's IR back end.  The calling convention and the parts of C++ that are supported are in CPP65Generator.txt.
//
//Check only works out the types of the unit's expressions, for CCPPSemanticPass: no code is generated and it keeps
//going past what it can't type (CPPUnitCheck.cpp).
typedef CArrayTemplatePtr<CCPPSyntaxClassSpecifier>	CArrayCPPNameClasses;


class CCPPUnit : public CCPPRuntimeWriter
{
protected:
	CCPPSyntaxTree*			mpcTree;
	CCPPTokeniser*			mpcTokeniser;
	CCPP65Types				mcTypes;
	CCPPTarget*				mpcTarget;

	CChars					mszCode;
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
	CArrayCPPValuePacks	mapalliPacks;		//The non-type packs' values (NewValuePack).
	CArrayCPPSymbols		mapcPackSymbols;	//The symbols made for function and template parameter packs' elements.
	CArrayCPPPackElements	masPackElements;	//Template parameter packs' elements in each bindings (PackElement).
	CArrayCPPExpandedArgumentLists	masExpandedArguments;	//Argument lists with their packs expanded (ExpandArguments).
	CArrayCPPSymbols		mapcHiddenSymbols;	//The symbols of the variables made for range-for statements.
	CMapCPPRangeFors		mcRangeFors;		//What each range-for statement is made of, by its node.
	CMapCPPLiteralCalls		mcLiteralCalls;		//The call each user-defined literal is, by its node.
	CCPPTemplate*			mpcInitializerList;	//std::initializer_list, once it is found (InitializerListTemplate).
	CCPPClass*				mpcTypeInfo;		//std::type_info, once it is found (TypeInfoClass).
	CArrayCPPLambdas		mapsLambdas;		//Every lambda's closure, as each function it is in makes it.
	CArraySyntax			mapcMemberSpecialisations;	//template<> R Class<Arguments>::f() { ... }, each given to its instance.
	CArraySyntax			mapcStaticMemberTemplates;	//template<class T> int Class<T>::s = ...;, defined for each instance (DefineInstanceStatics).
	CCPPVariable*			mpcDefining;		//The variable of a template's instance GlobalDeclaration is defining, or NULL.
	CArrayCPPVariables		mapcVariableInstances;	//Variable templates' instances (VariableInstance), in mapcGlobals too.
	CCPPSyntaxImplicitArgument*	mpcImplicitZero;	//The int 0 of s++'s operator++(0) (ImplicitZero).
	CMapLongTemplate<CCPPSyntaxImplicitArgument*>	mcImplicitValues;	//The arguments the IR front end gives values (ImplicitValue).
	int						miUnevaluated;		//How deep in operands that aren't evaluated (sizeof, decltype) the unit is.
	CArrayCPPSymbols		mapcPackElements;	//Its elements' symbols.
	CArrayCPPFunctions	mapcQueue;			//Functions to generate, first queued first...
	size					muiQueueNext;		//...from this one: those before it have been.
	CArraySyntax			mapcFailed;			//Declarators of functions whose declarations were not supported.
	CMapCPPTypeOfs		mcTypeOfs;			//TypeOf's answers, by expression.
	CArrayChars				maszEmitted;		//Labels of the functions, vtables, thunks and runtime routines written.
	int						miWarningChecks;	//Above 0 while a warning check works out a type (TypeOf can fold the same constant again).
	CArraySyntax			mapcConstantTypes;	//The expressions whose types Constant is finding (ConstantTypeOf).
	bool					mbPureVirtual;
	bool					mbBadCast;			//A dynamic_cast to a reference calls R_BadCast (UseBadCast).
	bool					mbRecursiveInit;	//A guarded static local calls R_RecursiveInit if it is reached again while being made.
	bool					mbExitLists;		//Something is pushed on an exit list: the unit writes R_RunExitLists (UseExitLists).
	bool					mbExitListsWritten;
	CArrayChars				maszExitFinalisers;	//The labels of static locals destroyed at exit whose R_Finalise is still to write...
	CArrayCPP65Types		mapsExitFinalisers;	//...their types...
	CArrayInt				maiExitShared;		//...and whether each is a shared function's (its R_Finalise COMDAT; WriteExitRoutines).

	CCPPUnitData			mcData;				//What the unit has other than code: globals, the init and fini tables, strings.
	CCPPContext				mcContext;			//Where the compiler is: the template arguments, the namespace, the linkage, the function.
	CCPPBinder				mcBinder;			//What each call, name and member is, worked out in one place.
	CCPPDeclarators			mcDeclarators;
	CCPPMangler				mcMangler;			//Makes the labels.
	CArrayCPPVariables	mapcLocals;
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
	bool					mbLineTable;		//The code has .line and .fingerprint directives, for the linker's line table (SetLineTable).
	bool					mbNoFloats;			//Floating point in the code it writes is an error (SetNoFloats): a kernal's units.
	int64					mlliCompileTime;	//When the unit was compiled (seconds since 1970), for .fingerprint.
	CSpillStore*			mpcSpillStore;		//Where the functions' code waits until the unit is assembled (SetSpillStore),
	uint32					muiSpillUnit;		//by the unit's number.
	CMemorySpillStore		mcMemorySpillStore;	//The spill store when none is set.
	CScratchPadAllocator	mcBackendScratch;	//The W65C816 back end's working arrays and strings, for one function at a time (CIRW65C816Backend::SetScratch).
	CArrayLargeTemplate<uint16>	mauiCodePieces;	//Per function written by the IR front end: how many pieces its code is in.
	CCPPSemanticPass*		mpcSemanticPass;	//Check only: what names resolved to goes here.  NULL when generating.
	bool					mbRemember;			//A kept check: the pass is told each expression's type and what changes its place too.
	CCPPCompileStatistics*	mpcStatistics;		//What compiling the unit is counted and timed in (SetStatistics): the builder's for the unit...
	CCPPCompileStatistics	mcOwnStatistics;	//...or the generator's own when none is set.
	CCPPCompileOptions*		mpcOptions;			//How the unit is compiled where there is a choice (SetOptions): the builder's, or the defaults.

public:
	//What the classes that work for the unit ask it for.
	CCPP65Types*			GetTypes(void);
	CCPPContext*			GetContext(void);
	CCPPBinder*				GetBinder(void);
	CCPPConstants*			GetConstants(void) const;
	CCPPDeclarators*		GetDeclarators(void);
	CCPPMangler*			GetMangler(void);
	CCPPUnitData*			GetData(void);
	CCPP65TypeChecker*		GetTypeChecker(void);
	CChars*					GetCode(void);
	CArrayCPPTemplates*		GetTemplates(void);
	CArrayCPPVariables*		GetLocals(void);
	CArrayLargeTemplate<uint16>*	GetCodePieces(void);
	int*					GetLabelCounter(void);
	CCPPTarget*				GetTarget(void) const;
	CIRImports*				GetImports(void) const;
	CCPPTokeniser*			GetTokeniser(void) const;
	CCPPSyntaxTree*			GetTree(void) const;
	CArrayCPPSymbols*		GetPackElements(void);
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
	void					SetLineTable(bool bLineTable, int64 lliCompileTime);
	void					SetNoFloats(bool bNoFloats);
	void					SetSpillStore(CSpillStore* pcStore, uint32 uiUnit = 0);
	void					SetStatistics(CCPPCompileStatistics* pcStatistics);
	void					SetOptions(CCPPCompileOptions* pcOptions);
	CCPPCompileOptions*		GetOptions(void) const;
	CCPPCompileStatistics*	GetStatistics(void) const;
	CScratchPadAllocator*	GetBackendScratch(void);
	void					GetMissingCallees(CArrayChars* paszNames);
	bool					Check(CCPPSemanticPass* pcPass, bool bRemember = false);

	//Declarations, functions and statements.  CPPUnit.cpp
	bool					Error(CCPPSyntax* pcNode, const char* szMessage);
	void					Warning(CCPPSyntax* pcNode, const char* szMessage);

	bool					ErrorAt(int iToken, const char* szMessage);
	void					AppendMacroUse(SPreprocessedToken* psSource, CChars* pszMessage);
	void					BeginQuiet(void);
	bool					EndQuiet(void);
	void					BeginTrial(void);
	bool					EndTrial(void);
	void					BeginLoud(void);
	void					EndLoud(void);
	CChars*					Line(const char* szText);
	void					AppendName(CChars* psz, int64 lliName);
	int64					GetName(CCPPSyntaxDeclarator* pcDeclarator);

	CCPPFunction*			DeclareFunction(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, bool bDefinition, CCPPClass* pcClass, bool bPure, CCPPTemplate* pcTemplate = NULL, CChars* pszArguments = NULL);
	CCPPFunction*			DeclareSignature(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, bool bDefinition, CCPPClass* pcClass, bool bPure, CCPPTemplate* pcTemplate, CChars* pszArguments);
	SCPP65Type*				FunctionType(CCPPFunction* pcFunction);
	bool					IsDeclaredNoexcept(CCPPSyntax* pcSpecification, ECPPFunctionKind eKind, int64 lliName);
	bool					CheckOverrides(CCPPClass* pcClass);
	bool					HasVirtSpecifier(CCPPSyntaxFunctionSuffix* pcSuffix, int64 lliName);
	bool					IsAllocationFunction(int64 lliName);
	bool					IsDefinedWhereDeclared(CCPPSyntaxName* pcName);
	bool					ResolveReturn(CCPPFunction* pcFunction, CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator);
	SCPP65Type*				ReturnedType(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, CCPPSyntaxReturnStatement* pcReturn);
	void					FindReturns(CCPPSyntax* pcNode, CArraySyntax* papcReturns);
	SCPP65Type*				ReturnDeclaratorType(SCPP65Type* psReturn, CCPPSyntaxDeclarator* pcDeclarator);
	void					EnterFunctionScope(SCPPFunctionScope* psScope, CCPPFunction* pcFunction);
	void					LeaveFunctionScope(SCPPFunctionScope* psScope);
	CCPPFunction*			AddFunction(CCPPClass* pcClass, int64 lliName, ECPPFunctionKind eKind, SCPP65Type* psReturn);
	CCPPFunction*			FirstFunctionOfName(int64 lliName);
	void					RequireFunction(CCPPFunction* pcFunction);
	bool					Emit(CChars* pszLabel);
	bool					Declarations(CArraySyntax* papcDeclarations);
	bool					IsScopeDeclaration(CCPPSyntax* pcDeclaration);
	bool					IsOutsideMemberTemplate(CCPPSyntax* pcDeclaration);
	bool					GlobalDeclaration(CCPPSyntaxSimpleDeclaration* pcDeclaration);
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
	void					BeginFunction(CCPPFunction* pcFunction);
	void					EndFunction(CCPPFunction* pcFunction);
	void					AppendUnsupported(CChars* psz);
	bool					AllocateLocals(CCPPSyntax* pcNode);
	SCPPRangeFor*			GetRangeFor(CCPPSyntaxRangeForStatement* pcFor);
	CCPPSymbol*				NewHiddenVariable(const char* szName, CCPPSyntaxRangeForStatement* pcFor);
	CCPPSyntaxIdExpression*	NewHiddenName(CCPPSymbol* pcSymbol, CCPPSyntax* pcParent);
	bool					RangeForLocals(CCPPSyntaxRangeForStatement* pcFor);
	SCPPLambda*				GetLambda(CCPPSyntaxLambdaExpression* pcSyntax);
	void					NameLambda(SCPPLambda* psLambda);
	bool					LambdaNumber(CCPPSyntax* pcNode, CCPPSyntaxLambdaExpression* pcLambda, size* puiNumber);
	bool					IsSharedFunction(CCPPFunction* pcFunction);
	SCPP65Type*				LambdaType(CCPPSyntaxLambdaExpression* pcSyntax);
	bool					LambdaCaptures(SCPPLambda* psLambda);
	bool					UsedCaptures(SCPPLambda* psLambda, CCPPSyntax* pcNode);
	bool					UseThis(SCPPLambda* psLambda, CCPPSyntax* pcNode);
	bool					IsEnclosingLocal(CCPPSymbol* pcSymbol, SCPPLambda* psLambda);
	bool					IsEnclosingConstant(CCPPSymbol* pcSymbol, SCPPLambda* psLambda);
	SCPP65Type*				EnclosingConstantType(CCPPSymbol* pcSymbol);
	bool					IsOdrUse(CCPPSyntax* pcNode);
	bool					IsThisMember(CCPPSymbol* pcSymbol);
	SCPPLambdaCapture*		FindLambdaCapture(SCPPLambda* psLambda, CCPPSymbol* pcSymbol);
	bool					AddCapture(SCPPLambda* psLambda, CCPPSymbol* pcSymbol, CCPPSyntax* pcInitialiser, bool bReference, bool bOwn, CCPPSyntax* pcNode);
	bool					AddThisCapture(SCPPLambda* psLambda, CCPPSyntax* pcNode);
	size					AddClosureField(SCPPLambda* psLambda, SCPP65Type* psType);
	bool					LayoutClosure(SCPPLambda* psLambda);
	bool					LambdaCall(SCPPLambda* psLambda);
	CCPPFunction*			LambdaFunction(SCPPLambda* psLambda, CArrayCPP65Types* papsParameters);
	CCPPFunction*			AddLambdaFunction(SCPPLambda* psLambda, int64 lliName, CArrayCPP65Types* papsParameters, SCPP65Type* psReturn);
	void					GenericLambdaCall(SCPPLambda* psLambda, CArraySyntax* papcArguments);
	SCPPLambda*				WritingLambda(void);
	SCPPLambda*				BodyLambda(CCPPFunction* pcFunction);
	bool					LambdaError(SCPPLambda* psLambda, CCPPSyntax* pcNode, const char* szMessage);
	bool					ReportOnce(CCPPSyntax* pcNode, const char* szMessage);
	SCPPLambdaCapture*		FindCapture(CCPPSymbol* pcSymbol);
	CCPPFunction*			OuterFunction(void);
	CCPPClass*				ThisClass(void);
	void					FreeLambdas(void);
	bool					RangeIterators(CCPPSyntaxRangeForStatement* pcFor, SCPP65Type** ppsBegin, SCPP65Type** ppsEnd, CCPPFunction** ppcBegin, CCPPFunction** ppcEnd, bool* pbMember);
	bool					ExternLocal(CCPPSyntaxInitDeclarator* pcInit, CCPPSyntaxDeclSpecifiers* pcSpecifiers);
	bool					StaticLocal(CCPPSyntaxInitDeclarator* pcInit, CCPPSyntaxDeclSpecifiers* pcSpecifiers);
	bool					AnonymousLocal(CCPPSyntaxClassSpecifier* pcSpecifier);
	bool					StaticAssert(CCPPSyntaxStaticAssert* pcAssert);
	bool					ClassStaticAsserts(CCPPSyntaxClassSpecifier* pcSpecifier);
	bool					CheckEnumerators(CCPPSyntax* pcType);
	bool					WriteExitRoutines(void);
	bool					IsCLinkage(CCPPSyntaxLinkageSpecification* pcLinkage);
	void					AppendVariableLabel(CChars* psz, CCPPVariable* pcVariable);
	void					AppendGuardLabel(CChars* psz, CCPPVariable* pcVariable);
	void					AppendRuntime(void);
	void					UseBadCast(void) { mbBadCast = true; }
	void					UseRecursiveInit(void) { mbRecursiveInit = true; }
	void					UseExitLists(void) { mbExitLists = true; }
	CArrayChars*			GetRuntimeCalls(void) override;
	bool					BeginRuntime(const char* szName) override;
	CChars*					GetRuntimeCode(void) override;
	void					RuntimeError(const char* szMessage) override;
	bool					GlobalStartUpRoutines(SCPP65Type* psType, CCPPSyntax* pcInitialiser, CChars* pszData, bool bConstruct, bool bDestroy, bool bThread);
	bool					BoundTemporary(SCPP65Type* psReference, CCPPSyntax* pcInitialiser, CChars* pszLabel, bool bGlobal, const char* szOptions, CChars* pszBound, int* piOffset, bool* pbResult);
	bool					HasUsedAttribute(CCPPSyntaxDeclarator* pcDeclarator);
	bool					HasAttribute(CCPPSyntaxDeclarator* pcDeclarator, const char* szName);
	bool					HasAttribute(CArraySyntax* papcAttributes, const char* szName);
	bool					IsRestrictPointer(CCPPSyntaxDeclarator* pcDeclarator);
	SCPP65Type*				QualifiedPointer(SCPP65Type* psType, CCPPSyntaxPointerOperator* pcPointer);
	SCPP65Type*				ParameterType(SCPP65Type* psType);
	SCPP65Type*				ParameterVariableType(SCPP65Type* psType, CCPPSyntaxParameter* pcParameter);
	bool					IsBss(CCPPSyntax* pcInitialiser, SCPP65Type* psType);
	int						DataAlignment(SCPP65Type* psType, bool bClass);

	void					ReportTypeOf(CCPPSyntax* pcExpression);
	bool					ReportError(CCPPSyntax* pcNode, const char* szMessage);
	bool					ReportErrorAt(int iToken, const char* szMessage);
	bool					HasWarning(CCPPSyntax* pcNode, const char* szText);

	//Classes, templates, virtual functions, overloads and labels.  CPPUnitClasses.cpp, CPPUnitTemplates.cpp,
	//CPPUnitVtables.cpp, CPPUnitOverloads.cpp and CPPUnitLabels.cpp
	CCPPClass*			NewClass(CCPPSymbol* pcSymbol, int64 lliName);
	CCPPClass*			GetClass(CCPPSymbol* pcSymbol, CCPPSyntax* pcNode);
	CCPPClass*				MemberScopeClass(CCPPSymbol* pcSymbol);
	CCPPClass*			GetUnnamedClass(CCPPSyntaxClassSpecifier* pcSpecifier);
	CCPPClass*			FindClass(int64 lliName, CCPPScope* pcLookup);
	CCPPScope*				ComponentLookup(CCPPSyntaxNameComponent* pcComponent);
	CCPPClass*			GetComponentClass(CCPPSyntaxNameComponent* pcComponent, CCPPSymbol* pcSymbol, CCPPSyntax* pcNode, CCPPClass* pcContext = NULL);
	CCPPClass*			FindBaseInstance(CCPPClass* pcClass, CCPPTemplate* pcTemplate);
	CCPPClass*			QualifierClass(CCPPClass* pcClass, CCPPSyntaxNameComponent* pcComponent, CCPPSyntax* pcNode);
	int						AlignasAlignment(CArraySyntax* papcSpecifiers);
	CCPPClass*			GetNameClass(CCPPSyntaxName* pcName, CCPPSyntax* pcNode);
	bool					TemplateDeclaration(CCPPSyntaxTemplateDeclaration* pcTemplateDeclaration);
	bool					ExplicitInstantiation(CCPPSyntax* pcDeclaration, bool bExtern);
	bool					ExplicitFunctionInstantiation(CCPPSyntaxInitDeclarator* pcInit, bool bExtern);
	bool					IsInstantiatedElsewhere(CCPPFunction* pcFunction);
	CCPPTemplate*			GetTemplate(CCPPSymbol* pcSymbol);
	CCPPTemplate*			MemberClassTemplate(CCPPSymbol* pcSymbol, CCPPClass* pcClass);
	CCPPTemplate*			GetTemplateByName(int64 lliName, ECPPSyntax eDefinition, CCPPScope* pcLookup);
	CCPPTemplate*			GetComponentTemplate(CCPPSyntaxNameComponent* pcComponent);
	CCPPTemplateArguments*			BindArguments(CCPPSyntaxTemplateDeclaration* pcTemplate, CArraySyntax* papcArguments, CArrayCPP65Types* papsDeduced, CChars* pszArguments, CCPPSyntax* pcNode, CArrayInt64* palliDeduced = NULL);
	SCPPBinding*			FindBinding(CCPPSymbol* pcParameter);
	CCPPClass*			InstantiateClass(CCPPTemplate* pcTemplate, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	CCPPClass*			InstanceOfClass(CCPPTemplate* pcTemplate, CCPPTemplateArguments* pcBindings, CChars* pszArguments, CCPPSyntax* pcNode);
	CCPPClass*			PlaceholderClass(CCPPSymbol* pcParameter);
	CCPPTemplateArguments*	BindAsItself(CCPPSyntaxTemplateDeclaration* pcDeclaration, CChars* pszArguments, CCPPSyntax* pcNode);
	CCPPClass*			InstantiateClassAsItself(CCPPTemplate* pcTemplate);
	CCPPFunction*		InstantiateFunctionAsItself(CCPPTemplate* pcTemplate);
	CCPPFunction*		InstanceOfFunction(CCPPTemplate* pcTemplate, CCPPTemplateArguments* pcBindings, CChars* pszArguments, CCPPSyntax* pcNode);
	void				CheckTemplatesAsThemselves(void);
	bool				IsCheckedAsWritten(void) const;
	CCPPClass*			UndefinedInstance(CCPPSymbol* pcSymbol, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	CCPPFunction*			InstantiateFunction(CCPPTemplate* pcTemplate, CArraySyntax* papcExplicit, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	CCPPTemplate*			NewTemplate(CCPPSymbol* pcSymbol, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntax* pcDefinition, bool bMember);
	bool					IsClassTemplateDefinition(CCPPSyntax* pcDeclaration);
	bool					MemberTemplate(CCPPClass* pcClass, CCPPSyntaxTemplateDeclaration* pcTemplateDeclaration);
	bool					IsConstSuffix(CCPPSyntaxFunctionSuffix* pcSuffix);
	ECPPRefQualifier		RefQualifier(CCPPSyntaxFunctionSuffix* pcSuffix);
	bool					DefineMemberTemplate(CCPPTemplate* pcTemplate, CCPPSyntax* pcNode);
	CCPPTemplateArguments*			RenameBindings(CCPPClass* pcClass, CCPPSyntaxTemplateDeclaration* pcDeclaration);
	CCPPTemplateArguments*			NewBindings(void);
	CArrayCPP65Types*		NewPack(void);
	CArrayInt64*			NewValuePack(void);
	CCPPSymbol*				FindPacks(CCPPSyntax* pcPattern, CArrayCPPBindingPtrs* papsPacks);
	bool					ExpandPattern(CCPPSyntax* pcPattern, CCPPSyntax* pcParent, CArraySyntax* papcExpanded);
	CArraySyntax*			ExpandArguments(CArraySyntax* papcArguments);
	void					BeginUnevaluated(void) { miUnevaluated++; }
	void					EndUnevaluated(void) { miUnevaluated--; }
	bool					IsUnevaluated(void) const;
	CArraySyntax*			ListElements(CCPPSyntax* pcList);
	CCPPSymbol*				PackElement(SCPPBinding* psPack, size k);
	bool					ExpandTemplateArguments(CArraySyntax* papcArguments, CArrayCPPExpandedArguments* pasExpanded);
	SCPP65Type*				ExpandedArgumentType(SCPPExpandedArgument* psArgument);
	bool					ExpandedArgumentValue(SCPPExpandedArgument* psArgument, int64* plliValue);
	bool					PackSize(CCPPSymbol* pcPack, int64* plliValue);
	bool					IsEntityParameter(CCPPSyntaxParameter* pcParameter);
	bool					BindEntity(SCPPBinding* psBinding, SCPPExpandedArgument* psArgument, CChars* pszArguments);
	bool					BindTemplate(SCPPBinding* psBinding, CCPPSyntax* pcArgument, CCPPSyntax* pcNode, CChars* pszArguments);
	bool					BindTypePack(SCPPBinding* psBinding, CArrayCPPExpandedArguments* pasArguments, size uiFirst, CArrayCPP65Types* papsDeduced, CChars* pszArguments);
	bool					BindValuePack(SCPPBinding* psBinding, CArrayCPPExpandedArguments* pasArguments, size uiFirst, CChars* pszArguments);
	void					AppendMangledValue(CChars* pszArguments, int64 lliValue);
	SCPPBinding*			PackBinding(CCPPSyntaxParameter* pcParameter);

	void					DeclareClassTemplate(CCPPSyntaxElaboratedTypeSpecifier* pcSpecifier, CCPPSyntaxTemplateDeclaration* pcDeclaration);
	void					DefineClassTemplate(CCPPSymbol* pcSymbol, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntaxClassSpecifier* pcSpecifier);
	CCPPTemplate*			Specialisation(CCPPSymbol* pcSymbol, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntax* pcDefinition, CCPPSyntaxNameComponent* pcPattern);
	CCPPTemplate*			OutsideMemberOwner(CCPPTemplate* pcPrimary, CCPPSyntaxNameComponent* pcClass, CCPPSyntaxTemplateDeclaration* pcDeclaration);
	CCPPTemplate*			FindClassSpecialisation(CCPPTemplate* pcPrimary, CCPPTemplateArguments* pcArguments, CCPPTemplateArguments** ppcBindings, CCPPSyntax* pcNode);
	CCPPTemplate*			FindFunctionSpecialisation(CCPPTemplate* pcPrimary, CChars* pszArguments, CCPPSyntax* pcNode);
	bool					CompleteClass(CCPPClass* pcClass, CCPPSyntax* pcNode);
	bool					CompleteObjectClass(CCPPClass* pcClass, CCPPSyntax* pcNode);
	bool					ResolveBases(CCPPClass* pcClass, CCPPSyntax* pcNode);
	bool					AddBase(CCPPClass* pcClass, CCPPSyntaxBaseSpecifier* pcBaseSpecifier);
	bool					AddPackBases(CCPPClass* pcClass, CCPPSyntaxBaseSpecifier* pcBaseSpecifier);
	bool					LayoutClass(CCPPClass* pcClass, CCPPSyntax* pcNode);
	void					DefineOutsideMember(CCPPClass* pcClass, CCPPTemplate* pcMember);
	void					MemberSpecialisation(CCPPSyntaxFunctionDefinition* pcDefinition);
	bool					IsStaticMemberDefinition(CCPPSyntaxSimpleDeclaration* pcSimple);
	bool					DefineInstanceData(void);
	bool					DefineInstanceStatics(bool* pbResult);
	bool					DefineVariableInstances(bool* pbResult);
	CCPPSyntaxName*			VariableTemplateName(CCPPSyntaxSimpleDeclaration* pcSimple);
	CCPPVariable*			VariableInstance(CCPPSymbol* pcSymbol, CCPPSyntaxNameComponent* pcComponent, CCPPSyntax* pcNode);
	CCPPSyntaxInitDeclarator*	VariableDeclarator(CCPPVariable* pcVariable);
	CCPPTemplate*			FindVariableSpecialisation(CCPPTemplate* pcPrimary, CChars* pszArguments, CCPPSyntax* pcNode);
	bool					VariableTemplateValue(CCPPSyntaxIdExpression* pcId, int64* plliValue);
	bool					IsInstanceStaticDefined(CCPPClass* pcClass, CCPPSyntaxName* pcName);
	CCPPVariable*			OwnStatic(CCPPClass* pcClass, CCPPSymbol* pcSymbol);
	void					SpecialiseMembers(CCPPClass* pcClass);
	void					SpecialiseMember(CCPPClass* pcClass, CCPPSyntaxFunctionDefinition* pcDefinition);
	void					DefineOutsideMembers(CCPPTemplate* pcMember);
	bool					ClassMember(CCPPClass* pcClass, CCPPSyntax* pcMember);
	bool					ClassBitField(CCPPClass* pcClass, CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxInitDeclarator* pcInit);
	bool					ClassError(int iToken, const char* szMessage);
	void					ReportClassErrors(void);
	void					ReportUndeclaredTypes(void);
	void					UndeclaredType(CCPPSyntaxNamedTypeSpecifier* pcNamed);
	void					ReportNamedInstances(void);
	bool					IsNameClass(CCPPSyntaxClassSpecifier* pcClass);
	void					NamedInstance(CCPPSyntaxNameComponent* pcComponent, CArrayCPPNameClasses* papcClasses);
	void					AppendTokensText(int iFirstToken, int iLastToken, CChars* pszDest);
	void					EnsureVtable(CCPPClass* pcClass);
	void					EnsureTypeInfo(CCPPClass* pcClass);
	void					AppendVtableParts(CCPPClass* pcClass, CChars* pszLabel, CArrayCPPVtableEntries* pasEntries);
	void					EnsureVTT(CCPPClass* pcClass);
	void					AppendVTTEntries(CCPPClass* pcComplete, CCPPClass* pcClass, int iOffset, CChars* pszWords);
	size					VTTEntries(CCPPClass* pcClass);
	int						VTTIndex(CCPPClass* pcClass, CCPPClass* pcBase);
	void					AppendTypeInfoBases(CCPPClass* pcClass, int iOffset, uint32 uiFlags);
	bool					IsPublicBase(CCPPClass* pcClass, CCPPClass* pcBase);
	void					AppendTypeInfoHeader(CChars* pszName, ECPP65TypeInfoKind eKind);
	void					AppendTypeInfoName(CChars* pszInfo, SCPP65Type* psType, bool bShared, CChars* pszName);
	void					AppendTypeName(CChars* psz, SCPP65Type* psType);
	void					AppendTypeNameAround(CChars* psz, SCPP65Type* psType, CChars* pszInner);
	void					AppendClassName(CChars* psz, CCPPClass* pcClass);
	void					AppendScopeName(CChars* psz, CCPPScope* pcScope);
	void					AppendIdentifier(CChars* psz, int64 lliName);
	void					AppendVtableHeader(CCPPClass* pcClass, int iVtablePointer);
	void					AppendVirtualCallThunk(CChars* psz, CCPPFunction* pcFunction);
	void					AppendThunkLabel(CChars* pszLabel, bool bComdat);
	void					AppendVtableEntry(CCPPFunction* pcFunction, int iAdjust);
	CCPPVariable*			FindStatic(CCPPClass* pcClass, int64 lliName);
	bool					IsConstantMember(CCPPVariable* pcVariable);
	void					ClassUsing(CCPPClass* pcClass, CCPPSyntaxUsingDeclaration* pcUsing);
	void					InheritConstructors(CCPPClass* pcClass);
	bool					IsConstObject(CCPPSyntax* pcExpression);
	bool					IsConstPointer(CCPPSyntax* pcExpression);
	bool					IsConstThis(void);
	bool					IsDerived(CCPPClass* pcClass, CCPPClass* pcBase, int* piOffset);
	SCPPVirtualBase*		VirtualBaseOf(CCPPClass* pcClass, CCPPClass* pcBase, int* piBelow);
	bool					CompleteOffset(CCPPClass* pcClass, CCPPClass* pcBase, int* piOffset);
	int64					GetFunctionName(CCPPSyntaxNameComponent* pcComponent);
	int64					GetOperatorName(ECppTokens eToken, bool bArray);
	SCPP65Type*				ArrowType(CCPPSyntax* pcObject, CCPPSyntax* pcNode);
	int64					GetComponentName(CCPPSyntaxNameComponent* pcComponent);


	//Literals, constants, warnings and TypeOf.  CPPUnitExpressions.cpp
	SCPP65Type*				IntegerLiteralType(CCTInt* pcInt);

	CCPPSymbol*				NewPackElement(CCPPSymbol* pcPack);
	bool					HasPackExpansion(CArraySyntax* papcArguments);
	CCPPSyntax*				ExpansionPattern(CCPPSyntax* pcNode);

	bool					IsBuiltin(int64 lliName);
	const SCPPBuiltin*	FindBuiltin(int64 lliName);
	SCPP65Type*				BuiltinOperandType(const SCPPBuiltin* psBuiltin);
	bool					IsIntrinsic(int64 lliName);
	char*					BuiltinName(int64 lliName, int* piLength);
	bool					QuietConstant(CCPPSyntax* pcExpression, int64* plliValue);
	bool					IsInlineMemcpy(int64 lliName, CArraySyntax* papcArguments);

	bool					IsAssignmentOperator(ECppTokens eOperator);

	bool					FunctionNameText(int64 lliName, CChars* pszText);
	bool					IsFunctionName(CCPPSyntaxIdExpression* pcId);
	SCPP65Type*				Literal(CCPPSyntaxLiteralExpression* pcLiteral);
	CCPPSyntaxIdExpression*	FunctionNameFor(CCPPSyntax* pcExpression, SCPP65Type* psTo);
	CCPPSyntaxIdExpression*	MemberFunctionNameFor(CCPPSyntax* pcExpression, SCPP65Type* psTo);
	bool					TokenConstant(CCPPToken* pcToken, int64* plliValue);
	bool					IsStringLiteral(CCPPSyntax* pcExpression);
	CCTUserLiteral*			UserLiteralToken(CCPPSyntax* pcLiteral);
	SCPPLiteralCall*		GetLiteralCall(CCPPSyntax* pcLiteral);
	CCPPSyntaxImplicitArgument*	LiteralArgument(CCPPSyntax* pcLiteral, SCPP65Type* psType, CArraySyntax* papcArguments);
	void					UserLiteralSpelling(int32 iToken, CChars* pszSpelling);
	bool					MayThrow(CCPPSyntax* pcExpression);
	bool					IsAmbiguousName(CCPPSyntaxName* pcName);
	CCPPSyntax*				ImplicitZero(void);
	CCPPSyntaxImplicitArgument*	ImplicitValue(CCPPSyntax* pcFor, SCPP65Type* psType, int iWhich = 0);
	CCPPSyntax*				NewSize(CCPPSyntaxNewExpression* pcNew);
	SCPP65Type*				NewElementType(CCPPSyntaxNewExpression* pcNew);
	void					NewArguments(CCPPSyntaxNewExpression* pcNew, CArraySyntax* papcArguments);
	CCPPSyntaxIdExpression*	FunctionNameOf(CCPPSyntax* pcExpression);
	bool					IsMemberPointerOperator(CCPPSyntax* pcExpression);
	bool					HasMember(CCPPClass* pcClass, int64 lliName);
	bool					HasOperator(CCPPClass* pcClass, int64 lliName, size uiArguments);
	CCPPSymbol*				QualifiedEnumerator(CCPPSyntaxName* pcName, CCPPSyntax* pcNode, CCPPClass** ppcClass);
	CCPPSymbol*				QualifiedMember(CCPPSyntaxName* pcName, ECPPSymbolKind eKind, CCPPSyntax* pcNode, CCPPClass** ppcClass);
	CCPPClass*				QualifiersClass(CCPPSyntaxName* pcName, CCPPSyntax* pcNode);
	CCPPClass*				QualifiedClass(CCPPSyntaxName* pcName, CCPPSyntax* pcNode);
	CCPPClass*				MemberClass(CCPPClass* pcClass, CCPPSyntaxNameComponent* pcComponent, CCPPSyntax* pcNode);
	void					ReportFailedDeclaration(int64 lliName);
	SCPP65Type*				TypeOf(CCPPSyntax* pcExpression);
	SCPP65Type*				DeclaredTypeOf(CCPPSyntax* pcExpression);
	SCPP65Type*				UnevaluatedTypeOf(CCPPSyntax* pcExpression);
	bool					NamesFunction(CCPPSyntax* pcExpression);
	SCPP65Type*				ValueTypeOf(CCPPSyntax* pcExpression);
	bool					CheckInitialiser(CCPPSyntax* pcValue, SCPP65Type* psTo);
	bool					CheckBracedValue(CCPPSyntax* pcValue, SCPP65Type* psTo);

	SCPP65Type*				PlaceTypeOf(CCPPSyntax* pcExpression);

	bool					Supported(SCPP65Type* psType, CCPPSyntax* pcNode);


	bool					Constant(CCPPSyntax* pcExpression, int64* plliValue);
	bool					ConstantError(CCPPSyntax* pcNode, const char* szMessage);
	void					ConstantMessage(const char* szMessage, CChars* pszDest);
	bool					FoldConstant(CCPPSyntax* pcExpression, int64* plliValue);
	bool					RunConstant(CCPPSyntax* pcExpression, int64* plliValue);
	bool					NamesConstexpr(CCPPSyntax* pcNode);
	int64					ConstantOfType(SCPP65Type* psType, int64 lliValue);
	bool					ConstexprCall(CCPPSyntaxCallExpression* pcCall, int64* plliValue);
	SCPP65Type*				ConstantTypeOf(CCPPSyntax* pcExpression);
	bool					BinaryConstant(CCPPSyntaxBinaryExpression* pcBinary, int64 lliLeft, int64 lliRight, int64* plliValue);
	void					ConstantOverflow(CCPPSyntax* pcExpression, int64 lliValue, bool bUnsignedWraps);
	bool					IsSignedOverflow(ECppTokens eOperator, int iBits, int64 lliLeft, int64 lliRight, int64 lliResult);
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
	bool					TypeTraitValue(int iTrait, CArrayCPP65Types* papsTypes);
	bool					IsTriviallyCopyableType(SCPP65Type* psType);
	bool					IsStandardLayoutType(SCPP65Type* psType);
	bool					EnumeratorValue(CCPPSymbol* pcSymbol, int64* plliValue);
	SCPP65Type*				EnumType(CCPPSymbol* pcEnum, CCPPSyntaxEnumSpecifier* pcSpecifier);
	SCPP65Type*				EnumeratorType(CCPPSymbol* pcSymbol);
	CCPPSyntax*				SkipParentheses(CCPPSyntax* pcExpression);
	bool					IsNullPointerLiteral(CCPPSyntax* pcExpression);
	bool					IsNullPointerConstant(CCPPSyntax* pcExpression);
	SCPP65Type*				QualifiedFieldType(CCPPSyntax* pcExpression);
	bool					DecodeString(CCPPSyntax* pcString, CArrayUint8* pauiBytes);
	bool					DecodeStringToken(CCPPSyntax* pcString, CCTString* pcToken, int iElementBytes, CArrayUint8* pauiBytes);
	void					DecodeRawStringToken(CCTString* pcToken, int iElementBytes, CArrayUint8* pauiBytes);
	int						StringElementBytes(CCPPSyntax* pcString);
	SCPP65Type*				StringElementType(CCPPSyntax* pcString);
	SCPP65Type*				StringArrayType(CCPPSyntax* pcString);
	uint32					StringElement(CArrayUint8* pauiBytes, int iElementBytes, size uiElement);

	//Objects and copies.  CPPUnitObjects.cpp
	bool					IsObject(CCPPSyntax* pcExpression);
	bool					IsOverloadedComma(CCPPSyntaxBinaryExpression* pcBinary);
	CCPPClass*				ConditionalClass(SCPP65Type* psTrue, SCPP65Type* psFalse);
	CCPPClass*				MemberObjectClass(CCPPSyntaxMemberExpression* pcMember);
	bool					BindMemberOf(CCPPSyntaxMemberExpression* pcMember, CCPPBoundName* pcBound);
	CCPPSymbol*				MemberEnumerator(CCPPSyntaxMemberExpression* pcMember);
	SCPPField*				BitFieldOf(CCPPSyntax* pcExpression);
	SCPP65Type*				OperandType(SCPP65Type* psType, CCPPSyntax* pcExpression);
	SCPP65Type*				VariadicArgumentType(SCPP65Type* psType);
	SCPP65Type*				CalleePointerType(CCPPSyntax* pcCallee);
	SCPP65Type*				ConditionalType(SCPP65Type* psTrue, SCPP65Type* psFalse);
	SCPP65Type*				CompositePointerType(SCPP65Type* psLeft, SCPP65Type* psRight);
	SCPP65Type*				CompositePointee(SCPP65Type* psLeft, SCPP65Type* psRight, bool bFirstLevel);
	bool					IsStdName(int64 lliName, CCPPScope* pcScope, const char* szName);
	bool					IsNamed(int64 lliName, const char* szName);
	CCPPClass*				TypeInfoClass(void);
	bool					IsTypeInfoMember(CCPPFunction* pcFunction);
	SCPP65Type*				TypeidType(CCPPSyntaxTypeidExpression* pcTypeid, bool* pbDynamic);
	void					AppendTypeidLabel(CChars* psz, SCPP65Type* psType);
	CCPPTemplate*			InitializerListTemplate(void);
	SCPP65Type*				InitializerListElement(SCPP65Type* psType);
	SCPP65Type*				InitializerListOf(SCPP65Type* psElement, CCPPSyntax* pcNode);
	SCPP65Type*				BracedListType(CCPPSyntax* pcList, CCPPSyntax* pcNode);
	bool					InitializerListLayout(CCPPClass* pcClass, bool* pbEnd);
	SCPP65Type*				CastClassType(CCPPSyntax* pcExpression);
	CCPPClass*			ObjectClass(CCPPSyntax* pcExpression);
	bool					IsMadeObject(CCPPSyntax* pcExpression, CCPPClass* pcClass);
	bool					IsRvalue(CCPPSyntax* pcExpression);
	bool					IsReturnedLocal(CCPPSyntaxIdExpression* pcId);
	bool					IsObjectLvalue(CCPPSyntax* pcExpression);
	bool					IsUncapturedConstant(CCPPSyntax* pcExpression);
	SCPP65Type*				PlaceOf(CCPPSyntax* pcExpression);
	bool					BindsToTemporary(SCPP65Type* psReference, CCPPSyntax* pcArgument);
	bool					MakesObject(CCPPSyntax* pcExpression);

	//Check only.  CPPUnitCheck.cpp
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
	void					Typed(CCPPSyntax* pcExpression, SCPP65Type* psType);
	void					PlaceTyped(CCPPSyntax* pcExpression, SCPP65Type* psType);
	void					Changed(CCPPSyntax* pcExpression, uint8 uiChanges);
	CCPPFunction*			FindDeclaredFunction(CCPPSyntaxDeclarator* pcDeclarator);

	//Floating point.  CPPUnitFloat.cpp
	int64					FloatBits(SCPP65Type* psType, double dValue);
	bool					FloatConstant(CCPPSyntax* pcExpression, double* pdValue);
	bool					FloatBuiltin(CCPPSyntax* pcExpression, double* pdValue, SCPP65Type** ppsType);
	bool					TokenFloat(CCPPToken* pcToken, double* pdValue, SCPP65Type** ppsType);
	bool					FloatValue(CCPPSyntax* pcExpression, double* pdValue, SCPP65Type** ppsType);
};


#endif // __C_P_P_UNIT_H__
