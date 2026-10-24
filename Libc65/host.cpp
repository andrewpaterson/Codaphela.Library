//The host file device for the 65C64, in C++ for the 65C64 generator.  The arguments are stored, then the
//command; the result is ready as soon as the command is stored.
#include <host.h>


#define HOST_DEVICE			0x00FF0200
#define HOST_ARGUMENT_0		(*(volatile unsigned int*)(HOST_DEVICE + 0x00))
#define HOST_ARGUMENT_1		(*(volatile unsigned int*)(HOST_DEVICE + 0x04))
#define HOST_ARGUMENT_2		(*(volatile unsigned int*)(HOST_DEVICE + 0x08))
#define HOST_ARGUMENT_3		(*(volatile unsigned int*)(HOST_DEVICE + 0x0C))
#define HOST_RESULT			(*(volatile unsigned int*)(HOST_DEVICE + 0x10))
#define HOST_RESULT_HIGH	(*(volatile unsigned int*)(HOST_DEVICE + 0x14))
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


static unsigned int Command(int iCommand)
{
	HOST_COMMAND = (unsigned char)iCommand;
	return HOST_RESULT;
}


static long long CommandLong(int iCommand)
{
	unsigned int	uiLow;
	unsigned int	uiHigh;

	HOST_COMMAND = (unsigned char)iCommand;
	uiLow = HOST_RESULT;
	uiHigh = HOST_RESULT_HIGH;
	return (long long)(((unsigned long long)uiHigh << 32) | uiLow);
}


static unsigned int PathCommand(int iCommand, const char* szPath)
{
	HOST_ARGUMENT_0 = (unsigned int)szPath;
	return Command(iCommand);
}


int HostOpen(const char* szPath, int iMode)
{
	HOST_ARGUMENT_0 = (unsigned int)szPath;
	HOST_ARGUMENT_1 = (unsigned int)iMode;
	return (int)Command(HOST_OPEN);
}


int HostClose(int iHandle)
{
	HOST_ARGUMENT_0 = (unsigned int)iHandle;
	return (int)Command(HOST_CLOSE);
}


unsigned int HostRead(int iHandle, void* pvBuffer, unsigned int uiBytes)
{
	HOST_ARGUMENT_0 = (unsigned int)iHandle;
	HOST_ARGUMENT_1 = (unsigned int)pvBuffer;
	HOST_ARGUMENT_2 = uiBytes;
	return Command(HOST_READ_FILE);
}


unsigned int HostWrite(int iHandle, const void* pvBuffer, unsigned int uiBytes)
{
	HOST_ARGUMENT_0 = (unsigned int)iHandle;
	HOST_ARGUMENT_1 = (unsigned int)pvBuffer;
	HOST_ARGUMENT_2 = uiBytes;
	return Command(HOST_WRITE_FILE);
}


int HostSeek(int iHandle, long long llOffset, int iOrigin)
{
	HOST_ARGUMENT_0 = (unsigned int)iHandle;
	HOST_ARGUMENT_1 = (unsigned int)llOffset;
	HOST_ARGUMENT_2 = (unsigned int)((unsigned long long)llOffset >> 32);
	HOST_ARGUMENT_3 = (unsigned int)iOrigin;
	return (int)Command(HOST_SEEK);
}


long long HostTell(int iHandle)
{
	HOST_ARGUMENT_0 = (unsigned int)iHandle;
	return CommandLong(HOST_TELL);
}


long long HostSize(int iHandle)
{
	HOST_ARGUMENT_0 = (unsigned int)iHandle;
	return CommandLong(HOST_SIZE);
}


int HostTruncate(int iHandle, long long llSize)
{
	HOST_ARGUMENT_0 = (unsigned int)iHandle;
	HOST_ARGUMENT_1 = (unsigned int)llSize;
	HOST_ARGUMENT_2 = (unsigned int)((unsigned long long)llSize >> 32);
	return (int)Command(HOST_TRUNCATE_FILE);
}


int HostFlush(int iHandle)
{
	HOST_ARGUMENT_0 = (unsigned int)iHandle;
	return (int)Command(HOST_FLUSH);
}


int HostDelete(const char* szPath)
{
	return (int)PathCommand(HOST_DELETE, szPath);
}


int HostRename(const char* szOld, const char* szNew)
{
	HOST_ARGUMENT_0 = (unsigned int)szOld;
	HOST_ARGUMENT_1 = (unsigned int)szNew;
	return (int)Command(HOST_RENAME);
}


int HostExists(const char* szPath)
{
	return (int)PathCommand(HOST_EXISTS, szPath);
}


long long HostFileSize(const char* szPath)
{
	HOST_ARGUMENT_0 = (unsigned int)szPath;
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


int HostCurrentDirectory(char* szDest, unsigned int uiSize)
{
	HOST_ARGUMENT_0 = (unsigned int)szDest;
	HOST_ARGUMENT_1 = uiSize;
	return (int)Command(HOST_CURRENT_DIR);
}


int HostFindFile(const char* szDirectory, unsigned int uiIndex, char* szName, unsigned int uiSize)
{
	HOST_ARGUMENT_0 = (unsigned int)szDirectory;
	HOST_ARGUMENT_1 = uiIndex;
	HOST_ARGUMENT_2 = (unsigned int)szName;
	HOST_ARGUMENT_3 = uiSize;
	return (int)Command(HOST_FIND_FILE_CMD);
}

