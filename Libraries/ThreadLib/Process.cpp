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
// szWorkingDirectory (NULL for this program's).  False if it couldn't be
// started, or the last program started hasn't exited.
//
//////////////////////////////////////////////////////////////////////////
bool CProcess::Start(const char* szExecutable, CArrayChars* paszArguments, const char* szWorkingDirectory)
{
	CArrayChars	aszNone;
	bool		bResult;

	if (mbRunning || mpvProcess)
	{
		return false;
	}

	aszNone.Init();
	bResult = StartProcess(szExecutable, paszArguments ? paszArguments : &aszNone, szWorkingDirectory);
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
	int		iExitCode;

	if (!mbRunning)
	{
		return false;
	}

	ReadAvailable(paszLines);
	if (!HasExited(&iExitCode))
	{
		return true;
	}

	//What it wrote before it exited.
	ReadAvailable(paszLines);
	EndOutput(paszLines);
	miExitCode = iExitCode;
	mbRunning = false;
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

