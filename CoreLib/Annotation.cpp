/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "TextParser.h"
#include "Annotation.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnnotation::Init(void)
{
	mszName.Init();
	mmszszParameters.Init(4);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnnotation::Init(char* szSource)
{
	CTextParser	cParser;
	char		szIdentifier[1024];
	char		szValue[1024];
	TRISTATE	tResult;

	Init();

	cParser.Init(szSource);
	tResult = cParser.GetIdentifier(szIdentifier);
	if (tResult != TRITRUE)
	{
		cParser.Kill();
		return;
	}
	mszName.Init(szIdentifier);
	
	tResult = cParser.GetExactCharacter(':');
	if (tResult == TRITRUE)
	{
		for (;;)
		{
			tResult = cParser.GetIdentifier(szIdentifier);
			if (tResult != TRITRUE)
			{
				break;
			}
			tResult = cParser.GetExactCharacter('=');
			if (tResult == TRITRUE)
			{
				tResult = cParser.GetCharacterSequence(szValue);
				if (tResult != TRITRUE)
				{
					szValue[0] = '\0';
				}
			}
			else
			{
				szValue[0] = '\0';
			}
			mmszszParameters.Put(szIdentifier, szValue);
		}
	}

	cParser.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnnotation::Kill(void)
{
	mmszszParameters.Kill();
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CAnnotation::GetParameter(char* szKey)
{
	//This returns the empty string ("") if the keys exists but has no value.
	return mmszszParameters.GetWithKey(szKey);
}

