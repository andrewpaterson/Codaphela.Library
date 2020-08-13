#ifndef __INDEX_TREE_MEMORY_CONFIG_H__
#define __INDEX_TREE_MEMORY_CONFIG_H__
#include "IndexTreeConfig.h"


class CIndexTreeMemoryConfig : public CIndexTreeConfig
{
public:
	void	Init(CMallocator* pcMalloc, EIndexKeyReverse eKeyReverse, int iMaxDataSize, int iMaxKeySize, CIndexTreeDataOrderer* pcDataOrderer);
	BOOL	Init(CFileReader* pcFileReader);
	void	Kill(void);

	BOOL	Write(CFileWriter* pcFileWriter);

protected:
	BOOL	Read(CFileReader* pcFileReader);
};


#endif // __INDEX_TREE_MEMORY_CONFIG_H__

