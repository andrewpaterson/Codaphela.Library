#ifndef __INDEX_TREE_CHILD_NODE_H__
#define __INDEX_TREE_CHILD_NODE_H__
#include "BaseLib/Define.h"
#include "BaseLib/FileIndex.h"


#define INDEX_TREE_FILE_NODE_UNALLOCATED	0xFFFFFFFF
#define INDEX_TREE_FILE_NODE_MEMORY			0x01010101
#define INDEX_TREE_FILE_NODE_FILE			0x02020202


struct SIndexTreeChildFile
{
	char	acUnused[12];
	int		iType;
};


class CIndexTreeNodeFile;
class CIndexTreeChildNode
{
public:
	union
	{
		CFilePosIndex					mcFile;
		CIndexTreeNodeFile*			mpcMemory;
		SIndexTreeChildFile			msType;
	} u;

	void	Init(CFilePosIndex cFile);
	void	Init(CFilePosIndex* pcFile);
	void	Init(int iFile, filePos ulliFilePos);
	void	Init(CIndexTreeNodeFile* pcMemory);
	void	Clear(void);

	BOOL	IsValid(void);
	BOOL	IsMemory(void);
	BOOL	IsFile(void);
	BOOL	IsUnallocated(void);
};


#endif // __INDEX_TREE_CHILD_NODE_H__

