#include "PointerFunctions.h"
#include "TextParser.h"
#include "XMLWhitespace.h"


bool IsXMLWhitespace(char c);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SkipXMLWhitespace(CTextParser* pcTextParser)
{
	char		cCurrent;

	for (;;)
	{
		if (pcTextParser->IsOutside())
		{
			return;
		}

		cCurrent = pcTextParser->Current();

		//Nice clean white space...
		if (IsXMLWhitespace(cCurrent))
		{
			pcTextParser->StepRight();
		}

		//Possibly nasty comments...
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
bool IsXMLWhitespace(char c)
{
	return ((c == ' ') ||
			(c == '\n') ||
			(c == '\t'));
}

