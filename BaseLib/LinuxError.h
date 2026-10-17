#ifndef __LINUX_ERROR_H__
#define __LINUX_ERROR_H__
#ifdef LINUX_GNU_32
#include "LogString.h"
#include "StringHelper.h"


char* LinuxErrorCodeToString(int iErrno);


#endif // LINUX_GNU_32
#endif // __LINUX_ERROR_H__

