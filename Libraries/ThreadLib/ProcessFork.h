#ifndef __PROCESS_FORK_H__
#define __PROCESS_FORK_H__
#include "BaseLib/ArrayChars.h"


#ifdef _WIN32
//Appends szArgument to a Windows command line, quoted so the C runtime splits it back into the same argument.
void AppendQuotedArgument(CChars* psz, char* szArgument);
#endif // _WIN32


//The full name of this program's executable.
bool GetThisExecutable(CChars* pszDest);

//Starts this program again as a new process.  Each parameter is passed as one
//command line argument, even if it contains spaces.
bool ForkProcess(CArrayChars* paszParameters, bool bWaitForProcessToExit);

bool ForkProcess(char* szParameter1, bool bWaitForProcessToExit);
bool ForkProcess(char* szParameter1, char* szParameter2, bool bWaitForProcessToExit);
bool ForkProcess(char* szParameter1, char* szParameter2, char* szParameter3, bool bWaitForProcessToExit);
bool ForkProcess(char* szParameter1, char* szParameter2, char* szParameter3, char* szParameter4, bool bWaitForProcessToExit);
bool ForkProcess(char* szParameter1, char* szParameter2, char* szParameter3, char* szParameter4, char* szParameter5, bool bWaitForProcessToExit);
bool ForkProcess(char* szParameter1, char* szParameter2, char* szParameter3, char* szParameter4, char* szParameter5, char* szParameter6, bool bWaitForProcessToExit);


#endif // __PROCESS_FORK_H__

