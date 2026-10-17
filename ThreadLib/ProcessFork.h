#ifndef __PROCESS_FORK_H__
#define __PROCESS_FORK_H__
#include "BaseLib/ArrayChars.h"


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

