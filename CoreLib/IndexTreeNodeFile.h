#ifndef __INDEX_TREE_NODE_FILE_H__
#define __INDEX_TREE_NODE_FILE_H__
#include "BaseLib/FileIndex.h"
#include "BaseLib/IndexTreeNode.h"
#include "IndexTreeChildNode.h"


//sizeof(CIndexTreeNodeFile) is 32 bytes but only 26 bytes are used.
//sizeof(CFileIndex) is 16 bytes but only 12 bytes are used.

//Calculation to get to 32 bytes seems to be sizeof(CIndexTreeNode) 16 bytes + sizeof(CFileIndex) 16 bytes.
class CIndexTreeNodeFile : public CIndexTreeNode
{
private:
	CFileIndex				mcFileIndex;

public:
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, CFileIndex cFileIndex, unsigned char uiIndexInParent);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, unsigned char uiIndexInParent);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, unsigned short uiObjectSize, unsigned char uiIndexInParent);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, CFileIndex cFileIndex, unsigned char uiIndexInParent);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent);

	int						InitFromBuffer(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, void* pvBuffer, int iBufferSize, CFileIndex cFileIndex);

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

	void					SetChildrensParent(void);

	int						CalculateBufferSize(void);
	int						WriteToBuffer(void* pvBuffer, int iBufferSize);

	BOOL					ValidateNodesEmpty(void);
	void					Print(CChars* psz, BOOL bHex);
	void					Dump(void);
};


#endif // __INDEX_TREE_NODE_FILE_H__

