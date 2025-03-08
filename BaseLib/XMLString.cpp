#include "PointerFunctions.h"
#include "TextParser.h"
#include "XMLString.h"


//////////////////////////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////////////////////////
TRISTATE ParseXMLString(CTextParser* pcTextParser, char* szString, size* piLength)
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

