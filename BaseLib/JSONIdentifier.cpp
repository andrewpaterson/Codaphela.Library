#include "PointerFunctions.h"
#include "TextParser.h"
#include "JSONIdentifier.h"


TRISTATE GetJSONIdentifierCharacter(CTextParser* pcTextParser, char* pc, bool bFirst);


//////////////////////////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////////////////////////
TRISTATE ParseJSONExactIdentifier(CTextParser* pcTextParser, char* szIdentifier)
{
	char		cCurrent;
	size		iPos;
	TRISTATE	tResult;

	iPos = 0;
	pcTextParser->PushPosition();

	//Make sure we're not off the end of the file.
	if (pcTextParser->IsOutside())
	{
		pcTextParser->PopPosition();
		pcTextParser->SetErrorEndOfFile();
		return TRIERROR;
	}

	for (;;)
	{
		if (szIdentifier[iPos] == 0)
		{
			//Got all the way to the NULL character.
			//If there are additional identifier characters then we do not have the right identifier.
			if (!pcTextParser->IsOutside())
			{
				tResult = GetJSONIdentifierCharacter(pcTextParser, &cCurrent, iPos == 0);
				if (tResult == TRITRUE)
				{
					//Put the parser back where it was.
					pcTextParser->PopPosition();
					return TRIFALSE;
				}
			}
			pcTextParser->PassPosition();
			return TRITRUE;
		}

		if (!pcTextParser->IsOutside())
		{
			cCurrent = pcTextParser->Current();
			if (cCurrent == szIdentifier[iPos])
			{
				pcTextParser->StepRight();
				iPos++;
			}
			else
			{
				//Put the parser back where it was.
				pcTextParser->PopPosition();
				return TRIFALSE;
			}
		}
		else
		{
			//Put the parser back where it was.
			pcTextParser->PopPosition();
			return TRIFALSE;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE ParseJSONIdentifier(CTextParser* pcTextParser, char* szIdentifier, size* piLength)
{
	char		c;
	bool		bFirst;
	size		iPos;
	TRISTATE	tResult;

	bFirst = true;
	iPos = 0;
	pcTextParser->PushPosition();

	//Make sure we're not off the end of the file.
	if (pcTextParser->IsOutside())
	{
		pcTextParser->PopPosition();
		pcTextParser->SetErrorEndOfFile();
		return TRIERROR;
	}

	for (;;)
	{
		if (!pcTextParser->IsOutside())
		{
			tResult = GetJSONIdentifierCharacter(pcTextParser, &c, bFirst);
			if (tResult != TRITRUE)
			{
				if (bFirst)
				{
					if (szIdentifier)
					{
						szIdentifier[iPos] = 0;
					}

					pcTextParser->PopPosition();
					return TRIFALSE;
				}
				else
				{
					if (szIdentifier)
					{
						szIdentifier[iPos] = 0;
					}

					pcTextParser->PassPosition();

					SafeAssign(piLength, iPos);
					return TRITRUE;
				}
			}
			else
			{
				if (szIdentifier)
				{
					szIdentifier[iPos] = c;
				}
			}
		}
		else
		{
			if (bFirst)
			{
				pcTextParser->PopPosition();
				pcTextParser->SetErrorEndOfFile();
				return TRIERROR;
			}
			else
			{
				if (szIdentifier)
				{
					szIdentifier[iPos] = 0;
				}

				pcTextParser->PassPosition();

				SafeAssign(piLength, iPos);
				return TRITRUE;
			}
		}
		bFirst = false;
		iPos++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE GetJSONIdentifierCharacter(CTextParser* pcTextParser, char* pc, bool bFirst)
{
	char	cCurrent;

	if (!pcTextParser->IsOutside())
	{
		cCurrent = pcTextParser->Current();
		*pc = cCurrent;
		//The first character of an identifier must be one of these...
		if (((cCurrent >= 'a') && (cCurrent <= 'z')) || ((cCurrent >= 'A') && (cCurrent <= 'Z')) || (cCurrent == '_'))
		{
			pcTextParser->StepRight();
			return TRITRUE;
		}

		//Additional characters can also be...
		if (!bFirst)
		{
			if ((cCurrent >= '0') && (cCurrent <= '9'))
			{
				pcTextParser->StepRight();
				return TRITRUE;
			}
		}
		return TRIFALSE;
	}
	else
	{
		pcTextParser->SetErrorEndOfFile();
		return TRIERROR;
	}
}

