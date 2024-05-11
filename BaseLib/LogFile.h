#ifndef __LOG_FILE_H__
#define __LOG_FILE_H__
#include "AbstractFile.h"
#include "ListVariable.h"
#include "LogFileCommands.h"


//This is an IO operation write log.  Not an error logging utility.
class CLogFile : public CAbstractFile
{
protected:
	CListVariable		macCommands;
	filePos				miPosition;
	filePos				miLength;

	filePos				miBackingFileLength;
	CAbstractFile*		mpcBackingFile;
	bool				mbTouched;
	EFileMode			meFileMode;

	bool				mbOpenedBackingFile;
	bool				mbBegun;
	bool				mbBackingFileExists;

public:
	void					Init(CAbstractFile* pcBackingFile);
	void					Kill(void);

	bool					Begin(void);
	bool					Commit(void);
	bool					End(void);

	bool					Open(EFileMode eFileMode);
	bool					Close(void);
	size					Write(const void* pvSource, size iSize, size iCount);
	size					Read(void* pvDest, size iSize, size iCount);
	filePos					Tell(void);
	filePos					Size(void);
	bool					Truncate(filePos iSize);
	bool					Seek(filePos iOffset, EFileSeekOrigin iSeekOrigin);
	bool					Eof(void);
	bool					IsOpen(void);
	bool					Flush(void);
	bool					Delete(void);
	char*					GetFilename(void);

	bool					Commit(CAbstractFile* pcFile);

	void					Dump(void);
	size					GetNumWrites(void);
	size					GetWriteSize(size iIndex);
	size					GetNumCommands(void);
	CLogFileCommandWrite*	GetWriteData(size iWrite);
	bool					TestFindHoles(size iWriteIndex, CArrayIntAndPointer* papvOverlapping, filePos iPosition, size iLength);
	bool					IsBegun(void);

protected:
	bool					FindTouchingWriteCommands(size iStartIndex, CArrayIntAndPointer* papvOverlapping, filePos iPosition, size iLength, bool bMustOverlap);
	bool					Overlaps(filePos iPosition, size iLength, CLogFileCommandWrite* psWrite);
	bool					AmalgamateOverlappingWrites(CArrayIntAndPointer* papvOverlapping, const void* pvSource, filePos iPosition, size iLength);
	bool					FindHoles(CArrayIntAndPointer* papvOverlapping, filePos iPosition, size iLength);
	void					UpdateLength(void);
	size					ReadFromBackingFile(void* pvDest, size iSize, size iCount);
	size					ReadWithNoTouchingWrites(void* pvDest, size iSize, size iCount);
	size					ReadFirstTouchingWrites(size iWriteIndex, void* pvDest, size iSize, size iCount);
	size					ReadNextTouchingWrites(size iWriteIndex, void* pvDest, size iSize, size iCount);
	void					CopyWritesToRead(CArrayIntAndPointer* papvOverlapping, size iByteSize, void* pvDest);
	size					FindNextWriteCommand(size iIndex);

	bool					ValidateBegun(char* szMethod, char* szTask, CAbstractFile* pcFile);

	void					GetSafeFilename(CChars* pszDest, CAbstractFile* pcFile);
	void					ExecuteCommandErrorString(CChars* pszDest, char* szMethod, CLogFileCommand* psCommand, CAbstractFile* pcFile);
	void					AddCommandErrorString(CChars* pszDest, char* szMethod, CLogFileCommand* psCommand, CAbstractFile* pcFile);

	CLogFileCommandOpen*	AddOpenCommand(EFileMode eFileMode);
	CLogFileCommandWrite*	AddWriteCommand(filePos iPosition, void* pvSource, size iByteLength);
	CLogFileCommandWrite*	AddWriteCommand(filePos iPosition, size iByteLength);
	CLogFileCommandClose*	AddCloseCommand(void);
	CLogFileCommandDelete*	AddDeleteCommand(void);
};


CLogFile* LogFile(CAbstractFile* pcFile);


#endif // __LOG_FILE_H__

