#include "JavaSyntaxParserErrors.h"
#include "JavaSyntaxParser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxParser::Init(CJavaSyntaxMemory* pcSyntaxes, CJavaTokenParser* pcTokenParser)
{
	mpcLogger = pcTokenParser->GetLogger();

	mpcTokens = pcTokenParser->GetTokenMemory();
	mpcSyntaxes = pcSyntaxes;

	mpcDefinitions = pcTokenParser->GetTokenDefinitions();
	mpcFirstToken = pcTokenParser->GetFirstToken();
	mpcCurrentToken = mpcFirstToken;

	mapcPositions.Init();
	PushPosition();

	mszFilename.Init(pcTokenParser->GetFilename());

	mcSyntaxTree.Init();
	mcError.Init(&mcSyntaxTree, NULL);
	mcMismatch.Init(&mcSyntaxTree, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxParser::Kill(void)
{
	mcMismatch.Kill();
	mcError.Kill();
	mcSyntaxTree.Kill();

	PopPosition();
	mapcPositions.Kill();

	mszFilename.Kill();

	mpcSyntaxes = NULL;
	mpcTokens = NULL;

	mpcDefinitions = NULL;
	mpcFirstToken = NULL;

	mpcLogger = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxParser::Parse(void)
{
	CJavaSyntaxFile*		pcFile;
	CJavaSyntaxPackage*		pcPackage;
	CJavaSyntaxImport*		pcImport;
	CJavaSyntaxClass*		pcClass;
	CJavaSyntaxEnum*		pcEnum;
	CJavaSyntaxInterface*	pcInterface;

	SkipComments();

	if (HasNext())
	{
		pcFile = mpcSyntaxes->CreateFile(&mcSyntaxTree, NULL);
		pcFile->SetFileName(mszFilename.Text());
		mcSyntaxTree.SetRoot(pcFile);

		for (;;)
		{
			pcPackage = ParsePackage(pcFile);
			if (pcPackage->IsPackage())
			{
				if (!pcFile->SetPackage(pcPackage))
				{
					mpcLogger->Error(EXPECTED_CLASS_OR_INTERFACE);
					return FALSE;
				}
				continue;
			}
			else if (pcPackage->IsError())
			{
				return FALSE;
			}

			pcImport = ParseImport(pcFile);
			if (pcImport->IsImport())
			{
				pcFile->AddImport(pcImport);
				continue;
			}
			else if (pcImport->IsError())
			{
				return FALSE;
			}

			pcClass = ParseClass(pcFile);
			if (pcClass->IsClass())
			{

			}
			else if (pcClass->IsError())
			{
				return FALSE;
			}

			pcEnum = ParseEnum(pcFile);
			if (pcEnum->IsEnum())
			{

			}
			else if (pcEnum->IsError())
			{
				return FALSE;
			}

			pcInterface = ParseInterface(pcFile);
			if (pcInterface->IsInterface())
			{

			}
			else if (pcInterface->IsError())
			{
				return FALSE;
			}

			break;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxParser::Parse(BOOL bFailOnError)
{
	BOOL	bResult;

	bResult = Parse();
	if (bResult == TRITRUE)
	{
		return TRUE;
	}
	else
	{
		CChars	szError;

		szError.Init();
		szError.Append("[");
		szError.Append(&mszFilename);
		szError.Append("] Syntax error:");
		szError.AppendNewLine();
//		mcParser.PrintPosition(&szError);
		szError.DumpKill();

		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxParser::PrettyPrint(CChars* pszDest)
{

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxParser::TypePrint(CChars* pszDest)
{
	mcSyntaxTree.Print(pszDest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxParser::Dump(BOOL bIncludeType)
{
	CChars	szDest;

	szDest.Init();

	if (bIncludeType)
	{
		TypePrint(&szDest);
	}
	else
	{
		PrettyPrint(&szDest);
	}

	szDest.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxParser::IsKeyword(CJavaToken* pcToken, EJavaTokenKeyword eKeyword)
{
	CJavaTokenKeyword* pcKeyword;
	
	if (pcToken && pcToken->IsKeyword())
	{
		pcKeyword = (CJavaTokenKeyword*)pcToken;
		return pcKeyword->Is(eKeyword);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxParser::IsSeparator(CJavaToken* pcToken, EJavaTokenSeparator eSeparator)
{
	CJavaTokenSeparator* pcSeparator;
	
	if (pcToken && pcToken->IsSeparator())
	{
		pcSeparator = (CJavaTokenSeparator*)pcToken;
		return pcSeparator->Is(eSeparator);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxParser::IsScope(CJavaToken* pcToken, EJavaTokenScope eGeneric)
{
	CJavaTokenScope* pcGeneric;

	if (pcToken && pcToken->IsScope())
	{
		pcGeneric = (CJavaTokenScope*)pcToken;
		return pcGeneric->Is(eGeneric);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxParser::IsAmbiguous(CJavaToken* pcToken, ECJavaTokenAmbiguous eAmbiguous)
{
	CCJavaTokenAmbiguous* pcAmbiguous;

	if (pcToken && pcToken->IsAmbiguous())
	{
		pcAmbiguous = (CCJavaTokenAmbiguous*)pcToken;
		return pcAmbiguous->Is(eAmbiguous);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxParser::IsOperator(CJavaToken* pcToken, EJavaTokenOperator eOperator)
{
	CJavaTokenOperator* pcOperator;

	if (pcToken && pcToken->IsOperator())
	{
		pcOperator = (CJavaTokenOperator*)pcToken;
		return pcOperator->Is(eOperator);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxParser::IsLiteral(CJavaToken* pcToken, EJavaTokenLiteralType eLiteralType)
{
	CJavaTokenLiteral* pcLiteral;

	if (pcToken && pcToken->IsLiteral())
	{
		pcLiteral = (CJavaTokenLiteral*)pcToken;
		return pcLiteral->IsLiteralType(eLiteralType);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxParser::IsIdentifier(CJavaToken* pcToken)
{
	return (pcToken && pcToken->IsIdentifier());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxParser::Next(void)
{
	if (mpcCurrentToken)
	{
		mpcCurrentToken = mpcCurrentToken->GetNext();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxParser::HasNext(void)
{
	return mpcCurrentToken != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxParser::SkipComments(void)
{
	while (mpcCurrentToken && mpcCurrentToken->IsComment())
	{
		mpcCurrentToken = mpcCurrentToken->GetNext();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaSyntaxPackage* CJavaSyntaxParser::ParsePackage(CJavaSyntax* pcParent)
{
	CJavaSyntaxPackage*		pcPackage;
	CJavaTokenIdentifier*	pcIdentifier;

	PushPosition();
	if (!GetKeyword(JK_package))
	{
		return Mismatch<CJavaSyntaxPackage>();
	}
	else
	{
		pcPackage = mpcSyntaxes->CreatePackage(&mcSyntaxTree, pcParent);
		if (pcPackage == NULL)
		{
			return Error<CJavaSyntaxPackage>(OUT_OUF_MEMORY);
		}
			
		for (;;)
		{
			pcIdentifier = GetIdentifier();
			if (pcIdentifier == NULL)
			{
				return Error<CJavaSyntaxPackage>(EXPECTED_IDENTIFIER);
			}

			pcPackage->AddIdentifier(pcIdentifier);

			if (GetSeparator(JS_Dot))
			{
			}
			else if (GetSeparator(JS_Semicolon))
			{
				PassPosition();
				return pcPackage;
			}
			else
			{
				return Error<CJavaSyntaxPackage>(EXPECTED_DOT_OR_SEMICOLON);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaSyntaxImport* CJavaSyntaxParser::ParseImport(CJavaSyntax* pcParent)
{
	CJavaSyntaxImport*		pcImport;
	CJavaTokenIdentifier*	pcIdentifier;
	int						iIdentifierCount;
	CCJavaTokenAmbiguous*	pcAmbiguousAsterisk;
	CJavaTokenScope*		pcScopeAsterisk;

	PushPosition();
	if (!GetKeyword(JK_import))
	{
		return Mismatch<CJavaSyntaxImport>();
	}
	else
	{
		pcImport = mpcSyntaxes->CreateImport(&mcSyntaxTree, pcParent);
		if (pcImport == NULL)
		{
			return Error<CJavaSyntaxImport>(OUT_OUF_MEMORY);
		}

		if (GetKeyword(JK_static))
		{
			pcImport->SetStatic(TRUE);
		}

		iIdentifierCount = 0;
		for (;;)
		{
			if ((iIdentifierCount > 0) && GetAmbiguous(JA_Asterisk, &pcAmbiguousAsterisk))
			{
				pcImport->SetWild(TRUE);
				if (GetSeparator(JS_Semicolon))
				{
					pcScopeAsterisk = CreateScope(pcAmbiguousAsterisk->GetPosition(), JG_Asterisk);
					ReplaceAmbiguous(pcAmbiguousAsterisk, pcScopeAsterisk);
					PassPosition();
					return pcImport;
				}
				else
				{
					return Error<CJavaSyntaxImport>(EXPECTED_SEMICOLON);
				}
			}
			else
			{
				pcIdentifier = GetIdentifier();
				if (pcIdentifier == NULL)
				{
					return Error<CJavaSyntaxImport>(EXPECTED_IDENTIFIER);
				}

				iIdentifierCount++;
				pcImport->AddIdentifier(pcIdentifier);

				if (GetSeparator(JS_Dot))
				{
				}
				else if (GetSeparator(JS_Semicolon))
				{
					PassPosition();
					return pcImport;
				}
				else
				{
					return Error<CJavaSyntaxImport>(EXPECTED_DOT_OR_SEMICOLON);
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaSyntaxClass* CJavaSyntaxParser::ParseClass(CJavaSyntax* pcParent)
{
	CJavaSyntaxClass*		pcClass;
	BOOL					bPublic;
	BOOL					bAbstract;
	BOOL					bFinal;
	BOOL					bOpen;
	CJavaSyntaxType*		pcType;

	PushPosition();

	ParseClassModifier(&bPublic, &bAbstract, &bFinal);

	if (bFinal && bAbstract)
	{
		return Error<CJavaSyntaxClass>(EXPECTED_CLASS);
	}

	if (!GetKeyword(JK_class))
	{
		return Mismatch<CJavaSyntaxClass>();
	}
	else
	{
		pcClass = mpcSyntaxes->CreateClass(&mcSyntaxTree, pcParent);
		if (pcClass == NULL)
		{
			return Error<CJavaSyntaxClass>(OUT_OUF_MEMORY);
		}

		pcClass->SetPublic(bPublic);
		pcClass->SetAbstract(bAbstract);
		pcClass->SetFinal(bFinal);

		pcType = ParseType(pcClass);
		if (pcType->IsType())
		{
			pcClass->SetSyntaxType(pcType);
		}
		else if (pcType->IsError())
		{
			return Error<CJavaSyntaxClass>();
		}
		else
		{
			return Error<CJavaSyntaxClass>(EXPECTED_IDENTIFIER);
		}

		bOpen = GetSeparator(JS_CurlyBracketLeft);

		if (!bOpen)
		{
			return Error<CJavaSyntaxClass>(EXPECTED_OPEN_CURLY_OR_ANGLE_BRACKET);
		}
		return pcClass;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxParser::ParseClassModifier(BOOL* pbPublic, BOOL* pbAbstract, BOOL* pbFinal)
{
	*pbPublic = FALSE;
	*pbAbstract = FALSE;
	*pbFinal = FALSE;

	for (;;)
	{
		if (!*pbPublic)
		{
			*pbPublic = GetKeyword(JK_public);
			if (*pbPublic)
			{
				continue;
			}
		}

		if (!*pbAbstract)
		{
			*pbAbstract = GetKeyword(JK_abstract);
			if (*pbAbstract)
			{
				continue;
			}
		}

		if (!*pbFinal)
		{
			*pbFinal = GetKeyword(JK_final);
			if (*pbFinal)
			{
				continue;
			}
		}

		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaSyntaxEnum* CJavaSyntaxParser::ParseEnum(CJavaSyntax* pcParent)
{
	CJavaSyntaxEnum*	pcEnum;
	BOOL				bPublic;

	PushPosition();

	bPublic = GetKeyword(JK_public);
	if (!GetKeyword(JK_import))
	{
		return Mismatch<CJavaSyntaxEnum>();
	}
	else
	{
		pcEnum = mpcSyntaxes->CreateEnum(&mcSyntaxTree, pcParent);
		if (pcEnum == NULL)
		{
			return Error<CJavaSyntaxEnum>(OUT_OUF_MEMORY);
		}

		for (;;)
		{
			return pcEnum;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaSyntaxInterface* CJavaSyntaxParser::ParseInterface(CJavaSyntax* pcParent)
{
	CJavaSyntaxInterface*	pcInterface;
	BOOL					bPublic;
	BOOL					bAbstract;

	PushPosition();

	bPublic = GetKeyword(JK_public);
	bAbstract = GetKeyword(JK_abstract);
	if (bAbstract && !bPublic)
	{
		bPublic = GetKeyword(JK_public);
	}

	if (!GetKeyword(JK_import))
	{
		return Mismatch<CJavaSyntaxInterface>();
	}
	else
	{
		pcInterface = mpcSyntaxes->CreateInterface(&mcSyntaxTree, pcParent);
		if (pcInterface == NULL)
		{
			return Error<CJavaSyntaxInterface>(OUT_OUF_MEMORY);
		}

		for (;;)
		{
			return pcInterface;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaSyntaxType* CJavaSyntaxParser::ParseType(CJavaSyntax* pcParent)
{
	CJavaSyntaxType*		pcType;
	CJavaSyntaxGeneric*		pcGeneric;
	CJavaTokenIdentifier*	pcIdentifier;
	CCJavaTokenAmbiguous*	pcAmbiguousAngleBracketLeft;
	CJavaTokenScope*		pcScopeAngleBracketLeft;
	CCJavaTokenAmbiguous*	pcAmbiguousAngleBracketRight;
	CJavaTokenScope*		pcScopeAngleBracketRight;

	PushPosition();

	pcIdentifier = GetIdentifier();
	if (pcIdentifier == NULL)
	{
		return Mismatch<CJavaSyntaxType>();
	}

	pcType = mpcSyntaxes->CreateType(&mcSyntaxTree, pcParent);
	if (pcType == NULL)
	{
		return Error<CJavaSyntaxType>(OUT_OUF_MEMORY);
	}

	pcType->SetName(pcIdentifier);

	if (GetAmbiguous(JA_AngleBracketLeft, &pcAmbiguousAngleBracketLeft))
	{
		for (;;)
		{
			pcGeneric = ParseGeneric(pcType);
			if (pcGeneric->IsGeneric())
			{
				pcType->AddGeneric(pcGeneric);
			}
			else
			{
				return Error<CJavaSyntaxType>(EXPECTED_GENERIC);
			}

			if (GetSeparator(JS_Comma))
			{
				continue;
			}
			else if (GetAmbiguous(JA_AngleBracketRight, &pcAmbiguousAngleBracketRight))
			{
				pcScopeAngleBracketLeft = CreateScope(pcAmbiguousAngleBracketLeft->GetPosition(), JG_AngleBracketLeft);
				pcScopeAngleBracketRight = CreateScope(pcAmbiguousAngleBracketRight->GetPosition(), JG_AngleBracketRight);
				ReplaceAmbiguous(pcAmbiguousAngleBracketLeft, pcScopeAngleBracketLeft);
				ReplaceAmbiguous(pcAmbiguousAngleBracketRight, pcScopeAngleBracketRight);
				PassPosition();
				return pcType;
			}
			else
			{
				return Error<CJavaSyntaxType>(EXPECTED_CLOSE_ANGLE_BRACKET_OR_COMMA);
			}
		}
	}
	else if (GetAmbiguous(JA_AngleBracketRight, &pcAmbiguousAngleBracketRight))
	{
		pcScopeAngleBracketRight = CreateScope(pcAmbiguousAngleBracketRight->GetPosition(), JG_AngleBracketRight);
		ReplaceAmbiguous(pcAmbiguousAngleBracketRight, pcScopeAngleBracketRight);
		PassPosition();
		return pcType;
	}
	else
	{
		return Error<CJavaSyntaxType>(EXPECTED_OPEN_OR_CLOSE_ANGLE_BRACKET);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaSyntaxGeneric* CJavaSyntaxParser::ParseGeneric(CJavaSyntax* pcParent)
{
	CJavaSyntaxGeneric*			pcGeneric;
	CCJavaTokenAmbiguous*		pcAmbiguousQuestionMark;
	CJavaTokenScope*			pcScopeQuestionMark;
	BOOL						bQuestionMark;
	CJavaSyntaxType*			pcExtendsType;
	CJavaSyntaxType*			pcGenericType;

	PushPosition();
	bQuestionMark = FALSE;
	pcGenericType = NULL;
	pcGeneric = mpcSyntaxes->CreateGeneric(&mcSyntaxTree, pcParent);

	if (GetAmbiguous(JA_QuestionMark, &pcAmbiguousQuestionMark))
	{
		bQuestionMark = TRUE;
		pcGeneric->SetWildCard(TRUE);
	}
	else
	{
		pcGenericType = ParseType(pcGeneric);
		pcGeneric->SetGenericType(pcGenericType);
	}

	if ((bQuestionMark && pcGenericType != NULL) || ((!bQuestionMark && pcGenericType == NULL)))
	{
		return Error<CJavaSyntaxGeneric>(EXPECTED_IDENTIFIER_OR_QUESTION_MARK);
	}

	PushPosition();
	if (GetKeyword(JK_extends))
	{
		PassPosition();
		pcExtendsType = ParseType(pcGeneric);
		if (pcExtendsType->IsType())
		{
			pcGeneric->SetExtends(pcExtendsType);
		}
		else if (pcExtendsType->IsError())
		{
			return Error<CJavaSyntaxGeneric>();
		}
	}
	else if (GetAmbiguous(JA_AngleBracketLeft))
	{
		PopPosition();
		ParseType(pcGeneric);  //Can't do this.
	}
	else
	{
		PassPosition();
	}

	if (bQuestionMark)
	{
		pcScopeQuestionMark = CreateScope(pcAmbiguousQuestionMark->GetPosition(), JG_QuestionMark);
		ReplaceAmbiguous(pcAmbiguousQuestionMark, pcScopeQuestionMark);
	}

	PassPosition();
	return pcGeneric;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxParser::GetKeyword(EJavaTokenKeyword eKeyword)
{
	if (IsKeyword(mpcCurrentToken, eKeyword))
	{
		Next();
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxParser::GetSeparator(EJavaTokenSeparator eSeparator)
{
	if (IsSeparator(mpcCurrentToken, eSeparator))
	{
		Next();
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenIdentifier* CJavaSyntaxParser::GetIdentifier(void)
{
	CJavaTokenIdentifier*	pcIdentifier;

	pcIdentifier = (CJavaTokenIdentifier*)mpcCurrentToken;
	if (IsIdentifier(pcIdentifier))
	{
		Next();
		return pcIdentifier;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxParser::GetScope(EJavaTokenScope eGeneric)
{
	if (IsScope(mpcCurrentToken, eGeneric))
	{
		Next();
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxParser::GetAmbiguous(ECJavaTokenAmbiguous eAmbiguous, CCJavaTokenAmbiguous** ppcAmbiguous)
{
	if (IsAmbiguous(mpcCurrentToken, eAmbiguous))
	{
		SafeAssign(ppcAmbiguous, (CCJavaTokenAmbiguous*)mpcCurrentToken);
		Next();
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxParser::GetOperator(EJavaTokenOperator eOperator)
{
	if (IsOperator(mpcCurrentToken, eOperator))
	{
		Next();
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxParser::PushPosition(void)
{
	mapcPositions.Push(mpcCurrentToken);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxParser::PopPosition(void)
{
	mpcCurrentToken = mapcPositions.Pop();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxParser::PassPosition(void)
{
	mapcPositions.Pop();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxParser::ReplaceAmbiguous(CJavaToken* pcSearch, CJavaToken* pcReplacement)
{
	CJavaToken*		pcCurrent;
	CJavaToken*		pcPrevious;

	pcPrevious = NULL;
	pcCurrent = mpcFirstToken;
	while (pcCurrent)
	{
		if (pcCurrent == pcSearch)
		{
			if (pcPrevious)
			{
				pcPrevious->SetNext(pcReplacement);
			}
			pcReplacement->SetNext(pcCurrent->GetNext());
			return TRUE;
		}
		pcPrevious = pcCurrent;
		pcCurrent = pcCurrent->GetNext();
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenScope* CJavaSyntaxParser::CreateScope(STextPosition* psPosition, EJavaTokenScope eScope)
{
	CJavaTokenScope*				pcScope;
	CJavaTokenScopeDefinition*	pcDefinition;

	pcDefinition = mpcDefinitions->GetScope(JG_Asterisk);
	pcScope = mpcTokens->CreateScope(psPosition, pcDefinition);
	return pcScope;
}

