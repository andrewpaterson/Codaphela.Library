#ifdef _WIN32
#include "BaseLib/Chars.h"
#include "BaseLib/Logger.h"
#include "BaseLib/StackMemory.h"
#include "BaseLib/WindowsHeaders.h"
#include "BaseLib/WindowsError.h"
#include "ProcessFork.h"


//////////////////////////////////////////////////////////////////////////
//
//	Quotes szArgument the way the C runtime splits a command line back into
//	argv: backslashes are literal unless they come before a quote.
//
//////////////////////////////////////////////////////////////////////////
void AppendQuotedArgument(CChars* psz, char* szArgument)
{
    size    uiBackslashes;
    char*   pc;

    if ((szArgument[0] != '\0') && (strpbrk(szArgument, " \t\"") == NULL))
    {
        psz->Append(szArgument);
        return;
    }

    psz->Append('"');
    uiBackslashes = 0;
    for (pc = szArgument; *pc != '\0'; pc++)
    {
        if (*pc == '\\')
        {
            uiBackslashes++;
        }
        else if (*pc == '"')
        {
            psz->Append('\\', uiBackslashes * 2 + 1);
            psz->Append('"');
            uiBackslashes = 0;
        }
        else
        {
            psz->Append('\\', uiBackslashes);
            psz->Append(*pc);
            uiBackslashes = 0;
        }
    }
    //Backslashes before the closing quote are doubled so it stays a quote.
    psz->Append('\\', uiBackslashes * 2);
    psz->Append('"');
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ForkProcess(CArrayChars* paszParameters, bool bWaitForProcessToExit)
{
    CStackMemory<>          cTemp;
    unsigned long           uiSize;
    unsigned long           uiLength;
    CChars                  szCommandLine;
    char*                   sz;
    size                    uiNumParameters;
    size                    i;
    STARTUPINFO             sStartUpInfo;
    PROCESS_INFORMATION     sProcessInformation;

    //GetModuleFileName fills the whole buffer when the path does not fit.
    uiSize = (unsigned long)cTemp.GetStackSize();
    sz = (char*)cTemp.Init();
    for (;;)
    {
        uiLength = GetModuleFileName(NULL, sz, uiSize);
        if (uiLength < uiSize)
        {
            break;
        }
        cTemp.Kill();
        uiSize *= 2;
        sz = (char*)cTemp.Init(uiSize);
    }

    szCommandLine.Init();
    AppendQuotedArgument(&szCommandLine, sz);
    cTemp.Kill();

    ZeroMemory(&sStartUpInfo, sizeof(STARTUPINFO));
    sStartUpInfo.cb = sizeof(STARTUPINFO);
    ZeroMemory(&sProcessInformation, sizeof(PROCESS_INFORMATION));

    uiNumParameters = paszParameters->NumElements();
    for (i = 0; i < uiNumParameters; i++)
    {
        szCommandLine.Append(" ");
        AppendQuotedArgument(&szCommandLine, paszParameters->GetText(i));
    }

    // Start the child process. 
    if (!CreateProcess(NULL,    // No module name (use command line)
        szCommandLine.Text(),
        NULL,                   // Process handle not inheritable
        NULL,                   // Thread handle not inheritable
        false,                  // Set handle inheritance to false
        0,                      // No creation flags
        NULL,                   // Use parent's environment block
        NULL,                   // Use parent's starting directory 
        &sStartUpInfo,          // Pointer to STARTUPINFO structure
        &sProcessInformation))  // Pointer to PROCESS_INFORMATION structure
    {
        szCommandLine.Kill();
        gcLogger.Error2(__METHOD__, "CreateProcess failed [", WindowsErrorCodeToString(GetLastError()), "].", NULL);
        return false;
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

    return true;
}


#endif // _WIN32

