#ifndef __CHUNK_FILE_FILE_SYSTEM_H__
#define __CHUNK_FILE_FILE_SYSTEM_H__
#include "ArrayChars.h"
#include "ArrayUint.h"
#include "ChunkFile.h"


struct SChunkFilenameIterator
{
	CChars		szFullName;
	CArrayUint	aiIndex;
	bool		bDone;
	CChars		szValue;

	void Init(void);
	void Kill(void);
};


class CChunkFileFileSystem : public CFileIO
{
protected:
	CArrayChars		maszOpenChunkNames;		//The open chunks containing the last one, written or read (reads leave them open).
	CChars			mszLastChunkName;		//The last open chunk name.
	CChunkFile*		mpcChunkFile;

public:
	void		_Init(void);
	void		Init(CChunkFile* pcChunkFile);
	void		Kill(void);

	bool		WriteOpen(void);
	bool		WriteOpen(uint32 iUserID);
	bool		WriteChunkBegin(const char* szChunkName);
	bool		WriteChunkEnd(void);
	bool		WriteClose(void);

	bool		ReadOpen(void);
	bool		ReadChunkBegin(const char* szChunkName);
	bool		ReadChunkEnd(void);
	bool		ReadClose(void);

	char*		StartNameIteration(SChunkFilenameIterator* psIter);
	char*		IterateName(SChunkFilenameIterator* psIter);
	bool		StopIteration(SChunkFilenameIterator* psIter);

	CChunkFile* GetChunkFile(void);

protected:
	bool		WriteChunkBegin(void);
	bool		WriteChunkEnd(const char* szChunkName);

	size		FindChunkNamesMatching(CArrayChars* paszOpenChunkNames, CArrayChars* paszChunkNames);
	bool		WriteUnmatchedChunkEnds(size iMatchingOpen);
	bool		WriteNewUnmatchedChunks(size iMatchingOpen, CArrayChars* paszChunkNames);

	size		NumOpenOnPath(CArrayChars* paszChunkNames);
	void		ReadOpenChunksEnd(size uiKeep);
	bool		ReadChunkBeginFrom(CArrayChars* paszChunkNames, size uiDepth);

	lsize		Write(const void* pvSource, lsize iSize, lsize iCount);
	lsize		Read(void* pvDest, lsize iSize, lsize iCount);
};


#endif // __CHUNK_FILE_FILE_SYSTEM_H__

