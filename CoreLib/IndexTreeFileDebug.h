#ifndef __INDEX_TREE_FILE_DEBUG_H__
#define __INDEX_TREE_FILE_DEBUG_H__
#include "BaseLib/Define.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Chars.h"

struct SIndexTreeDebugNode
{
	unsigned int	uiIndexInFile;
	int				iFileNumber;

	BOOL			bEmpty;

	unsigned short	uiDataSize;
	unsigned short	iFileNodeSize;

	unsigned char	uiFirstIndex;
	unsigned char	uiLastIndex;

	unsigned char	uiIndexInParent;
	unsigned char	sFlags;

	void InitFromBuffer(void* pvBuffer, int iBufferSize, int iFile, unsigned int uiIndex);
	void InitBroken(unsigned char uiIndexInParent);
	void Print(CChars* sz);
	void Dump(void);
};


typedef CArrayTemplate<SIndexTreeDebugNode> CArrayIndexTreeDebugNode;


#endif // __INDEX_TREE_FILE_DEBUG_H__

