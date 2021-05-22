#ifndef __INDEX_TREE_MEMORY_CONFIG_H__
#define __INDEX_TREE_MEMORY_CONFIG_H__
#include "IndexTreeConfig.h"


class CIndexTreeMemoryConfig : public CIndexTreeConfig
{
public:
	void	Init(CLifeInit<CMallocator> cMalloc, EIndexKeyReverse eKeyReverse, int iMaxDataSize, int iMaxKeySize, CLifeInit<CIndexTreeDataOrderer> cDataOrderer);
	BOOL	Init(CFileReader* pcFileReader);
	void	Kill(void);

	BOOL	Write(CFileWriter* pcFileWriter);
	BOOL	Read(CFileReader* pcFileReader);
};


#endif // __INDEX_TREE_MEMORY_CONFIG_H__

