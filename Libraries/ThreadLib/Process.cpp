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
#include "Process.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProcess::Init(void)
{
	mpvProcess = NULL;
	mpvOutput = NULL;
	mpvErrors = NULL;
	mszPartial.Init();
	miExitCode = 0;
	mbRunning = false;
}


//////////////////////////////////////////////////////////////////////////
//
// A program still running is stopped.
//
//////////////////////////////////////////////////////////////////////////
void CProcess::Kill(void)
{
	if (mbRunning)
	{
		TerminateProcess();
	}
	CloseProcess();
	mszPartial.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Runs szExecutable (found on the path if it has no folder) with
// paszArguments (NULL for none), each one argument even with spaces, in
// szWorkingDirectory (NULL for this program's).  Its output and errors are
// read as lines with Poll.  False if it couldn't be started, or the last
// program started hasn't exited.
//
//////////////////////////////////////////////////////////////////////////
bool CProcess::Start(const char* szExecutable, CArrayChars* paszArguments, const char* szWorkingDirectory)
{
	return Start(szExecutable, paszArguments, szWorkingDirectory, false);
}


//////////////////////////////////////////////////////////////////////////
//
// As Start, but its output and errors are read apart, every byte as it
// was written, with PollCapture.
//
//////////////////////////////////////////////////////////////////////////
bool CProcess::StartCapture(const char* szExecutable, CArrayChars* paszArguments, const char* szWorkingDirectory)
{
	return Start(szExecutable, paszArguments, szWorkingDirectory, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CProcess::Start(const char* szExecutable, CArrayChars* paszArguments, const char* szWorkingDirectory, bool bCapture)
{
	CArrayChars	aszNone;
	bool		bResult;

	if (mbRunning || mpvProcess)
	{
		return false;
	}

	aszNone.Init();
	bResult = StartProcess(szExecutable, paszArguments ? paszArguments : &aszNone, szWorkingDirectory, bCapture);
	aszNone.Kill();
	mbRunning = bResult;
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the lines of output there are now to paszLines, without their
// line ends, without waiting.  Returns true while the program is running;
// once it has exited the last of its output is added (a last line with no
// line end too) and false is returned.
//
//////////////////////////////////////////////////////////////////////////
bool CProcess::Poll(CArrayChars* paszLines)
{
	if (!mbRunning)
	{
		return false;
	}

	ReadLines(paszLines);
	if (!Exited())
	{
		return true;
	}

	//What it wrote before it exited.
	ReadLines(paszLines);
	EndOutput(paszLines);
	CloseProcess();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds all the program's output to paszLines as it waits for it to exit.
//
//////////////////////////////////////////////////////////////////////////
void CProcess::Wait(CArrayChars* paszLines)
{
	while (Poll(paszLines))
	{
		Pause();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Appends the output and errors there are now of a program started with
// StartCapture to pszOutput and pszErrors, without waiting.  Returns true
// while the program is running; once it has exited the last of them are
// appended and false is returned.  The output can hold any bytes, '\0's
// too; Length says how much there is.
//
//////////////////////////////////////////////////////////////////////////
bool CProcess::PollCapture(CChars* pszOutput, CChars* pszErrors)
{
	if (!mbRunning)
	{
		return false;
	}

	ReadCaptured(pszOutput, pszErrors);
	if (!Exited())
	{
		return true;
	}

	ReadCaptured(pszOutput, pszErrors);
	CloseProcess();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Appends all the output and errors of a program started with
// StartCapture as it waits for it to exit.
//
//////////////////////////////////////////////////////////////////////////
void CProcess::WaitCapture(CChars* pszOutput, CChars* pszErrors)
{
	while (PollCapture(pszOutput, pszErrors))
	{
		Pause();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Stops the program.  Poll finds it exited.
//
//////////////////////////////////////////////////////////////////////////
void CProcess::Terminate(void)
{
	if (mbRunning)
	{
		TerminateProcess();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True once the program has exited, with its exit code kept.
//
//////////////////////////////////////////////////////////////////////////
bool CProcess::Exited(void)
{
	int		iExitCode;

	if (!HasExited(&iExitCode))
	{
		return false;
	}
	miExitCode = iExitCode;
	mbRunning = false;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Reads what is in the output's pipe now, as lines.
//
//////////////////////////////////////////////////////////////////////////
void CProcess::ReadLines(CArrayChars* paszLines)
{
	char	acBuffer[4096];
	size	uiRead;

	for (;;)
	{
		uiRead = ReadPipe(mpvOutput, acBuffer, sizeof(acBuffer));
		if (uiRead == 0)
		{
			return;
		}
		AppendOutput(acBuffer, uiRead, paszLines);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Reads what is in both pipes now.  Each is emptied in turn so a program
// writing a lot of both never waits on a full pipe for long.
//
//////////////////////////////////////////////////////////////////////////
void CProcess::ReadCaptured(CChars* pszOutput, CChars* pszErrors)
{
	ReadAll(mpvOutput, pszOutput);
	ReadAll(mpvErrors, pszErrors);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProcess::ReadAll(void* pvPipe, CChars* pszDest)
{
	char	acBuffer[4096];
	size	uiRead;

	for (;;)
	{
		uiRead = ReadPipe(pvPipe, acBuffer, sizeof(acBuffer));
		if (uiRead == 0)
		{
			return;
		}
		pszDest->Append(acBuffer, uiRead);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Splits szOutput into lines.  Carriage returns before line feeds go.
//
//////////////////////////////////////////////////////////////////////////
void CProcess::AppendOutput(const char* szOutput, size uiLength, CArrayChars* paszLines)
{
	size	i;

	for (i = 0; i < uiLength; i++)
	{
		if (szOutput[i] == '\n')
		{
			if (mszPartial.EndsWith('\r'))
			{
				mszPartial.RemoveLastCharacter();
			}
			paszLines->Add(mszPartial.Text());
			mszPartial.Clear();
		}
		else
		{
			mszPartial.Append(szOutput[i]);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The output after the last line end is a line.
//
//////////////////////////////////////////////////////////////////////////
void CProcess::EndOutput(CArrayChars* paszLines)
{
	if (mszPartial.EndsWith('\r'))
	{
		mszPartial.RemoveLastCharacter();
	}
	if (!mszPartial.Empty())
	{
		paszLines->Add(mszPartial.Text());
		mszPartial.Clear();
	}
}


bool CProcess::IsRunning(void) { return mbRunning; }
int CProcess::GetExitCode(void) { return miExitCode; }  //Once Poll has found it exited.

