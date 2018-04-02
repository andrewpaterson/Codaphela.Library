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
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, CFileIndex cFileIndex);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, unsigned char uiObjectSize);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, CFileIndex cFileIndex);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent);
	int						Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, void* pvBuffer, int iBufferSize, CFileIndex cFileIndex);

	void					SetFileIndex(int iFile,  filePos ulliFilePos);
	CFileIndex*				GetFileIndex(void);

	CIndexTreeChildNode*	Get(unsigned char uiIndex);
	void					Set(unsigned char uiIndex, CIndexTreeNodeFile* pcNode);
	void					Set(unsigned char uiIndex, CFileIndex cFileNode);

	void					Clear(unsigned char uiIndex);
	BOOL					ClearAndUncontain(unsigned char uiIndex);

	CIndexTreeChildNode*	GetNode(int i);
	CIndexTreeChildNode*	GetNodes(void);
	CIndexTreeChildNode*	GetFirstNode(void);
	int						NumInitialisedIndexes(void);

	void					Contain(unsigned char uiIndex);
	BOOL					Uncontain(unsigned char uiIndex);

	void					RemapChildNodes(CIndexTreeNodeFile* pcOldNode, CIndexTreeNodeFile* pcNewNode);

	unsigned char			FindPrevLastIndex(void);
	unsigned char			FindNextFirstIndex(void);
	unsigned char			FindIndex(CIndexTreeChildNode* pcChild);
	unsigned char			FindIndex(CIndexTreeNodeFile* pcChild);

	void					SetChildsParent(void);

	int						CalculateBufferSize(void);
	int						WriteToBuffer(void* pvBuffer, int iBufferSize);

	BOOL					ValidateNodesEmpty(void);
};


#endif // __INDEX_TREE_NODE_FILE_H__

