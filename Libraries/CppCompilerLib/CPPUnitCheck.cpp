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
#include "SourceFile.h"
#include "CPPSyntaxTree.h"
#include "CPPSemanticPass.h"
#include "CTIdentifier.h"
#include "CTReservedWord.h"
#include "CPPTokenMarks.h"
#include "CPPUnit.h"
#include "CPPConstants.h"


//////////////////////////////////////////////////////////////////////////
//
// Works out the types of the unit's expressions without generating any
// code, for pcPass, which is told what names resolved to.  Every class the
// unit defines is completed and every function it defines is checked
// (inline ones, member functions defined in their classes and template
// instances too), each statement on its own: one that can't be typed is
// an error and the others are still checked.  Returns false if anything
// couldn't be (GetErrors).
//
// A declaration is only wrong to the compiler when what it declares is
// used, so a type nothing declares in one that nothing uses ("int
// Use(CLater* pc);" in a class, "CNever* Make(void);") was no error: a
// check says so at the end (ReportUndeclaredTypes), as a C++ compiler
// would where the name is written.
//
// A template nothing instantiates is checked in its own words too, its
// parameters standing for themselves, by a kept check and by a pass that
// asks for it (SetTemplatesAsThemselves: the IDE's index, which then has
// what such a template's body names).
//
// With bRemember the pass is also told the type of each expression and
// what changes the place of one (Typed, PlaceTyped and Changed), for a
// check that is kept (CCPPSemanticPass::Check).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::Check(CCPPSemanticPass* pcPass, bool bRemember)
{
	CCPPSyntaxTranslationUnit*	pcRoot;

	mpcSemanticPass = pcPass;
	mbRemember = bRemember;
	pcRoot = mpcTree->GetRoot();
	if (pcRoot)
	{
		Declarations(&pcRoot->macpDeclarations);
		CheckClasses(&pcRoot->macpDeclarations);
	}
	do
	{
		if (mbRemember || pcPass->IsTemplatesAsThemselves())
		{
			CheckTemplatesAsThemselves();
		}
	}
	while (CheckDefinitions());
	mcDiagnostics.ReportDeferred();
	ReportUndeclaredTypes();
	ReportNamedInstances();
	CheckContext();
	mpcSemanticPass = NULL;
	mbRemember = false;
	return mcDiagnostics.GetErrors()->NumErrors() == 0;
}


//Finds the types a unit's declarations name that nothing declares (CCPPUnit::ReportUndeclaredTypes).
class CCPPUndeclaredTypeFinder : public CCPPSyntaxChildVisitor
{
public:
	CCPPUnit*	mpcUnit;
	int			miTemplates;	//How many template declarations the node is in.

	void	Visit(CCPPSyntax* pcChild) override
	{
		bool	bTemplate;

		bTemplate = (pcChild->GetKind() == CPPS_TemplateDeclaration);
		miTemplates += bTemplate ? 1 : 0;
		if ((pcChild->GetKind() == CPPS_NamedTypeSpecifier) && (miTemplates == 0))
		{
			mpcUnit->UndeclaredType((CCPPSyntaxNamedTypeSpecifier*)pcChild);
		}
		pcChild->VisitFields(this);
		miTemplates -= bTemplate ? 1 : 0;
	}
};


//////////////////////////////////////////////////////////////////////////
//
// An error for each type a declaration names that nothing declares,
// whether or not what it declares is used.  Not in a template (a name
// there may be one only an instance has), and not in the compiler's own
// headers, which are skimmed.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::ReportUndeclaredTypes(void)
{
	CCPPUndeclaredTypeFinder	cFinder;

	if (mpcTree->GetRoot())
	{
		cFinder.mpcUnit = this;
		cFinder.miTemplates = 0;
		mpcTree->GetRoot()->VisitFields(&cFinder);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A type named by one plain name that the parser found nothing for: an
// error at the name, unless there is one there already (what it declares
// was used).  A name with a scope before it is left: its scope may be one
// the parser can't look in.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::UndeclaredType(CCPPSyntaxNamedTypeSpecifier* pcNamed)
{
	SPreprocessedToken*	psSource;
	CCPPToken*			pcToken;
	size				i;
	int					iToken;

	if (!pcNamed->mpcName || pcNamed->mpcName->mpcSymbol || !pcNamed->mpcName->IsSimple())
	{
		return;
	}
	iToken = pcNamed->mpcName->miFirstToken;
	pcToken = (iToken >= 0) ? mpcTokeniser->Get((size)iToken) : NULL;
	psSource = pcToken ? mpcTokeniser->GetSource(pcToken) : NULL;
	if (!psSource || (psSource->pcFile && psSource->pcFile->IsSystemFile()))
	{
		return;
	}
	for (i = 0; i < mcDiagnostics.GetErrors()->NumErrors(); i++)
	{
		if (mcDiagnostics.GetErrors()->GetToken(i) == iToken)
		{
			return;
		}
	}
	mcDiagnostics.ErrorAt(iToken, "This type is not declared");
}


//////////////////////////////////////////////////////////////////////////
//
// Checks the functions waiting to be generated, then every function with
// a definition not checked yet.  Checking can declare more (a template's
// instances, a class's members), so it is done until it returns false.
// A member of a class template's instance that nothing calls isn't
// instantiated in C++, so what is wrong in it isn't an error: it is
// checked quietly, only for its names.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckDefinitions(void)
{
	CCPPFunction*	pcFunction;
	size			i;
	bool			bChecked;

	bChecked = GenerateQueued();
	for (i = 0; i < mapcFunctions.NumElements(); i++)
	{
		pcFunction = mapcFunctions.GetPtr(i);
		if (pcFunction->mpcDefinition && !pcFunction->mbGenerated && !pcFunction->mbImplicit)
		{
			if (pcFunction->mpcBindings)
			{
				mcDiagnostics.BeginQuiet();
				GenerateFunction(pcFunction);
				mcDiagnostics.EndQuiet();
			}
			else
			{
				GenerateFunction(pcFunction);
			}
			bChecked = true;
		}
	}
	return bChecked;
}


//////////////////////////////////////////////////////////////////////////
//
// Completes every class the declarations define (in namespaces and
// extern "C" too), so their member functions are declared and those
// defined in the class are checked, whether or not the unit uses them.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::CheckClasses(CArraySyntax* papcDeclarations)
{
	size							i;
	CCPPSyntax*						pcDeclaration;
	CCPPSyntaxSimpleDeclaration*	pcSimple;
	CCPPSyntaxClassSpecifier*		pcClassSpecifier;
	CCPPSymbol*						pcSymbol;
	CCPPClass*					pcClass;

	for (i = 0; i < papcDeclarations->NumElements(); i++)
	{
		pcDeclaration = papcDeclarations->GetPtr(i);
		switch (pcDeclaration->GetKind())
		{
		case CPPS_SimpleDeclaration:
			pcSimple = (CCPPSyntaxSimpleDeclaration*)pcDeclaration;
			if (!pcSimple->mpcSpecifiers || !pcSimple->mpcSpecifiers->mpcType || (pcSimple->mpcSpecifiers->mpcType->GetKind() != CPPS_ClassSpecifier))
			{
				break;
			}
			pcClassSpecifier = (CCPPSyntaxClassSpecifier*)pcSimple->mpcSpecifiers->mpcType;
			pcSymbol = pcClassSpecifier->mpcSymbol;
			if (pcSymbol && (pcSymbol->meKind == CPPSK_Class))
			{
				pcClass = GetClass(pcSymbol, pcClassSpecifier);
				CompleteClass(pcClass, pcClassSpecifier);
			}
			break;
		case CPPS_LinkageSpecification:
			CheckClasses(&((CCPPSyntaxLinkageSpecification*)pcDeclaration)->macpDeclarations);
			break;
		case CPPS_Namespace:
			CheckClasses(&((CCPPSyntaxNamespace*)pcDeclaration)->macpDeclarations);
			break;
		default:
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A function's parameters and locals are given their types, then each
// of its member initialisers and statements is checked.  A local that
// isn't supported is an error; the rest of the function is still checked.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::CheckFunction(CCPPFunction* pcFunction)
{
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	size							i;

	pcDefinition = pcFunction->mpcDefinition;
	if (!pcDefinition || pcFunction->mbImplicit)
	{
		return;
	}

	EnterFunction(pcFunction);
	AddParameters(pcFunction);
	CheckAbstractSignature(pcFunction, pcDefinition->mpcDeclarator);
	CheckMemberInitialisers(pcFunction);
	AllocateLocals(pcDefinition->mpcBody);
	for (i = 0; i < pcDefinition->macpMemberInitializers.NumElements(); i++)
	{
		CheckChildren(pcDefinition->macpMemberInitializers.GetPtr(i));
	}
	CheckNode(pcDefinition->mpcBody);
	LeaveFunction();
}


//////////////////////////////////////////////////////////////////////////
//
// The functions defined in the unit's own source file (not a header) that
// nothing called, so weren't written: an inline member function, say.
// Their bodies are checked as written functions' are, as what is wrong in
// a definition is wrong whether or not it is called.  A template's member
// isn't: it is checked as each instance is made.  [intro.compliance] 2
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::CheckUncalledFunctions(void)
{
	CCPPFunction*	pcFunction;
	CCPPClass*		pcClass;
	size			i;

	if (mpcSemanticPass)
	{
		return;
	}

	//The classes defined there are laid out (their members declared and checked) though nothing uses them.
	for (i = 0; i < mapcClasses.NumElements(); i++)
	{
		pcClass = mapcClasses.GetPtr(i);
		if (pcClass->IsComplete() || pcClass->IsBeingMade() || !pcClass->mpcSpecifier || pcClass->mpcTemplate || pcClass->mpcBindings)
		{
			continue;
		}
		if (IsInSourceFile(pcClass->mpcSpecifier))
		{
			CompleteClass(pcClass, pcClass->mpcSpecifier);
		}
	}
	GenerateQueued();

	for (i = 0; i < mapcFunctions.NumElements(); i++)
	{
		pcFunction = mapcFunctions.GetPtr(i);
		if (!pcFunction->mpcDefinition || pcFunction->mbGenerated || pcFunction->mbImplicit || pcFunction->mbDeleted || pcFunction->mpcTemplate || pcFunction->mpcBindings ||
			(pcFunction->mpcDefinition->meBody == CPPFB_Default) || (pcFunction->mpcDefinition->meBody == CPPFB_Delete) || !pcFunction->mpcDefinition->mpcBody)
		{
			continue;
		}
		if (pcFunction->mpcClass && (pcFunction->mpcClass->mpcTemplate || pcFunction->mpcClass->mpcBindings || pcFunction->mpcClass->mpsLambda))
		{
			//(A closure's functions are its lambda's: written where the lambda is.)
			continue;
		}
		if (!IsInSourceFile(pcFunction->mpcDefinition))
		{
			continue;
		}
		pcFunction->mbGenerated = true;
		mcContext.EnterBindings(pcFunction->mpcBindings);
		CheckFunction(pcFunction);
		mcContext.LeaveBindings();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcNode is written in a source file (the unit's own, as a rule)
// and not in a header.  What a source file declares is checked there
// whether or not it is used; what a header declares is checked in the
// units that use it, as every unit that includes it has it.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsInSourceFile(CCPPSyntax* pcNode)
{
	CCPPToken*			pcToken;
	SPreprocessedToken*	psSource;

	pcToken = (pcNode->miFirstToken >= 0) ? mpcTokeniser->Get((size)pcNode->miFirstToken) : NULL;
	psSource = pcToken ? mpcTokeniser->GetSource(pcToken) : NULL;
	return psSource && psSource->pcFile && IsSourceFileName(psSource->pcFile->FullName());
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a file's name is a source file's, not a header's.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsSourceFileName(const char* szName)
{
	const char*	szDot;

	szDot = strrchr(szName, '.');
	return szDot && ((strcmp(szDot, ".cpp") == 0) || (strcmp(szDot, ".c") == 0) || (strcmp(szDot, ".cc") == 0) || (strcmp(szDot, ".cxx") == 0));
}


//////////////////////////////////////////////////////////////////////////
//
// The expressions in a statement (or in anything else in a function):
// each is typed, and each local is initialised.  Classes and enums
// declared in a function aren't looked into.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::CheckNode(CCPPSyntax* pcNode)
{
	CCPPSyntax*	pcValue;
	SCPP65Type*	psValue;

	if (!pcNode)
	{
		return;
	}
	if (IsCheckedExpression(pcNode))
	{
		CheckExpression(pcNode);
		return;
	}
	switch (pcNode->GetKind())
	{
	case CPPS_InitDeclarator:
		if (pcNode->mpcParent && (pcNode->mpcParent->GetKind() == CPPS_SimpleDeclaration))
		{
			CheckLocal((CCPPSyntaxInitDeclarator*)pcNode);
			return;
		}
		break;
	case CPPS_SwitchStatement:
		CheckChildren(pcNode);
		psValue = SwitchType((CCPPSyntaxSwitchStatement*)pcNode);
		CheckSwitch((CCPPSyntaxSwitchStatement*)pcNode, psValue);
		CheckSwitchValues((CCPPSyntaxSwitchStatement*)pcNode, psValue);
		return;
	case CPPS_BreakStatement:
	case CPPS_ContinueStatement:
		CheckJump(pcNode, pcNode->GetKind() == CPPS_ContinueStatement);
		return;
	case CPPS_GotoStatement:
		CheckGoto((CCPPSyntaxGotoStatement*)pcNode);
		return;
	case CPPS_ReturnStatement:
		//An overloaded function's name returned as a pointer to (or a reference to) a function is the overload of
		//that type, chosen by the function's return type.  [over.over] 1
		pcValue = ((CCPPSyntaxReturnStatement*)pcNode)->mpcValue;
		psValue = (pcValue && mcContext.GetFunction() && mcContext.GetFunction()->mpsReturn) ? mcContext.GetFunction()->mpsReturn : NULL;
		psValue = (psValue && psValue->IsReference() && psValue->psBase->IsFunction()) ? mcTypes.Pointer(psValue->psBase) : psValue;
		if (psValue && FunctionNameFor(pcValue, psValue))
		{
			mcTypeChecker.ValueFor(pcValue, psValue, false);
			return;
		}

		//What is returned is the function's type's: an enum class's value isn't another's without a cast.
		CheckChildren(pcNode);
		if (mcContext.GetFunction())
		{
			CheckReturn(mcContext.GetFunction(), (CCPPSyntaxReturnStatement*)pcNode);
		}
		pcValue = ((CCPPSyntaxReturnStatement*)pcNode)->mpcValue;
		psValue = (pcValue && mcContext.GetFunction() && mcContext.GetFunction()->mpsReturn) ? TypeOf(pcValue) : NULL;
		if (psValue && !psValue->IsClass() && !mcContext.GetFunction()->mpsReturn->IsClass() && !mcContext.GetFunction()->mpsReturn->IsVoid())
		{
			mcTypeChecker.KeepsEnum(psValue, mcContext.GetFunction()->mpsReturn, pcValue);
		}
		return;
	case CPPS_IfStatement:
	case CPPS_WhileStatement:
	case CPPS_DoStatement:
	case CPPS_ForStatement:
		//What a statement tests is a condition: an enum class's value isn't one.
		CheckChildren(pcNode);
		switch (pcNode->GetKind())
		{
		case CPPS_IfStatement:		pcValue = ((CCPPSyntaxIfStatement*)pcNode)->mpcCondition;		break;
		case CPPS_WhileStatement:	pcValue = ((CCPPSyntaxWhileStatement*)pcNode)->mpcCondition;	break;
		case CPPS_DoStatement:		pcValue = ((CCPPSyntaxDoStatement*)pcNode)->mpcCondition;		break;
		default:					pcValue = ((CCPPSyntaxForStatement*)pcNode)->mpcCondition;		break;
		}
		psValue = (pcValue && IsCheckedExpression(pcValue)) ? TypeOf(pcValue) : NULL;
		if (psValue && psValue->IsScopedEnum())
		{
			mcTypeChecker.Truth(psValue, pcValue);
		}
		psValue = ConditionVariableType(pcValue);
		if (psValue && psValue->IsArray())
		{
			//[stmt.select] 2
			mcDiagnostics.ReportError(pcValue, "A condition's variable isn't an array");
		}
		return;
	case CPPS_EnumSpecifier:
		CheckEnumerators(pcNode);
		return;
	case CPPS_StaticAssert:
		StaticAssert((CCPPSyntaxStaticAssert*)pcNode);
		return;
	case CPPS_CaseStatement:
		//A case or default label is in a switch's statement.  [stmt.label] 2
		CheckCaseInSwitch(pcNode);
		break;
	case CPPS_LambdaExpression:
		//A lambda alone (return's value, say): typed, then its body checked in its own scope (CheckLambda), not walked
		//as the function's statements.
		if (mcTypeChecker.Value(pcNode))
		{
			CheckLambdas(pcNode);
		}
		return;
	case CPPS_DeclarationStatement:
		//A namespace or a function defined in a block (a lambda's definition is no declaration statement).
		if (!CheckBlockDeclaration(((CCPPSyntaxDeclarationStatement*)pcNode)->mpcDeclaration))
		{
			return;
		}
		CheckLocalClass(((CCPPSyntaxDeclarationStatement*)pcNode)->mpcDeclaration);
		break;
	case CPPS_ClassSpecifier:
	case CPPS_LazyDeclaration:
	case CPPS_SkippedStatement:
	case CPPS_SkippedDeclaration:
		return;
	default:
		break;
	}
	CheckChildren(pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// The type a switch's cases are compared as, for CheckSwitch where the IR
// front end doesn't work it out: the condition's, promoted.  NULL where it
// isn't an integer's (what is wrong with it is said where it is used).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::SwitchType(CCPPSyntaxSwitchStatement* pcSwitch)
{
	SCPP65Type*	psType;

	if (!pcSwitch->mpcCondition)
	{
		return NULL;
	}
	mcDiagnostics.BeginQuiet();
	psType = (pcSwitch->mpcCondition->GetKind() == CPPS_SimpleDeclaration) ? ConditionVariableType(pcSwitch->mpcCondition) : TypeOf(pcSwitch->mpcCondition);
	mcDiagnostics.EndQuiet();
	psType = psType ? psType->WithoutReference() : NULL;
	if (psType && psType->IsClass())
	{
		//An object is an integer by its class's one conversion function to one that isn't explicit.  [stmt.switch] 2, [conv] 5
		psType = mcBinder.ContextualType(pcSwitch->mpcCondition, CPPXT_Integer);
	}
	return (psType && psType->IsInteger()) ? mcTypes.Promote(psType) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// What the semantic pass checks of a switch that the IR front end does
// where it writes one: the condition isn't a float or a pointer, and a
// case's value isn't narrowed as the type the condition is compared as
// (psType).  [stmt.switch] 2, [expr.const] 3
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::CheckSwitchValues(CCPPSyntaxSwitchStatement* pcSwitch, SCPP65Type* psType)
{
	CArraySyntax				apcCases;
	CCPPSyntaxCaseStatement*	pcCase;
	SCPP65Type*					psCondition;
	int64						lliValue;
	size						i;
	bool						bConstant;

	if (!pcSwitch->mpcCondition)
	{
		return;
	}
	mcDiagnostics.BeginQuiet();
	psCondition = (pcSwitch->mpcCondition->GetKind() == CPPS_SimpleDeclaration) ? ConditionVariableType(pcSwitch->mpcCondition) : TypeOf(pcSwitch->mpcCondition);
	mcDiagnostics.EndQuiet();
	psCondition = psCondition ? psCondition->WithoutReference() : NULL;
	if (psCondition && (psCondition->IsFloat() || psCondition->IsPointer()))
	{
		mcDiagnostics.ReportError(pcSwitch->mpcCondition, "A switch's condition is an integer or an enum");
		return;
	}
	if (!psType)
	{
		return;
	}
	apcCases.Init();
	SwitchCases(pcSwitch->mpcBody, &apcCases);
	for (i = 0; i < apcCases.NumElements(); i++)
	{
		pcCase = (CCPPSyntaxCaseStatement*)apcCases.GetPtr(i);
		mcDiagnostics.BeginQuiet();
		bConstant = pcCase->mpcValue && Constant(pcCase->mpcValue, &lliValue);
		mcDiagnostics.EndQuiet();
		if (bConstant)
		{
			mcTypeChecker.ConstantNotNarrowed(pcCase->mpcValue, psType);
		}
	}
	apcCases.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A switch's labels: one default; case values that differ as the type the
// condition is compared as (psType: NULL to not compare them); and none
// that jumps past an initialised local into its scope.  An error if not.
// [stmt.switch] 2, [stmt.dcl] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckSwitch(CCPPSyntaxSwitchStatement* pcSwitch, SCPP65Type* psType)
{
	CArraySyntax	apcCases;
	CArrayInt64		alliValues;
	CCPPSyntaxCaseStatement*	pcCase;
	size			i;
	size			uiDefaults;
	int64			lliValue;
	bool			bInScope;
	bool			bResult;

	apcCases.Init();
	alliValues.Init();
	SwitchCases(pcSwitch->mpcBody, &apcCases);
	bResult = true;
	uiDefaults = 0;
	for (i = 0; i < apcCases.NumElements(); i++)
	{
		pcCase = (CCPPSyntaxCaseStatement*)apcCases.GetPtr(i);
		if (!pcCase->mpcValue)
		{
			uiDefaults++;
			if (uiDefaults == 2)
			{
				bResult = mcDiagnostics.ReportOnce(pcCase, "A switch has one default label");
			}
			continue;
		}
		mpcConstants->ClearReason();
		mcDiagnostics.BeginQuiet();
		if (!Constant(pcCase->mpcValue, &lliValue))
		{
			mcDiagnostics.EndQuiet();
			//A case's value is a constant.  [stmt.switch] 2
			mcDiagnostics.BeginLoud();
			bResult = ConstantError(pcCase->mpcValue, "A case's value is a constant");
			mcDiagnostics.EndLoud();
			continue;
		}
		mcDiagnostics.EndQuiet();
		if (!psType)
		{
			continue;
		}
		lliValue = ConstantOfType(psType, lliValue);
		if (alliValues.Contains(&lliValue))
		{
			bResult = mcDiagnostics.ReportOnce(pcCase, "Two cases of the switch have one value");
		}
		alliValues.Add(lliValue);
	}
	alliValues.Kill();
	apcCases.Kill();

	bInScope = false;
	return SwitchJumps(pcSwitch->mpcBody, &bInScope) && bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// break or continue: an error if no loop (or, for break, switch) holds it
// in the function, lambda or class it is in.  [stmt.break] 1, [stmt.cont] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckJump(CCPPSyntax* pcJump, bool bContinue)
{
	CCPPSyntax*	pcNode;

	for (pcNode = pcJump->mpcParent; pcNode; pcNode = pcNode->mpcParent)
	{
		switch (pcNode->GetKind())
		{
		case CPPS_WhileStatement:
		case CPPS_DoStatement:
		case CPPS_ForStatement:
		case CPPS_RangeForStatement:
			return true;
		case CPPS_SwitchStatement:
			if (!bContinue)
			{
				return true;
			}
			break;
		case CPPS_LambdaExpression:
		case CPPS_ClassSpecifier:
		case CPPS_FunctionDefinition:
			return mcDiagnostics.ReportError(pcJump, bContinue ? "continue outside a loop" : "break outside a loop or switch");
		default:
			break;
		}
	}
	return mcDiagnostics.ReportError(pcJump, bContinue ? "continue outside a loop" : "break outside a loop or switch");
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a declaration statement's local, pcInit, is one a jump may not
// go past into its scope: initialised, or of a class (or an array of one)
// made or destroyed by code.  [stmt.dcl] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsJumpGuardedLocal(CCPPSyntaxSimpleDeclaration* pcDeclaration, CCPPSyntaxInitDeclarator* pcInit)
{
	CCPPVariable*	pcVariable;
	SCPP65Type*		psType;

	if (pcDeclaration->mpcSpecifiers && (pcDeclaration->mpcSpecifiers->muiFlags & (CPPDS_STATIC | CPPDS_EXTERN | CPPDS_TYPEDEF | CPPDS_THREAD_LOCAL)))
	{
		return false;
	}
	if (pcInit->mpcDeclarator->IsFunction())
	{
		return false;
	}
	pcVariable = pcInit->mpcSymbol ? GetVariable(pcInit->mpcSymbol) : NULL;
	psType = pcVariable ? pcVariable->mpsType : NULL;
	while (psType && psType->IsArray())
	{
		psType = psType->psBase;
	}
	return pcInit->mpcInitializer || (psType && psType->IsClass() && (psType->pcClass->NeedsConstructor() || psType->pcClass->mpcDestructor));
}


//////////////////////////////////////////////////////////////////////////
//
// The locals (IsJumpGuardedLocal) in scope at pcNode: those declared before
// it in each block that holds it, up to pcRoot.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::LocalsInScope(CCPPSyntax* pcNode, CCPPSyntax* pcRoot, CArraySyntax* papcLocals)
{
	CCPPSyntax*						pcChild;
	CCPPSyntax*						pcParent;
	CCPPSyntax*						pcStatement;
	CCPPSyntaxSimpleDeclaration*	pcDeclaration;
	CCPPSyntaxInitDeclarator*		pcInit;
	CArraySyntax*					papcStatements;
	size							i;
	size							j;

	pcChild = pcNode;
	for (pcParent = pcNode->mpcParent; pcParent && (pcChild != pcRoot); pcParent = pcParent->mpcParent)
	{
		if (pcParent->GetKind() == CPPS_CompoundStatement)
		{
			papcStatements = &((CCPPSyntaxCompoundStatement*)pcParent)->macpStatements;
			for (i = 0; (i < papcStatements->NumElements()) && (papcStatements->GetPtr(i) != pcChild); i++)
			{
				pcStatement = papcStatements->GetPtr(i);
				pcDeclaration = (pcStatement->GetKind() == CPPS_DeclarationStatement) ? (CCPPSyntaxSimpleDeclaration*)((CCPPSyntaxDeclarationStatement*)pcStatement)->mpcDeclaration : NULL;
				if (!pcDeclaration || (pcDeclaration->GetKind() != CPPS_SimpleDeclaration))
				{
					continue;
				}
				for (j = 0; j < pcDeclaration->macpDeclarators.NumElements(); j++)
				{
					pcInit = (CCPPSyntaxInitDeclarator*)pcDeclaration->macpDeclarators.GetPtr(j);
					if (IsJumpGuardedLocal(pcDeclaration, pcInit))
					{
						papcLocals->Add(pcInit);
					}
				}
			}
		}
		pcChild = pcParent;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The label statement named as iNameToken's identifier in pcNode (not in a
// lambda or class in it), or NULL.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPUnit::FindLabel(CCPPSyntax* pcNode, int iNameToken)
{
	CCPPSyntax*	pcFound;
	CCPPToken*	pcName;
	CCPPToken*	pcLabel;
	size		i;

	if (!pcNode || (pcNode->GetKind() == CPPS_LambdaExpression) || (pcNode->GetKind() == CPPS_ClassSpecifier))
	{
		return NULL;
	}
	if (pcNode->GetKind() == CPPS_LabelStatement)
	{
		pcName = mpcTokeniser->Get(iNameToken);
		pcLabel = mpcTokeniser->Get(((CCPPSyntaxLabelStatement*)pcNode)->miName);
		if (pcName && pcLabel && pcName->IsIdentifier() && pcLabel->IsIdentifier() && (((CCTIdentifier*)pcName)->miIdentifier == ((CCTIdentifier*)pcLabel)->miIdentifier))
		{
			return pcNode;
		}
	}
	for (i = 0; i < pcNode->NumChildren(); i++)
	{
		pcFound = FindLabel(pcNode->GetChild(i), iNameToken);
		if (pcFound)
		{
			return pcFound;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A goto: an error if it jumps into the scope of a local it goes past the
// making of (one in scope at its label that isn't at the goto).
// [stmt.dcl] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckGoto(CCPPSyntaxGotoStatement* pcGoto)
{
	CCPPSyntax*		pcRoot;
	CCPPSyntax*		pcLabel;
	CArraySyntax	apcAtGoto;
	CArraySyntax	apcAtLabel;
	size			i;
	bool			bResult;

	for (pcRoot = pcGoto->mpcParent; pcRoot && pcRoot->mpcParent; pcRoot = pcRoot->mpcParent)
	{
		if ((pcRoot->mpcParent->GetKind() == CPPS_FunctionDefinition) || (pcRoot->mpcParent->GetKind() == CPPS_LambdaExpression))
		{
			break;
		}
	}
	pcLabel = pcRoot ? FindLabel(pcRoot, pcGoto->miLabel) : NULL;
	if (!pcLabel)
	{
		return true;
	}
	apcAtGoto.Init();
	apcAtLabel.Init();
	LocalsInScope(pcGoto, pcRoot, &apcAtGoto);
	LocalsInScope(pcLabel, pcRoot, &apcAtLabel);
	bResult = true;
	for (i = 0; i < apcAtLabel.NumElements(); i++)
	{
		if (apcAtGoto.Find(apcAtLabel.GetPtr(i)) == ARRAY_ELEMENT_NOT_FOUND)
		{
			bResult = mcDiagnostics.ReportError(pcGoto, "The goto jumps into the scope of an object it hasn't made: past its declaration, or into its block");
			break;
		}
	}
	apcAtLabel.Kill();
	apcAtGoto.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of the variable a condition declares (if (T x = v)), or NULL if
// it declares none.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::ConditionVariableType(CCPPSyntax* pcCondition)
{
	CCPPSyntaxSimpleDeclaration*	pcDeclaration;
	CCPPSyntaxInitDeclarator*		pcInit;
	CCPPVariable*					pcVariable;

	if (!pcCondition || (pcCondition->GetKind() != CPPS_SimpleDeclaration))
	{
		return NULL;
	}
	pcDeclaration = (CCPPSyntaxSimpleDeclaration*)pcCondition;
	if (pcDeclaration->macpDeclarators.NumElements() != 1)
	{
		return NULL;
	}
	pcInit = (CCPPSyntaxInitDeclarator*)pcDeclaration->macpDeclarators.GetPtr(0);
	pcVariable = pcInit->mpcSymbol ? GetVariable(pcInit->mpcSymbol) : NULL;
	return pcVariable ? pcVariable->mpsType : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A case or default label: an error if no switch's statement holds it (in
// the function, lambda or class it is in).  [stmt.label] 2
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckCaseInSwitch(CCPPSyntax* pcCase)
{
	CCPPSyntax*	pcNode;

	for (pcNode = pcCase->mpcParent; pcNode; pcNode = pcNode->mpcParent)
	{
		switch (pcNode->GetKind())
		{
		case CPPS_SwitchStatement:
			return true;
		case CPPS_LambdaExpression:
		case CPPS_ClassSpecifier:
		case CPPS_FunctionDefinition:
			pcNode = NULL;
			break;
		default:
			continue;
		}
		break;
	}
	return mcDiagnostics.ReportError(pcCase, ((CCPPSyntaxCaseStatement*)pcCase)->mpcValue ? "A case label is only in a switch" : "A default label is only in a switch");
}


//////////////////////////////////////////////////////////////////////////
//
// A class defined in a block: what final and override say of it is
// checked where it is defined (CheckVirtSpecifiers), as a class's at
// namespace scope is.  [class.virtual] 4, 5, [class] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckLocalClass(CCPPSyntax* pcDeclaration)
{
	CCPPSyntaxDeclSpecifiers*	pcSpecifiers;
	bool								bResult;

	pcSpecifiers = (pcDeclaration && (pcDeclaration->GetKind() == CPPS_SimpleDeclaration)) ? ((CCPPSyntaxSimpleDeclaration*)pcDeclaration)->mpcSpecifiers : NULL;
	if (!pcSpecifiers || !pcSpecifiers->mpcType || (pcSpecifiers->mpcType->GetKind() != CPPS_ClassSpecifier))
	{
		return true;
	}
	mcDiagnostics.BeginLoud();
	bResult = CheckVirtSpecifiers((CCPPSyntaxClassSpecifier*)pcSpecifiers->mpcType);
	mcDiagnostics.EndLoud();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A declaration in a block: an error for one that is only at namespace
// scope (a namespace's, a function's definition).  True for any other.
// [basic.namespace] 1, [dcl.fct.def.general] 2, [class.local] 2
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckBlockDeclaration(CCPPSyntax* pcDeclaration)
{
	if (!pcDeclaration)
	{
		return true;
	}
	switch (pcDeclaration->GetKind())
	{
	case CPPS_Namespace:
		return mcDiagnostics.ReportError(pcDeclaration, "A namespace is declared outside any function");
	case CPPS_FunctionDefinition:
		return mcDiagnostics.ReportError(pcDeclaration, "A function is defined outside any function: a local class's member functions are defined in the class");
	default:
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Why a string literal doesn't initialise an array of psElement: the
// array's elements are its kind of character.  [dcl.init.string] 1
//
//////////////////////////////////////////////////////////////////////////
const char* CCPPUnit::StringArrayError(CCPPSyntax* pcString, SCPP65Type* psElement)
{
	if (!psElement->IsInteger() || (psElement->eKind == CPP65TK_Bool))
	{
		return "A string initialises an array of characters only";
	}
	return (StringElementBytes(pcString) == 1) ? "A string without a prefix (or u8) initialises an array of char only" : "A wide string initialises an array of its own kind of character only: wchar_t for L, char16_t for u, char32_t for U";
}


//////////////////////////////////////////////////////////////////////////
//
// The case and default labels of a switch, in order (not those of
// switches inside it).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::SwitchCases(CCPPSyntax* pcNode, CArraySyntax* papcCases)
{
	size	i;

	if (!pcNode || (pcNode->GetKind() == CPPS_SwitchStatement) || (pcNode->GetKind() == CPPS_LambdaExpression) || (pcNode->GetKind() == CPPS_ClassSpecifier))
	{
		return;
	}
	if (pcNode->GetKind() == CPPS_CaseStatement)
	{
		papcCases->Add(pcNode);
	}
	for (i = 0; i < pcNode->NumChildren(); i++)
	{
		SwitchCases(pcNode->GetChild(i), papcCases);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether no case label of a switch (in pcNode and what follows) jumps into
// the scope of a local that is initialised, or whose class makes or
// destroys it by code: *pbInScope says one has been passed in the scope the
// walk is in.  A scope's declarations stop at its end; a label's statement
// is in the scope the label is in.  [stmt.dcl] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::SwitchJumps(CCPPSyntax* pcNode, bool* pbInScope)
{
	CCPPSyntaxSimpleDeclaration*	pcDeclaration;
	CCPPSyntaxInitDeclarator*		pcInit;
	CCPPVariable*					pcVariable;
	SCPP65Type*						psType;
	size							i;
	bool							bOuter;
	bool							bResult;

	if (!pcNode || (pcNode->GetKind() == CPPS_SwitchStatement) || (pcNode->GetKind() == CPPS_LambdaExpression) || (pcNode->GetKind() == CPPS_ClassSpecifier))
	{
		return true;
	}
	switch (pcNode->GetKind())
	{
	case CPPS_CaseStatement:
		if (*pbInScope)
		{
			return mcDiagnostics.ReportOnce(pcNode, "The case label jumps past an initialised local into its scope: put the local in a block of its own");
		}
		return SwitchJumps(((CCPPSyntaxCaseStatement*)pcNode)->mpcStatement, pbInScope);

	case CPPS_LabelStatement:
		return SwitchJumps(((CCPPSyntaxLabelStatement*)pcNode)->mpcStatement, pbInScope);

	case CPPS_DeclarationStatement:
		pcDeclaration = (CCPPSyntaxSimpleDeclaration*)((CCPPSyntaxDeclarationStatement*)pcNode)->mpcDeclaration;
		if (!pcDeclaration || (pcDeclaration->GetKind() != CPPS_SimpleDeclaration) || (pcDeclaration->mpcSpecifiers && (pcDeclaration->mpcSpecifiers->muiFlags & (CPPDS_STATIC | CPPDS_EXTERN | CPPDS_TYPEDEF | CPPDS_THREAD_LOCAL))))
		{
			return true;
		}
		for (i = 0; i < pcDeclaration->macpDeclarators.NumElements(); i++)
		{
			pcInit = (CCPPSyntaxInitDeclarator*)pcDeclaration->macpDeclarators.GetPtr(i);
			if (pcInit->mpcDeclarator->IsFunction())
			{
				continue;
			}
			pcVariable = pcInit->mpcSymbol ? GetVariable(pcInit->mpcSymbol) : NULL;
			psType = pcVariable ? pcVariable->mpsType : NULL;
			while (psType && psType->IsArray())
			{
				psType = psType->psBase;
			}
			if (pcInit->mpcInitializer || (psType && psType->IsClass() && (psType->pcClass->NeedsConstructor() || psType->pcClass->mpcDestructor)))
			{
				*pbInScope = true;
			}
		}
		return true;

	default:
		bOuter = *pbInScope;
		bResult = true;
		for (i = 0; i < pcNode->NumChildren(); i++)
		{
			bResult &= SwitchJumps(pcNode->GetChild(i), pbInScope);
		}
		*pbInScope = bOuter;
		return bResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What a return statement gives its function: no value (but a void one)
// to a void function, and to an object of a class one its constructor
// that isn't explicit makes (return copy initialises).  An error if not.
// [stmt.return] 2
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckReturn(CCPPFunction* pcFunction, CCPPSyntaxReturnStatement* pcReturn)
{
	SCPP65Type*				psReturn;
	SCPP65Type*				psValue;
	CArraySyntax			apcValue;
	CArraySyntax*			papcArguments;
	CCPPBoundConstruction	cBound;
	bool					bResult;

	psReturn = pcFunction->mpsReturn;
	if (!pcReturn->mpcValue && psReturn && (psReturn != &mcTypes.msUndeduced) && (pcFunction->meKind == CPPFK_Function) && !psReturn->IsVoid())
	{
		return mcDiagnostics.ReportOnce(pcReturn, "The function returns a value: return; is for a void function");
	}
	if (!pcReturn->mpcValue || !psReturn || (psReturn == &mcTypes.msUndeduced) || (pcFunction->meKind != CPPFK_Function))
	{
		return true;
	}
	if (psReturn->IsVoid())
	{
		if (pcReturn->mpcValue->GetKind() == CPPS_InitializerList)
		{
			return mcDiagnostics.ReportOnce(pcReturn, "A void function returns no value");
		}
		mcDiagnostics.BeginQuiet();
		psValue = TypeOf(pcReturn->mpcValue);
		mcDiagnostics.EndQuiet();
		return !psValue || psValue->IsVoid() || mcDiagnostics.ReportOnce(pcReturn, "A void function returns no value");
	}
	if (!psReturn->IsClass() || !CompleteClass(psReturn->pcClass, pcReturn))
	{
		return true;
	}
	apcValue.Init();
	apcValue.Add(pcReturn->mpcValue);
	papcArguments = &apcValue;
	if (pcReturn->mpcValue->GetKind() == CPPS_InitializerList)
	{
		papcArguments = ListElements(pcReturn->mpcValue);
	}
	else
	{
		mcDiagnostics.BeginQuiet();
		psValue = TypeOf(pcReturn->mpcValue);
		mcDiagnostics.EndQuiet();
		psValue = psValue ? psValue->WithoutReference() : NULL;
		if (!psValue || (psValue->IsClass() && IsDerived(psValue->pcClass, psReturn->pcClass, NULL)))
		{
			apcValue.Kill();
			return true;
		}
	}
	bResult = true;
	mcDiagnostics.BeginQuiet();
	if ((papcArguments->NumElements() > 0) && psReturn->pcClass->mbConstructors && mcBinder.BindConstruction(psReturn->pcClass, papcArguments, pcReturn, &cBound, false) && (cBound.meHow == CPPBO_Constructor))
	{
		mcDiagnostics.EndQuiet();
		if (cBound.mpcConstructor->mbExplicit)
		{
			bResult = mcDiagnostics.ReportOnce(pcReturn, "An explicit constructor isn't used by return: write the class's name before the value");
		}
	}
	else
	{
		mcDiagnostics.EndQuiet();
	}
	apcValue.Kill();
	return bResult;
}


//Checks each of a node's children (CCPPUnit::CheckChildren).
class CCPP65CheckChildren : public CCPPSyntaxChildVisitor
{
public:
	CCPPUnit*	mpcUnit;

	void	Visit(CCPPSyntax* pcChild) override	{ mpcUnit->CheckNode(pcChild); }
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::CheckChildren(CCPPSyntax* pcNode)
{
	CCPP65CheckChildren	cChildren;

	cChildren.mpcUnit = this;
	pcNode->VisitFields(&cChildren);
}


//////////////////////////////////////////////////////////////////////////
//
// An expression is typed as a whole.  If it can't be, it is an error and
// its parts are typed on their own (quietly), so the names in them are
// still resolved: in f(a.Get(), x) with no f that fits, a.Get() still is.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::CheckExpression(CCPPSyntax* pcExpression)
{
	if (mcTypeChecker.Value(pcExpression))
	{
		CheckLambdas(pcExpression);
		return;
	}
	mcDiagnostics.BeginQuiet();
	CheckChildren(pcExpression);
	mcDiagnostics.EndQuiet();
}


//Checks the lambdas in each of a node's children (CCPPUnit::CheckLambdas).
class CCPP65CheckLambdas : public CCPPSyntaxChildVisitor
{
public:
	CCPPUnit*	mpcUnit;

	void	Visit(CCPPSyntax* pcChild) override	{ mpcUnit->CheckLambdas(pcChild); }
};


//////////////////////////////////////////////////////////////////////////
//
// The bodies of the lambdas in an expression that has been typed: typing
// one makes its closure, and says nothing of what its body does.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::CheckLambdas(CCPPSyntax* pcNode)
{
	CCPP65CheckLambdas	cChildren;

	if (!pcNode)
	{
		return;
	}
	if (pcNode->GetKind() == CPPS_LambdaExpression)
	{
		CheckLambda((CCPPSyntaxLambdaExpression*)pcNode);
		return;
	}
	cChildren.mpcUnit = this;
	pcNode->VisitFields(&cChildren);
}


//////////////////////////////////////////////////////////////////////////
//
// A lambda's body is checked as its closure's operator() (a lambda in it
// in its turn), in the function the lambda is in, whether or not anything
// calls it.  A generic lambda's operator() is one for each call's
// arguments, checked when it is called.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::CheckLambda(CCPPSyntaxLambdaExpression* pcLambda)
{
	SCPPLambda*			psLambda;
	CCPPFunction*		pcCall;

	mcDiagnostics.BeginQuiet();
	psLambda = GetLambda(pcLambda);
	mcDiagnostics.EndQuiet();
	pcCall = psLambda ? psLambda->pcCall : NULL;
	if (!pcCall || pcCall->mbGenerated)
	{
		return;
	}
	pcCall->mbGenerated = true;
	EnterFunction(pcCall);
	AddParameters(pcCall, pcCall->mpcDeclarator);
	AllocateLocals(pcLambda->mpcDefinition->mpcBody);
	CheckNode(pcLambda->mpcDefinition->mpcBody);
	LeaveFunction();
}


//////////////////////////////////////////////////////////////////////////
//
// A local (or static local) variable's initialiser, as the IR front end
// initialises it.  If it can't be, its expressions are typed on their own.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::CheckLocal(CCPPSyntaxInitDeclarator* pcInit)
{
	CCPPVariable*					pcVariable;
	CCPPSyntaxSimpleDeclaration*	pcDeclaration;

	if (pcInit->mpcDeclarator->IsFunction())
	{
		DeclareBlockFunction(((CCPPSyntaxSimpleDeclaration*)pcInit->mpcParent)->mpcSpecifiers, pcInit->mpcDeclarator);
		return;
	}
	CheckPointerExceptions(pcInit);
	pcVariable = pcInit->mpcSymbol ? GetVariable(pcInit->mpcSymbol) : NULL;
	//alignas on it is a power of two (AlignasAlignment says so).  [dcl.align] 2
	pcDeclaration = (CCPPSyntaxSimpleDeclaration*)pcInit->mpcParent;
	if (pcDeclaration && (pcDeclaration->GetKind() == CPPS_SimpleDeclaration) && pcDeclaration->mpcSpecifiers)
	{
		AlignasAlignment(&pcDeclaration->mpcSpecifiers->macpSpecifiers);
	}
	AlignasAlignment(&pcInit->mpcDeclarator->macpAttributes);
	if (pcVariable && CheckDeclared(pcVariable->mpsType, pcInit, pcInit->mpcInitializer))
	{
		return;
	}
	if (pcInit->mpcInitializer)
	{
		mcDiagnostics.BeginQuiet();
		CheckNode(pcInit->mpcInitializer);
		mcDiagnostics.EndQuiet();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The expressions the type checker types (not initialiser lists, which
// are only typed for what they initialise).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsCheckedExpression(CCPPSyntax* pcNode)
{
	ECPPSyntax	eKind;

	eKind = pcNode->GetKind();
	if ((eKind == CPPS_InitializerList) || (eKind == CPPS_ParenthesisedInitializer))
	{
		return false;
	}
	return (eKind >= CPPS_LiteralExpression) && (eKind <= CPPS_NoexceptExpression);
}


//////////////////////////////////////////////////////////////////////////
//
// The type checker found which function pcName calls (or names): the
// semantic pass is told, when checking.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::Resolved(CCPPSyntaxName* pcName, CCPPFunction* pcFunction)
{
	CCPPSyntaxName*	pcDeclared;
	CCPPSymbol*		pcSymbol;

	if (!mpcSemanticPass || !pcName || !pcName->GetLast() || !pcFunction)
	{
		return;
	}
	pcSymbol = NULL;
	if (pcFunction->mpcDeclarator)
	{
		pcDeclared = pcFunction->mpcDeclarator->GetName();
		pcSymbol = pcDeclared ? pcDeclared->mpcSymbol : NULL;
	}
	if (!pcSymbol)
	{
		pcSymbol = pcName->mpcSymbol;
	}
	mpcSemanticPass->Resolved(pcName->GetLast()->miName, pcSymbol, pcFunction->mpcDeclarator);
}


//////////////////////////////////////////////////////////////////////////
//
// The type checker found the member (a field or a static member) pcName
// names.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::Resolved(CCPPSyntaxName* pcName, CCPPSymbol* pcSymbol)
{
	if (!mpcSemanticPass || !pcName || !pcName->GetLast())
	{
		return;
	}
	mpcSemanticPass->Resolved(pcName->GetLast()->miName, pcSymbol, NULL);
}



//////////////////////////////////////////////////////////////////////////
//
// A kept check has every template the unit declares as itself too
// (Refactorings.txt in the IDE, T3): a class template's instance with its
// parameters standing for themselves is completed, so its member
// functions are declared, those defined outside it with them; and a
// function template's, and a member template's of a class that isn't an
// instance (or is one as itself), is declared.  CheckDefinitions then
// checks their bodies, quietly: what is in a body that only a template
// argument would have (pv->Init(), a > b) can't be typed, which is no
// error, and the rest is typed in the template's own words, M* and
// CArray<M>&.  Completing a class can declare more templates (its member
// templates), so it is done before each round of definitions.
//
// An explicit specialisation has no parameters and is what it is; a
// partial specialisation is left (its instance as itself would need the
// pattern's arguments).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::CheckTemplatesAsThemselves(void)
{
	CCPPTemplate*	pcTemplate;
	CCPPClass*		pcClass;
	size			i;

	for (i = 0; i < mapcTemplates.NumElements(); i++)
	{
		pcTemplate = mapcTemplates.GetPtr(i);
		if (pcTemplate->mbAsItself)
		{
			continue;
		}
		pcTemplate->mbAsItself = true;
		if (!pcTemplate->mpcDeclaration || (pcTemplate->mpcDeclaration->macpParameters.NumElements() == 0) || (pcTemplate->mpcPrimary != pcTemplate) || pcTemplate->IsDeclaredClass())
		{
			continue;
		}

		if (pcTemplate->mpcDefinition && (pcTemplate->mpcDefinition->GetKind() == CPPS_ClassSpecifier))
		{
			mcDiagnostics.BeginQuiet();
			pcClass = InstantiateClassAsItself(pcTemplate);
			if (pcClass)
			{
				CompleteClass(pcClass, pcTemplate->mpcDefinition);
			}
			mcDiagnostics.EndQuiet();
		}
		else if (!pcTemplate->mbMember || (pcTemplate->mbMemberTemplate && pcTemplate->mpcClass && (!pcTemplate->mpcClass->mpcBindings || pcTemplate->mpcClass->mpcBindings->IsSelf())))
		{
			//A function template, or a member template in a class that isn't another instance.  A class template's
			//members defined outside it are declared with its instance (LayoutClass).
			mcDiagnostics.BeginQuiet();
			InstantiateFunctionAsItself(pcTemplate);
			mcDiagnostics.EndQuiet();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether what is being checked is written as it is checked: it is not
// in a template, or the template's parameters stand for themselves.  In
// an instance with arguments an expression's type is the instance's
// (CFoo*, where the template says M*), which isn't the type of what is
// written.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsCheckedAsWritten(void) const
{
	return !mcContext.GetBindings() || mcContext.GetBindings()->IsSelf();
}


//////////////////////////////////////////////////////////////////////////
//
// The type checker typed pcExpression: a kept check's pass is told, if
// it is checked as written.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::Typed(CCPPSyntax* pcExpression, SCPP65Type* psType)
{
	if (mbRemember && IsCheckedAsWritten())
	{
		mpcSemanticPass->Typed(pcExpression, psType);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The type checker found the type of the place pcExpression is.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::PlaceTyped(CCPPSyntax* pcExpression, SCPP65Type* psType)
{
	if (mbRemember && IsCheckedAsWritten())
	{
		mpcSemanticPass->PlaceTyped(pcExpression, psType);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The type checker found the place pcExpression is can be changed there
// (CPPPC_): bound to a reference that isn't const, its address taken, or
// a member function that isn't const called on it.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::Changed(CCPPSyntax* pcExpression, uint8 uiChanges)
{
	if (mbRemember)
	{
		mpcSemanticPass->Changed(pcExpression, uiChanges);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The function pcDeclarator declares or defines, NULL if the unit has
// none: not an instance of a template, which the template's declarator
// is shared by.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPUnit::FindDeclaredFunction(CCPPSyntaxDeclarator* pcDeclarator)
{
	CCPPFunction*	pcFunction;
	size			i;

	for (i = 0; pcDeclarator && (i < mapcFunctions.NumElements()); i++)
	{
		pcFunction = mapcFunctions.GetPtr(i);
		if (pcFunction->mpcBindings || pcFunction->mpcTemplate)
		{
			continue;
		}
		if ((pcFunction->mpcDeclarator == pcDeclarator) || (pcFunction->mpcDefinition && (pcFunction->mpcDefinition->mpcDeclarator == pcDeclarator)))
		{
			return pcFunction;
		}
	}
	return NULL;
}


//Finds the classes that are names for a template's instance, and then each place such an instance is written out
//(CCPPUnit::ReportNamedInstances).  Each starts with a marked token, and a node that has none of them among its
//tokens is not entered: nothing under it can be one.
class CCPPNamedInstanceFinder : public CCPPSyntaxChildVisitor
{
public:
	CCPPUnit*				mpcUnit;
	CArrayCPPNameClasses*	mpapcClasses;
	CCPPTokenMarks*			mpcMarks;		//The tokens that start what is being found.
	bool					mbClasses;		//Finding the classes, not the instances.
	int						miTemplates;	//How many template declarations the node is in,
	int						miInitializers;	//and how many constructors' initialisers.
	uint64					muiVisited;		//The nodes looked at.

	void	Visit(CCPPSyntax* pcChild) override
	{
		bool	bTemplate;
		bool	bInitializer;

		if (!mpcMarks->Any(pcChild->miFirstToken, pcChild->miLastToken))
		{
			return;
		}
		muiVisited++;
		bTemplate = (pcChild->GetKind() == CPPS_TemplateDeclaration);
		bInitializer = (pcChild->GetKind() == CPPS_MemberInitializer);
		miTemplates += bTemplate ? 1 : 0;
		miInitializers += bInitializer ? 1 : 0;
		if (mbClasses && (pcChild->GetKind() == CPPS_ClassSpecifier) && (miTemplates == 0) && mpcUnit->IsNameClass((CCPPSyntaxClassSpecifier*)pcChild))
		{
			mpapcClasses->Add((CCPPSyntaxClassSpecifier*)pcChild);
		}
		if (!mbClasses && (pcChild->GetKind() == CPPS_NameComponent) && (miTemplates == 0) && (miInitializers == 0))
		{
			mpcUnit->NamedInstance((CCPPSyntaxNameComponent*)pcChild, mpapcClasses);
		}
		pcChild->VisitFields(this);
		miInitializers -= bInitializer ? 1 : 0;
		miTemplates -= bTemplate ? 1 : 0;
	}
};


//////////////////////////////////////////////////////////////////////////
//
// A class can be the name of a template's instance: "class CTank :
// public CTankTemplate<CGauge> {};", where a typedef would be another
// name for the same type, is a type of its own that can be declared
// ahead of its definition.  Nothing says a class is one; it is one if a
// template's instance is its only base and it has nothing of its own but
// constructors (which hand what they are given to the instance's).
//
// Where such a class is declared, writing the instance out
// ("CTankTemplate<CGauge>* pcTank") names another type than the one the
// rest of the code uses: a warning says to use the name.  Not in the
// class's own list of bases or its constructors' initialisers, which
// have to name the instance; not in a template; not above the class; and
// not for an instance written another way than the class's base is (a
// typedef of its argument, say), which isn't looked for.  A class in a
// header whose declarations nothing has named yet isn't parsed, and so
// isn't found.
//
// The tree is not walked whole, which cost a tenth of a compile: a class
// starts with its class key and an instance with the identifier the
// class's base starts with, those tokens are marked, and only the nodes
// that have one are entered (CCPPTokenMarks).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::ReportNamedInstances(void)
{
	CCPPNamedInstanceFinder	cFinder;
	CArrayCPPNameClasses	apcClasses;
	CCPPTokenMarks			cMarks;

	if (!mpcTree->GetRoot())
	{
		return;
	}
	apcClasses.Init();
	cFinder.mpcUnit = this;
	cFinder.mpapcClasses = &apcClasses;
	cFinder.mpcMarks = &cMarks;
	cFinder.miTemplates = 0;
	cFinder.miInitializers = 0;
	cFinder.muiVisited = 0;
	cFinder.mbClasses = true;
	MarkClassKeys(&cMarks);
	mpcTree->GetRoot()->VisitFields(&cFinder);
	cMarks.Kill();
	if (apcClasses.NumElements() > 0)
	{
		cFinder.mbClasses = false;
		MarkNameClassBases(&cMarks, &apcClasses);
		mpcTree->GetRoot()->VisitFields(&cFinder);
		cMarks.Kill();
	}
	mpcStatistics->Count(CPPCC_NamedInstanceNodes, cFinder.muiVisited);
	apcClasses.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Marks the unit's class keys (class, struct, union): a class's definition
// has one among its tokens.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::MarkClassKeys(CCPPTokenMarks* pcMarks)
{
	CCPPToken*			pcToken;
	CCTReservedWord*	pcWord;
	size				uiTokens;
	size				i;

	uiTokens = mpcTokeniser->NumTokens();
	pcMarks->Init(uiTokens);
	for (i = 0; i < uiTokens; i++)
	{
		pcToken = mpcTokeniser->Get(i);
		pcWord = (pcToken && pcToken->IsReservedWord()) ? (CCTReservedWord*)pcToken : NULL;
		pcMarks->Next(pcWord && ((pcWord->GetToken() == CT_class) || (pcWord->GetToken() == CT_struct) || (pcWord->GetToken() == CT_union)));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Marks the tokens that an instance one of the classes is the name of can
// start with: the identifier the class's base starts with (CTankTemplate,
// or the namespace it is qualified by), as NamedInstance takes an instance
// to be written as the base is from its first token.  Every token is
// marked if a base starts with anything but an identifier.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::MarkNameClassBases(CCPPTokenMarks* pcMarks, CArrayCPPNameClasses* papcClasses)
{
	CArrayInt32		aiStarts;
	CCPPSyntaxName*	pcBase;
	CCPPToken*		pcToken;
	int32*			paiStarts;
	int32			iIdentifier;
	size			uiTokens;
	size			uiLow;
	size			uiHigh;
	size			uiMiddle;
	size			i;
	bool			bAll;

	uiTokens = mpcTokeniser->NumTokens();
	aiStarts.Init();
	bAll = false;
	for (i = 0; i < papcClasses->NumElements(); i++)
	{
		pcBase = ((CCPPSyntaxBaseSpecifier*)papcClasses->GetPtr(i)->macpBases.GetPtr(0))->mpcName;
		pcToken = ((pcBase->miFirstToken >= 0) && ((size)pcBase->miFirstToken < uiTokens)) ? mpcTokeniser->Get((size)pcBase->miFirstToken) : NULL;
		if (pcToken && pcToken->IsIdentifier())
		{
			aiStarts.AddIfUnique(((CCTIdentifier*)pcToken)->miIdentifier);
		}
		else
		{
			bAll = true;
		}
	}
	aiStarts.QuickSort();
	paiStarts = aiStarts.GetData();

	pcMarks->Init(uiTokens);
	for (i = 0; i < uiTokens; i++)
	{
		pcToken = mpcTokeniser->Get(i);
		uiLow = 0;
		uiHigh = (!bAll && pcToken && pcToken->IsIdentifier()) ? aiStarts.NumElements() : 0;
		iIdentifier = (uiHigh > 0) ? ((CCTIdentifier*)pcToken)->miIdentifier : 0;
		while (uiLow < uiHigh)
		{
			uiMiddle = (uiLow + uiHigh) / 2;
			if (paiStarts[uiMiddle] < iIdentifier)
			{
				uiLow = uiMiddle + 1;
			}
			else
			{
				uiHigh = uiMiddle;
			}
		}
		pcMarks->Next(bAll || ((uiLow < aiStarts.NumElements()) && pcToken && pcToken->IsIdentifier() && (paiStarts[uiLow] == ((CCTIdentifier*)pcToken)->miIdentifier)));
	}
	aiStarts.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a class is the name of a template's instance: it has a name,
// one base that is written with template arguments, and no members but
// constructors (declared or defined), with the access they are given.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsNameClass(CCPPSyntaxClassSpecifier* pcClass)
{
	CCPPSyntaxBaseSpecifier*	pcBase;
	CCPPSyntax*					pcMember;
	CCPPSyntaxDeclarator*		pcDeclarator;
	CCPPSyntaxNameComponent*	pcName;
	CChars						szClass;
	CChars						szMember;
	size						i;
	bool						bConstructor;

	if (!pcClass->mpcName || !pcClass->mpcName->GetLast() || (pcClass->macpBases.NumElements() != 1))
	{
		return false;
	}
	pcBase = (CCPPSyntaxBaseSpecifier*)pcClass->macpBases.GetPtr(0);
	if ((pcBase->GetKind() != CPPS_BaseSpecifier) || !pcBase->mpcName || !pcBase->mpcName->GetLast() || !pcBase->mpcName->GetLast()->mbTemplateArguments)
	{
		return false;
	}
	szClass.Init();
	AppendTokensText(pcClass->mpcName->GetLast()->miName, pcClass->mpcName->GetLast()->miName, &szClass);
	bConstructor = true;
	for (i = 0; bConstructor && (i < pcClass->macpMembers.NumElements()); i++)
	{
		pcMember = pcClass->macpMembers.GetPtr(i);
		pcDeclarator = NULL;
		if (pcMember->GetKind() == CPPS_AccessSpecifier)
		{
			continue;
		}
		if (pcMember->GetKind() == CPPS_FunctionDefinition)
		{
			pcDeclarator = ((CCPPSyntaxFunctionDefinition*)pcMember)->mpcDeclarator;
		}
		else if ((pcMember->GetKind() == CPPS_SimpleDeclaration) && (((CCPPSyntaxSimpleDeclaration*)pcMember)->macpDeclarators.NumElements() == 1) &&
				 (((CCPPSyntaxSimpleDeclaration*)pcMember)->macpDeclarators.GetPtr(0)->GetKind() == CPPS_InitDeclarator))
		{
			pcDeclarator = ((CCPPSyntaxInitDeclarator*)((CCPPSyntaxSimpleDeclaration*)pcMember)->macpDeclarators.GetPtr(0))->mpcDeclarator;
		}
		pcName = (pcDeclarator && pcDeclarator->mpcName) ? pcDeclarator->mpcName->GetLast() : NULL;
		bConstructor = pcName && (pcName->meKind == CPPNC_Identifier) && (pcDeclarator->macpPrefix.NumElements() == 0) && (pcDeclarator->macpSuffixes.NumElements() > 0);
		if (bConstructor)
		{
			szMember.Init();
			AppendTokensText(pcName->miName, pcName->miName, &szMember);
			bConstructor = szMember.Equals(szClass.Text());
			szMember.Kill();
		}
	}
	szClass.Kill();
	return bConstructor;
}


//////////////////////////////////////////////////////////////////////////
//
// A name's component written with template arguments: a warning if it is
// a template's instance that a class declared above it is the name of.
// It is the instance if it is written as the class's base is, from the
// start of its name to the end of its arguments.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::NamedInstance(CCPPSyntaxNameComponent* pcComponent, CArrayCPPNameClasses* papcClasses)
{
	CCPPSyntaxClassSpecifier*	pcClass;
	CCPPSyntaxName*				pcBase;
	CCPPSyntaxName*				pcName;
	SPreprocessedToken*			psSource;
	CChars						szInstance;
	CChars						szBase;
	CChars						szMessage;
	size						i;

	if (!pcComponent->mbTemplateArguments || !pcComponent->mpcParent || (pcComponent->mpcParent->GetKind() != CPPS_Name) || (pcComponent->miFirstToken < 0))
	{
		return;
	}
	pcName = (CCPPSyntaxName*)pcComponent->mpcParent;
	psSource = mpcTokeniser->GetSource(mpcTokeniser->Get((size)pcName->miFirstToken));
	if (!psSource || (psSource->pcFile && psSource->pcFile->IsSystemFile()))
	{
		return;
	}
	szInstance.Init();
	AppendTokensText(pcName->miFirstToken, pcComponent->miLastToken, &szInstance);
	for (i = 0; i < papcClasses->NumElements(); i++)
	{
		pcClass = papcClasses->GetPtr(i);
		pcBase = ((CCPPSyntaxBaseSpecifier*)pcClass->macpBases.GetPtr(0))->mpcName;
		if ((pcBase == pcName) || (pcClass->miLastToken > pcName->miFirstToken))
		{
			continue;
		}
		szBase.Init();
		AppendTokensText(pcBase->miFirstToken, pcBase->miLastToken, &szBase);
		if (szBase.Equals(szInstance.Text()))
		{
			szMessage.Init(szInstance);
			szMessage.Append(" has the name ");
			AppendTokensText(pcClass->mpcName->GetLast()->miName, pcClass->mpcName->GetLast()->miName, &szMessage);
			szMessage.Append(": use the name, which is a type of its own");
			mcDiagnostics.Warning(pcName, szMessage.Text());
			szMessage.Kill();
			szBase.Kill();
			break;
		}
		szBase.Kill();
	}
	szInstance.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A run of tokens as they are written, with nothing between them:
// "CTankTemplate<CGauge>" however it was spaced.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::AppendTokensText(int iFirstToken, int iLastToken, CChars* pszDest)
{
	SPreprocessedToken*	psSource;
	int					i;

	for (i = iFirstToken; (i >= 0) && (i <= iLastToken) && ((size)i < mpcTokeniser->NumTokens()); i++)
	{
		psSource = mpcTokeniser->GetSource(mpcTokeniser->Get((size)i));
		if (psSource && psSource->szText)
		{
			pszDest->Append(psSource->szText, psSource->uiLength);
		}
	}
}
