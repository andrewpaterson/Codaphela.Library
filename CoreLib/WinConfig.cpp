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
#include "BaseLib/TextParser.h"
#include "PropertiesFile.h"
#include "WinConfig.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinConfig::Init(void)
{
	if (!Read())
	{
		Default();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CWinConfig::Read(void)
{
	CPropertiesFile				cFile;
	char*						szResolution;
	char*						szAspect;
	char*						szMode;
	char*						szWindowed;
	CTextParser					cParser;
	unsigned long long int		ulli;
	int							i;

	cFile.Init("Config.txt");
	if (!cFile.Read())
	{
		cFile.Kill();
		return FALSE;
	}

	szResolution = cFile.Get("Resolution");
	if (szResolution)
	{
		cParser.Init(szResolution);
		cParser.GetDigits(&ulli, &i, NULL, TRUE, FALSE);
		miWidth = (int)ulli;
		cParser.GetExactCaseInsensitiveCharacter('X');
		cParser.GetDigits(&ulli, &i, NULL, TRUE, FALSE);
		miHeight = (int)ulli;
		cParser.Kill();
	}
	else
	{
		miWidth = 0;
		miHeight = 0;
	}
	
	szAspect = cFile.Get("AspectRatio");
	if (szAspect)
	{
		cParser.Init(szAspect);
		cParser.GetDigits(&ulli, &i, NULL, TRUE, FALSE);
		mfAspect = (float)ulli;
		cParser.GetExactCharacter(':');
		cParser.GetDigits(&ulli, &i, NULL, TRUE, FALSE);
		mfAspect /= (float)ulli;
		cParser.Kill();
	}
	else
	{
		mfAspect = 0.0f;
	}

	szWindowed = cFile.Get("Windowed");
	mbWindowed = TRUE;
	if (szWindowed)
	{
		if (StringInsensitiveCompare(szWindowed, "FALSE") == 0)
		{
			mbWindowed = FALSE;
		}
	}

	szMode = cFile.Get("Mode");
	meMode = WM_DX_HAL;
	if (szMode)
	{
		if (StringInsensitiveCompare(szMode, "DX_HAL") == 0)
		{
			meMode = WM_DX_HAL;
		}		
		if (StringInsensitiveCompare(szMode, "DX_REF") == 0)
		{
			meMode = WM_DX_REF;
		}		
		if (StringInsensitiveCompare(szMode, "DX_SW") == 0)
		{
			meMode = WM_DX_SW;
		}
	}

	cFile.Kill();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinConfig::Default(void)
{
	miWidth = 0;
	miHeight = 0;
	mbWindowed = TRUE;
	mfAspect = 0;
	meMode = WM_DX_HAL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinConfig::Write(void)
{

}

