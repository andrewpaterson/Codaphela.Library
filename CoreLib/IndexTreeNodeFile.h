#ifndef __INDEX_TREE_NODE_FILE_H__
#define __INDEX_TREE_NODE_FILE_H__
#include "BaseLib/FileIndex.h"
#include "BaseLib/IndexTreeNode.h"


class CIndexTreeNodeFile : public CIndexTreeNode
{
private:
	CFileIndex				mcFileIndex;

public:
	void					Init(CIndexTreeBlock* pcIndexTree, CIndexTreeNodeFile* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex);
	void					Init(CIndexTreeBlock* pcIndexTree, CIndexTreeNodeFile* pcParent);

	CIndexTreeNodeFile*		Get(unsigned char uiIndex);
};


#endif // __INDEX_TREE_NODE_FILE_H__

