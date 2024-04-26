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
	int					miLastWriteOpenIndex;
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
	filePos					Write(const void* pvSource, filePos iSize, filePos iCount);
	filePos					Read(void* pvDest, filePos iSize, filePos iCount);
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
	int						GetNumWrites(void);
	filePos					GetWriteSize(int iIndex);
	int						GetNumCommands(void);
	CLogFileCommandWrite*	GetWriteData(int iWrite);
	bool					TestFindHoles(int iWriteIndex, CArrayIntAndPointer* papvOverlapping, filePos iPosition, filePos iLength);
	bool					IsBegun(void);

protected:
	bool					FindTouchingWriteCommands(int iStartIndex, CArrayIntAndPointer* papvOverlapping, filePos iPosition, filePos iLength, bool bMustOverlap);
	bool					Overlaps(filePos iPosition, filePos iLength, CLogFileCommandWrite* psWrite);
	bool					AmalgamateOverlappingWrites(CArrayIntAndPointer* papvOverlapping, const void* pvSource, filePos iPosition, filePos iLength);
	bool					FindHoles(CArrayIntAndPointer* papvOverlapping, filePos iPosition, filePos iLength);
	void					UpdateLength(void);
	filePos					ReadFromBackingFile(void* pvDest, filePos iSize, filePos iCount);
	filePos					ReadWithNoTouchingWrites(void* pvDest, filePos iSize, filePos iCount);
	filePos					ReadFirstTouchingWrites(int iWriteIndex, void* pvDest, filePos iSize, filePos iCount);
	filePos					ReadNextTouchingWrites(int iWriteIndex, void* pvDest, filePos iSize, filePos iCount);
	void					CopyWritesToRead(CArrayIntAndPointer* papvOverlapping, filePos iByteSize, void* pvDest);
	int						FindNextWriteCommand(int iIndex);

	bool					ValidateBegun(char* szMethod, char* szTask, CAbstractFile* pcFile);

	void					GetSafeFilename(CChars* pszDest, CAbstractFile* pcFile);
	void					ExecuteCommandErrorString(CChars* pszDest, char* szMethod, CLogFileCommand* psCommand, CAbstractFile* pcFile);
	void					AddCommandErrorString(CChars* pszDest, char* szMethod, CLogFileCommand* psCommand, CAbstractFile* pcFile);

	CLogFileCommandOpen*	AddOpenCommand(EFileMode eFileMode);
	CLogFileCommandWrite*	AddWriteCommand(filePos iPosition, void* pvSource, filePos iByteLength);
	CLogFileCommandWrite*	AddWriteCommand(filePos iPosition, filePos iByteLength);
	CLogFileCommandClose*	AddCloseCommand(void);
	CLogFileCommandDelete*	AddDeleteCommand(void);
};


CLogFile* LogFile(CAbstractFile* pcFile);


#endif // __LOG_FILE_H__

