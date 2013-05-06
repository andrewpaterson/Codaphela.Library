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
#include <stdlib.h>
#include "DebugOutput.h"
#include "Logger.h"
#include "Define.h"  //For EngineOutput
#include "Chars.h"
#include "AbstractFile.h"
#include "DiskFile.h"
#include "FileUtil.h"


CLogger		gcLogger;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::Init(void)
{
	Init("Log.txt");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::Init(char* szName)
{
	CFileUtil cFileUtil;

	cFileUtil.Delete(szName);
	Init(DiskFile(szName), szName);
	mbFreeFile = TRUE;
	mbEngineOut = TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::Init(CAbstractFile* pcFile, char* szName)
{
	char	s[512];

	mpcFile = pcFile;
	mbEnabled = TRUE;

	if (szName)
	{
		sprintf(s, "-----------------------------------------------------------\n- Log File: [%s]\n-----------------------------------------------------------\n\n", szName);
		Add(s);
	}

	mbFreeFile = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::Kill(void)
{
	if (mbFreeFile)
	{
		mpcFile->Kill();
		free(mpcFile);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::Add(char* szText)
{
	if (mbEnabled)
	{
		if (mbEngineOut)
		{
			EngineOutput(szText);
		}
		mpcFile->Open(EFM_ReadWrite_Create);
		mpcFile->Seek(0, EFSO_END);
		if (mpcFile)
		{
			mpcFile->Write(szText, (int)strlen(szText), 1);
			mpcFile->Close();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::Error(char* szText)
{
	Add("ERROR", szText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::Error2(char* szText, ...)
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
			szError.Append(sz);
			sz = va_arg(vaMarker, char*);
		}
		va_end(vaMarker);

		Error(szError.Text());
		szError.Kill();
	}
	else
	{
		Error("");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::Warning(char* szText)
{
	Add("WARNING", szText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::Info(char* szText)
{
	Add("INFO", szText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::Debug(char* szText)
{
	Add("DEBUG", szText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::Add(char* szErrorLevel, char* szText)
{
	char	szMessage[16384];
	int		iLength;

	if (szText == NULL)
	{
		Error("Tried to log NULL message");
		return;
	}

	iLength = (int)strlen(szText);
	if (szErrorLevel)
	{
		iLength += (int)strlen(szErrorLevel);
	}

	if (iLength > 16000)
	{
		//This is really slow because the file must be opened and closed every add...
		if (szErrorLevel)
		{
			Add(szErrorLevel);
			Add(": ");
		}
		Add(szText);
		Add("\n");
	}
	else
	{
		if (szErrorLevel)
		{
			strcpy(szMessage, szErrorLevel);
			strcat(szMessage, ": ");
			strcat(szMessage, szText);
		}
		else
		{
			strcpy(szMessage, szText);
		}
		strcat(szMessage, "\n");
		Add(szMessage);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::Disable(void)
{
	mbEnabled = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::Enable(void)
{
	mbEnabled = TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::SetEngineOutput(BOOL bEngineOut)
{
	mbEngineOut = bEngineOut;
}
