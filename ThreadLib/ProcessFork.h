#ifndef __PROCESS_FORK_H__
#define __PROCESS_FORK_H__

BOOL ForkProcess(char* szCommandLineParameters, BOOL bWaitForProcessToExit);
BOOL ForkProcess(char* szParameter1, char* szParameter2, BOOL bWaitForProcessToExit);
BOOL ForkProcess(char* szParameter1, char* szParameter2, char* szParameter3, BOOL bWaitForProcessToExit);

#endif // __PROCESS_FORK_H__



