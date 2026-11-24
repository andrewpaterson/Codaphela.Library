/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "TextParser.h"
#include "YAMLWhitespace.h"


//////////////////////////////////////////////////////////////////////////
//
// YAML's white space between the parts of a line: spaces and tabs.  Line
// ends are not skipped because they end values and indentation means
// structure; comments are not skipped because a # is only a comment
// after white space (CYAMLParser decides).
//
//////////////////////////////////////////////////////////////////////////
void SkipYAMLWhitespace(CTextParser* pcTextParser)
{
	char	cCurrent;

	for (;;)
	{
		if (pcTextParser->IsOutside())
		{
			return;
		}

		cCurrent = pcTextParser->Current();
		if (IsYAMLWhitespace(cCurrent))
		{
			pcTextParser->StepRight();
		}
		else
		{
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsYAMLWhitespace(char c)
{
	return (c == ' ') || (c == '\t');
}

