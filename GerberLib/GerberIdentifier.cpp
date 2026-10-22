#include "BaseLib/PointerFunctions.h"
#include "BaseLib/TextParser.h"
#include "BaseLib/Logger.h"
#include "GerberIdentifier.h"


TRISTATE GetGerberIdentifierCharacter(CTextParser* pcTextParser, char* pc, bool bFirst);


//////////////////////////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////////////////////////
TRISTATE ParseGerberExactIdentifier(CTextParser* pcTextParser, const char* szIdentifier)
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
				tResult = GetGerberIdentifierCharacter(pcTextParser, &cCurrent, iPos == 0);
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
TRISTATE ParseGerberIdentifier(CTextParser* pcTextParser, char* szIdentifier, size* piLength)
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
			tResult = GetGerberIdentifierCharacter(pcTextParser, &c, bFirst);
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
TRISTATE ParseGerberMacroVariable(CTextParser* pcTextParser, char* szIdentifier, size* piLength)
{
	char		cCurrent;
	size		iPos;

	//A macro variable is a $ followed by digits, eg: $1.  The x following a variable is the multiply operator.
	pcTextParser->PushPosition();

	if (pcTextParser->IsOutside())
	{
		pcTextParser->PopPosition();
		pcTextParser->SetErrorEndOfFile();
		return TRIERROR;
	}

	if (pcTextParser->Current() != '$')
	{
		pcTextParser->PopPosition();
		return TRIFALSE;
	}

	if (szIdentifier)
	{
		szIdentifier[0] = '$';
	}
	pcTextParser->StepRight();
	iPos = 1;

	for (;;)
	{
		if (pcTextParser->IsOutside())
		{
			break;
		}

		cCurrent = pcTextParser->Current();
		if ((cCurrent < '0') || (cCurrent > '9'))
		{
			break;
		}

		if (szIdentifier)
		{
			szIdentifier[iPos] = cCurrent;
		}
		pcTextParser->StepRight();
		iPos++;
	}

	if (iPos == 1)
	{
		pcTextParser->PopPosition();
		return TRIFALSE;
	}

	if (szIdentifier)
	{
		szIdentifier[iPos] = 0;
	}
	pcTextParser->PassPosition();
	SafeAssign(piLength, iPos);
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE ParseGerberExactIdentifierUnset(CTextParser* pcTextParser, const char* szIdentifier)
{
	gcLogger.Error2(__METHOD__, " Gerber Identifier Parser not set.", NULL);
	return TRIERROR;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE ParseGerberdentifierUnset(CTextParser* pcTextParser, char* szIdentifier, size* piLength)
{
	gcLogger.Error2(__METHOD__, " Gerber Identifier Parser not set.", NULL);
	return TRIERROR;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE GetGerberIdentifierCharacter(CTextParser* pcTextParser, char* pc, bool bFirst)
{
	char	cCurrent;

	if (!pcTextParser->IsOutside())
	{
		cCurrent = pcTextParser->Current();
		*pc = cCurrent;
		//The first character of an identifier must be one of these...
		if (((cCurrent >= 'a') && (cCurrent <= 'z')) || 
			((cCurrent >= 'A') && (cCurrent <= 'Z')) || 
			(cCurrent == '_') || 
			(cCurrent == '.') ||
			(cCurrent == '$') ||
			(cCurrent == '&'))
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

