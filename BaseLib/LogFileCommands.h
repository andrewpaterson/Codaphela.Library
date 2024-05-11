#ifndef __LOG_FILE_COMMANDS_H__
#define __LOG_FILE_COMMANDS_H__
#include "FileMode.h"
#include "AbstractFile.h"



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

	bool	IsWrite(void);
	bool	IsOpen(void);
	bool	IsClose(void);
	bool	IsDelete(void);

	char*	GetType(void);
};


class CLogFileCommandOpen : public CLogFileCommand
{
public:
	EFileMode			eMode;

	void Init(EFileMode eMode);

	bool Open(CAbstractFile* pcFile);
};


class CLogFileCommandWrite : public CLogFileCommand
{
public:
	filePos		iPosition;
	size		iSize;

	void	Init(filePos iPosition, size iSize);
	void	Init(filePos iPosition, void* pvSource, size iSize);

	bool	Write(CAbstractFile* pcFile);
	void*	GetData(void);
};


class CLogFileCommandClose : public CLogFileCommand
{
public:
	void Init(void);

	bool Close(CAbstractFile* pcFile);
};


class CLogFileCommandDelete : public CLogFileCommand
{
public:
	void Init(void);

	bool Delete(CAbstractFile* pcFile);
};


#endif // __LOG_FILE_COMMANDS_H__

