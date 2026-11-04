/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela ThreadLib

Codaphela ThreadLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela ThreadLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela ThreadLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifdef _WIN32
#include "BaseLib/Logger.h"
#include "BaseLib/WindowsHeaders.h"
#include "BaseLib/WindowsError.h"
#include "ProcessFork.h"
#include "Process.h"


//////////////////////////////////////////////////////////////////////////
//
// The program writes its output and errors into one pipe this process
// reads, and reads its input from nothing (NUL).  It has no console
// window.
//
//////////////////////////////////////////////////////////////////////////
bool CProcess::StartProcess(const char* szExecutable, CArrayChars* paszArguments, const char* szWorkingDirectory)
{
	SECURITY_ATTRIBUTES		sInherited;
	HANDLE					hRead;
	HANDLE					hWrite;
	HANDLE					hNull;
	STARTUPINFO				sStartUpInfo;
	PROCESS_INFORMATION		sProcessInformation;
	CChars					szCommandLine;
	size					i;
	bool					bResult;

	ZeroMemory(&sInherited, sizeof(sInherited));
	sInherited.nLength = sizeof(sInherited);
	sInherited.bInheritHandle = TRUE;
	if (!CreatePipe(&hRead, &hWrite, &sInherited, 0))
	{
		gcLogger.Error2(__METHOD__, "CreatePipe failed [", WindowsErrorCodeToString(GetLastError()), "].", NULL);
		return false;
	}

	//Only the program's end of the pipe is passed to it.
	SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);
	hNull = CreateFile("NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &sInherited, OPEN_EXISTING, 0, NULL);

	szCommandLine.Init();
	AppendQuotedArgument(&szCommandLine, (char*)szExecutable);
	for (i = 0; i < paszArguments->NumElements(); i++)
	{
		szCommandLine.Append(' ');
		AppendQuotedArgument(&szCommandLine, paszArguments->GetText(i));
	}

	ZeroMemory(&sStartUpInfo, sizeof(sStartUpInfo));
	sStartUpInfo.cb = sizeof(sStartUpInfo);
	sStartUpInfo.dwFlags = STARTF_USESTDHANDLES;
	sStartUpInfo.hStdInput = hNull;
	sStartUpInfo.hStdOutput = hWrite;
	sStartUpInfo.hStdError = hWrite;
	ZeroMemory(&sProcessInformation, sizeof(sProcessInformation));
	bResult = CreateProcess(NULL, szCommandLine.Text(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, szWorkingDirectory, &sStartUpInfo, &sProcessInformation) != 0;
	if (!bResult)
	{
		gcLogger.Error2(__METHOD__, "CreateProcess failed [", WindowsErrorCodeToString(GetLastError()), "] for [", szCommandLine.Text(), "].", NULL);
	}
	szCommandLine.Kill();

	//The program has its own copies; when it (and what it starts) closes them, the pipe ends.
	CloseHandle(hWrite);
	if (hNull != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hNull);
	}
	if (!bResult)
	{
		CloseHandle(hRead);
		return false;
	}

	CloseHandle(sProcessInformation.hThread);
	mpvProcess = sProcessInformation.hProcess;
	mpvOutput = hRead;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Reads what is in the pipe now, without waiting for more.
//
//////////////////////////////////////////////////////////////////////////
void CProcess::ReadAvailable(CArrayChars* paszLines)
{
	char	acBuffer[4096];
	DWORD	uiAvailable;
	DWORD	uiRead;

	if (!mpvOutput)
	{
		return;
	}
	for (;;)
	{
		if (!PeekNamedPipe((HANDLE)mpvOutput, NULL, 0, NULL, &uiAvailable, NULL) || (uiAvailable == 0))
		{
			return;
		}
		if (!ReadFile((HANDLE)mpvOutput, acBuffer, (uiAvailable < sizeof(acBuffer)) ? uiAvailable : sizeof(acBuffer), &uiRead, NULL) || (uiRead == 0))
		{
			return;
		}
		AppendOutput(acBuffer, uiRead, paszLines);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CProcess::HasExited(int* piExitCode)
{
	DWORD	uiExitCode;

	if (WaitForSingleObject((HANDLE)mpvProcess, 0) != WAIT_OBJECT_0)
	{
		return false;
	}
	GetExitCodeProcess((HANDLE)mpvProcess, &uiExitCode);
	*piExitCode = (int)uiExitCode;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Its exit code is 1.
//
//////////////////////////////////////////////////////////////////////////
void CProcess::TerminateProcess(void)
{
	::TerminateProcess((HANDLE)mpvProcess, 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProcess::CloseProcess(void)
{
	if (mpvOutput)
	{
		CloseHandle((HANDLE)mpvOutput);
		mpvOutput = NULL;
	}
	if (mpvProcess)
	{
		CloseHandle((HANDLE)mpvProcess);
		mpvProcess = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A moment between looks at a program being waited for.
//
//////////////////////////////////////////////////////////////////////////
void CProcess::Pause(void)
{
	Sleep(5);
}


#endif // _WIN32

