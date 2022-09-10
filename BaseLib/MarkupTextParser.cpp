/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#include "BaseLib/Logger.h"
#include "BaseLib/TextParser.h"
#include "MarkupTextParser.h"



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupTag* CMarkupTextParser::GetTag(CMarkupTag* pcParentTag, char* szTagName, bool bLogErrors)
{
	CMarkupTag* pcTag;

	pcTag = pcParentTag->GetTag(szTagName);
	if ((pcTag == NULL) && (bLogErrors))
	{
		LogErrorTagNotFound(pcParentTag, szTagName);
	}
	return pcTag;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMarkupTextParser::ReadFloat(CMarkupTag* pcFloatTag, float* pcFloat, bool bLogErrors)
{
	CChars			szFloat;
	CTextParser		cTextParser;
	double 			fValue;
	TRISTATE		tResult;

	szFloat.Init();
	pcFloatTag->GetText(&szFloat);
	cTextParser.Init(&szFloat);
	tResult = cTextParser.GetFloat(&fValue);
	cTextParser.Kill();
	if (tResult != TRITRUE)
	{
		if (bLogErrors)
		{
			LogError(pcFloatTag, "Tag body could not be parsed as a float.");
		}
		szFloat.Kill();
		return false;
	}
	*pcFloat = (float)fValue;
	szFloat.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMarkupTextParser::ReadInteger(CMarkupTag* pcIntTag, int* pcInt, bool bLogErrors)
{
	CChars					szInt;
	CTextParser				cTextParser;
	TRISTATE				tResult;

	szInt.Init();
	pcIntTag->GetText(&szInt);
	cTextParser.Init(&szInt);
	tResult = cTextParser.GetInteger(pcInt);
	cTextParser.Kill();
	if (tResult != TRITRUE)
	{
		if (bLogErrors)
		{
			LogError(pcIntTag, "Tag body could not be parsed as an integer.");
		}

		szInt.Kill();
		return false;
	}
	szInt.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMarkupTextParser::ReadBool(CMarkupTag* pcIntTag, bool* pcBool, bool bLogErrors)
{
	CChars					szBool;
	CTextParser				cTextParser;
	TRISTATE				tResult;

	szBool.Init();
	pcIntTag->GetText(&szBool);
	cTextParser.Init(&szBool);
	tResult = cTextParser.GetExactCaseInsensitiveCharacterSequence("true");
	if (tResult = TRITRUE)
	{
		*pcBool = true;
	}
	else if (tResult == TRIERROR)
	{
		if (bLogErrors)
		{
			LogError(pcIntTag, "Tag body could not be parsed as a boolean.");
		}
		szBool.Kill();
		return false;
	}

	tResult = cTextParser.GetExactCaseInsensitiveCharacterSequence("false");
	if (tResult = TRITRUE)
	{
		*pcBool = false;
	}
	else
	{
		if (bLogErrors)
		{
			LogError(pcIntTag, "Tag body could not be parsed as a boolean.");
		}
		szBool.Kill();
		return false;
	}

	cTextParser.Kill();
	szBool.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupTextParser::LogError(CMarkupTag* pcTag, char* szError)
{
	CChars	szError2;

	szError2.Init();
	TagToError(&szError2, pcTag);
	szError2.Append(" ");
	szError2.Append(szError);
	gcLogger.Error(szError2.Text());
	szError2.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupTextParser::LogErrorTagNotFound(CMarkupTag* pcParentTag, char* szExpectedTag)
{
	CChars	szError;

	szError.Init();
	TagToError(&szError, pcParentTag);
	szError.Append(" Tag '");
	szError.Append(szExpectedTag);
	szError.Append("' not found.");
	gcLogger.Error(szError.Text());
	szError.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupTextParser::LogErrorTagWasEmpty(CMarkupTag* pcTag)
{
	CChars	szError;

	szError.Init();
	TagToError(&szError, pcTag);
	szError.Append(" Tag body did not contain text.");
	gcLogger.Error(szError.Text());
	szError.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupTextParser::TagToError(CChars* psz, CMarkupTag* pcParentTag)
{
	CMarkupTag* pcTag;
	CChars		szTemp;

	if (pcParentTag)
	{
		pcTag = pcParentTag;
		while (pcTag)
		{
			psz->Insert(0, &pcTag->mszName);
			psz->Insert(0, '.');
			pcTag = pcTag->mpcParent;
		}
		if (!psz->Empty())
		{
			psz->RemoveCharacter(0);
		}

		szTemp.Init("XML Line ");
		szTemp.Append(pcParentTag->miLine + 1);  //miLine is zero indexed.
		szTemp.Append(", ");
		psz->Insert(0, &szTemp);
		szTemp.Kill();
	}
}

