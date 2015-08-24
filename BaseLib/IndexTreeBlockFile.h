#ifndef __INDEX_TREE_BLOCK_FILE_H__
#define __INDEX_TREE_BLOCK_FILE_H__
#include "Chars.h"
#include "ArrayVoidPtr.h"
#include "ArrayTemplateEmbedded.h"


class CIndexTreeBlockFile
{
protected:

public:
	void	Init(void);
	void	Init(CMallocator* pcMalloc);
	void	Kill(void);
};


#endif // __INDEX_TREE_BLOCK_FILE_H__

