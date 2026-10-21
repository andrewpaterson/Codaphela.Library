#include "BaseLib/PointerFunctions.h"
#include "BaseLib/TextParser.h"
#include "BaseLib/Logger.h"
#include "GerberString.h"


TRISTATE ParseGerberString(CTextParser* pcTextParser, char* szString, size* piLength, char* szEndCharacters);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE ParseGerberCommentString(CTextParser* pcTextParser, char* szString, size* piLength)
{
	return ParseGerberString(pcTextParser, szString, piLength, "*");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE ParseGerberFieldString(CTextParser* pcTextParser, char* szString, size* piLength)
{
	return ParseGerberString(pcTextParser, szString, piLength, "*,");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE ParseGerberStringUnset(CTextParser* pcTextParser, char* szString, size* piLength)
{
	gcLogger.Error2(__METHOD__, " Gerber String Parser not set.", NULL);
	return TRIERROR;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE ParseGerberString(CTextParser* pcTextParser, char* szString, size* piLength, char* szEndCharacters)
{
	size		iPos;
	char		cCurrent;
	TRISTATE	tResult;
	char		szEscape[4];
	size		uiEscapeLength;
	size		i;
	size		uiNumEndCharacters;

	uiNumEndCharacters = strlen(szEndCharacters);
	if (!pcTextParser->IsOutside())
	{
		iPos = 0;
		for (;;)
		{
			if (!pcTextParser->IsOutside())
			{
				pcTextParser->PushPosition();
				for (i = 0; i < uiNumEndCharacters; i++)
				{
					tResult = pcTextParser->GetExactCharacter(szEndCharacters[i], false);
					if (tResult == TRITRUE)
					{
						if (szString)
						{
							szString[iPos] = 0;
						}

						pcTextParser->PassPosition();

						SafeAssign(piLength, iPos);
						return TRITRUE;
					}
					else if (tResult == TRIERROR)
					{
						pcTextParser->PopPosition();
						return TRIFALSE;
					}
				}

				tResult = ParseGerberEscapeCharacter(pcTextParser, szEscape, &uiEscapeLength);
				if (tResult == TRITRUE)
				{
					if (szString)
					{
						memcpy(&szString[iPos], szEscape, uiEscapeLength);
					}
					iPos += uiEscapeLength;

					pcTextParser->PassPosition();
				}
				else if (tResult == TRIERROR)
				{
					pcTextParser->PopPosition();
					return TRIFALSE;
				}
				else if (tResult == TRIFALSE)
				{
					tResult = pcTextParser->GetCharacter(&cCurrent);
					if (tResult == TRIERROR)
					{
						pcTextParser->PopPosition();
						return TRIFALSE;
					}

					if (szString)
					{
						szString[iPos] = cCurrent;
					}
					iPos++;

					pcTextParser->PassPosition();
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
		pcTextParser->SetErrorEndOfFile();
		return TRIERROR;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE ParseGerberEscapeCharacter(CTextParser* pcTextParser, char* szUTF8, size* puiLength)
{
	TRISTATE	tResult;
	uint64		ulli;
	uint16		uiNumDigits;
	uint16		uiMaxDigits;

	//\\uXXXX is a 16 bit and \\UXXXXXXXX a 32 bit Unicode code point.  Strings are UTF-8.  3.4.3
	tResult = pcTextParser->GetExactCharacterSequence("\\u", false);
	ReturnOnError(tResult);
	
	if (tResult == TRITRUE)
	{
		uiMaxDigits = 4;
	}
	else
	{
		tResult = pcTextParser->GetExactCharacterSequence("\\U", false);
		ReturnOnErrorAndFalse(tResult);

		uiMaxDigits = 8;
	}

	tResult = pcTextParser->GetHexadecimalPart(&ulli, &uiNumDigits, uiMaxDigits);
	ReturnOnError(tResult);
	ReturnErrorOnFalse(tResult);

	if (ulli < 0x80)
	{
		szUTF8[0] = (char)ulli;
		*puiLength = 1;
	}
	else if (ulli < 0x800)
	{
		szUTF8[0] = (char)(0xC0 | (ulli >> 6));
		szUTF8[1] = (char)(0x80 | (ulli & 0x3F));
		*puiLength = 2;
	}
	else if (ulli < 0x10000)
	{
		if ((ulli >= 0xD800) && (ulli <= 0xDFFF))
		{
			//Surrogates are not characters.
			return TRIERROR;
		}
		szUTF8[0] = (char)(0xE0 | (ulli >> 12));
		szUTF8[1] = (char)(0x80 | ((ulli >> 6) & 0x3F));
		szUTF8[2] = (char)(0x80 | (ulli & 0x3F));
		*puiLength = 3;
	}
	else if (ulli <= 0x10FFFF)
	{
		szUTF8[0] = (char)(0xF0 | (ulli >> 18));
		szUTF8[1] = (char)(0x80 | ((ulli >> 12) & 0x3F));
		szUTF8[2] = (char)(0x80 | ((ulli >> 6) & 0x3F));
		szUTF8[3] = (char)(0x80 | (ulli & 0x3F));
		*puiLength = 4;
	}
	else
	{
		return TRIERROR;
	}
	return TRITRUE;
}

