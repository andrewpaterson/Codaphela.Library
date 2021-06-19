#ifndef __CHUNK_FILE_FILE_SYSTEM_H__
#define __CHUNK_FILE_FILE_SYSTEM_H__
#include "BaseLib/ArrayChars.h"
#include "BaseLib/ChunkFile.h"


struct SChunkFileNameIterator
{
	CChars		szFullName;
	CArrayInt	aiIndex;
	BOOL		bDone;
	CChars		szValue;

	void Init(void);
	void Kill(void);
};


class CChunkFileFileSystem : public CFileIO
{
protected:
	CArrayChars		maszOpenChunkNames;		//All the chunk names not including the last one.
	CChars			mszLastChunkName;		//The last open chunk name.
	CChunkFile*		mpcChunkFile;

public:
	void		_Init(void);
	void		Init(CChunkFile* pcChunkFile);
	void		Kill(void);

	BOOL		WriteOpen(void);
	BOOL		WriteOpen(int iUserID);
	BOOL		WriteChunkBegin(char* szChunkName);
	BOOL		WriteChunkEnd(void);
	BOOL		WriteClose(void);

	BOOL		ReadOpen(void);
	BOOL		ReadChunkBegin(char* szChunkName);
	BOOL		ReadChunkEnd(void);
	BOOL		ReadClose(void);

	char*		StartNameIteration(SChunkFileNameIterator* psIter);
	char*		IterateName(SChunkFileNameIterator* psIter);
	BOOL		StopIteration(SChunkFileNameIterator* psIter);

	CChunkFile* GetChunkFile(void);

protected:
	BOOL	WriteChunkBegin(void);
	BOOL	WriteChunkEnd(char* szChunkName);

	int		FindChunkNamesMatching(CArrayChars* paszOpenChunkNames, CArrayChars* paszChunkNames);
	BOOL	WriteUnmatchedChunkEnds(int iMatchingOpen);
	BOOL	WriteNewUnmatchedChunks(int iMatchingOpen, CArrayChars* paszChunkNames);

	filePos	Write(const void* pvSource, filePos iSize, filePos iCount);
	filePos	Read(void* pvDest, filePos iSize, filePos iCount);
};


#endif // !__CHUNK_FILE_FILE_SYSTEM_H__

