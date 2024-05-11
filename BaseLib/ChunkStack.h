#ifndef __CHUNK_STACK_H__
#define __CHUNK_STACK_H__
#include "ArrayTemplate.h"
#include "ChunkFileIndex.h"
#include "ChunkFileHeader.h"
#include "MD5.h"


class CChunkStackElement
{
public:
	CChunkHeader			sHeader;
	filePos					iChunkHeaderPos;
	bool					bContainsChunks;
	CChunkIndex				cChunkIndex;
	SMD5Context				sMD5Context;

	void Init(filePos iFilePos);
};


typedef CArrayTemplate<CChunkStackElement> CChunkStack;


#endif // __CHUNK_STACK_H__

