#ifndef __WINDOWS_ERROR_H__
#define __WINDOWS_ERROR_H__
#ifdef _WIN32
#include "WindowsHeaders.h"
#include "LogString.h"
#include "StringHelper.h"


char* WindowsErrorCodeToString(DWORD errorMessageID);


#endif // _WIN32
#endif // __WINDOWS_ERROR_H__

