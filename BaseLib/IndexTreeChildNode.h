#ifndef __INDEX_TREE_CHILD_NODE_H__
#define __INDEX_TREE_CHILD_NODE_H__
#include "Define.h"
#include "FileIndex.h"


#define INDEX_TREE_FILE_NODE_UNALLOCATED	0xFFFFFFFF
#define INDEX_TREE_FILE_NODE_MEMORY			0x01010101
#define INDEX_TREE_FILE_NODE_FILE			0x02020202


class CIndexTreeNodeFile;
class CIndexTreeChildNode
{
public:
	uint32		iType;
	union
	{
		CFileDataIndex				mcFile;
		CIndexTreeNodeFile*			mpcMemory;
	} u;

	void	Init(CFileDataIndex cFile);
	void	Init(CFileDataIndex* pcFile);
	void	Init(uint32 iFile, uint32 uiIndex);
	void	Init(CIndexTreeNodeFile* pcMemory);
	void	Clear(void);

	bool	IsValid(void);
	bool	IsMemory(void);
	bool	IsFile(void);
	bool	IsUnallocated(void);
};


#endif // __INDEX_TREE_CHILD_NODE_H__

