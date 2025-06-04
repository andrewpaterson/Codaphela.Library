#include "Logger.h"
#include "PointerFunctions.h"
#include "TextParser.h"
#include "XMLString.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool XMLParserIsAllowedTextChar(uint8 cChar)
{
	if ((cChar == '&') || (cChar == '>') || (cChar == '<'))
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE XMLParserParseTextReplacement(CTextParser* pcTextParser, CChars* pszText)
{
	TRISTATE	tResult;

	tResult = pcTextParser->GetExactCaseInsensitiveCharacterSequence("amp");
	if (tResult == TRITRUE)
	{
		pszText->Append('&');
	}
	else if (tResult == TRIERROR)
	{
		return TRIERROR;
	}
	else
	{
		tResult = tResult = pcTextParser->GetExactCaseInsensitiveCharacterSequence("gt");
		if (tResult == TRITRUE)
		{
			pszText->Append('>');
		}
		else if (tResult == TRIERROR)
		{
			return TRIERROR;
		}
		else
		{
			tResult = pcTextParser->GetExactCaseInsensitiveCharacterSequence("lt");
			if (tResult == TRITRUE)
			{
				pszText->Append('<');
			}
			else if (tResult == TRIERROR)
			{
				return TRIERROR;
			}
			else
			{
				tResult = pcTextParser->GetExactCaseInsensitiveCharacterSequence("apos");
				if (tResult == TRITRUE)
				{
					pszText->Append('\'');
				}
				else if (tResult == TRIERROR)
				{
					return TRIERROR;
				}
				else
				{
					tResult = tResult = pcTextParser->GetExactCaseInsensitiveCharacterSequence("quot");
					if (tResult == TRITRUE)
					{
						pszText->Append('"');
					}
					else
					{
						return TRIFALSE;
					}
				}
			}
		}
	}
	tResult = tResult = pcTextParser->GetExactCharacter(';', false);
	if (tResult == TRIERROR)
	{
		return TRIERROR;
	}
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////////////////////////
TRISTATE ParseXMLStringSuccess(CChars* psz, char* szString, size* piLength)
{
	if (piLength)
	{
		*piLength = psz->Length();
	}
	if (szString)
	{
		psz->CopyIntoBuffer(szString, ARRAY_ELEMENT_NOT_FOUND);
	}
	psz->Kill();

	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////////////////////////
TRISTATE ParseXMLTextString(CTextParser* pcTextParser, char* szString, size* piLength)
{
	char		c;
	TRISTATE	tResult;
	size		i;
	CChars		sz;

	sz.Init();
	for (i = 0;; i++)
	{
		tResult = pcTextParser->GetCharacter(&c);
		if (tResult != TRITRUE)
		{
			if (i == 0)
			{
				return TRIFALSE;
			}
			return ParseXMLStringSuccess(&sz, szString, piLength);
		}

		if (XMLParserIsAllowedTextChar(c))
		{
			sz.Append(c);
		}
		else
		{
			if (c == '&')
			{
				pcTextParser->PushPosition();
				tResult = XMLParserParseTextReplacement(pcTextParser, &sz);
				if (tResult == TRIERROR)
				{
					pcTextParser->PopPosition();
					sz.Kill();
					return TRIERROR;
				}
				else if (tResult == TRIFALSE)
				{
					pcTextParser->StepLeft();
					if (i == 0)
					{
						sz.Kill();
						return TRIFALSE;
					}
					return ParseXMLStringSuccess(&sz, szString, piLength);
				}
			}
			else
			{
				pcTextParser->StepLeft();
				if (i == 0)
				{
					sz.Kill();
					return TRIFALSE;
				}
				return ParseXMLStringSuccess(&sz, szString, piLength);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////////////////////////
TRISTATE ParseXMLValueString(CTextParser* pcTextParser, char* szString, size* piLength)
{
	size		iPos;
	char		cCurrent;
	TRISTATE	tSingle;
	TRISTATE	tDouble;

	if (!pcTextParser->IsOutside())
	{
		tDouble = pcTextParser->GetExactCharacter('"', false);
		if (tDouble == TRIFALSE)
		{
			tSingle = pcTextParser->GetExactCharacter('\'', false);
		}
		if ((tDouble == TRITRUE) || (tSingle == TRITRUE))
		{
			iPos = 0;
			for (;;)
			{
				if (!pcTextParser->IsOutside())
				{
					cCurrent = pcTextParser->Current();
					if (((cCurrent == '"') && (tDouble == TRITRUE)) ||
						((cCurrent == '\'') && (tSingle == TRITRUE)))
					{
						if (szString)
							szString[iPos] = 0;

						pcTextParser->StepRight();

						SafeAssign(piLength, iPos);
						return TRITRUE;
					}
					else
					{
						if (szString)
							szString[iPos] = cCurrent;

						iPos++;
						pcTextParser->StepRight();
					}
				}
				else
				{
					//This has never been tested.
					pcTextParser->SetErrorSyntaxError();
					return TRIERROR;
				}
			}
		}
		else
		{
			//No quote so not a string.
			return tDouble;
		}
	}
	else
	{
		pcTextParser->SetErrorEndOfFile();
		return TRIERROR;
	}
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE ParseXMLStringUnset(CTextParser* pcTextParser, char* szString, size* piLength)
{
	gcLogger.Error2(__METHOD__, " XML String Parser not set.", NULL);
	return TRIERROR;
}

