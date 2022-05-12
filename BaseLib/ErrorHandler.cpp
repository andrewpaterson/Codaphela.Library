/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#include <string.h>
#include "ErrorHandler.h"
#include "Logger.h"
#include "Numbers.h"
#include "Chars.h"

CUserError	gcUserError;


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CUserError::Init(void)
{
	mbUserError = FALSE;
	mszUserError[0] = 0;
}


void CUserError::Kill(void)
{
	Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CUserError::Set(const char* szError)
{
	gcLogger.Add("FATAL ERROR: ");
	gcLogger.Add(szError);
	gcLogger.Add("\n");
	if (!mbUserError)
	{
		strcpy(mszUserError, szError);
		mbUserError = TRUE;
		gcLogger.Break();
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CUserError::Get(void)
{
	if (mbUserError)
	{
		return mszUserError;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CUserError::Set2(const char* szText, ...)
{
	va_list		vaMarker;
	char*		sz;
	CChars		szError;

	if (szText)
	{
		szError.Init(szText);
		va_start(vaMarker, szText);
		sz = va_arg(vaMarker, char*);
		while (sz != NULL)
		{
			if (sz > CORRUPT_MEMORY)
			{
				szError.Append(sz);
			}
			else
			{
				szError.Append(":Error args corrupt!");
				break;
			}
			sz = va_arg(vaMarker, char*);
		}
		va_end(vaMarker);

		Set(szError.Text());
		szError.Kill();
	}
	else
	{
		Set("");
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CUserError::Set(CChars* pszError, BOOL bKillErrorString)
{
	Set(pszError->Text());
	if (bKillErrorString)
	{
		pszError->Kill();
	}
	return FALSE;
}

