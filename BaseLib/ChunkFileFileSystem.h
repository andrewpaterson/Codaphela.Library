#ifndef __CHUNK_FILE_FILE_SYSTEM_H__
#define __CHUNK_FILE_FILE_SYSTEM_H__
#include "ArrayChars.h"
#include "ChunkFile.h"


struct SChunkFilenameIterator
{
	CChars		szFullName;
	CArrayInt	aiIndex;
	bool		bDone;
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

	bool		WriteOpen(void);
	bool		WriteOpen(uint32 iUserID);
	bool		WriteChunkBegin(char* szChunkName);
	bool		WriteChunkEnd(void);
	bool		WriteClose(void);

	bool		ReadOpen(void);
	bool		ReadChunkBegin(char* szChunkName);
	bool		ReadChunkEnd(void);
	bool		ReadClose(void);

	char*		StartNameIteration(SChunkFilenameIterator* psIter);
	char*		IterateName(SChunkFilenameIterator* psIter);
	bool		StopIteration(SChunkFilenameIterator* psIter);

	CChunkFile* GetChunkFile(void);

protected:
	bool		WriteChunkBegin(void);
	bool		WriteChunkEnd(char* szChunkName);

	uint32		FindChunkNamesMatching(CArrayChars* paszOpenChunkNames, CArrayChars* paszChunkNames);
	bool		WriteUnmatchedChunkEnds(size iMatchingOpen);
	bool		WriteNewUnmatchedChunks(size iMatchingOpen, CArrayChars* paszChunkNames);

	size		Write(const void* pvSource, size iSize, size iCount);
	size		Read(void* pvDest, size iSize, size iCount);
};


#endif // __CHUNK_FILE_FILE_SYSTEM_H__

