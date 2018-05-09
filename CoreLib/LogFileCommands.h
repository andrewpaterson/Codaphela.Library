#ifndef __LOG_FILE_COMMANDS_H__
#define __LOG_FILE_COMMANDS_H__
#include "BaseLib/FileMode.h"
#include "BaseLib/AbstractFile.h"



enum ELogFileCommand
{
	LFC_Open,
	LFC_Write,
	LFC_Close,
	LFC_Delete

//	LFC_Truncate
};


class CLogFileCommand
{
public:
	ELogFileCommand		eCommand;

	void	Init(ELogFileCommand eCommand);

	BOOL	IsWrite(void);
	BOOL	IsOpen(void);
	BOOL	IsClose(void);
	BOOL	IsDelete(void);

	char*	GetType(void);
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

