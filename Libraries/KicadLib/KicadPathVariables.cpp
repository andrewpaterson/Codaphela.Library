/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela KicadLib

Codaphela KicadLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela KicadLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela KicadLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <stdlib.h>
#include <string.h>
#include "BaseLib/FileUtil.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/Logger.h"
#include "BaseLib/StringHelper.h"
#include "KicadPathVariables.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadPathVariables::Init(void)
{
	maszNames.Init();
	maszValues.Init();
	maszKicadNames.Init();
	maszKicadValues.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadPathVariables::Kill(void)
{
	maszKicadValues.Kill();
	maszKicadNames.Kill();
	maszValues.Kill();
	maszNames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Comes before the environment's and KiCad's.
//
//////////////////////////////////////////////////////////////////////////
void CKicadPathVariables::Set(const char* szName, const char* szValue)
{
	size	uiIndex;

	uiIndex = maszNames.GetIndex(szName);
	if (uiIndex != ARRAY_ELEMENT_NOT_FOUND)
	{
		maszValues.Get(uiIndex)->Set(szValue);
		return;
	}
	maszNames.Add(szName);
	maszValues.Add(szValue);
}


//////////////////////////////////////////////////////////////////////////
//
// The variable's value, or NULL if none of the three has it.
//
//////////////////////////////////////////////////////////////////////////
const char* CKicadPathVariables::Get(const char* szName)
{
	size		uiIndex;
	const char*	szValue;

	uiIndex = maszNames.GetIndex(szName);
	if (uiIndex != ARRAY_ELEMENT_NOT_FOUND)
	{
		return maszValues.GetText(uiIndex);
	}
	szValue = getenv(szName);
	if (szValue != NULL)
	{
		return szValue;
	}
	uiIndex = maszKicadNames.GetIndex(szName);
	if (uiIndex != ARRAY_ELEMENT_NOT_FOUND)
	{
		return maszKicadValues.GetText(uiIndex);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The kicad_common.json of the newest KiCad whose settings are in the
// user's settings directory: %APPDATA%\kicad\<version> on Windows,
// $XDG_CONFIG_HOME/kicad/<version> (~/.config) elsewhere.  False if
// there's none.
//
//////////////////////////////////////////////////////////////////////////
bool CKicadPathVariables::LoadKicadCommon(void)
{
	CChars		szDirectory;
	CChars		szFileName;
	CFileUtil	cFileUtil;
	CArrayChars	aszVersions;
	CChars		szVersion;
	CChars		szIgnored;
	size		i;
	float64		fVersion;
	float64		fNewest;
	size		uiNewest;
	const char*	szBase;
	bool		bResult;

	szDirectory.Init();
	szBase = getenv("APPDATA");
	if (szBase != NULL)
	{
		szDirectory.Append(szBase);
	}
	else if (getenv("XDG_CONFIG_HOME") != NULL)
	{
		szDirectory.Append(getenv("XDG_CONFIG_HOME"));
	}
	else if (getenv("HOME") != NULL)
	{
		szDirectory.Append(getenv("HOME"));
		cFileUtil.AppendToPath(&szDirectory, ".config");
	}
	else
	{
		szDirectory.Kill();
		return false;
	}
	cFileUtil.AppendToPath(&szDirectory, "kicad");

	aszVersions.Init();
	cFileUtil.FindAllDirectories(szDirectory.Text(), &aszVersions);
	uiNewest = ARRAY_ELEMENT_NOT_FOUND;
	fNewest = 0;
	for (i = 0; i < aszVersions.NumElements(); i++)
	{
		szVersion.Init();
		szIgnored.Init();
		cFileUtil.SplitPath(aszVersions.GetText(i), &szVersion, &szIgnored);
		fVersion = atof(szVersion.Text());
		if (fVersion > fNewest)
		{
			fNewest = fVersion;
			uiNewest = i;
		}
		szIgnored.Kill();
		szVersion.Kill();
	}

	bResult = false;
	if (uiNewest != ARRAY_ELEMENT_NOT_FOUND)
	{
		szFileName.Init(aszVersions.GetText(uiNewest));
		cFileUtil.AppendToPath(&szFileName, "kicad_common.json");
		if (cFileUtil.Exists(szFileName.Text()))
		{
			bResult = LoadKicadCommon(szFileName.Text());
		}
		szFileName.Kill();
	}
	aszVersions.Kill();
	szDirectory.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CKicadPathVariables::LoadKicadCommon(const char* szFileName)
{
	CTextFile	cFile;
	bool		bResult;

	cFile.Init();
	bResult = cFile.Read(szFileName);
	if (bResult)
	{
		bResult = ReadKicadCommon(cFile.Text());
	}
	cFile.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static const char* SkipKicadJSONSpace(const char* sz)
{
	while ((*sz == ' ') || (*sz == '\t') || (*sz == '\r') || (*sz == '\n'))
	{
		sz++;
	}
	return sz;
}


//////////////////////////////////////////////////////////////////////////
//
// A JSON string at sz (its opening quote) into psz.  Returns the end of
// it, past its closing quote, or NULL if it isn't one.  A \u escape
// past ASCII becomes '?'.
//
//////////////////////////////////////////////////////////////////////////
static const char* ReadKicadJSONString(const char* sz, CChars* psz)
{
	char	c;
	int		iCode;
	char	szHex[5];

	if (*sz != '"')
	{
		return NULL;
	}
	sz++;
	for (;;)
	{
		c = *sz;
		if (c == '\0')
		{
			return NULL;
		}
		sz++;
		if (c == '"')
		{
			return sz;
		}
		if (c != '\\')
		{
			psz->Append(c);
			continue;
		}
		c = *sz;
		if (c == '\0')
		{
			return NULL;
		}
		sz++;
		switch (c)
		{
		case 'n':	psz->Append('\n');	break;
		case 't':	psz->Append('\t');	break;
		case 'r':	psz->Append('\r');	break;
		case 'b':	psz->Append('\b');	break;
		case 'f':	psz->Append('\f');	break;
		case 'u':
			if ((sz[0] == '\0') || (sz[1] == '\0') || (sz[2] == '\0') || (sz[3] == '\0'))
			{
				return NULL;
			}
			memcpy(szHex, sz, 4);
			szHex[4] = '\0';
			iCode = (int)strtol(szHex, NULL, 16);
			psz->Append((iCode > 0) && (iCode < 0x80) ? (char)iCode : '?');
			sz += 4;
			break;
		default:	psz->Append(c);		break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The path variables in kicad_common.json's text: the strings of the
// "vars" object in its "environment" object.  "vars" may be null.
// False if the text doesn't have them.
//
//////////////////////////////////////////////////////////////////////////
bool CKicadPathVariables::ReadKicadCommon(const char* szJSON)
{
	const char*		sz;
	CChars			szName;
	CChars			szValue;

	sz = strstr(szJSON, "\"environment\"");
	if (sz == NULL)
	{
		return false;
	}
	sz = strstr(sz, "\"vars\"");
	if (sz == NULL)
	{
		return false;
	}
	sz = SkipKicadJSONSpace(sz + 6);
	if (*sz != ':')
	{
		return false;
	}
	sz = SkipKicadJSONSpace(sz + 1);
	if (strncmp(sz, "null", 4) == 0)
	{
		return true;
	}
	if (*sz != '{')
	{
		return false;
	}
	sz = SkipKicadJSONSpace(sz + 1);
	while (*sz != '}')
	{
		szName.Init();
		szValue.Init();
		sz = ReadKicadJSONString(sz, &szName);
		if (sz != NULL)
		{
			sz = SkipKicadJSONSpace(sz);
			sz = (*sz == ':') ? SkipKicadJSONSpace(sz + 1) : NULL;
		}
		if (sz != NULL)
		{
			sz = ReadKicadJSONString(sz, &szValue);
		}
		if (sz == NULL)
		{
			szValue.Kill();
			szName.Kill();
			return gcLogger.Error2(__METHOD__, " KiCad's path variables aren't a JSON object of strings.", NULL);
		}
		if (maszKicadNames.GetIndex(szName.Text()) == ARRAY_ELEMENT_NOT_FOUND)
		{
			maszKicadNames.Add(szName.Text());
			maszKicadValues.Add(szValue.Text());
		}
		szValue.Kill();
		szName.Kill();

		sz = SkipKicadJSONSpace(sz);
		if (*sz == ',')
		{
			sz = SkipKicadJSONSpace(sz + 1);
		}
		else if (*sz != '}')
		{
			return gcLogger.Error2(__METHOD__, " KiCad's path variables aren't a JSON object of strings.", NULL);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The full path of szPath with its variables (${NAME}) put in.  A path
// that's still relative is from the directory of szRelativeTo, a file.
// False if a variable isn't known.
//
//////////////////////////////////////////////////////////////////////////
bool CKicadPathVariables::Resolve(const char* szPath, const char* szRelativeTo, CChars* pszDest)
{
	const char*		sz;
	const char*		szEnd;
	const char*		szValue;
	CChars			szName;
	CChars			szFile;
	CChars			szDirectory;
	CFileUtil		cFileUtil;

	pszDest->Init();
	sz = szPath;
	while (*sz != '\0')
	{
		if ((sz[0] == '$') && (sz[1] == '{'))
		{
			szEnd = strchr(sz + 2, '}');
			if (szEnd == NULL)
			{
				pszDest->Kill();
				return gcLogger.Error2(__METHOD__, " Path [", szPath, "] has a variable with no closing brace.", NULL);
			}
			szName.Init(sz + 2, 0, (size)(szEnd - (sz + 2)));
			szValue = Get(szName.Text());
			if (szValue == NULL)
			{
				gcLogger.Error2(__METHOD__, " Path [", szPath, "] uses variable [", szName.Text(), "], which isn't set.", NULL);
				szName.Kill();
				pszDest->Kill();
				return false;
			}
			pszDest->Append(szValue);
			szName.Kill();
			sz = szEnd + 1;
		}
		else
		{
			pszDest->Append(*sz);
			sz++;
		}
	}

	cFileUtil.FixSeparators(pszDest);
	if (!cFileUtil.IsAbsolutePath(pszDest->Text()) && (szRelativeTo != NULL))
	{
		szFile.Init();
		szDirectory.Init();
		cFileUtil.SplitPath(szRelativeTo, &szFile, &szDirectory);
		cFileUtil.AppendToPath(&szDirectory, pszDest->Text());
		pszDest->Set(&szDirectory);
		szDirectory.Kill();
		szFile.Kill();
	}
	cFileUtil.FullPath(pszDest);
	return true;
}
