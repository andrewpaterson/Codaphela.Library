#include "PointerFunctions.h"
#include "TextParser.h"
#include "JSONWhitespace.h"


bool IsJSONWhitespace(char c);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SkipJSONWhitespace(CTextParser* pcTextParser)
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
		if (IsJSONWhitespace(cCurrent))
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
bool IsJSONWhitespace(char c)
{
	return ((c == ' ') ||
		(c == '\n') ||
		(c == '\t') ||
		(c == '\f') ||
		(c == '\v') ||
		(c == '\r'));
}

