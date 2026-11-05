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
#ifndef __PROCESS_H__
#define __PROCESS_H__
#include "BaseLib/ArrayChars.h"
#include "BaseLib/Chars.h"


//Another program run with its output read back, like a compiler run by a build.  Started with Start its standard
//output and error are one stream, read a line at a time without waiting (Poll) so whatever runs it can carry on, such
//as a window's tick.  Started with StartCapture its output is kept exactly as it was written, every byte, and its
//errors apart from it (PollCapture), for output that is data, like a file's contents from git.  Its standard input is
//empty.  Output after the program exits, from programs it started that are still running, isn't waited for.
class CProcess
{
protected:
	void*	mpvProcess;		//The process's handle (Windows) or id (Linux).
	void*	mpvOutput;		//The read end of its output's pipe.
	void*	mpvErrors;		//The read end of its errors' pipe, NULL unless captured.
	CChars	mszPartial;		//Output after the last line end.
	int		miExitCode;
	bool	mbRunning;

public:
	void	Init(void);
	void	Kill(void);

	bool	Start(const char* szExecutable, CArrayChars* paszArguments, const char* szWorkingDirectory = NULL);
	bool	Poll(CArrayChars* paszLines);
	void	Wait(CArrayChars* paszLines);

	bool	StartCapture(const char* szExecutable, CArrayChars* paszArguments, const char* szWorkingDirectory = NULL);
	bool	PollCapture(CChars* pszOutput, CChars* pszErrors);
	void	WaitCapture(CChars* pszOutput, CChars* pszErrors);

	void	Terminate(void);

	bool	IsRunning(void);
	int		GetExitCode(void);

protected:
	bool	Start(const char* szExecutable, CArrayChars* paszArguments, const char* szWorkingDirectory, bool bCapture);
	bool	Exited(void);
	void	ReadLines(CArrayChars* paszLines);
	void	ReadCaptured(CChars* pszOutput, CChars* pszErrors);
	void	ReadAll(void* pvPipe, CChars* pszDest);
	void	AppendOutput(const char* szOutput, size uiLength, CArrayChars* paszLines);
	void	EndOutput(CArrayChars* paszLines);

	//By platform.
	bool	StartProcess(const char* szExecutable, CArrayChars* paszArguments, const char* szWorkingDirectory, bool bCapture);
	size	ReadPipe(void* pvPipe, char* pcBuffer, size uiMaxLength);
	void	ClosePipe(void* pvPipe);
	bool	HasExited(int* piExitCode);
	void	TerminateProcess(void);
	void	CloseProcess(void);
	void	Pause(void);
};


#endif // __PROCESS_H__

