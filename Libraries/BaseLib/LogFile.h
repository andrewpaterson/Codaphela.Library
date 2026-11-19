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
	bool				mbSync;

public:
	void					Init(CAbstractFile* pcBackingFile);
	void					Kill(void) override;

	bool					Begin(void);
	bool					Commit(void);
	bool					End(void);
	void					Abort(void);

	bool					Open(EFileMode eFileMode) override;
	bool					Close(void) override;
	lsize					Write(const void* pvSource, lsize iSize, lsize iCount) override;
	lsize					Read(void* pvDest, lsize iSize, lsize iCount) override;
	filePos					Tell(void) override;
	filePos					Size(void) override;
	bool					Truncate(filePos iSize) override;
	bool					Seek(filePos iOffset, EFileSeekOrigin iSeekOrigin) override;
	bool					Eof(void);
	bool					IsOpen(void) override;
	bool					Flush(void) override;
	bool					Delete(void) override;
	const char*				GetFilename(void) const override;

	bool					Commit(CAbstractFile* pcFile);

	void					Print(CChars* psz);
	void					Dump(void);
	size					GetNumWrites(void);
	lsize					GetWriteSize(size iIndex);
	size					GetNumCommands(void) const;
	CLogFileCommandWrite*	GetWriteData(size iWrite);
	bool					TestFindHoles(size iWriteIndex, CArrayIntAndPointer* papvOverlapping, filePos iPosition, lsize iLength);
	bool					IsBegun(void) const;
	void					SetSync(bool bSync);

protected:
	bool					FindTouchingWriteCommands(size iStartIndex, CArrayIntAndPointer* papvOverlapping, filePos iPosition, lsize iLength, bool bMustOverlap);
	bool					Overlaps(filePos iPosition, lsize iLength, CLogFileCommandWrite* psWrite);
	bool					AmalgamateOverlappingWrites(CArrayIntAndPointer* papvOverlapping, const void* pvSource, filePos iPosition, lsize iLength);
	bool					ExtendLastWrite(CArrayIntAndPointer* papvOverlapping, const void* pvSource, filePos iPosition, lsize iLength);
	bool					FindHoles(CArrayIntAndPointer* papvOverlapping, filePos iPosition, lsize iLength);
	void					UpdateLength(void);
	lsize					ReadFromBackingFile(void* pvDest, lsize iSize, lsize iCount);
	lsize					ReadWithNoTouchingWrites(void* pvDest, lsize iSize, lsize iCount);
	lsize					ReadFirstTouchingWrites(size iWriteIndex, void* pvDest, lsize iSize, lsize iCount);
	lsize					ReadNextTouchingWrites(size iWriteIndex, void* pvDest, lsize iSize, lsize iCount);
	void					CopyWritesToRead(CArrayIntAndPointer* papvOverlapping, lsize iByteSize, void* pvDest);
	size					FindNextWriteCommand(size iIndex);

	bool					FailBegun(char* szMethod, const char* szTask, CAbstractFile* pcFile);
	bool					FailFlush(char* szMethod, CAbstractFile* pcFile);

	void					GetSafeFilename(CChars* pszDest, CAbstractFile* pcFile);
	void					ExecuteCommandErrorString(CChars* pszDest, char* szMethod, CLogFileCommand* psCommand, CAbstractFile* pcFile);
	void					AddCommandErrorString(CChars* pszDest, char* szMethod, CLogFileCommand* psCommand, CAbstractFile* pcFile);

	CLogFileCommandOpen*	AddOpenCommand(EFileMode eFileMode);
	CLogFileCommandWrite*	AddWriteCommand(filePos iPosition, void* pvSource, lsize iByteLength);
	CLogFileCommandWrite*	AddWriteCommand(filePos iPosition, lsize iByteLength);
	CLogFileCommandClose*	AddCloseCommand(void);
	CLogFileCommandDelete*	AddDeleteCommand(void);
	CLogFileCommandTruncate*	AddTruncateCommand(filePos iSize);
};


CLogFile* LogFile(CAbstractFile* pcFile);


#endif // __LOG_FILE_H__

