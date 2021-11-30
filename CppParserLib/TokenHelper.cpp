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
CPPToken* DuplicatePPToken(CPPToken* pcSource, CMemoryStackExtended* pcStack)
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
		pcDirective = CPPDirective::Construct(pcStack->Add(sizeof(CPPDirective)));
		pcDirective->Copy((CPPDirective*)pcSource, pcStack);
		return pcDirective;
	}
	else if (pcSource->IsWhiteSpace())
	{
		pcWhiteSpace = CPPWhiteSpace::Construct(pcStack->Add(sizeof(CPPWhiteSpace)));
		pcWhiteSpace->Copy((CPPWhiteSpace*)pcSource, pcStack);
		return pcWhiteSpace;
	}
	else if (pcSource->IsReplacement())
	{
		pcReplacement = CPPReplacement::Construct(pcStack->Add(sizeof(CPPReplacement)));
		pcReplacement->Copy((CPPReplacement*)pcSource, pcStack);
		return pcReplacement;
	}
	else if (pcSource->IsText())
	{
		if (((CPPText*)pcSource)->HasSource())
		{
			pcTextWithSource = CPPTextWithSource::Construct(pcStack->Add(sizeof(CPPTextWithSource)));
			pcTextWithSource->Copy((CPPTextWithSource*)pcSource, pcStack);
			return pcTextWithSource;
		}
		else
		{
			pcText = CPPText::Construct(pcStack->Add(sizeof(CPPText)));
			pcText->Copy((CPPText*)pcSource, pcStack);
			return pcText;
		}
	}
	else if (pcSource->IsHash())
	{
		pcHashes = CPPHashes::Construct(pcStack->Add(sizeof(CPPHashes)));
		pcHashes->Copy((CPPHashes*)pcSource, pcStack);
		return pcHashes;
	}
	else if (pcSource->IsLine())
	{
		pcLine = CPPLine::Construct(pcStack->Add(sizeof(CPPLine)));
		pcLine->Copy((CPPLine*)pcSource, pcStack);
		return pcLine;
	}
	else if (pcSource->IsBlock())
	{
		pcBlock = CPPBlock::Construct(pcStack->Add(sizeof(CPPBlock)));
		pcBlock->Copy((CPPBlock*)pcSource, pcStack);
		return pcBlock;
	}	
	else if (pcSource->IsHolder())
	{
		pcHolder = CPPHolder::Construct(pcStack->Add(sizeof(CPPHolder)));
		pcHolder->Copy((CPPHolder*)pcSource, pcStack);
		return pcHolder;
	}
	else
	{
		gcUserError.Set("Cannot Duplicate Token");
		exit(1);
	}
	return NULL;
}

