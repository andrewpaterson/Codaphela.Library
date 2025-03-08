#include "PointerFunctions.h"
#include "TextParser.h"
#include "CPPWhitespace.h"


bool SkipCStyleComment(CTextParser* pcTextParser, char** pszBegin = NULL, char** pszEnd = NULL);
void SkipLeftCStyleComment(CTextParser* pcTextParser);
bool SkipCPPStyleComment(CTextParser* pcTextParser, char** pszBegin = NULL, char** pszEnd = NULL);
bool IsCPPWhitespace(char c);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SkipCPPWhitespace(CTextParser* pcTextParser)
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
		if (IsCPPWhitespace(cCurrent))
		{
			pcTextParser->StepRight();
		}

		//Possibly nasty comments...
		else if (cCurrent == '/')
		{
			pcTextParser->StepRight();

			if (!pcTextParser->IsOutside())
			{
				cCurrent = pcTextParser->Current();
				if (cCurrent == '*')
				{
					//Put the parser back where it was.
					pcTextParser->StepLeft();
					if (!SkipCStyleComment(pcTextParser))
					{
						break;
					}
				}
				else if (cCurrent == '/')
				{
					//Put the parser back where it was.
					pcTextParser->StepLeft();
					if (!SkipCPPStyleComment(pcTextParser))
					{
						break;
					}
				}
				else
				{
					//Was something other than white-space starting with /
					pcTextParser->StepLeft();
					break;
				}
			}
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
bool IsCPPWhitespace(char c)
{
	return ((c == ' ')  || 
			(c == '\n') || 
			(c == '\t') ||
			(c == '\f') ||
			(c == '\v') ||
			(c == '\r'));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SkipCStyleComment(CTextParser* pcTextParser, char** pszBegin, char** pszEnd)
{
	char	cCurrent;
	size	iDepth;

	iDepth = 0;

	SafeAssign(pszBegin, NULL);
	SafeAssign(pszEnd, NULL);

	pcTextParser->PushPosition();
	for (;;)
	{
		if (pcTextParser->IsOutside())
		{
			pcTextParser->PassPosition();
			return true;
		}

		cCurrent = pcTextParser->Current();
		if (cCurrent == '/')
		{
			pcTextParser->StepRight();
			if (!pcTextParser->IsOutside())
			{
				cCurrent = pcTextParser->Current();
				if (cCurrent == '*')
				{
					iDepth++;
					if (iDepth == 1)
					{
						SafeAssign(pszBegin, pcTextParser->Position(1));
					}
				}
				else
				{
					//Wasn't a comment start... step back.
					pcTextParser->StepLeft();
				}
			}
			else
			{
				pcTextParser->PassPosition();
				return true;
			}
		}
		else if (cCurrent == '*')
		{
			pcTextParser->StepRight();
			if (!pcTextParser->IsOutside())
			{
				cCurrent = pcTextParser->Current();
				if (cCurrent == '/')
				{
					SafeAssign(pszEnd, pcTextParser->Position(-2));

					pcTextParser->StepRight();
					return true;
				}
				else
				{
					//Wasn't the end of a comment... step back...
					pcTextParser->StepLeft();
				}
			}
			else
			{
				pcTextParser->PassPosition();
				return true;
			}
		}

		if (iDepth == 0)
		{
			//No more nested comments...  bail..
			return true;
		}
		pcTextParser->StepRight();
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SkipLeftCStyleComment(CTextParser* pcTextParser)
{
	char	cCurrent;
	size	iDepth;

	iDepth = 0;

	pcTextParser->PushPosition();
	for (;;)
	{
		if (pcTextParser->IsOutside())
		{
			pcTextParser->PassPosition();
			return;
		}

		cCurrent = pcTextParser->Current();
		if (cCurrent == '/')
		{
			pcTextParser->StepLeft();
			if (!pcTextParser->IsOutside())
			{
				cCurrent = pcTextParser->Current();
				if (cCurrent == '*')
				{
					iDepth++;
				}
				else
				{
					//Wasn't a comment start... step back.
					pcTextParser->StepRight();
				}
			}
			else
			{
				pcTextParser->PassPosition();
				return;
			}
		}
		else if (cCurrent == '*')
		{
			pcTextParser->StepLeft();
			if (!pcTextParser->IsOutside())
			{
				cCurrent = pcTextParser->Current();
				if (cCurrent == '/')
				{
					iDepth--;
				}
				else
				{
					//Wasn't the end of a comment... step back...
					pcTextParser->StepRight();
				}
			}
			else
			{
				pcTextParser->PassPosition();
				return;
			}
		}

		if (iDepth == 0)
		{
			//No more nested comments...  bail..
			return;
		}
		pcTextParser->StepLeft();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SkipCPPStyleComment(CTextParser* pcTextParser, char** pszBegin, char** pszEnd)
{
	char	cCurrent;
	size	iCount;

	if (pcTextParser->IsOutside())
	{
		return true;
	}

	pcTextParser->PushPosition();
	cCurrent = pcTextParser->Current();

	SafeAssign(pszBegin, NULL);
	SafeAssign(pszEnd, NULL);

	if (cCurrent == '/')
	{
		pcTextParser->StepRight();
		if (!pcTextParser->IsOutside())
		{
			cCurrent = pcTextParser->Current();
			if (cCurrent == '/')
			{
				SafeAssign(pszBegin, pcTextParser->Position(1));

				for (iCount = 0;; iCount++)
				{
					pcTextParser->StepRight();
					if (!pcTextParser->IsOutside())
					{
						cCurrent = pcTextParser->Current();

						if (cCurrent == '\n')
						{
							SafeAssign(pszEnd, pcTextParser->Position(-1));

							//This is the end of the line and the end of the comment.
							pcTextParser->StepRight();
							pcTextParser->PassPosition();
							return true;
						}
					}
					else
					{
						SafeAssign(pszEnd, pcTextParser->End());

						pcTextParser->PassPosition();
						return true;
					}
				}
			}
			else
			{
				pcTextParser->PopPosition();
				return true;
			}
		}
		else
		{
			//Wasn't a comment.
			pcTextParser->StepLeft();
			return true;
		}
	}
	pcTextParser->PopPosition();
	return true;
}

