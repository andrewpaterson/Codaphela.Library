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
// reads, or into a pipe each if they are captured, and reads its input
// from /dev/null.  A program that can't be run exits with 127, as a
// shell's does.
//
//////////////////////////////////////////////////////////////////////////
bool CProcess::StartProcess(const char* szExecutable, CArrayChars* paszArguments, const char* szWorkingDirectory, bool bCapture)
{
	int				aiPipe[2];
	int				aiErrors[2];
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
	aiErrors[0] = -1;
	aiErrors[1] = -1;
	if (bCapture && (pipe(aiErrors) != 0))
	{
		gcLogger.Error2(__METHOD__, "pipe failed [", LinuxErrorCodeToString(errno), "].", NULL);
		close(aiPipe[0]);
		close(aiPipe[1]);
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
		dup2(bCapture ? aiErrors[1] : aiPipe[1], 2);
		close(aiPipe[0]);
		close(aiPipe[1]);
		if (bCapture)
		{
			close(aiErrors[0]);
			close(aiErrors[1]);
		}
		if (szWorkingDirectory && (chdir(szWorkingDirectory) != 0))
		{
			_exit(127);
		}
		execvp(szExecutable, aszArguments);
		_exit(127);
	}
	cArguments.Kill();
	close(aiPipe[1]);
	if (bCapture)
	{
		close(aiErrors[1]);
	}
	if (iProcess < 0)
	{
		gcLogger.Error2(__METHOD__, "fork failed [", LinuxErrorCodeToString(errno), "].", NULL);
		close(aiPipe[0]);
		if (bCapture)
		{
			close(aiErrors[0]);
		}
		return false;
	}

	mpvProcess = (void*)(size_t)iProcess;
	fcntl(aiPipe[0], F_SETFL, fcntl(aiPipe[0], F_GETFL) | O_NONBLOCK);
	mpvOutput = (void*)(size_t)(aiPipe[0] + 1);  //+1 so file descriptor 0 isn't NULL.
	if (bCapture)
	{
		fcntl(aiErrors[0], F_SETFL, fcntl(aiErrors[0], F_GETFL) | O_NONBLOCK);
		mpvErrors = (void*)(size_t)(aiErrors[0] + 1);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Reads what is in pvPipe now, without waiting for more.  0 if nothing
// is, or pvPipe is NULL.
//
//////////////////////////////////////////////////////////////////////////
size CProcess::ReadPipe(void* pvPipe, char* pcBuffer, size uiMaxLength)
{
	ssize_t	iRead;

	if (!pvPipe)
	{
		return 0;
	}
	iRead = read((int)(size_t)pvPipe - 1, pcBuffer, uiMaxLength);
	if (iRead <= 0)
	{
		return 0;
	}
	return (size)iRead;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProcess::ClosePipe(void* pvPipe)
{
	close((int)(size_t)pvPipe - 1);
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
		ClosePipe(mpvOutput);
		mpvOutput = NULL;
	}
	if (mpvErrors)
	{
		ClosePipe(mpvErrors);
		mpvErrors = NULL;
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

