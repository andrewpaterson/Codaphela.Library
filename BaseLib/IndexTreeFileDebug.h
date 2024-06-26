#ifndef __INDEX_TREE_FILE_DEBUG_H__
#define __INDEX_TREE_FILE_DEBUG_H__
#include "Define.h"
#include "ArrayTemplate.h"
#include "Chars.h"


struct SIndexTreeDebugNode
{
	uint32	uiIndexInFile;
	uint32	uiFileIndex;

	bool	bEmpty;

	uint16	uiDataSize;
	uint16	iFileNodeSize;

	uint8	uiFirstIndex;
	uint8	uiLastIndex;

	uint8	uiIndexInParent;
	uint8	sFlags;

	int		InitFromBuffer(void* pvBuffer, size iBufferSize, uint32 uiFileIndex, uint32 uiIndexInFile);
	void	InitBroken(size uiIndexInParent);
	void	Print(CChars* sz);
	void	Dump(void);
};


typedef CArrayTemplate<SIndexTreeDebugNode> CArrayIndexTreeDebugNode;


#endif // __INDEX_TREE_FILE_DEBUG_H__

