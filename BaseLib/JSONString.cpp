#include "TextParser.h"
#include "JSONString.h"


//////////////////////////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////////////////////////
TRISTATE ParseJSONString(CTextParser* pcTextParser, char* szString, size* piLength)
{
	size		iPos;
	char		cCurrent;
	TRISTATE	tReturn;
	TRISTATE	tResult;
	char		cEscape;

	if (!pcTextParser->IsOutside())
	{
		tResult = pcTextParser->GetExactCharacter('"', false);
		if (tResult == TRITRUE)
		{
			iPos = 0;
			for (;;)
			{
				if (!pcTextParser->IsOutside())
				{
					cCurrent = pcTextParser->Current();
					if (cCurrent == '"')
					{
						if (szString)
							szString[iPos] = 0;

						pcTextParser->StepRight();

						SafeAssign(piLength, iPos);
						return TRITRUE;
					}
					//We have an escape character...
					else if (cCurrent == '\\')
					{
						pcTextParser->StepRight();
						tReturn = pcTextParser->GetEscapeCode(&cEscape);
						if (szString)
							szString[iPos] = cEscape;

						iPos++;
						if (tReturn == TRIFALSE)
						{
							pcTextParser->SetErrorSyntaxError();
							return TRIERROR;
						}
						else if (tReturn == TRIERROR)
						{
							//Don't set the error here, it's already been set by GetEscapeCode
							return TRIERROR;
						}
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
			return tResult;
		}
	}
	else
	{
		pcTextParser->SetErrorEndOfFile();
		return TRIERROR;
	}
}

