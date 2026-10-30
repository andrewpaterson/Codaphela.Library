//Memory and string functions.
#ifndef __LIBC65_STRING_H__
#define __LIBC65_STRING_H__
#include <stddef.h>


extern "C"
{
void*	memcpy(void* pvDest, const void* pvSource, size_t uiBytes);
void*	memmove(void* pvDest, const void* pvSource, size_t uiBytes);
void*	memset(void* pvDest, int iValue, size_t uiBytes);
int		memcmp(const void* pv1, const void* pv2, size_t uiBytes);
void*	memchr(const void* pv, int iValue, size_t uiBytes);

size_t	strlen(const char* sz);
char*	strcpy(char* szDest, const char* szSource);
char*	strncpy(char* szDest, const char* szSource, size_t uiChars);
char*	strcat(char* szDest, const char* szSource);
char*	strncat(char* szDest, const char* szSource, size_t uiChars);
int		strcmp(const char* sz1, const char* sz2);
int		strncmp(const char* sz1, const char* sz2, size_t uiChars);
char*	strchr(const char* sz, int c);
char*	strrchr(const char* sz, int c);
char*	strstr(const char* sz, const char* szFind);

#if defined(__W65C816__)
//16 bit counts (string816_16.asm), for BaseLib's natural arrays: at most 65535 bytes.
void*	memcpy16(void* pvDest, const void* pvSource, unsigned short uiBytes);
void*	memmove16(void* pvDest, const void* pvSource, unsigned short uiBytes);
void*	memset16(void* pvDest, int iValue, unsigned short uiBytes);
int		memcmp16(const void* pv1, const void* pv2, unsigned short uiBytes);
#endif // __W65C816__
}


#endif // __LIBC65_STRING_H__
