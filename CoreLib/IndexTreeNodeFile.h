#ifndef __INDEX_TREE_NODE_FILE_H__
#define __INDEX_TREE_NODE_FILE_H__
#include "BaseLib/FileIndex.h"
#include "BaseLib/IndexTreeNode.h"
#include "IndexTreeChildNode.h"


class CIndexTreeNodeFile : public CIndexTreeNode
{
private:
	CFileIndex				mcFileIndex;

public:
	void					Init(CIndexTreeBlock* pcIndexTree, CIndexTreeNodeFile* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex);
	void					Init(CIndexTreeBlock* pcIndexTree, CIndexTreeNodeFile* pcParent, CFileIndex cFileIndex);
	void					Init(CIndexTreeBlock* pcIndexTree, CIndexTreeNodeFile* pcParent);

	CIndexTreeNodeFile*		Get(unsigned char uiIndex);
	void					Set(unsigned char uiIndex, CIndexTreeNodeFile* pcNode);

	BOOL					Clear(unsigned char uiIndex);

	CIndexTreeNodeFile*		GetNode(int i);
	CIndexTreeNodeFile**	GetNodes(void);
	int						NumInitialisedIndexes(void);

	void					Contain(unsigned char uiIndex);
	BOOL					Uncontain(unsigned char uiIndex);

	void					RemapChildNodes(CIndexTreeNodeFile* pcOldNode, CIndexTreeNodeFile* pcNewNode);

	unsigned char			FindPrevLastIndex(void);
	unsigned char			FindNextFirstIndex(void);
	unsigned char			FindIndex(CIndexTreeNodeFile* pcChild);

	void					SetChildsParent(void);

	BOOL					ValidateNodesEmpty(void);
};


#endif // __INDEX_TREE_NODE_FILE_H__

