#ifndef __LOG_FILE_COMMANDS_H__
#define __LOG_FILE_COMMANDS_H__
#include "BaseLib/FileMode.h"
#include "BaseLib/AbstractFile.h"


#define LOG_FILE_COMMAND_CHUNK_SIZE	4096


enum ELogFileCommand
{
	LFC_Open,
	LFC_Write,
	LFC_Close,
	LFC_Delete
};


class CLogFileCommand
{
public:
	ELogFileCommand		eCommand;

	void Init(ELogFileCommand eCommand);

	BOOL IsWrite(void);
	BOOL IsOpen(void);
	BOOL IsClose(void);
	BOOL IsDelete(void);
};


class CLogFileCommandOpen : public CLogFileCommand
{
public:
	EFileMode			eMode;

	void Init(EFileMode eMode);

	BOOL Open(CAbstractFile* pcFile);
};


class CLogFileCommandWrite : public CLogFileCommand
{
public:
	filePos				iPosition;
	filePos				iSize;

	void	Init(filePos iPosition, filePos iSize);
	void	Init(filePos iPosition, void* pvSource, filePos iSize);

	BOOL	Write(CAbstractFile* pcFile);
	void*	GetData(void);
};


class CLogFileCommandClose : public CLogFileCommand
{
public:
	void Init(void);

	BOOL Close(CAbstractFile* pcFile);
};


class CLogFileCommandDelete : public CLogFileCommand
{
public:
	void Init(void);

	BOOL Delete(CAbstractFile* pcFile);
};


#endif // __LOG_FILE_COMMANDS_H__

