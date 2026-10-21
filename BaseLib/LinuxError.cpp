#ifdef LINUX_GNU
#include <string.h>
#include "LinuxError.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* LinuxErrorCodeToString(int iErrno)
{
    size    iCount = IncrementLogToStringCount();
    char    szBuffer[LOG_TO_STRING_MAX_LENGTH];
    char*   szMessage;

    if (iErrno == 0)
    {
        gaszLogToStringScratchPad[iCount][0] = '\0';
    }
    else
    {
        //The GNU strerror_r returns the message, which may or may not be in szBuffer.
        szMessage = strerror_r(iErrno, szBuffer, LOG_TO_STRING_MAX_LENGTH);
        StrCpySafe(gaszLogToStringScratchPad[iCount], szMessage, LOG_TO_STRING_MAX_LENGTH);
    }
    return gaszLogToStringScratchPad[iCount];
}


#endif // LINUX_GNU

