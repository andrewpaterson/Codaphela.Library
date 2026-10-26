//The host file device for the 65C64, in C++ for the 65C64 generator.  The arguments are stored, then the
//command; the result is ready as soon as the command is stored.
#include <host.h>


#define HOST_DEVICE			0x00FF0200
#define HOST_ARGUMENT_0		(*(volatile uint32_t*)(HOST_DEVICE + 0x00))
#define HOST_ARGUMENT_1		(*(volatile uint32_t*)(HOST_DEVICE + 0x04))
#define HOST_ARGUMENT_2		(*(volatile uint32_t*)(HOST_DEVICE + 0x08))
#define HOST_ARGUMENT_3		(*(volatile uint32_t*)(HOST_DEVICE + 0x0C))
#define HOST_RESULT			(*(volatile uint32_t*)(HOST_DEVICE + 0x10))
#define HOST_RESULT_HIGH	(*(volatile uint32_t*)(HOST_DEVICE + 0x14))
#define HOST_COMMAND		(*(volatile unsigned char*)(HOST_DEVICE + 0x18))

#define HOST_OPEN			1
#define HOST_CLOSE			2
#define HOST_READ_FILE		3
#define HOST_WRITE_FILE		4
#define HOST_SEEK			5
#define HOST_TELL			6
#define HOST_SIZE			7
#define HOST_TRUNCATE_FILE	8
#define HOST_FLUSH			9
#define HOST_DELETE			10
#define HOST_EXISTS			11
#define HOST_FILE_SIZE		12
#define HOST_MAKE_DIR		13
#define HOST_REMOVE_DIR		14
#define HOST_IS_DIRECTORY	15
#define HOST_TOUCH			16
#define HOST_CURRENT_DIR	17
#define HOST_FIND_FILE_CMD	18
#define HOST_RENAME			19


static uint32_t Command(int iCommand)
{
	HOST_COMMAND = (unsigned char)iCommand;
	return HOST_RESULT;
}


static long long CommandLong(int iCommand)
{
	uint32_t	uiLow;
	uint32_t	uiHigh;

	HOST_COMMAND = (unsigned char)iCommand;
	uiLow = HOST_RESULT;
	uiHigh = HOST_RESULT_HIGH;
	return (long long)(((unsigned long long)uiHigh << 32) | uiLow);
}


static uint32_t PathCommand(int iCommand, const char* szPath)
{
	HOST_ARGUMENT_0 = (uint32_t)szPath;
	return Command(iCommand);
}


int HostOpen(const char* szPath, int iMode)
{
	HOST_ARGUMENT_0 = (uint32_t)szPath;
	HOST_ARGUMENT_1 = (uint32_t)iMode;
	return (int)Command(HOST_OPEN);
}


int HostClose(int iHandle)
{
	HOST_ARGUMENT_0 = (uint32_t)iHandle;
	return (int)Command(HOST_CLOSE);
}


uint32_t HostRead(int iHandle, void* pvBuffer, uint32_t uiBytes)
{
	HOST_ARGUMENT_0 = (uint32_t)iHandle;
	HOST_ARGUMENT_1 = (uint32_t)pvBuffer;
	HOST_ARGUMENT_2 = uiBytes;
	return Command(HOST_READ_FILE);
}


uint32_t HostWrite(int iHandle, const void* pvBuffer, uint32_t uiBytes)
{
	HOST_ARGUMENT_0 = (uint32_t)iHandle;
	HOST_ARGUMENT_1 = (uint32_t)pvBuffer;
	HOST_ARGUMENT_2 = uiBytes;
	return Command(HOST_WRITE_FILE);
}


int HostSeek(int iHandle, long long llOffset, int iOrigin)
{
	HOST_ARGUMENT_0 = (uint32_t)iHandle;
	HOST_ARGUMENT_1 = (uint32_t)llOffset;
	HOST_ARGUMENT_2 = (uint32_t)((unsigned long long)llOffset >> 32);
	HOST_ARGUMENT_3 = (uint32_t)iOrigin;
	return (int)Command(HOST_SEEK);
}


long long HostTell(int iHandle)
{
	HOST_ARGUMENT_0 = (uint32_t)iHandle;
	return CommandLong(HOST_TELL);
}


long long HostSize(int iHandle)
{
	HOST_ARGUMENT_0 = (uint32_t)iHandle;
	return CommandLong(HOST_SIZE);
}


int HostTruncate(int iHandle, long long llSize)
{
	HOST_ARGUMENT_0 = (uint32_t)iHandle;
	HOST_ARGUMENT_1 = (uint32_t)llSize;
	HOST_ARGUMENT_2 = (uint32_t)((unsigned long long)llSize >> 32);
	return (int)Command(HOST_TRUNCATE_FILE);
}


int HostFlush(int iHandle)
{
	HOST_ARGUMENT_0 = (uint32_t)iHandle;
	return (int)Command(HOST_FLUSH);
}


int HostDelete(const char* szPath)
{
	return (int)PathCommand(HOST_DELETE, szPath);
}


int HostRename(const char* szOld, const char* szNew)
{
	HOST_ARGUMENT_0 = (uint32_t)szOld;
	HOST_ARGUMENT_1 = (uint32_t)szNew;
	return (int)Command(HOST_RENAME);
}


int HostExists(const char* szPath)
{
	return (int)PathCommand(HOST_EXISTS, szPath);
}


long long HostFileSize(const char* szPath)
{
	HOST_ARGUMENT_0 = (uint32_t)szPath;
	return CommandLong(HOST_FILE_SIZE);
}


int HostMakeDir(const char* szPath)
{
	return (int)PathCommand(HOST_MAKE_DIR, szPath);
}


int HostRemoveDir(const char* szPath)
{
	return (int)PathCommand(HOST_REMOVE_DIR, szPath);
}


int HostIsDirectory(const char* szPath)
{
	return (int)PathCommand(HOST_IS_DIRECTORY, szPath);
}


int HostTouch(const char* szPath)
{
	return (int)PathCommand(HOST_TOUCH, szPath);
}


int HostCurrentDirectory(char* szDest, uint32_t uiSize)
{
	HOST_ARGUMENT_0 = (uint32_t)szDest;
	HOST_ARGUMENT_1 = uiSize;
	return (int)Command(HOST_CURRENT_DIR);
}


int HostFindFile(const char* szDirectory, uint32_t uiIndex, char* szName, uint32_t uiSize)
{
	HOST_ARGUMENT_0 = (uint32_t)szDirectory;
	HOST_ARGUMENT_1 = uiIndex;
	HOST_ARGUMENT_2 = (uint32_t)szName;
	HOST_ARGUMENT_3 = uiSize;
	return (int)Command(HOST_FIND_FILE_CMD);
}

