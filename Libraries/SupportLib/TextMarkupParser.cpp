/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "TextMarkupParser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void STextStyleRun::Init(size uiStart, size uiLength, STextStyleOverride* psOverride)
{
	this->uiStart = uiStart;
	this->uiLength = uiLength;
	sOverride = *psOverride;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextMarkupParser::Init(void)
{
	mpszText = NULL;
	mpasRuns = NULL;
	masGroups.Init();
	muiRunStart = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextMarkupParser::Kill(void)
{
	masGroups.Kill();
	mpasRuns = NULL;
	mpszText = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The plain text is appended to pszText and its runs added to pasRuns
// (their positions are in pszText, after what was already in it).  False
// if a group wasn't closed by the end of the markup; its text is still in
// its style.
//
//////////////////////////////////////////////////////////////////////////
bool CTextMarkupParser::Parse(const char* szMarkup, CChars* pszText, CArrayTextStyleRun* pasRuns)
{
	size				uiPosition;
	size				uiLength;
	STextMarkupGroup*	psGroup;
	STextStyleOverride	sOverride;
	int					iKind;
	bool				bClosed;

	mpszText = pszText;
	mpasRuns = pasRuns;
	masGroups.ReInit();
	muiRunStart = mpszText->Length();
	if (szMarkup == NULL)
	{
		return true;
	}

	uiPosition = 0;
	while (szMarkup[uiPosition] != '\0')
	{
		if (masGroups.NumElements() > 0)
		{
			uiLength = ReadClose(&szMarkup[uiPosition], masGroups.Get(masGroups.NumElements() - 1)->iKind);
			if (uiLength > 0)
			{
				EndRun();
				masGroups.RemoveTail();
				uiPosition += uiLength;
				continue;
			}
		}

		uiLength = ReadEscape(&szMarkup[uiPosition], mpszText);
		if (uiLength > 0)
		{
			uiPosition += uiLength;
			continue;
		}

		sOverride.Init();
		uiLength = ReadOpen(&szMarkup[uiPosition], &sOverride, &iKind);
		if (uiLength > 0)
		{
			EndRun();
			psGroup = masGroups.Add();
			psGroup->sOverride = *GetOverride(masGroups.NumElements() - 1);
			sOverride.ApplyTo(&psGroup->sOverride);
			psGroup->iKind = iKind;
			uiPosition += uiLength;
			continue;
		}

		mpszText->Append(szMarkup[uiPosition]);
		uiPosition++;
	}

	EndRun();
	bClosed = masGroups.NumElements() == 0;
	masGroups.ReInit();
	mpszText = NULL;
	mpasRuns = NULL;
	return bClosed;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTextMarkupParser::ReadEscape(const char* szMarkup, CChars* pszText)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The plain text since the last run ended is a run in the innermost
// group's changes.
//
//////////////////////////////////////////////////////////////////////////
void CTextMarkupParser::EndRun(void)
{
	size	uiEnd;

	uiEnd = mpszText->Length();
	if (uiEnd > muiRunStart)
	{
		mpasRuns->Add()->Init(muiRunStart, uiEnd - muiRunStart, GetOverride(masGroups.NumElements()));
	}
	muiRunStart = uiEnd;
}


//////////////////////////////////////////////////////////////////////////
//
// The changes of the innermost of the first uiNumGroups groups: nothing
// if there are none.
//
//////////////////////////////////////////////////////////////////////////
STextStyleOverride* CTextMarkupParser::GetOverride(size uiNumGroups)
{
	if (uiNumGroups == 0)
	{
		msNone.Init();
		return &msNone;
	}
	return &masGroups.Get(uiNumGroups - 1)->sOverride;
}

