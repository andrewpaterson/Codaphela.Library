#ifdef LINUX_GNU_32
#include <thread>
#include <errno.h>
#include <limits.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/Logger.h"
#include "BaseLib/LinuxError.h"
#include "ProcessFork.h"


extern char** environ;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ForkProcess(char* szCommandLineParameters, bool bWaitForProcessToExit)
{
    char            szExecutable[PATH_MAX + 1];
    ssize_t         iLength;
    CArrayChars     aszParameters;
    char**          aszArguments;
    size            uiNumParameters;
    size            uiNumArguments;
    size            i;
    char*           szParameter;
    pid_t           iProcess;
    int             iResult;

    iLength = readlink("/proc/self/exe", szExecutable, PATH_MAX);
    if (iLength == -1)
    {
        gcLogger.Error2(__METHOD__, "readlink failed [", LinuxErrorCodeToString(errno), "].", NULL);
        return false;
    }
    szExecutable[iLength] = '\0';

    aszParameters.Init();
    if (szCommandLineParameters)
    {
        aszParameters.Split(szCommandLineParameters, ' ');
    }
    uiNumParameters = aszParameters.NumElements();

    //Like Windows command line parsing; runs of spaces separate parameters.
    aszArguments = (char**)malloc((uiNumParameters + 2) * sizeof(char*));
    aszArguments[0] = szExecutable;
    uiNumArguments = 1;
    for (i = 0; i < uiNumParameters; i++)
    {
        szParameter = aszParameters.GetText(i);
        if (szParameter[0] != '\0')
        {
            aszArguments[uiNumArguments] = szParameter;
            uiNumArguments++;
        }
    }
    aszArguments[uiNumArguments] = NULL;

    // Start the child process.
    iResult = posix_spawn(&iProcess, szExecutable, NULL, NULL, aszArguments, environ);
    free(aszArguments);
    aszParameters.Kill();
    if (iResult != 0)
    {
        gcLogger.Error2(__METHOD__, "posix_spawn failed [", LinuxErrorCodeToString(iResult), "].", NULL);
        return false;
    }

    if (bWaitForProcessToExit)
    {
        // Wait until child process exits.
        waitpid(iProcess, NULL, 0);
    }
    else
    {
        //Windows forgets the child once its handles are closed.  A Linux child
        //stays a zombie until it is waited for, so wait for it in the background.
        std::thread([iProcess]() { waitpid(iProcess, NULL, 0); }).detach();
    }

    return true;
}


#endif // LINUX_GNU_32

