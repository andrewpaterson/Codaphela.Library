/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#include "PPHolder.h"
#include "PPBlock.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPToken* DuplicatePPToken(CPPToken* pcSource, CPPTokens* pcTokens)
{
	CPPDirective*			pcDirective;
	CPPLine*				pcLine;
	CPPWhiteSpace*			pcWhiteSpace;
	CPPReplacement*			pcReplacement;
	CPPText*				pcText;
	CPPTextWithSource*		pcTextWithSource;
	CPPHolder*				pcHolder;
	CPPHashes*				pcHashes;
	CPPBlock*				pcBlock;

	if (pcSource->IsDirective())
	{
		pcDirective = pcTokens->AddDirective();
		pcDirective->Copy((CPPDirective*)pcSource, pcTokens);
		return pcDirective;
	}
	else if (pcSource->IsWhiteSpace())
	{
		pcWhiteSpace = pcTokens->AddWhiteSpace();
		pcWhiteSpace->Copy((CPPWhiteSpace*)pcSource, pcTokens);
		return pcWhiteSpace;
	}
	else if (pcSource->IsReplacement())
	{
		pcReplacement = pcTokens->AddReplacement();
		pcReplacement->Copy((CPPReplacement*)pcSource, pcTokens);
		return pcReplacement;
	}
	else if (pcSource->IsText())
	{
		if (((CPPText*)pcSource)->HasSource())
		{
			pcTextWithSource = pcTokens->AddTextWithSource();
			pcTextWithSource->Copy((CPPTextWithSource*)pcSource, pcTokens);
			return pcTextWithSource;
		}
		else
		{
			pcText = pcTokens->AddText();
			pcText->Copy((CPPText*)pcSource, pcTokens);
			return pcText;
		}
	}
	else if (pcSource->IsHash())
	{
		pcHashes = pcTokens->AddHashes();
		pcHashes->Copy((CPPHashes*)pcSource, pcTokens);
		return pcHashes;
	}
	else if (pcSource->IsLine())
	{
		pcLine = pcTokens->AddLine();
		pcLine->Copy((CPPLine*)pcSource, pcTokens);
		return pcLine;
	}
	else if (pcSource->IsBlock())
	{
		pcBlock = pcTokens->AddBlock();
		pcBlock->Copy((CPPBlock*)pcSource, pcTokens);
		return pcBlock;
	}	
	else if (pcSource->IsHolder())
	{
		pcHolder = pcTokens->AddHolder();
		pcHolder->Copy((CPPHolder*)pcSource, pcTokens);
		return pcHolder;
	}
	else
	{
		gcUserError.Set("Cannot Duplicate Token");
		exit(1);
	}
	return NULL;
}

