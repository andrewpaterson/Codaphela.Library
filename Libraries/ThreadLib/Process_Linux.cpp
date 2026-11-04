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
#ifdef LINUX_GNU
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include "BaseLib/Logger.h"
#include "BaseLib/LinuxError.h"
#include "BaseLib/StackMemory.h"
#include "Process.h"


//////////////////////////////////////////////////////////////////////////
//
// The program writes its output and errors into one pipe this process
// reads, and reads its input from /dev/null.  A program that can't be
// run exits with 127, as a shell's does.
//
//////////////////////////////////////////////////////////////////////////
bool CProcess::StartProcess(const char* szExecutable, CArrayChars* paszArguments, const char* szWorkingDirectory)
{
	int				aiPipe[2];
	int				iNull;
	pid_t			iProcess;
	char**			aszArguments;
	CStackMemory<>	cArguments;
	size			uiNumArguments;
	size			i;

	if (pipe(aiPipe) != 0)
	{
		gcLogger.Error2(__METHOD__, "pipe failed [", LinuxErrorCodeToString(errno), "].", NULL);
		return false;
	}

	//argv is the program followed by the arguments, and ends with NULL.
	uiNumArguments = paszArguments->NumElements();
	aszArguments = (char**)cArguments.Init((uiNumArguments + 2) * sizeof(char*));
	aszArguments[0] = (char*)szExecutable;
	for (i = 0; i < uiNumArguments; i++)
	{
		aszArguments[i + 1] = paszArguments->GetText(i);
	}
	aszArguments[uiNumArguments + 1] = NULL;

	iProcess = fork();
	if (iProcess == 0)
	{
		//The program: only what async signal safe calls can do until exec.
		iNull = open("/dev/null", O_RDONLY);
		if (iNull >= 0)
		{
			dup2(iNull, 0);
			close(iNull);
		}
		dup2(aiPipe[1], 1);
		dup2(aiPipe[1], 2);
		close(aiPipe[0]);
		close(aiPipe[1]);
		if (szWorkingDirectory && (chdir(szWorkingDirectory) != 0))
		{
			_exit(127);
		}
		execvp(szExecutable, aszArguments);
		_exit(127);
	}
	cArguments.Kill();
	close(aiPipe[1]);
	if (iProcess < 0)
	{
		gcLogger.Error2(__METHOD__, "fork failed [", LinuxErrorCodeToString(errno), "].", NULL);
		close(aiPipe[0]);
		return false;
	}

	fcntl(aiPipe[0], F_SETFL, fcntl(aiPipe[0], F_GETFL) | O_NONBLOCK);
	mpvProcess = (void*)(size_t)iProcess;
	mpvOutput = (void*)(size_t)(aiPipe[0] + 1);  //+1 so file descriptor 0 isn't NULL.
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
	ssize_t	iRead;

	if (!mpvOutput)
	{
		return;
	}
	for (;;)
	{
		iRead = read((int)(size_t)mpvOutput - 1, acBuffer, sizeof(acBuffer));
		if (iRead <= 0)
		{
			return;
		}
		AppendOutput(acBuffer, (size)iRead, paszLines);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A program stopped by a signal exits with 128 and the signal, as a
// shell's does.
//
//////////////////////////////////////////////////////////////////////////
bool CProcess::HasExited(int* piExitCode)
{
	int	iStatus;

	if (waitpid((pid_t)(size_t)mpvProcess, &iStatus, WNOHANG) != (pid_t)(size_t)mpvProcess)
	{
		return false;
	}
	*piExitCode = WIFEXITED(iStatus) ? WEXITSTATUS(iStatus) : 128 + WTERMSIG(iStatus);
	mpvProcess = NULL;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProcess::TerminateProcess(void)
{
	kill((pid_t)(size_t)mpvProcess, SIGKILL);
}


//////////////////////////////////////////////////////////////////////////
//
// A program still running when its process is killed is waited for, so
// it doesn't stay a zombie.
//
//////////////////////////////////////////////////////////////////////////
void CProcess::CloseProcess(void)
{
	if (mpvOutput)
	{
		close((int)(size_t)mpvOutput - 1);
		mpvOutput = NULL;
	}
	if (mpvProcess)
	{
		waitpid((pid_t)(size_t)mpvProcess, NULL, 0);
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
	usleep(5000);
}


#endif // LINUX_GNU

