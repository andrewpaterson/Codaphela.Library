/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "Log.h"


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
void CUserError::Set(const char* szError)
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

