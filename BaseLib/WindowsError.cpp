#include "WindowsError.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* WindowsErrorCodeToString(DWORD errorMessageID)
{
#ifdef _WIN32
    int iCount = IncrementLogToStringCount();
    int iLen;

    if (errorMessageID == 0)
    {
        gaszLogToStringScratchPad[iCount][0] = '\0';
    }
    else
    {
        LPSTR messageBuffer = nullptr;
        size size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

        StrCpySafe(gaszLogToStringScratchPad[iCount], messageBuffer, LOG_TO_STRING_MAX_LENGTH);
        iLen = strlen(gaszLogToStringScratchPad[iCount]);
        gaszLogToStringScratchPad[iCount][iLen - 2] = '\0';

        LocalFree(messageBuffer);
    }
    return gaszLogToStringScratchPad[iCount];
#elif
    return "";
#endif
}

