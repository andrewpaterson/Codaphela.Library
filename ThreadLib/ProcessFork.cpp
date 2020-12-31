#include "BaseLib/Numbers.h"
#include "BaseLib/Chars.h"
#include "BaseLib/Logger.h"
#include "BaseLib/StackMemory.h"
#include "BaseLib/WindowsHeaders.h"
#include "WindowsError.h"
#include "ProcessFork.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL ForkProcess(char* szParameter1, char* szParameter2, BOOL bWaitForProcessToExit)
{
    CChars  sz;
    BOOL    bResult;

    sz.Init(szParameter1);
    sz.Append(" ");
    sz.Append(szParameter2);

    bResult = ForkProcess(sz.Text(), bWaitForProcessToExit);

    sz.Kill();
    return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL ForkProcess(char* szParameter1, char* szParameter2, char* szParameter3, BOOL bWaitForProcessToExit)
{
    CChars  sz;
    BOOL    bResult;

    sz.Init(szParameter1);
    sz.Append(" ");
    sz.Append(szParameter2);
    sz.Append(" ");
    sz.Append(szParameter3);

    bResult = ForkProcess(sz.Text(), bWaitForProcessToExit);

    sz.Kill();
    return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL ForkProcess(char* szParameter1, char* szParameter2, char* szParameter3, char* szParameter4, BOOL bWaitForProcessToExit)
{
    CChars  sz;
    BOOL    bResult;

    sz.Init(szParameter1);
    sz.Append(" ");
    sz.Append(szParameter2);
    sz.Append(" ");
    sz.Append(szParameter3);
    sz.Append(" ");
    sz.Append(szParameter4);

    bResult = ForkProcess(sz.Text(), bWaitForProcessToExit);

    sz.Kill();
    return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL ForkProcess(char* szParameter1, char* szParameter2, char* szParameter3, char* szParameter4, char* szParameter5, BOOL bWaitForProcessToExit)
{
    CChars  sz;
    BOOL    bResult;

    sz.Init(szParameter1);
    sz.Append(" ");
    sz.Append(szParameter2);
    sz.Append(" ");
    sz.Append(szParameter3);
    sz.Append(" ");
    sz.Append(szParameter4);
    sz.Append(" ");
    sz.Append(szParameter5);

    bResult = ForkProcess(sz.Text(), bWaitForProcessToExit);

    sz.Kill();
    return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL ForkProcess(char* szParameter1, char* szParameter2, char* szParameter3, char* szParameter4, char* szParameter5, char* szParameter6, BOOL bWaitForProcessToExit)
{
    CChars  sz;
    BOOL    bResult;

    sz.Init(szParameter1);
    sz.Append(" ");
    sz.Append(szParameter2);
    sz.Append(" ");
    sz.Append(szParameter3);
    sz.Append(" ");
    sz.Append(szParameter4);
    sz.Append(" ");
    sz.Append(szParameter5);
    sz.Append(" ");
    sz.Append(szParameter6);

    bResult = ForkProcess(sz.Text(), bWaitForProcessToExit);

    sz.Kill();
    return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL ForkProcess(char* szCommandLineParameters, BOOL bWaitForProcessToExit)
{
    CStackMemory<>          cTemp;
    unsigned long           uiLength;
    unsigned long           uiError;
    CChars                  szCommandLine;
    char* sz;
    STARTUPINFO             sStartUpInfo;
    PROCESS_INFORMATION     sProcessInformation;

    sz = (char*)cTemp.Init();
    uiLength = GetModuleFileName(NULL, sz, cTemp.GetStackSize());
    uiError = GetLastError();
    if (uiError == ERROR_INSUFFICIENT_BUFFER)
    {
        sz = (char*)cTemp.Init(uiLength + 1);
        uiLength = GetModuleFileName(NULL, sz, uiLength + 1);
    }

    szCommandLine.Init(sz);
    cTemp.Kill();

    ZeroMemory(&sStartUpInfo, sizeof(STARTUPINFO));
    sStartUpInfo.cb = sizeof(STARTUPINFO);
    ZeroMemory(&sProcessInformation, sizeof(PROCESS_INFORMATION));

    if (szCommandLineParameters)
    {
        szCommandLine.Append(" ");
        szCommandLine.Append(szCommandLineParameters);
    }

    // Start the child process. 
    if (!CreateProcess(NULL,    // No module name (use command line)
        szCommandLine.Text(),
        NULL,                   // Process handle not inheritable
        NULL,                   // Thread handle not inheritable
        FALSE,                  // Set handle inheritance to FALSE
        0,                      // No creation flags
        NULL,                   // Use parent's environment block
        NULL,                   // Use parent's starting directory 
        &sStartUpInfo,          // Pointer to STARTUPINFO structure
        &sProcessInformation))  // Pointer to PROCESS_INFORMATION structure
    {
        szCommandLine.Kill();
        gcLogger.Error2(__METHOD__, "CreateProcess failed [", WindowsErrorCodeToString(GetLastError()), "].", NULL);
        return FALSE;
    }
    szCommandLine.Kill();

    // Wait until child process exits.
    if (bWaitForProcessToExit)
    {
        WaitForSingleObject(sProcessInformation.hProcess, INFINITE);
    }

    // Close process and thread handles. 
    CloseHandle(sProcessInformation.hProcess);
    CloseHandle(sProcessInformation.hThread);

    return TRUE;
}

