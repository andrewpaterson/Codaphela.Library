#ifndef __CHUNK_FILE_NAMES_H__
#define __CHUNK_FILE_NAMES_H__
#include "BaseLib/ArrayString.h"
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


class CChunkFileNames : public CChunkFile
{
protected:
	CArrayString	maszOpenChunkNames;  //All the chunk names not including the last one.
	CChars			mszLastChunkName;  //The last open chunk name.

public:
	void	Init(CAbstractFile*	pcFile);
	void	Kill(void);

	BOOL	WriteChunkBegin(char* szChunkName);
	BOOL	WriteChunkEnd(void);

	BOOL	WriteClose(void);

	BOOL	ReadChunkBegin(char* szChunkName);
	BOOL	ReadChunkEnd(void);

	char*	StartNameIteration(SChunkFileNameIterator* psIter);
	char*	IterateName(SChunkFileNameIterator* psIter);
	BOOL	StopIteration(SChunkFileNameIterator* psIter);

protected:
	BOOL	WriteChunkBegin(void);
	BOOL	WriteChunkEnd(char* szChunkName);

	int		FindChunkNamesMatching(CArrayString* paszOpenChunkNames, CArrayString* paszChunkNames);
	BOOL	WriteUnmatchedChunkEnds(int iMatchingOpen);
	BOOL	WriteNewUnmatchedChunks(int iMatchingOpen, CArrayString* paszChunkNames);

	CChars*	PrivateGetChunkName(int iIndex);
};


#endif // __CHUNK_FILE_NAMES_H__

