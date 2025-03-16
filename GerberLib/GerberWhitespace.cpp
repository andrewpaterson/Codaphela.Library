#include "BaseLib/PointerFunctions.h"
#include "BaseLib/TextParser.h"
#include "GerberWhitespace.h"


bool IsGerberWhitespace(char c);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SkipGerberWhitespace(CTextParser* pcTextParser)
{
	char	cCurrent;

	for (;;)
	{
		if (pcTextParser->IsOutside())
		{
			return;
		}

		cCurrent = pcTextParser->Current();

		//Nice clean white space...
		if (IsGerberWhitespace(cCurrent))
		{
			pcTextParser->StepRight();
		}
		else
		{
			//Was not white-space at all.
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsGerberWhitespace(char c)
{
	return ((c == ' ') ||
		(c == '\n') ||
		(c == '\t') ||
		(c == '\f') ||
		(c == '\v') ||
		(c == '\r'));
}


