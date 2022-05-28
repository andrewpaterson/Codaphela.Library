#include "BaseLib/Logger.h"
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
	pcTokenParser->GetText(&mcText);

	mcSyntaxTree.Init();
	mcError.Init(&mcSyntaxTree, NULL);
	mcMismatch.Init(&mcSyntaxTree, NULL);

	miErrors = 0;
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
CJavaSyntaxFile* CJavaSyntaxParser::GetSyntaxFile(void)
{
	return mcSyntaxTree.GetRoot();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CJavaSyntaxParser::GetFilename(void)
{
	return mszFilename.Text();
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
				pcFile->AddClass(pcClass);
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

	if (miErrors == 0)
	{
		mpcLogger->Info2("Syntax parsing file [", mszFilename.Text(), "] successful.", NULL);
	}
	else
	{
		mpcLogger->Info2("Syntax parsing file [", mszFilename.Text(), "] failed with [", IntToString(miErrors), "].", NULL);
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
char* CJavaSyntaxParser::PrettyPrint(CChars* pszDest)
{
	return mcSyntaxTree.PrettyPrint(pszDest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxParser::TypePrint(CChars* pszDest)
{
	mcSyntaxTree.TypePrint(pszDest);
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
	CJavaSyntaxType*		pcType;
	CJavaModifiers			cModifers;
	CJavaSyntaxClassBlock*	pcBlock;

	PushPosition();

	cModifers = ParseModifiers(JM_public | JM_protected | JM_private | JM_abstract | JM_final | JM_strictfp);

	if (!cModifers.IsValid())
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

		pcClass->SetModifiers(cModifers);

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

		pcBlock = ParseClassBlock(pcClass);
		if (pcBlock->IsClassBlock())
		{
			pcClass->SetBlock(pcBlock);
		}
		else if (pcBlock->IsError())
		{
			return Error<CJavaSyntaxClass>();
		}
		else if (pcBlock->IsMismatch())
		{
			return Error<CJavaSyntaxClass>(EXPECTED_SOMETHING_ELSE);
		}


		PassPosition();
		return pcClass;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaSyntaxClassBlock* CJavaSyntaxParser::ParseClassBlock(CJavaSyntax* pcParent)
{
	CJavaSyntaxClassBlock*	pcBlock;
	BOOL					bOpen;
	BOOL					bClose;
	CJavaSyntaxStatement*	pcStatement;

	PushPosition();

	bOpen = GetSeparator(JS_CurlyBracketLeft);
	if (!bOpen)
	{
		return Error<CJavaSyntaxClassBlock>(EXPECTED_OPEN_CURLY_OR_ANGLE_BRACKET);
	}

	pcBlock = mpcSyntaxes->CreateClassBlock(&mcSyntaxTree, pcParent);
	for (;;)
	{
		pcStatement = ParseClassBlockStatement(pcParent);
		if (pcStatement->IsStatement())
		{
			pcBlock->AddStatement(pcStatement);
			continue;
		}
		else if (pcStatement->IsError())
		{
			//pcBlock->ClearHierarchy();
			pcBlock->Clear();
			return Error<CJavaSyntaxClassBlock>();
		}

		bClose = GetSeparator(JS_CurlyBracketRight);
		if (bClose)
		{
			GetSeparator(JS_Semicolon);
			PassPosition();
			return pcBlock;
		}
		else
		{
			//pcBlock->ClearHierarchy();
			pcBlock->Clear();
			return Error<CJavaSyntaxClassBlock>(EXPECTED_CLOSE_CURLY_BRACKET);
		}
	}

	return pcBlock;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaSyntaxStatement* CJavaSyntaxParser::ParseClassBlockStatement(CJavaSyntax* pcParent)
{
	CJavaSyntaxVariableDeclaration*		pcVariable;
	PushPosition();

	pcVariable = ParseClassVariable(pcParent);
	if (pcVariable->IsVariableDeclaration())
	{
		PassPosition();
		return pcVariable;
	}
	else if (pcVariable->IsError())
	{
		return Error<CJavaSyntaxStatement>();
	}
	else
	{
		return Mismatch<CJavaSyntaxStatement>();
	}

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaSyntaxVariableDeclaration* CJavaSyntaxParser::ParseClassVariable(CJavaSyntax* pcParent)
{
	CJavaModifiers						cModifers;
	CJavaTokenKeyword*					pcModifer;
	CJavaTokenKeyword*					pcPrimitive;
	CJavaTokenIdentifier*				pcType;
	CJavaTokenIdentifier*				pcName;
	CJavaSyntaxVariableDeclaration*		pcVariable;
	CJavaSyntaxGeneric*					pcGeneric;
	int									iArrayDepth;
	CJavaSyntaxVariableInitialiser*		pcInitialiser;

	PushPosition();

	cModifers = ParseModifiers(JM_static | JM_final | JM_public | JM_protected | JM_private);
	pcModifer = GetModifierKeyword();
	if (pcModifer != NULL)
	{
		return Mismatch<CJavaSyntaxVariableDeclaration>();
	}

	pcVariable = mpcSyntaxes->CreateVariableDeclaration(&mcSyntaxTree, pcParent);
	pcVariable->SetModifiers(cModifers);

	pcPrimitive = GetPrimitveKeyword();
	if (pcPrimitive)
	{
		pcVariable->SetPrimitiveType(pcPrimitive);
	}
	else
	{
		for (;;)
		{
			pcType = GetIdentifier();
			if (pcType == NULL)
			{
				pcVariable->Clear();
				return Mismatch<CJavaSyntaxVariableDeclaration>();
			}

			pcVariable->AddIdentifierType(pcType);

			if (!GetSeparator(JS_Dot))
			{
				break;
			}
		}

		pcGeneric = ParseGeneric(pcVariable);
	}

	iArrayDepth = ParseArrayDeclaration();
	if (iArrayDepth == -1)
	{
		pcVariable->Clear();
		return Error<CJavaSyntaxVariableDeclaration>(EXPECTED_CLOSE_SQUARE_BRACKET);
	}

	pcName = GetIdentifier();
	if (pcName == NULL)
	{
		pcVariable->Clear();
		return Error<CJavaSyntaxVariableDeclaration>(EXPECTED_IDENTIFIER);
	}

	pcVariable->SetName(pcName);

	if (iArrayDepth == 0)
	{
		iArrayDepth = ParseArrayDeclaration();
		if (iArrayDepth == -1)
		{
			pcVariable->Clear();
			return Error<CJavaSyntaxVariableDeclaration>(EXPECTED_CLOSE_SQUARE_BRACKET);
		}
	}

	pcVariable->SetArrayDimension(iArrayDepth);

	if (!GetSeparator(JS_Semicolon))
	{
		pcInitialiser = ParseVariableInitialiser(pcParent);
		if (pcInitialiser->IsVariableInitialiser())
		{
			pcVariable->SetInitialiser(pcInitialiser);
		}
		else if (pcInitialiser->IsMismatch())
		{
			pcVariable->Clear();
			return Error<CJavaSyntaxVariableDeclaration>(EXPECTED_SOMETHING_ELSE);
		}
		else if (pcInitialiser->IsError())
		{
			pcVariable->Clear();
			return Error<CJavaSyntaxVariableDeclaration>();
		}
	}

	PassPosition();
	return pcVariable;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CJavaSyntaxParser::ParseArrayDeclaration(void)
{
	int		iArrayDepth;

	iArrayDepth = 0;
	for (;;)
	{
		if (GetSeparator(JS_SquareBracketLeft))
		{
			if (!GetSeparator(JS_SquareBracketRight))
			{
				return -1;
			}
			iArrayDepth++;
		}
		else
		{
			return iArrayDepth;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaSyntaxVariableInitialiser* CJavaSyntaxParser::ParseVariableInitialiser(CJavaSyntax* pcParent)
{
	PushPosition();

	return Error<CJavaSyntaxVariableInitialiser>();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaModifiers CJavaSyntaxParser::ParseModifiers(uint8 uiAllowedModifiers)
{
	BOOL			bResult;
	CJavaModifiers	cModifiers;

	cModifiers.Init();
	for (;;)
	{
		if ((uiAllowedModifiers & JM_public) && !cModifiers.IsPublic())
		{
			bResult = GetKeyword(JK_public);
			if (bResult)
			{
				cModifiers.Set(JM_public);
				continue;
			}
		}

		if ((uiAllowedModifiers & JM_protected) && !cModifiers.IsProtected())
		{
			bResult = GetKeyword(JK_protected);
			if (bResult)
			{
				cModifiers.Set(JM_protected);
				continue;
			}
		}

		if ((uiAllowedModifiers & JM_private) && !cModifiers.IsPrivate())
		{
			bResult = GetKeyword(JK_private);
			if (bResult)
			{
				cModifiers.Set(JM_private);
				continue;
			}
		}

		if ((uiAllowedModifiers & JM_final) && !cModifiers.IsFinal())
		{
			bResult = GetKeyword(JK_final);
			if (bResult)
			{
				cModifiers.Set(JM_final);
				continue;
			}
		}

		if ((uiAllowedModifiers & JM_static) && !cModifiers.IsStatic())
		{
			bResult = GetKeyword(JK_static);
			if (bResult)
			{
				cModifiers.Set(JM_static);
				continue;
			}
		}

		if ((uiAllowedModifiers & JM_abstract) && !cModifiers.IsAbstract())
		{
			bResult = GetKeyword(JK_abstract);
			if (bResult)
			{
				cModifiers.Set(JM_abstract);
				continue;
			}
		}

		if ((uiAllowedModifiers & JM_strictfp) && !cModifiers.IsStrictfp())
		{
			bResult = GetKeyword(JK_strictfp);
			if (bResult)
			{
				cModifiers.Set(JM_strictfp);
				continue;
			}
		}

		break;
	}

	return cModifiers;
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
CJavaSyntaxGeneric* CJavaSyntaxParser::ParseGeneric(CJavaSyntax* pcParent)
{
	CJavaSyntaxGeneric*		pcGeneric;
	CJavaSyntaxTypeCommon*	pcType;
	CCJavaTokenAmbiguous*	pcAmbiguousAngleBracketLeft;
	CJavaTokenScope*		pcScopeAngleBracketLeft;
	CCJavaTokenAmbiguous*	pcAmbiguousAngleBracketRight;
	CJavaTokenScope*		pcScopeAngleBracketRight;

	PushPosition();

	if (GetAmbiguous(JA_AngleBracketLeft, &pcAmbiguousAngleBracketLeft))
	{
		pcGeneric = mpcSyntaxes->CreateGeneric(&mcSyntaxTree, pcParent);
		for (;;)
		{
			if (GetSeparator(JS_Comma))
			{
				continue;
			}
			
			if (GetAmbiguous(JA_AngleBracketRight, &pcAmbiguousAngleBracketRight))
			{
				pcScopeAngleBracketLeft = CreateScope(pcAmbiguousAngleBracketLeft->GetPosition(), JG_AngleBracketLeft);
				pcScopeAngleBracketRight = CreateScope(pcAmbiguousAngleBracketRight->GetPosition(), JG_AngleBracketRight);
				ReplaceAmbiguous(pcAmbiguousAngleBracketLeft, pcScopeAngleBracketLeft);
				ReplaceAmbiguous(pcAmbiguousAngleBracketRight, pcScopeAngleBracketRight);
				PassPosition();
				return pcGeneric;
			}

			pcType = ParseTypeCommon(pcGeneric);
			if (pcType->IsTypeCommon())
			{
				pcGeneric->AddType(pcType);
			}
			else if (pcType->IsMismatch())
			{
				return Error<CJavaSyntaxGeneric>(EXPECTED_TYPE_OR_CLOSE_ANGLE_BRACKET);
			}
			else
			{
				return Error<CJavaSyntaxGeneric>();
			}
		}
	}
	else
	{
		return Mismatch<CJavaSyntaxGeneric>();
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

	pcGeneric = ParseGeneric(pcType);
	if (pcGeneric->IsGeneric())
	{
		pcType->SetGeneric(pcGeneric);
		PassPosition();
		return pcType;
	}
	else if (pcGeneric->IsMismatch())
	{
		PassPosition();
		return pcType;
	}
	else
	{
		PassPosition();
		return Error<CJavaSyntaxType>();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaSyntaxExtent* CJavaSyntaxParser::ParseExtent(CJavaSyntax* pcParent)
{
	CCJavaTokenAmbiguous*	pcAmbiguousQuestionMark;
	CJavaTokenIdentifier*	pcIdentifier;
	BOOL					bQuestionMark;
	CJavaSyntaxExtent*		pcExtent;
	CJavaTokenScope*		pcScopeQuestionMark;
	BOOL					bExtends;
	CJavaSyntaxType*		pcType;

	PushPosition();
	bQuestionMark = FALSE;
	pcIdentifier = NULL;

	if (GetAmbiguous(JA_QuestionMark, &pcAmbiguousQuestionMark))
	{
		bQuestionMark = TRUE;
	}
	else
	{
		pcIdentifier = GetIdentifier();
	}

	if (!bQuestionMark && pcIdentifier == NULL)
	{
		return Mismatch<CJavaSyntaxExtent>();
	}

	bExtends = GetKeyword(JK_extends);

	if (bQuestionMark && !bExtends)
	{
		pcExtent = mpcSyntaxes->CreateExtent(&mcSyntaxTree, pcParent);
		pcExtent->SetWildCard(TRUE);

		pcScopeQuestionMark = CreateScope(pcAmbiguousQuestionMark->GetPosition(), JG_QuestionMark);
		ReplaceAmbiguous(pcAmbiguousQuestionMark, pcScopeQuestionMark);

		PassPosition();
		return pcExtent;
	}
	else if (!bExtends)
	{
		return Mismatch<CJavaSyntaxExtent>();
	}

	pcExtent = mpcSyntaxes->CreateExtent(&mcSyntaxTree, pcParent);
	if (bQuestionMark)
	{
		pcExtent->SetWildCard(TRUE);

		pcScopeQuestionMark = CreateScope(pcAmbiguousQuestionMark->GetPosition(), JG_QuestionMark);
		ReplaceAmbiguous(pcAmbiguousQuestionMark, pcScopeQuestionMark);
	}
	else if (pcIdentifier)
	{
		pcExtent->SetName(pcIdentifier);
	}

	pcType = ParseType(pcExtent);
	if (pcType->IsError())
	{
		return Error<CJavaSyntaxExtent>();
	}
	else if (pcType->IsMismatch())
	{
		return Mismatch<CJavaSyntaxExtent>();
	}

	pcExtent->SetExtends(pcType);

	PassPosition(); 
	return pcExtent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaSyntaxTypeCommon* CJavaSyntaxParser::ParseTypeCommon(CJavaSyntax* pcParent)
{
	CJavaSyntaxType*	pcType;
	CJavaSyntaxExtent*	pcExtent;

	pcExtent = ParseExtent(pcParent);
	if (pcExtent->IsExtent())
	{
		return pcExtent;
	}
	else if (pcExtent->IsError())
	{
		return Error<CJavaSyntaxTypeCommon>();
	}

	pcType = ParseType(pcParent);
	if (pcType->IsType())
	{
		return pcType;
	}
	else if (pcType->IsError() || pcType->IsMismatch())
	{
		return pcType;
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Invalid parse return.", NULL);
	}

	return Mismatch<CJavaSyntaxTypeCommon>();
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
CJavaTokenKeyword* CJavaSyntaxParser::GetModifierKeyword(void)
{
	CJavaTokenKeyword*	pcKeyword;

	if (mpcCurrentToken && mpcCurrentToken->IsKeyword())
	{
		pcKeyword = (CJavaTokenKeyword*)mpcCurrentToken;
		if (pcKeyword->Is(JK_public) ||
			pcKeyword->Is(JK_protected) ||
			pcKeyword->Is(JK_private) ||
			pcKeyword->Is(JK_final) ||
			pcKeyword->Is(JK_static) ||
			pcKeyword->Is(JK_abstract) ||
			pcKeyword->Is(JK_strictfp))
		{
			Next();
			return pcKeyword;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenKeyword* CJavaSyntaxParser::GetPrimitveKeyword(void)
{
	CJavaTokenKeyword* pcKeyword;
	
	if (mpcCurrentToken && mpcCurrentToken->IsKeyword())
	{
		pcKeyword = (CJavaTokenKeyword*)mpcCurrentToken;
		if (pcKeyword->Is(JK_boolean) ||
			pcKeyword->Is(JK_byte) ||
			pcKeyword->Is(JK_char) ||
			pcKeyword->Is(JK_double) ||
			pcKeyword->Is(JK_float) ||
			pcKeyword->Is(JK_int) ||
			pcKeyword->Is(JK_long) ||
			pcKeyword->Is(JK_short))
		{
			Next();
			return pcKeyword;
		}
	}
	return NULL;
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxParser::PrivateError(char* szError)
{
	PassPosition();

	STextPosition* psPos;
	CTextPositionPrinter	cPrinter;
	CChars					sz;
	STextPosition			sPos;

	miErrors++;

	if (mpcCurrentToken)
	{
		psPos = mpcCurrentToken->GetPosition();
	}
	else
	{
		sPos.Init(mcText.msz, mcText.miLen, &(mcText.msz[mcText.miLen]));
		psPos = &sPos;
	}

	cPrinter.Init(&mcText, psPos, mszFilename.Text());
	sz.Init(szError);
	sz.AppendNewLine();
	cPrinter.PrintPosition(&sz);

	mpcLogger->Error(sz.Text());
	sz.Kill();
}

