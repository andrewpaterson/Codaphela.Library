#include "BaseLib/LogString.h"
#include "JavaParserLib/JavaTokenParser.h"
#include "TestLib/Assert.h"
#include "JavaTokenAssert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaToken* PrivateAssertKeyword(CJavaTokenDefinitions* pcDefinitions, CJavaToken* pcToken, EJavaTokenKeyword eKeyword, size iLine, char* szFile)
{
	CJavaTokenKeyword*			pcKeyword;
	CChars					sz;
	CJavaTokenKeywordDefinition* pcDefinition;

	AssertNotNull(pcToken);
	if (pcToken)
	{
		AssertTrue(pcToken->IsKeyword());
		if (pcToken->IsKeyword())
		{
			pcKeyword = (CJavaTokenKeyword*)pcToken;
			if (!pcKeyword->Is(eKeyword))
			{
				sz.Init();
				pcKeyword->Print(&sz);
				pcDefinition = pcDefinitions->GetKeyword(eKeyword);
				Failed((const char*)pcDefinition->GetName(), (const char*)sz.Text(), "", iLine, szFile, false);
				sz.Kill();
				BREAK();
			}

		}
		return pcToken->GetNext();
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaToken* PrivateAssertIdentifier(CJavaTokenDefinitions* pcDefinitions, CJavaToken* pcToken, char* szIdentifier, size iLine, char* szFile)
{
	CJavaTokenIdentifier*			pcIdentifier;
	CChars						sz;

	AssertNotNull(pcToken);
	if (pcToken)
	{
		AssertTrue(pcToken->IsIdentifier());
		if (pcToken->IsIdentifier())
		{
			pcIdentifier = (CJavaTokenIdentifier*)pcToken;
			if (!pcIdentifier->Is(szIdentifier))
			{
				sz.Init();
				pcIdentifier->Print(&sz);
				Failed((const char*)szIdentifier, (const char*)sz.Text(), "", iLine, szFile, false);
				sz.Kill();
				BREAK();
			}
		}
		return pcToken->GetNext();
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaToken* PrivateAssertSeparator(CJavaTokenDefinitions* pcDefinitions, CJavaToken* pcToken, EJavaTokenSeparator eSeparator, size iLine, char* szFile)
{
	CJavaTokenSeparator*				pcSeparator;
	CChars						sz;
	CJavaTokenSeparatorDefinition*	pcDefinition;

	AssertNotNull(pcToken);
	if (pcToken)
	{
		AssertTrue(pcToken->IsSeparator());
		if (pcToken->IsSeparator())
		{
			pcSeparator = (CJavaTokenSeparator*)pcToken;
			if (!pcSeparator->Is(eSeparator))
			{
				sz.Init();
				pcSeparator->Print(&sz);
				pcDefinition = pcDefinitions->GetSeparator(eSeparator);
				Failed((const char*)pcDefinition->GetName(), (const char*)sz.Text(), "", iLine, szFile, false);
				sz.Kill();
				BREAK();
			}
		}
		return pcToken->GetNext();
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaToken* PrivateAssertOperator(CJavaTokenDefinitions* pcDefinitions, CJavaToken* pcToken, EJavaTokenOperator eOperator, size iLine, char* szFile)
{
	CJavaTokenOperator*				pcOperator;
	CChars						sz;
	CJavaTokenOperatorDefinition*	pcDefinition;

	AssertNotNull(pcToken);
	if (pcToken)
	{
		pcOperator = (CJavaTokenOperator*)pcToken;
		AssertTrue(pcToken->IsOperator());
		if (pcToken->IsOperator())
		{
			if (!pcOperator->Is(eOperator))
			{
				sz.Init();
				pcOperator->Print(&sz);
				pcDefinition = pcDefinitions->GetOperator(eOperator);
				Failed((const char*)pcDefinition->GetName(), (const char*)sz.Text(), "", iLine, szFile, false);
				sz.Kill();
				BREAK();
			}
		}
		return pcToken->GetNext();
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaToken* PrivateAssertLiteral(CJavaTokenDefinitions* pcDefinitions, CJavaToken* pcToken, char* szString, size iLine, char* szFile)
{
	CJavaTokenString*	pcString;
	CJavaTokenLiteral*	pcLiteral;
	CChars				sz;

	AssertNotNull(pcToken);
	if (pcToken)
	{
		AssertTrue(pcToken->IsLiteral());
		if (pcToken->IsLiteral())
		{
			pcLiteral = (CJavaTokenLiteral*)pcToken;
			AssertTrue(pcLiteral->IsString());
			if (pcLiteral->IsString())
			{
				pcString = (CJavaTokenString*)pcToken;
				if (!pcString->Is(szString))
				{
					sz.Init();
					pcString->Print(&sz);
					Failed((const char*)szString, (const char*)sz.Text(), "", iLine, szFile, false);
					sz.Kill();
					BREAK();
				}
			}
		}
		return pcToken->GetNext();
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaToken* PrivateAssertLiteral(CJavaTokenDefinitions* pcDefinitions, CJavaToken* pcToken, int64 lli, size iLine, char* szFile)
{
	CJavaTokenInteger*	pcInteger;
	CJavaTokenLiteral*	pcLiteral;
	CChars			sz;

	AssertNotNull(pcToken);
	if (pcToken)
	{
		AssertTrue(pcToken->IsLiteral());
		if (pcToken->IsLiteral())
		{
			pcLiteral = (CJavaTokenLiteral*)pcToken;
			AssertTrue(pcLiteral->IsInteger());
			if (pcLiteral->IsInteger())
			{
				pcInteger = (CJavaTokenInteger*)pcToken;
				if (!pcInteger->Is(lli))
				{
					sz.Init();
					pcInteger->Print(&sz);
					Failed(LongToString(lli), (const char*)sz.Text(), "", iLine, szFile, false);
					sz.Kill();
					BREAK();
				}
			}
		}
		return pcToken->GetNext();
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaToken* PrivateAssertLiteral(CJavaTokenDefinitions* pcDefinitions, CJavaToken* pcToken, int32 i, size iLine, char* szFile)
{
	CJavaTokenInteger*	pcInteger;
	CJavaTokenLiteral*	pcLiteral;
	CChars			sz;

	AssertNotNull(pcToken);
	if (pcToken)
	{
		AssertTrue(pcToken->IsLiteral());
		if (pcToken->IsLiteral())
		{
			pcLiteral = (CJavaTokenLiteral*)pcToken;
			AssertTrue(pcLiteral->IsInteger());
			if (pcLiteral->IsInteger())
			{
				pcInteger = (CJavaTokenInteger*)pcToken;
				if (!pcInteger->Is(i))
				{
					sz.Init();
					pcInteger->Print(&sz);
					Failed(IntToString(i), (const char*)sz.Text(), "", iLine, szFile, false);
					sz.Kill();
					BREAK();
				}
			}
		}
		return pcToken->GetNext();
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaToken* PrivateAssertLiteral(CJavaTokenDefinitions* pcDefinitions, CJavaToken* pcToken, float64 d, size iLine, char* szFile)
{
	CJavaTokenFloat*		pcFloat;
	CJavaTokenLiteral*	pcLiteral;
	CChars			sz;

	AssertNotNull(pcToken);
	if (pcToken)
	{
		AssertTrue(pcToken->IsLiteral());
		if (pcToken->IsLiteral())
		{
			pcLiteral = (CJavaTokenLiteral*)pcToken;
			AssertTrue(pcLiteral->IsFloat());
			if (pcLiteral->IsFloat())
			{
				pcFloat = (CJavaTokenFloat*)pcToken;
				if (!pcFloat->Is(d))
				{
					sz.Init();
					pcFloat->Print(&sz);
					Failed(DoubleToString(d), (const char*)sz.Text(), "", iLine, szFile, false);
					sz.Kill();
					BREAK();
				}
			}
		}
		return pcToken->GetNext();
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaToken* PrivateAssertLiteral(CJavaTokenDefinitions* pcDefinitions, CJavaToken* pcToken, float32 f, size iLine, char* szFile)
{
	CJavaTokenFloat*		pcFloat;
	CJavaTokenLiteral*	pcLiteral;
	CChars			sz;

	AssertNotNull(pcToken);
	if (pcToken)
	{
		AssertTrue(pcToken->IsLiteral());
		if (pcToken->IsLiteral())
		{
			pcLiteral = (CJavaTokenLiteral*)pcToken;
			AssertTrue(pcLiteral->IsFloat());
			if (pcLiteral->IsFloat())
			{
				pcFloat = (CJavaTokenFloat*)pcToken;
				if (!pcFloat->Is(f))
				{
					sz.Init();
					pcFloat->Print(&sz);
					Failed(FloatToString(f), (const char*)sz.Text(), "", iLine, szFile, false);
					sz.Kill();
					BREAK();
				}
			}
		}
		return pcToken->GetNext();
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaToken* PrivateAssertLiteral(CJavaTokenDefinitions* pcDefinitions, CJavaToken* pcToken, char8 c, size iLine, char* szFile)
{
	CJavaTokenCharacter*	pcCharacter;
	CJavaTokenLiteral*	pcLiteral;
	CChars			sz;

	AssertNotNull(pcToken);
	if (pcToken)
	{
		AssertTrue(pcToken->IsLiteral());
		if (pcToken->IsLiteral())
		{
			pcLiteral = (CJavaTokenLiteral*)pcToken;
			AssertTrue(pcLiteral->IsCharacter());
			if (pcLiteral->IsCharacter())
			{
				pcCharacter = (CJavaTokenCharacter*)pcToken;
				if (!pcCharacter->Is(c))
				{
					sz.Init();
					pcCharacter->Print(&sz);
					Failed(CharToString(c), (const char*)sz.Text(), "", iLine, szFile, false);
					sz.Kill();
					BREAK();
				}
			}
		}
		return pcToken->GetNext();
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaToken* PrivateAssertLiteral(CJavaTokenDefinitions* pcDefinitions, CJavaToken* pcToken, char16 c, size iLine, char* szFile)
{
	CJavaTokenCharacter*		pcCharacter;
	CJavaTokenLiteral*		pcLiteral;
	CChars				sz;

	AssertNotNull(pcToken);
	if (pcToken)
	{
		AssertTrue(pcToken->IsLiteral());
		if (pcToken->IsLiteral())
		{
			pcLiteral = (CJavaTokenLiteral*)pcToken;
			AssertTrue(pcLiteral->IsCharacter());
			if (pcLiteral->IsCharacter())
			{
				pcCharacter = (CJavaTokenCharacter*)pcToken;
				if (!pcCharacter->Is(c))
				{
					sz.Init();
					pcCharacter->Print(&sz);
					Failed(CharToString(c), (const char*)sz.Text(), "", iLine, szFile, false);
					sz.Kill();
					BREAK();
				}
			}
		}
		return pcToken->GetNext();
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaToken* PrivateAssertLiteral(CJavaTokenDefinitions* pcDefinitions, CJavaToken* pcToken, bool b, size iLine, char* szFile)
{
	CJavaTokenBoolean*	pcBoolean;
	CJavaTokenLiteral*	pcLiteral;
	CChars			sz;

	AssertNotNull(pcToken);
	if (pcToken)
	{
		AssertTrue(pcToken->IsLiteral());
		if (pcToken->IsLiteral())
		{
			pcLiteral = (CJavaTokenLiteral*)pcToken;
			AssertTrue(pcLiteral->IsBoolean());
			if (pcLiteral->IsBoolean())
			{
				pcBoolean = (CJavaTokenBoolean*)pcToken;
				if (!pcBoolean->Is(b))
				{
					sz.Init();
					pcBoolean->Print(&sz);
					Failed(BoolToString(b), (const char*)sz.Text(), "", iLine, szFile, false);
					sz.Kill();
					BREAK();
				}
			}
		}
		return pcToken->GetNext();
	}

	return NULL;
}

