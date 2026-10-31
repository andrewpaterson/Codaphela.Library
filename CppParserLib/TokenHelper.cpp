/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "TokenHelper.h"
#include "PPDirective.h"
#include "PPLine.h"
#include "PPWhiteSpace.h"
#include "PPReplacement.h"
#include "PPText.h"
#include "PPTextWithSource.h"
#include "PPHashes.h"
#include "PPTokenReplacementsHolder.h"
#include "PPBlock.h"
#include "BaseLib/ErrorHandler.h"


//////////////////////////////////////////////////////////////////////////
//
// Copies pcSource, which must be an M, into a new M in pcTokens.
//
//////////////////////////////////////////////////////////////////////////
template<class M>
static CPPToken* DuplicateTypedPPToken(CPPToken* pcSource, CPPTokenMemory* pcTokens)
{
	M*	pcToken;

	pcToken = pcTokens->AddToken<M>();
	pcToken->Copy((M*)pcSource, pcTokens);
	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
// Copies pcSource into pcTokens.  Returns NULL, with gcUserError set, if
// pcSource is not a kind of token that can be copied.
//
//////////////////////////////////////////////////////////////////////////
CPPToken* DuplicatePPToken(CPPToken* pcSource, CPPTokenMemory* pcTokens)
{
	if (pcSource->IsDirective())
	{
		return DuplicateTypedPPToken<CPPDirective>(pcSource, pcTokens);
	}
	else if (pcSource->IsWhitespace())
	{
		return DuplicateTypedPPToken<CPPWhitespace>(pcSource, pcTokens);
	}
	else if (pcSource->IsReplacement())
	{
		return DuplicateTypedPPToken<CPPReplacement>(pcSource, pcTokens);
	}
	else if (pcSource->IsText())
	{
		if (((CPPText*)pcSource)->HasSource())
		{
			return DuplicateTypedPPToken<CPPTextWithSource>(pcSource, pcTokens);
		}
		else
		{
			return DuplicateTypedPPToken<CPPText>(pcSource, pcTokens);
		}
	}
	else if (pcSource->IsHash())
	{
		return DuplicateTypedPPToken<CPPHashes>(pcSource, pcTokens);
	}
	else if (pcSource->IsLine())
	{
		return DuplicateTypedPPToken<CPPLine>(pcSource, pcTokens);
	}
	else if (pcSource->IsBlock())
	{
		return DuplicateTypedPPToken<CPPBlock>(pcSource, pcTokens);
	}
	else if (pcSource->IsTokenReplacementsHolder())
	{
		return DuplicateTypedPPToken<CPPTokenReplacementsHolder>(pcSource, pcTokens);
	}
	else
	{
		gcUserError.Set("Cannot Duplicate Token");
		return NULL;
	}
}

