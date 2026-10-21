#ifndef __LINUX_ERROR_H__
#define __LINUX_ERROR_H__
#ifdef LINUX_GNU
#include "LogString.h"
#include "StringHelper.h"


char* LinuxErrorCodeToString(int iErrno);


#endif // LINUX_GNU
#endif // __LINUX_ERROR_H__

