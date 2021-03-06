#ifndef __PROCESS_FORK_H__
#define __PROCESS_FORK_H__


BOOL ForkProcess(char* szCommandLineParameters, BOOL bWaitForProcessToExit);

BOOL ForkProcess(char* szParameter1, char* szParameter2, BOOL bWaitForProcessToExit);
BOOL ForkProcess(char* szParameter1, char* szParameter2, char* szParameter3, BOOL bWaitForProcessToExit);
BOOL ForkProcess(char* szParameter1, char* szParameter2, char* szParameter3, char* szParameter4, BOOL bWaitForProcessToExit);
BOOL ForkProcess(char* szParameter1, char* szParameter2, char* szParameter3, char* szParameter4, char* szParameter5, BOOL bWaitForProcessToExit);
BOOL ForkProcess(char* szParameter1, char* szParameter2, char* szParameter3, char* szParameter4, char* szParameter5, char* szParameter6, BOOL bWaitForProcessToExit);


#endif // !__PROCESS_FORK_H__

