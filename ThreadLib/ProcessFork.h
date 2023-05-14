#ifndef __PROCESS_FORK_H__
#define __PROCESS_FORK_H__


bool ForkProcess(char* szCommandLineParameters, bool bWaitForProcessToExit);

bool ForkProcess(char* szParameter1, char* szParameter2, bool bWaitForProcessToExit);
bool ForkProcess(char* szParameter1, char* szParameter2, char* szParameter3, bool bWaitForProcessToExit);
bool ForkProcess(char* szParameter1, char* szParameter2, char* szParameter3, char* szParameter4, bool bWaitForProcessToExit);
bool ForkProcess(char* szParameter1, char* szParameter2, char* szParameter3, char* szParameter4, char* szParameter5, bool bWaitForProcessToExit);
bool ForkProcess(char* szParameter1, char* szParameter2, char* szParameter3, char* szParameter4, char* szParameter5, char* szParameter6, bool bWaitForProcessToExit);


#endif // !__PROCESS_FORK_H__

