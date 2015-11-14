#ifndef __INDEX_TREE_BLOCK_FILE_H__
#define __INDEX_TREE_BLOCK_FILE_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayVoidPtr.h"
#include "BaseLib/ArrayTemplateEmbedded.h"
#include "DurableFile.h"
#include "IndexTreeNodeFile.h"


class CIndexTreeBlockFile
{
protected:
	CIndexTreeNodeFile*		mpcRoot;
	CDurableFile			mcFile;

public:
	void	Init(void);
	void	Init(CMallocator* pcMalloc);
	void	Kill(void);
};


#endif // __INDEX_TREE_BLOCK_FILE_H__

