//The host's files: the simulator's file system, through the host file device at 00FF:0200 (see
//Assembler65Lib/Asm65HostFiles.h).  Paths are the host's.  A handle is 0 when a file could not be opened.
#ifndef __LIBC65_HOST_H__
#define __LIBC65_HOST_H__
#include <stdint.h>


//HostOpen's modes.
#define HOST_READ		0x01
#define HOST_WRITE		0x02
#define HOST_CREATE		0x04		//Create the file if it does not exist.
#define HOST_TRUNCATE	0x08		//Empty the file.
#define HOST_APPEND		0x10		//Every write goes at the end.

//HostSeek's origins.
#define HOST_SEEK_SET	0
#define HOST_SEEK_CUR	1
#define HOST_SEEK_END	2

//HostFindFile's results.
#define HOST_FIND_NONE		0
#define HOST_FIND_FILE		1
#define HOST_FIND_DIRECTORY	2


extern "C"
{
int				HostOpen(const char* szPath, int iMode);
int				HostClose(int iHandle);
uint32_t		HostRead(int iHandle, void* pvBuffer, uint32_t uiBytes);
uint32_t		HostWrite(int iHandle, const void* pvBuffer, uint32_t uiBytes);
int				HostSeek(int iHandle, long long llOffset, int iOrigin);
long long		HostTell(int iHandle);
long long		HostSize(int iHandle);
int				HostTruncate(int iHandle, long long llSize);
int				HostFlush(int iHandle);

int				HostDelete(const char* szPath);
int				HostRename(const char* szOld, const char* szNew);
int				HostExists(const char* szPath);
long long		HostFileSize(const char* szPath);
int				HostMakeDir(const char* szPath);
int				HostRemoveDir(const char* szPath);
int				HostIsDirectory(const char* szPath);
int				HostTouch(const char* szPath);
int				HostCurrentDirectory(char* szDest, uint32_t uiSize);
int				HostFindFile(const char* szDirectory, uint32_t uiIndex, char* szName, uint32_t uiSize);
}


#endif // __LIBC65_HOST_H__

