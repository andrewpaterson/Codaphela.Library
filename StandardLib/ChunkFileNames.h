#ifndef __CHUNK_FILE_NAMES_H__
#define __CHUNK_FILE_NAMES_H__
#include "BaseLib/ChunkFile.h"


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

protected:
	BOOL	WriteChunkBegin(void);
	BOOL	WriteChunkEnd(char* szChunkName);

	int		FindChunkNamesMatching(CArrayString* paszOpenChunkNames, CArrayString* paszChunkNames);
	BOOL	WriteUnmatchedChunkEnds(int iMatchingOpen);
	BOOL	WriteNewUnmatchedChunks(int iMatchingOpen, CArrayString* paszChunkNames);
};


#endif // __CHUNK_FILE_NAMES_H__

