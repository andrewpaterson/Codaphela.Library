/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "StdErrFile.h"


CLogger		gcLogger;

//How many times this thread has locked gcLogger.  The logger can be re-entered from its own outputs (a CLogFile
//logs its errors) so the lock must be recursive but CMutex is not.
thread_local size	giLoggerLockDepth = 0;


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


	msConfig.bBreakOnWarning = false;
	msConfig.bBreakOnError = false;

#ifdef BREAK_ON_ERROR
	msConfig.bBreakOnError = true;
#endif

#ifdef BREAK_ON_WARNING
	msConfig.bBreakOnError = true;
	msConfig.bBreakOnWarning = true;
#endif
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::Init(CAbstractFile* pcFile, const char* szName)
{
	char				s[512];
	CAbstractFile*		pcDebugFile;

	mcMutex.Init();
	mapcFiles.Init();

	if (pcFile)
	{
		mapcFiles.Add(pcFile);
	}

#ifdef LINUX_GNU
	//On Linux the console output goes to stderr: unbuffered and separate from program output on stdout.
	pcDebugFile = StdErrFile();
#else
	pcDebugFile = DebugOutputFile();
#endif // LINUX_GNU
	mapcFiles.Add(pcDebugFile);
	mpcDebugOutputFile = pcDebugFile;

	msConfig.bEnabled = true;
	msConfig.bDebugOutput = true;

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
	size			i;
	CAbstractFile*	pcFile;
	bool			bDebugFreed;

	Lock();
	bDebugFreed = false;
	for (i = 0; i < mapcFiles.NumElements(); i++)
	{
		pcFile = *mapcFiles.Get(i);
		if (pcFile == mpcDebugOutputFile)
		{
			bDebugFreed = true;
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
	Unlock();

	mcMutex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::Lock(void)
{
	if (giLoggerLockDepth == 0)
	{
		mcMutex.Lock();
	}
	giLoggerLockDepth++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::Unlock(void)
{
	giLoggerLockDepth--;
	if (giLoggerLockDepth == 0)
	{
		mcMutex.Unlock();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::Add(const char* szText)
{
	size			i;
	CAbstractFile*	pcFile;

	Lock();
	if (msConfig.bEnabled)
	{
		for (i = 0; i < mapcFiles.NumElements(); i++)
		{
			pcFile = *mapcFiles.Get(i);
			pcFile->Open(EFM_ReadWrite_Create);
			pcFile->Seek(0, EFSO_END);
			pcFile->Write(szText, strlen(szText), 1);
			pcFile->Close();
		}
	}
	Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLogger::Error(const char* szText)
{
	Add("ERROR", szText);
	Break();
	return false;
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
bool CLogger::Error2(const char* szText, ...)
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

	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLogger::Warning(const char* szText)
{
	Add("WARNING", szText);

	if (msConfig.bBreakOnWarning)
	{
		BREAK();
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLogger::Warning2(const char* szText, ...)
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

	return false;
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
	size	iLength;

	if (szText == NULL)
	{
		Error("Tried to log NULL message");
		return;
	}

	iLength = strlen(szText);
	if (szErrorLevel)
	{
		iLength += strlen(szErrorLevel);
	}

	if (iLength > (1 KB - 80))
	{
		//This is really slow because the file must be opened and closed every add...
		//Lock so another thread's message can't be interleaved between the parts.
		Lock();
		if (szErrorLevel)
		{
			Add(szErrorLevel);
			Add(": ");
		}
		Add(szText);
		Add("\n");
		Unlock();
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
	Lock();
	msConfig.bEnabled = false;
	Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::Enable(void)
{
	Lock();
	msConfig.bEnabled = true;
	Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::SetBreakOnError(bool bBreakOnError)
{
	Lock();
	msConfig.bBreakOnError = bBreakOnError;
	Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::SetBreakOnWarning(bool bBreakOnWarning)
{
	Lock();
	msConfig.bBreakOnWarning = bBreakOnWarning;
	Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SLogConfig CLogger::SetSilent(bool bBreakOnError)
{
	SLogConfig sExisting;

	Lock();
	GetConfig(&sExisting);

	msConfig.bEnabled = true;
	msConfig.bBreakOnError = bBreakOnError;
	msConfig.bBreakOnWarning = false;
	msConfig.bDebugOutput = false;

	RemoveOutput(mpcDebugOutputFile);
	Unlock();

	return sExisting;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::SetConfig(SLogConfig* psConfig)
{
	Lock();
	msConfig.bEnabled = psConfig->bEnabled;
	msConfig.bBreakOnError = psConfig->bBreakOnError;
	msConfig.bBreakOnWarning = psConfig->bBreakOnWarning;
	msConfig.bDebugOutput = psConfig->bDebugOutput;
	RemoveOutput(mpcDebugOutputFile);
	if (msConfig.bDebugOutput)
	{
		AddOutput(mpcDebugOutputFile);
	}
	Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::GetConfig(SLogConfig* psConfig)
{
	Lock();
	psConfig->bEnabled = msConfig.bEnabled;
	psConfig->bBreakOnError = msConfig.bBreakOnError;
	psConfig->bBreakOnWarning = msConfig.bBreakOnWarning;
	psConfig->bDebugOutput = msConfig.bDebugOutput;
	Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::AddOutput(CAbstractFile* pcFile)
{
	Lock();
	mapcFiles.Add(pcFile);
	Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogger::RemoveOutput(CAbstractFile* pcFile)
{
	size	iIndex;

	Lock();
	iIndex = mapcFiles.FindWithKey(&pcFile, 0, sizeof(CAbstractFile*));
	mapcFiles.RemoveAt(iIndex);
	Unlock();
}

