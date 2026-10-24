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
#ifndef __C_P_P_PARSER_H__
#define __C_P_P_PARSER_H__
#include "BaseLib/ArrayInt.h"
#include "CppTokeniser.h"
#include "CTReservedWord.h"
#include "CPPSyntaxes.h"


//Where the parser is.  bHalf is set when the first '>' of a '>>' closed a template argument list.  [temp.names] 3
struct SCPPParserPosition
{
	int		iToken;
	bool	bHalf;
};


//A member function body, parsed when its class is complete.  [class.mem] 6
struct SCPPDeferredBody
{
	CCPPSyntaxFunctionDefinition*	pcFunction;
	CCPPScope*						pcScope;
	int								iStart;
	int								iEnd;
};


typedef CArrayTemplate<SCPPDeferredBody>	CArrayDeferredBodies;


enum ECPPDeclarationContext
{
	CPPDC_Namespace,
	CPPDC_Class,
	CPPDC_Block,
	CPPDC_Parameter,
	CPPDC_Condition,
	CPPDC_ForInit,
	CPPDC_TypeId,
};


enum ECPPDeclaratorKind
{
	CPPDK_Named,		//A declarator-id is required.
	CPPDK_Abstract,		//No declarator-id, as in a type-id.
	CPPDK_Either,		//Parameters may or may not have names.
};


//C++14 recursive descent parser.  Names are looked up as they are parsed so types can be told from values.  Member
//function bodies are parsed when their class is complete.  Declarations in system headers are only skimmed: their
//function bodies are skipped and parse errors in them are ignored.
class CCPPParser
{
protected:
	CCPPTokeniser*			mpcTokeniser;
	CCPPSyntaxTree*			mpcTree;
	CCPPSyntaxMemory*		mpcMemory;
	CCPPSymbolTable*		mpcSymbols;
	CCPPScope*				mpcScope;
	int						miNumTokens;

	int						miToken;
	bool					mbHalf;
	CCTReservedWord			mcGreaterThan;			//The second '>' of a '>>'.

	int						miTentative;			//Parsing to see if something is a declaration; nothing is recorded.
	bool					mbFailed;				//The tentative parse failed.
	bool					mbNoGreater;			//In a template argument list, where '>' ends the argument.
	bool					mbMemberName;			//The name after . or ->, which is a member of a type the parser doesn't know.
	int						miClassDepth;
	bool					mbDeclaringTemplate;	//The declaration directly in a template declaration declares a template.
	CArrayDeferredBodies	macDeferred;
	int						miErrors;
	int						miSkimmedErrors;
	bool					mbReportSkimmed;
	int64					mlliFunction;
	int64					mlliFuncsig;
	int64					mlliFuncdname;
	int64					mlliPack;
	int						miPack;					//The current #pragma pack, as __pragma(pack(n)): 0 for none.
	CArrayInt				maiPacks;				//pack(push) saves miPack here.

public:
	void							Init(CCPPTokeniser* pcTokeniser, CCPPSyntaxTree* pcTree);
	void							Kill(void);
	bool							Parse(void);
	int								NumErrors(void);
	int								NumSkimmedErrors(void);
	void							ReportSkimmedErrors(bool bReport);

protected:
	//Tokens.  CppParser.cpp
	CCPPToken*						Peek(int iAhead = 0);
	bool							Is(ECppTokens eToken);
	bool							IsAhead(int iAhead, ECppTokens eToken);
	bool							IsIdentifier(void);
	bool							IsIdentifierAhead(int iAhead);
	int64							GetIdentifier(int iAhead = 0);
	bool							IsLiteral(void);
	bool							IsEnd(void);
	void							Next(void);
	bool							Accept(ECppTokens eToken);
	bool							Expect(ECppTokens eToken);
	bool							IsCloseAngle(void);
	bool							AcceptCloseAngle(void);
	SCPPParserPosition				Mark(void);
	void							Rewind(SCPPParserPosition sPosition);
	int								CurrentToken(void);
	int								PreviousToken(void);
	bool							IsSkimming(void);
	bool							IsSkimming(int iToken);
	bool							IsDeclaringTemplate(void);
	bool							Error(char* szError);
	bool							ErrorAt(int iToken, char* szError);
	void							BeginTentative(int* piSavedFailed);
	bool							EndTentative(int iSavedFailed);
	void							Resolve(int iToken, CCPPSymbol* pcSymbol, ECPPTokenRole eRole);
	bool							SkipBalanced(void);
	bool							SkipPragma(void);
	bool							SkipFunctionBody(void);
	void							SkipDeclaration(void);
	void							SkipStatement(void);
	CCPPScope*						PushScope(ECPPScopeKind eKind, CCPPSymbol* pcOwner);
	void							PopScope(CCPPScope* pcScope);
	CCPPSymbol*						Declare(int64 lliName, int iToken, ECPPSymbolKind eKind, CCPPSyntax* pcDeclaration, CCPPScope* pcScope);
	void							DeclareBuiltins(void);

	//Declarations.  CppParserDeclarations.cpp
	void							ParseDeclarations(CArraySyntax* papcDeclarations, CCPPSyntax* pcParent, bool bInBraces);
	CCPPSyntaxDeclaration*			ParseDeclaration(CCPPSyntax* pcParent, ECPPDeclarationContext eContext);
	CCPPSyntaxDeclaration*			ParseNamespace(CCPPSyntax* pcParent);
	CCPPSyntaxDeclaration*			ParseLinkageSpecification(CCPPSyntax* pcParent);
	CCPPSyntaxDeclaration*			ParseUsing(CCPPSyntax* pcParent);
	CCPPSyntaxDeclaration*			ParseStaticAssert(CCPPSyntax* pcParent);
	CCPPSyntaxDeclaration*			ParseAsmDeclaration(CCPPSyntax* pcParent);
	CCPPSyntaxDeclaration*			ParseSimpleDeclaration(CCPPSyntax* pcParent, ECPPDeclarationContext eContext);
	CCPPSyntaxDeclaration*			ParseFunctionDefinition(CCPPSyntax* pcParent, CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, ECPPDeclarationContext eContext, int iFirstToken);
	bool							ParseFunctionBody(CCPPSyntaxFunctionDefinition* pcFunction);
	bool							ParseMemberInitializers(CCPPSyntaxFunctionDefinition* pcFunction);
	bool							IsFunctionDefinitionStart(void);
	CCPPSymbol*						DeclareDeclarator(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, CCPPSyntax* pcDeclaration, ECPPDeclarationContext eContext);
	CCPPSyntaxDeclSpecifiers*		ParseDeclSpecifiers(CCPPSyntax* pcParent, ECPPDeclarationContext eContext);
	CCPPSyntaxAttribute*			ParseAttribute(CCPPSyntax* pcParent);
	bool							IsAttributeStart(void);
	void							ParseAttributes(CArraySyntax* papcAttributes, CCPPSyntax* pcParent);
	bool							IsDeclSpecifierKeyword(ECppTokens eToken);
	bool							IsSimpleTypeKeyword(ECppTokens eToken);
	bool							IsConstructorName(CCPPSyntaxName* pcName);
	CCPPSymbol*						GetTypeSymbol(CCPPSyntaxDeclSpecifiers* pcSpecifiers);

	//Classes, enums and templates.  CppParserClasses.cpp
	CCPPSyntaxTypeSpecifier*		ParseClassSpecifier(CCPPSyntax* pcParent, ECPPDeclarationContext eContext, bool bFriend);
	bool							ParseBaseSpecifiers(CCPPSyntaxClassSpecifier* pcClass);
	bool							ParseClassBody(CCPPSyntaxClassSpecifier* pcClass);
	void							ParseDeferredBodies(void);
	CCPPSyntaxTypeSpecifier*		ParseEnumSpecifier(CCPPSyntax* pcParent, ECPPDeclarationContext eContext);
	CCPPSyntaxDeclaration*			ParseTemplateDeclaration(CCPPSyntax* pcParent, ECPPDeclarationContext eContext);
	bool							ParseTemplateParameters(CCPPSyntaxTemplateDeclaration* pcTemplate);
	CCPPSyntax*						ParseTemplateParameter(CCPPSyntax* pcParent);

	//Declarators.  CppParserDeclarators.cpp
	CCPPSyntaxDeclarator*			ParseDeclarator(CCPPSyntax* pcParent, ECPPDeclaratorKind eKind, ECPPDeclarationContext eContext);
	CCPPSyntaxPointerOperator*		ParsePointerOperator(CCPPSyntax* pcParent);
	bool							IsPointerOperatorStart(void);
	bool							IsMemberPointerAhead(void);
	CCPPSyntaxFunctionSuffix*		ParseFunctionSuffix(CCPPSyntax* pcParent, CCPPScope* pcQualifierScope);
	CCPPSyntaxParameter*			ParseParameter(CCPPSyntax* pcParent);
	CCPPSyntaxArraySuffix*			ParseArraySuffix(CCPPSyntax* pcParent);
	CCPPSyntaxExceptionSpecification*	ParseExceptionSpecification(CCPPSyntax* pcParent);
	bool							IsParameterClause(void);
	bool							IsNestedDeclarator(ECPPDeclaratorKind eKind);
	void							ParseQualifiers(CArraySyntax* papcQualifiers, CCPPSyntax* pcParent, bool bRefQualifiers);
	CCPPSyntaxTypeId*				ParseTypeId(CCPPSyntax* pcParent);
	CCPPSyntaxTypeId*				ParseConversionTypeId(CCPPSyntax* pcParent);
	bool							IsTypeIdStart(void);
	bool							IsTypeIdAhead(ECppTokens eFollowing);
	CCPPSyntax*						ParseInitializer(CCPPSyntax* pcParent, ECPPInitializer* peKind);

	//Names.  CppParserNames.cpp
	CCPPSyntaxName*					ParseName(CCPPSyntax* pcParent, bool bDeclaration, CCPPScope** ppcQualifier);
	CCPPSyntaxNameComponent*		ParseOperatorName(CCPPSyntax* pcParent);
	bool							ParseTemplateArguments(CCPPSyntaxNameComponent* pcComponent);
	CCPPSyntax*						ParseTemplateArgument(CCPPSyntax* pcParent);
	bool							IsNameStart(void);
	CCPPSymbol*						LookupNameAhead(int* piLength);
	bool							IsTypeNameAhead(void);

	//Statements.  CppParserStatements.cpp
	CCPPSyntaxCompoundStatement*	ParseCompoundStatement(CCPPSyntax* pcParent, bool bNewScope);
	CCPPSyntaxStatement*			ParseStatement(CCPPSyntax* pcParent);
	CCPPSyntaxStatement*			ParseIfStatement(CCPPSyntax* pcParent);
	CCPPSyntaxStatement*			ParseSwitchStatement(CCPPSyntax* pcParent);
	CCPPSyntaxStatement*			ParseWhileStatement(CCPPSyntax* pcParent);
	CCPPSyntaxStatement*			ParseDoStatement(CCPPSyntax* pcParent);
	CCPPSyntaxStatement*			ParseForStatement(CCPPSyntax* pcParent);
	CCPPSyntaxStatement*			ParseReturnStatement(CCPPSyntax* pcParent);
	CCPPSyntaxStatement*			ParseCaseStatement(CCPPSyntax* pcParent);
	CCPPSyntaxStatement*			ParseTryStatement(CCPPSyntax* pcParent);
	CCPPSyntax*						ParseCondition(CCPPSyntax* pcParent);
	CCPPSyntaxStatement*			ParseDeclarationStatement(CCPPSyntax* pcParent, ECPPDeclarationContext eContext);
	bool							IsDeclarationStatement(ECPPDeclarationContext eContext);

	//Expressions.  CppParserExpressions.cpp
	CCPPSyntaxExpression*			ParseExpression(CCPPSyntax* pcParent);
	CCPPSyntaxExpression*			ParseAssignmentExpression(CCPPSyntax* pcParent);
	CCPPSyntaxExpression*			ParseConditionalExpression(CCPPSyntax* pcParent);
	CCPPSyntaxExpression*			ParseConstantExpression(CCPPSyntax* pcParent);
	CCPPSyntaxExpression*			ParseBinaryExpression(CCPPSyntax* pcParent, int iMinPrecedence);
	CCPPSyntaxExpression*			ParseCastExpression(CCPPSyntax* pcParent);
	CCPPSyntaxExpression*			ParseUnaryExpression(CCPPSyntax* pcParent);
	CCPPSyntaxExpression*			ParsePostfixExpression(CCPPSyntax* pcParent);
	CCPPSyntaxExpression*			ParsePrimaryExpression(CCPPSyntax* pcParent);
	CCPPSyntaxExpression*			ParseNewExpression(CCPPSyntax* pcParent);
	CCPPSyntaxExpression*			ParseDeleteExpression(CCPPSyntax* pcParent);
	CCPPSyntaxExpression*			ParseSizeofExpression(CCPPSyntax* pcParent);
	CCPPSyntaxExpression*			ParseNamedCastExpression(CCPPSyntax* pcParent);
	CCPPSyntaxExpression*			ParseTypeTraitExpression(CCPPSyntax* pcParent);
	CCPPSyntaxExpression*			ParseStringExpression(CCPPSyntax* pcParent);
	CCPPSyntaxExpression*			ParseConstructExpression(CCPPSyntax* pcParent, CCPPSyntaxTypeId* pcType);
	CCPPSyntaxInitializerList*		ParseBracedInitList(CCPPSyntax* pcParent);
	CCPPSyntax*						ParseInitializerClause(CCPPSyntax* pcParent);
	bool							ParseExpressionList(CArraySyntax* papcExpressions, CCPPSyntax* pcParent, ECppTokens eClose);
	int								GetBinaryPrecedence(void);
	bool							IsTypeTraitName(void);
	bool							IsStringPart(void);
	bool							IsCastAhead(void);
	CCPPSyntaxTypeId*				CreateSimpleTypeId(CCPPSyntax* pcParent);
};


#endif // !__C_P_P_PARSER_H__

