#include "ChunkStack.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChunkStackElement::Init(filePos iFilePos)
{
	memset(this, 0xEE, sizeof(CChunkStackElement));
	cChunkIndex.WriteInit();
	iChunkHeaderPos = iFilePos;
	bContainsChunks = false;
	sHeader.WriteInit();
}

