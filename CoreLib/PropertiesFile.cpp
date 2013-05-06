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
#include "PropertiesFile.h"
#include "TextParser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPropertiesFile::Init(char* szFileName)
{
	mszName.Init(szFileName);
	mcProperties.Init(4, FALSE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPropertiesFile::Kill(void)
{
	mcProperties.Kill();
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CPropertiesFile::Get(char* szProperty)
{
	return mcProperties.GetWithKey(szProperty);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPropertiesFile::Set(char* szProperty, char* szValue)
{
	mcProperties.Put(szProperty, szValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPropertiesFile::Read(void)
{
	CTextFile	cFile;
	CTextParser	cParser;
	char		sz[1024];
	int			iLen;
	CChars		szValue;
	CChars		szProperty;
	TRISTATE	tResult;
	char*		szStartOfValue;

	cFile.Init();
	if (!cFile.Read(mszName.Text()))
	{
		cFile.Kill();
		return FALSE;
	}

	cFile.mcText.PassifyNewlines();

	cParser.Init(&cFile.mcText);

	for (;;)
	{
		szProperty.Init();
		szValue.Init();

		tResult = cParser.GetIdentifier(sz, &iLen);
		if (tResult == TRITRUE)
		{
			szProperty.Append(sz);
		}
		else if (tResult == TRIERROR)
		{
			szProperty.Kill();
			szValue.Kill();
			break;
		}

		tResult = cParser.GetExactCharacter('=');
		if (tResult == TRITRUE) 
		{
			cParser.SkipWhiteSpace();
			szStartOfValue = cParser.mszParserPos;
			tResult = cParser.FindEndOfLine();

			if (tResult == TRITRUE)
			{
				if (szStartOfValue < cParser.mszParserPos)
				{
					szValue.AppendSubString(szStartOfValue, cParser.mszParserPos);
				}
				if (szProperty.Length() > 0)
				{
					mcProperties.Put(&szProperty, &szValue);
				}
			}
			else
			{
				szProperty.Kill();
				szValue.Kill();
				break;
			}
		}
		else
		{
			szProperty.Kill();
			szValue.Kill();
			break;
		}
		szProperty.Kill();
		szValue.Kill();
	}

	cParser.Kill();
	cFile.Kill();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPropertiesFile::Write(void)
{
	CTextFile	cFile;
	int			i;
	CChars*		pszProperty;
	CChars*		pszValue;

	cFile.Init();

	for (i = 0; i < mcProperties.NumElements(); i++)
	{
		mcProperties.GetAtIndex(i, &pszProperty, &pszValue);
		cFile.mcText.Append(pszProperty);
		if (pszValue->Length() > 0)
		{
			cFile.mcText.Append(" = ");
			cFile.mcText.Append(pszValue);
		}
		cFile.mcText.AppendNewLine();
	}

	if (!cFile.Write(mszName.Text()))
	{
		cFile.Kill();
		return FALSE;
	}
	cFile.Kill();
	return TRUE;
}

