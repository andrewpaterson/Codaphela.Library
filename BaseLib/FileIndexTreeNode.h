#ifndef __FILE_INDEX_TREE_NODE_H__
#define __FILE_INDEX_TREE_NODE_H__
#include "IndexTreeNode.h"
#include "AbstractFile.h"


class CFileIndexTreeNode : public CIndexTreeNodeMemory
{
protected:
	int			miFileNumber;
	filePos		miFileOffset;

public:
	void Init(void);
};


#endif // __FILE_INDEX_TREE_NODE_H__


