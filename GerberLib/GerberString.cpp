#include "BaseLib/PointerFunctions.h"
#include "BaseLib/TextParser.h"
#include "GerberString.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE ParseGerberString(CTextParser* pcTextParser, char* szString, size* piLength)
{
	size		iPos;
	char		cCurrent;
	TRISTATE	tResult;
	char		cEscape;

	if (!pcTextParser->IsOutside())
	{
		iPos = 0;
		for (;;)
		{
			if (!pcTextParser->IsOutside())
			{
				pcTextParser->PushPosition();
				tResult = pcTextParser->GetExactCharacter('*', false);
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

				tResult = ParseGerberEscapeCharacter(pcTextParser, &cEscape);
				if (tResult == TRITRUE)
				{
					if (szString)
					{
						szString[iPos] = cEscape;
					}
					iPos++;

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
TRISTATE ParseGerberEscapeCharacter(CTextParser* pcTextParser, char* szEscape)
{
	TRISTATE	tResult;
	uint64		ulli;
	uint16		uiNumDigits;
	uint16		uiMaxDigits;

	tResult = pcTextParser->GetExactCaseInsensitiveCharacterSequence("\\u", false);
	ReturnOnError(tResult);
	
	if (tResult == TRITRUE)
	{
		uiMaxDigits = 4;
	}
	else if (tResult == TRIFALSE)
	{
		tResult = pcTextParser->GetExactCaseInsensitiveCharacterSequence("\\U", false);
		ReturnOnErrorAndFalse(tResult);

		uiMaxDigits = 8;
	}

	tResult = pcTextParser->GetHexadecimalPart(&ulli, &uiNumDigits, uiMaxDigits);
	ReturnOnError(tResult);
	ReturnErrorOnFalse(tResult);

	*szEscape = (char)ulli;
	
	return TRITRUE;
}

