#ifndef __LOG_FILE_H__
#define __LOG_FILE_H__
#include "BaseLib/AbstractFile.h"
#include "ArrayVariable.h"


#define LOG_FILE_COMMAND_CHUNK_SIZE	4096


struct SLogFileCommandWrite
{
	filePos	iPosition;
	filePos	iSize;
};


class CLogFile : public CAbstractFile
{
public:
	CArrayVariable		mcWrites;
	filePos				miPosition;
	filePos				miLength;

	filePos				miFileLength;
	CAbstractFile*		mpcBackingFile;
	BOOL				mbTouched;

protected:
	void		Init(CAbstractFile* pcBackingFile);
	void		Kill(void);

	BOOL		Begin(void);
	BOOL		End(void);
	BOOL		Rewrite(void);
	BOOL		PrivateWrite(CAbstractFile* pcFile);

	BOOL		Open(EFileMode eFileMode);
	BOOL		Close(void);
	filePos		Write(filePos iDistance, const void* pvSource, filePos iSize, filePos iCount);
	filePos		Write(EFileSeekOrigin eOrigin, filePos iDistance, const void* pvSource, filePos iSize, filePos iCount);
	filePos		Write(const void* pvSource, filePos iSize, filePos iCount);
	filePos		Read(filePos iDistance, void* pvDest, filePos iSize, filePos iCount);
	filePos		Read(EFileSeekOrigin eOrigin, filePos iDistance, void* pvDest, filePos iSize, filePos iCount);
	filePos		Read(void* pvDest, filePos iSize, filePos iCount);
	void		Seek(EFileSeekOrigin eOrigin, filePos iDistance);
	filePos		Tell(void);
	filePos		Size(void);
	BOOL		IsEof(void);

	filePos		ReadFromFile(void* pvDest, filePos iSize, filePos iCount);
	filePos		SizeFromFile(void);

	void		Dump(void);

protected:
	BOOL		FindTouchingWriteCommands(CArrayPointer* papvOverlapping, filePos iPosition, filePos iLength, BOOL bMustOverlap);
	BOOL		Overlaps(filePos iPosition, filePos iLength, SLogFileCommandWrite* psWrite);
	BOOL		AmalgamateOverlappingWrites(CArrayPointer* papvOverlapping, const void* pvSource, filePos iPosition, filePos iLength);
	BOOL		FindHoles(CArrayPointer* papvOverlapping, filePos iPosition, filePos iLength);
	void		UpdateLength(void);
	filePos		PrivateRead(void* pvDest, filePos iSize, filePos iCount);
};


#endif // __LOG_FILE_H__
