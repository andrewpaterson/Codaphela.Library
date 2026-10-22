#ifdef LINUX_GNU
#include <errno.h>
#include <limits.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/Logger.h"
#include "BaseLib/StackMemory.h"
#include "BaseLib/LinuxError.h"
#include "BaseLib/NativeThread.h"
#include "ProcessFork.h"


extern char** environ;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void ReapProcess(void* pvProcess)
{
    waitpid((pid_t)(size_t)pvProcess, NULL, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ForkProcess(CArrayChars* paszParameters, bool bWaitForProcessToExit)
{
    char            szExecutable[PATH_MAX + 1];
    ssize_t         iLength;
    char**          aszArguments;
    CStackMemory<>  cArguments;
    size            uiNumParameters;
    size            i;
    pid_t           iProcess;
    int             iResult;
    CNativeThread   cReaper;

    iLength = readlink("/proc/self/exe", szExecutable, PATH_MAX);
    if (iLength == -1)
    {
        gcLogger.Error2(__METHOD__, "readlink failed [", LinuxErrorCodeToString(errno), "].", NULL);
        return false;
    }
    szExecutable[iLength] = '\0';

    //argv is the program followed by the parameters, and ends with NULL.
    uiNumParameters = paszParameters->NumElements();
    aszArguments = (char**)cArguments.Init((uiNumParameters + 2) * sizeof(char*));
    aszArguments[0] = szExecutable;
    for (i = 0; i < uiNumParameters; i++)
    {
        aszArguments[i + 1] = paszParameters->GetText(i);
    }
    aszArguments[uiNumParameters + 1] = NULL;

    // Start the child process.
    iResult = posix_spawn(&iProcess, szExecutable, NULL, NULL, aszArguments, environ);
    cArguments.Kill();
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
        cReaper.Init();
        cReaper.Start(ReapProcess, (void*)(size_t)iProcess);
        cReaper.Detach();
        cReaper.Kill();
    }

    return true;
}


#endif // LINUX_GNU

