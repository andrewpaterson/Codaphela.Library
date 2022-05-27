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
#include <stdlib.h>
#include "Logger.h"
#include "Define.h"
#include "Chars.h"
#include "AbstractFile.h"
#include "DiskFile.h"
#include "FileUtil.h"
#include "Validation.h"
#include "DebugOutput.h"
#include "DebugOutputFile.h"


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
void CLogger::Init(const char* szName)
{
	CFileUtil cFileUtil;

	if (szName)
	{
		cFileUtil.Delete(szName);
		Init(DiskFile(szName), szName);
	}
	else
	{
		Init(NULL, NULL);
	}


	msConfig.bBreakOnWarning = FALSE;
	msConfig.bBreakOnError = FALSE;

#ifdef BREAK_ON_ERROR
	msConfig.bBreakOnError = TRUE;
#endif

#ifdef BREAK_ON_WARNING
	msConfig.bBreakOnError = TRUE;
	msConfig.bBreakOnWarning = TRUE;
#endif
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::Init(CAbstractFile* pcFile, const char* szName)
{
	char				s[512];
	CDebugOutputFile*	pcDebugFile;

	mapcFiles.Init();

	if (pcFile)
	{
		mapcFiles.Add(&pcFile);
	}

	pcDebugFile = DebugOutputFile();
	mapcFiles.Add((CAbstractFile**)&pcDebugFile);
	mpcDebugOutputFile = pcDebugFile;

	msConfig.bEnabled = TRUE;
	msConfig.bDebugOutput = TRUE;

	if (szName)
	{
		sprintf(s, "-----------------------------------------------------------\n- Log File: [%s]\n-----------------------------------------------------------\n\n", szName);
		Add(s);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::Kill(void)
{
	int				i;
	CAbstractFile*	pcFile;
	BOOL			bDebugFreed;

	bDebugFreed = FALSE;
	for (i = 0; i < mapcFiles.NumElements(); i++)
	{
		pcFile = *mapcFiles.Get(i);
		if (pcFile == mpcDebugOutputFile)
		{
			bDebugFreed = TRUE;
		}
		if (pcFile->mbBasicFileMustFree)
		{
			SafeKill(pcFile);
		}
	}

	if (!bDebugFreed)
	{
		if (mpcDebugOutputFile->mbBasicFileMustFree)
		{
			SafeKill(mpcDebugOutputFile);
			free(mpcDebugOutputFile);
		}
	}
	mapcFiles.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::Add(const char* szText)
{
	int				i;
	CAbstractFile*	pcFile;

	if (msConfig.bEnabled)
	{
		for (i = 0; i < mapcFiles.NumElements(); i++)
		{
			pcFile = *mapcFiles.Get(i);
			pcFile->Open(EFM_ReadWrite_Create);
			pcFile->Seek(0, EFSO_END);
			pcFile->Write(szText, (int)strlen(szText), 1);
			pcFile->Close();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogger::Error(const char* szText)
{
	Add("ERROR", szText);
	Break();
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::Break(void)
{
	if (msConfig.bBreakOnError)
	{
		BREAK();
	} 
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogger::Error2(const char* szText, ...)
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

		Error(szError.Text());
		szError.Kill();
	}
	else
	{
		Error("");
	}

	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogger::Warning(const char* szText)
{
	Add("WARNING", szText);

	if (msConfig.bBreakOnWarning)
	{
		BREAK();
	}

	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogger::Warning2(const char* szText, ...)
{
	va_list		vaMarker;
	char* sz;
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

		Warning(szError.Text());
		szError.Kill();
	}
	else
	{
		Warning("");
	}

	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::Info2(const char* szText, ...)
{
	va_list		vaMarker;
	char*		sz;
	CChars		szInfo;

	if (szText)
	{
		szInfo.Init(szText);
		va_start(vaMarker, szText);
		sz = va_arg(vaMarker, char*);
		while (sz != NULL)
		{
			szInfo.Append(sz);
			sz = va_arg(vaMarker, char*);
		}
		va_end(vaMarker);

		Info(szInfo.Text());
		szInfo.Kill();
	}
	else
	{
		Info("");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::Info(const char* szText)
{
	Add("INFO", szText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::Debug2(const char* szText, ...)
{
	va_list		vaMarker;
	char*		sz;
	CChars		szInfo;

	if (szText)
	{
		szInfo.Init(szText);
		va_start(vaMarker, szText);
		sz = va_arg(vaMarker, char*);
		while (sz != NULL)
		{
			szInfo.Append(sz);
			sz = va_arg(vaMarker, char*);
		}
		va_end(vaMarker);

		Debug(szInfo.Text());
		szInfo.Kill();
	}
	else
	{
		Debug("");
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::Debug(const char* szText)
{
	Add("DEBUG", szText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::Add(const char* szErrorLevel, const char* szText)
{
	char	szMessage[1 KB];
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

	if (iLength > (1 KB - 80))
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
	msConfig.bEnabled = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::Enable(void)
{
	msConfig.bEnabled = TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::SetBreakOnError(BOOL bBreakOnError)
{
	msConfig.bBreakOnError = bBreakOnError;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::SetBreakOnWarning(BOOL bBreakOnWarning)
{
	msConfig.bBreakOnWarning = bBreakOnWarning;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SLogConfig CLogger::SetSilent(BOOL bBreakOnError)
{
	SLogConfig sExisting;

	GetConfig(&sExisting);

	msConfig.bEnabled = TRUE;
	msConfig.bBreakOnError = bBreakOnError;
	msConfig.bBreakOnWarning = FALSE;
	msConfig.bDebugOutput = FALSE;

	RemoveOutput(mpcDebugOutputFile);

	return sExisting;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::SetConfig(SLogConfig* psConfig)
{
	msConfig.bEnabled = psConfig->bEnabled;
	msConfig.bBreakOnError = psConfig->bBreakOnError;
	msConfig.bBreakOnWarning = psConfig->bBreakOnWarning;
	msConfig.bDebugOutput = psConfig->bDebugOutput;
	RemoveOutput(mpcDebugOutputFile);
	if (msConfig.bDebugOutput)
	{
		AddOutput(mpcDebugOutputFile);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::GetConfig(SLogConfig* psConfig)
{
	psConfig->bEnabled = msConfig.bEnabled;
	psConfig->bBreakOnError = msConfig.bBreakOnError;
	psConfig->bBreakOnWarning = msConfig.bBreakOnWarning;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::AddOutput(CAbstractFile* pcFile)
{
	mapcFiles.Add(&pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::RemoveOutput(CAbstractFile* pcFile)
{
	int	iIndex;

	iIndex = mapcFiles.FindWithKey(&pcFile, 0, sizeof(CAbstractFile*));
	mapcFiles.RemoveAt(iIndex);
}

