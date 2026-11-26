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
#ifndef __C_P_P_CHECK_H__
#define __C_P_P_CHECK_H__
#include "BaseLib/Chars.h"
#include "BaseLib/PrimitiveTypes.h"
#include "CPPSyntaxes.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "CPPTokenMarks.h"


class CCPPUnit;
class CCPPSemanticPass;
class CCPPFunction;
class CCPPSymbol;
struct SCPP65Type;


//The class specifiers that name classes (CCPPCheck::ReportNamedInstances).
typedef CArrayTemplatePtr<CCPPSyntaxClassSpecifier>	CArrayCPPNameClasses;


//The unit checked as written for the IDE (CCPPUnit::GetCheck), which the semantic pass drives (Check): every function
//and class of the source files worked out without code being made, what each expression's type is and what changes
//it told to the pass (Typed, PlaceTyped, Changed, Resolved), the jumps, switches and returns checked as the compiler
//checks them, templates checked as themselves, and the names of the classes the source names.
class CCPPCheck
{
protected:
	CCPPUnit*				mpcUnit;

public:
	void					Init(CCPPUnit* pcUnit);
	void					Kill(void);

	bool					Check(CCPPSemanticPass* pcPass, bool bRemember = false);
	void					ReportUndeclaredTypes(void);
	void					UndeclaredType(CCPPSyntaxNamedTypeSpecifier* pcNamed);
	bool					CheckDefinitions(void);
	void					CheckClasses(CArraySyntax* papcDeclarations);
	void					CheckFunction(CCPPFunction* pcFunction);
	void					CheckUncalledFunctions(void);
	bool					IsInSourceFile(CCPPSyntax* pcNode);
	bool					IsSourceFileName(const char* szName);
	void					CheckNode(CCPPSyntax* pcNode);
	SCPP65Type*				SwitchType(CCPPSyntaxSwitchStatement* pcSwitch);
	void					CheckSwitchValues(CCPPSyntaxSwitchStatement* pcSwitch, SCPP65Type* psType);
	bool					CheckSwitch(CCPPSyntaxSwitchStatement* pcSwitch, SCPP65Type* psType);
	bool					CheckJump(CCPPSyntax* pcJump, bool bContinue);
	bool					IsJumpGuardedLocal(CCPPSyntaxSimpleDeclaration* pcDeclaration, CCPPSyntaxInitDeclarator* pcInit);
	void					LocalsInScope(CCPPSyntax* pcNode, CCPPSyntax* pcRoot, CArraySyntax* papcLocals);
	CCPPSyntax*				FindLabel(CCPPSyntax* pcNode, int iNameToken);
	bool					CheckGoto(CCPPSyntaxGotoStatement* pcGoto);
	SCPP65Type*				ConditionVariableType(CCPPSyntax* pcCondition);
	bool					CheckCaseInSwitch(CCPPSyntax* pcCase);
	bool					CheckLocalClass(CCPPSyntax* pcDeclaration);
	bool					CheckBlockDeclaration(CCPPSyntax* pcDeclaration);
	const char*				StringArrayError(CCPPSyntax* pcString, SCPP65Type* psElement);
	void					SwitchCases(CCPPSyntax* pcNode, CArraySyntax* papcCases);
	bool					SwitchJumps(CCPPSyntax* pcNode, bool* pbInScope);
	bool					CheckReturn(CCPPFunction* pcFunction, CCPPSyntaxReturnStatement* pcReturn);
	void					CheckChildren(CCPPSyntax* pcNode);
	void					CheckExpression(CCPPSyntax* pcExpression);
	void					CheckLambdas(CCPPSyntax* pcNode);
	void					CheckLambda(CCPPSyntaxLambdaExpression* pcLambda);
	void					CheckLocal(CCPPSyntaxInitDeclarator* pcInit);
	bool					IsCheckedExpression(CCPPSyntax* pcNode);
	void					Resolved(CCPPSyntaxName* pcName, CCPPSymbol* pcSymbol);
	void					Resolved(CCPPSyntaxName* pcName, CCPPFunction* pcFunction);
	void					CheckTemplatesAsThemselves(void);
	bool					IsCheckedAsWritten(void) const;
	void					Typed(CCPPSyntax* pcExpression, SCPP65Type* psType);
	void					PlaceTyped(CCPPSyntax* pcExpression, SCPP65Type* psType);
	void					Changed(CCPPSyntax* pcExpression, uint8 uiChanges);
	CCPPFunction*			FindDeclaredFunction(CCPPSyntaxDeclarator* pcDeclarator);
	void					ReportNamedInstances(void);
	void					MarkClassKeys(CCPPTokenMarks* pcMarks);
	void					MarkNameClassBases(CCPPTokenMarks* pcMarks, CArrayCPPNameClasses* papcClasses);
	bool					IsNameClass(CCPPSyntaxClassSpecifier* pcClass);
	void					NamedInstance(CCPPSyntaxNameComponent* pcComponent, CArrayCPPNameClasses* papcClasses);
	void					AppendTokensText(int iFirstToken, int iLastToken, CChars* pszDest);
};


#endif // __C_P_P_CHECK_H__

