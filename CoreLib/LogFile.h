#ifndef __LOG_FILE_H__
#define __LOG_FILE_H__
#include "BaseLib/AbstractFile.h"
#include "ArrayVariable.h"
#include "LogFileCommands.h"



class CLogFile : public CAbstractFile
{
protected:
	CArrayVariable		macCommands;
	filePos				miPosition;
	filePos				miLength;

	filePos				miFileLength;
	CAbstractFile*		mpcBackingFile;
	BOOL				mbTouched;
	EFileMode			meFileMode;

	BOOL				mbOpenedBackingFile;
	int					miLastWriteOpenIndex;

public:
	void		Init(CAbstractFile* pcBackingFile);
	void		Kill(void);

	BOOL		Begin(void);
	BOOL		Commit(void);
	BOOL		Commit(CAbstractFile* pcFile);

	BOOL		Open(EFileMode eFileMode);
	BOOL		Close(void);
	filePos		Write(const void* pvSource, filePos iSize, filePos iCount);
	filePos		Read(void* pvDest, filePos iSize, filePos iCount);
	filePos		Tell(void);
	filePos		Size(void);
	BOOL		Seek(filePos iOffset, EFileSeekOrigin iSeekOrigin);
	BOOL		Eof(void);
	BOOL		IsOpen(void);
	BOOL		Flush(void);
	BOOL		Delete(void);

	void		Dump(void);
	int			GetNumWrites(void);
	filePos		GetWriteSize(int iIndex);
	int			GetNumCommands(void);
	BOOL		TestFindHoles(int iWriteIndex, CArrayIntAndPointer* papvOverlapping, filePos iPosition, filePos iLength);

protected:
	BOOL					FindTouchingWriteCommands(int iStartIndex, CArrayIntAndPointer* papvOverlapping, filePos iPosition, filePos iLength, BOOL bMustOverlap);
	BOOL					Overlaps(filePos iPosition, filePos iLength, CLogFileCommandWrite* psWrite);
	BOOL					AmalgamateOverlappingWrites(CArrayIntAndPointer* papvOverlapping, const void* pvSource, filePos iPosition, filePos iLength);
	BOOL					FindHoles(CArrayIntAndPointer* papvOverlapping, filePos iPosition, filePos iLength);
	void					UpdateLength(void);
	filePos					ReadFromBackingFile(void* pvDest, filePos iSize, filePos iCount);
	filePos					ReadWithNoTouchingWrites(void* pvDest, filePos iSize, filePos iCount);
	filePos					ReadFirstTouchingWrites(int iWriteIndex, void* pvDest, filePos iSize, filePos iCount);
	filePos					ReadNextTouchingWrites(int iWriteIndex, void* pvDest, filePos iSize, filePos iCount);
	void					CopyWritesToRead(CArrayIntAndPointer* papvOverlapping, filePos iByteSize, void* pvDest);
	int						FindNextWriteCommand(int iIndex);

	CLogFileCommandOpen*	AddOpenCommand(EFileMode eFileMode);
	CLogFileCommandWrite*	AddWriteCommand(filePos iPosition, void* pvSource, filePos iByteLength);
	CLogFileCommandWrite*	AddWriteCommand(filePos iPosition, filePos iByteLength);
	CLogFileCommandClose*	AddCloseCommand(void);
	CLogFileCommandDelete*	AddDeleteCommand(void);
};


CLogFile* LogFile(CAbstractFile* pcFile);


#endif // __LOG_FILE_H__
