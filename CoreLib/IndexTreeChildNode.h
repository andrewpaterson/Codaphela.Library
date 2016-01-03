#ifndef __INDEX_TREE_CHILD_NODE_H__
#define __INDEX_TREE_CHILD_NODE_H__
#include "BaseLib/Define.h"
#include "BaseLib/FileIndex.h"


#define INDEX_TREE_FILE_NODE_UNDEFINED	0x00000000
#define INDEX_TREE_FILE_NODE_MEMORY		0x01010101
#define INDEX_TREE_FILE_NODE_FILE		0x02020202


struct SIndexTreeChildFile
{
	char	ac[12];
	int		iType;
};


class CIndexTreeNodeFile;
class CIndexTreeChildNode
{
public:
	union
	{
		CFileIndex					mcFile;
		CIndexTreeNodeFile*			mpcMemory;
		SIndexTreeChildFile			msType;
	} u;

	void	Init(CFileIndex cFile);
	void	Init(CFileIndex* pcFile);
	void	Init(int iFile, filePos ulliFilePos);
	void	Init(CIndexTreeNodeFile* pcMemory);
	void	Clear(void);

	BOOL	IsValid(void);
	BOOL	IsMemory(void);
	BOOL	IsFile(void);
};


#endif // __INDEX_TREE_CHILD_NODE_H__

