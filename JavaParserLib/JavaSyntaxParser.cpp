#include "JavaSyntaxParser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxParser::Init(CJavaTokenDefinitions* pcDefinitions, char* szFilename, CJavaToken* pcFirstToken)
{
	mcSyntaxes.Init();

	mpcDefinitions = pcDefinitions;
	mpcFirstToken = pcFirstToken;
	mpcCurrentToken = mpcFirstToken;

	mapcPositions.Init();
	PushPosition();

	mcSyntaxTree.Init(szFilename);
	mcError.Init(&mcSyntaxTree);
	mcMismatch.Init(&mcSyntaxTree);
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

	mcSyntaxes.Kill();

	mpcDefinitions = NULL;
	mpcFirstToken = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxParser::Parse(void)
{
	CJavaSyntaxFile*		pcFile;
	CJavaSyntaxPackage*		pcPackage;

	SkipComments();

	if (mpcCurrentToken)
	{
		pcFile = mcSyntaxes.CreateFile(&mcSyntaxTree);
		mcSyntaxTree.SetRoot(pcFile);

		pcPackage = ParsePackage();
		if (pcPackage->IsError())
		{
			return FALSE;
		}
		else if (pcPackage->IsPackage())
		{
			if (!pcFile->SetPackage(pcPackage))
			{
				return FALSE;
			}
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
		szError.Append(mcSyntaxTree.GetFileName());
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

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxParser::Dump(BOOL bIncludeType)
{

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxParser::IsKeyword(CJavaToken* pcToken, EJavaKeyword eKeyword)
{
	CJavaKeyword* pcKeyword;
	
	if (pcToken && pcToken->IsKeyword())
	{
		pcKeyword = (CJavaKeyword*)pcToken;
		return pcKeyword->Is(eKeyword);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxParser::IsSeparator(CJavaToken* pcToken, EJavaSeparator eSeparator)
{
	CJavaSeparator* pcSeparator;
	
	if (pcToken && pcToken->IsSeparator())
	{
		pcSeparator = (CJavaSeparator*)pcToken;
		return pcSeparator->Is(eSeparator);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxParser::IsGeneric(CJavaToken* pcToken, EJavaGeneric eGeneric)
{
	CJavaGeneric* pcGeneric;

	if (pcToken && pcToken->IsGeneric())
	{
		pcGeneric = (CJavaGeneric*)pcToken;
		return pcGeneric->Is(eGeneric);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxParser::IsAmbiguous(CJavaToken* pcToken, EJavaAmbiguous eAmbiguous)
{
	CJavaAmbiguous* pcAmbiguous;

	if (pcToken && pcToken->IsAmbiguous())
	{
		pcAmbiguous = (CJavaAmbiguous*)pcToken;
		return pcAmbiguous->Is(eAmbiguous);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxParser::IsOperator(CJavaToken* pcToken, EJavaOperator eOperator)
{
	CJavaOperator* pcOperator;

	if (pcToken && pcToken->IsOperator())
	{
		pcOperator = (CJavaOperator*)pcToken;
		return pcOperator->Is(eOperator);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxParser::IsLiteral(CJavaToken* pcToken, EJavaLiteralType eLiteralType)
{
	CJavaLiteral* pcLiteral;

	if (pcToken && pcToken->IsLiteral())
	{
		pcLiteral = (CJavaLiteral*)pcToken;
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
CJavaSyntaxPackage* CJavaSyntaxParser::ParsePackage(void)
{
	CJavaSyntaxPackage*		pcPackage;
	CJavaIdentifier*		pcIdentifier;

	PushPosition();
	if (!GetKeyword(JK_package))
	{
		PopPosition();
		return (CJavaSyntaxPackage*)&mcMismatch;
	}
	else
	{
		pcPackage = mcSyntaxes.CreatePackage(&mcSyntaxTree);
		if (pcPackage == NULL)
		{
			//Error: Out of memory.
			PassPosition();
			return (CJavaSyntaxPackage*)&mcError;
		}

		for (;;)
		{
			pcIdentifier = GetIdentifier();
			if (pcIdentifier == NULL)
			{
				//Error: Identifier expected.
				PassPosition();
				return (CJavaSyntaxPackage*)&mcError;
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
				//Error: '.' or ';' expected.
				PassPosition();
				return (CJavaSyntaxPackage*)&mcError;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxParser::GetKeyword(EJavaKeyword eKeyword)
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
BOOL CJavaSyntaxParser::GetSeparator(EJavaSeparator eSeparator)
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
CJavaIdentifier* CJavaSyntaxParser::GetIdentifier(void)
{
	CJavaIdentifier*	pcIdentifier;

	pcIdentifier = (CJavaIdentifier*)mpcCurrentToken;
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

