#ifndef __INDEX_TREE_NODE_FILE_H__
#define __INDEX_TREE_NODE_FILE_H__
#include "FileIndex.h"
#include "IndexTreeNode.h"
#include "IndexTreeFileDataCallback.h"
#include "IndexTreeChildNode.h"


//Calculation to get to 32 bytes seems to be sizeof(CIndexTreeNode) 16 bytes + sizeof(CFileDataIndex) 8 bytes + sizeof(uint16) 2 bytes.
class CIndexTreeNodeFile : public CIndexTreeNode
{
private:
	CFileDataIndex			mcFileIndex;  //This index is not valid if the node is dirty; the node will need to be rewritten.

public:
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, uint8 uiFirstIndex, uint8 uiLastIndex, CFileDataIndex cFileIndex, uint8 uiIndexInParent);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, uint8 uiFirstIndex, uint8 uiLastIndex, uint8 uiIndexInParent);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, uint8 uiFirstIndex, uint8 uiLastIndex, uint16 uiDataSize, uint8 uiIndexInParent);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, CFileDataIndex cFileIndex, uint8 uiIndexInParent);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, uint8 uiIndexInParent);

	int						InitFromBuffer(void* pvBuffer, int iMaxBufferSize, CIndexTreeFileDataCallback* pcCallback);

	void					SetFileIndex(int iFile, uint32 uiIndex);
	CFileDataIndex*			GetFileIndex(void);

	CIndexTreeChildNode*	Get(uint8 uiIndex);
	bool					SetFile(uint8 uiIndex, CFileDataIndex cFileNode);
	bool					SetMemory(uint8 uiIndex, CIndexTreeNodeFile* pcNode);

	bool					ClearIndex(uint8 uiIndex);
	bool					ClearIndexAndUncontain(uint8 uiIndex);

	CIndexTreeChildNode*	GetNode(int i);
	CIndexTreeChildNode*	GetNodes(void);
	CIndexTreeChildNode*	GetFirstNode(void);
	int						NumValidIndexes(void);
	int						NumValidMemoryIndexes(void);

	void					RemapChildNodes(CIndexTreeNodeFile* pcOldNode, CIndexTreeNodeFile* pcNewNode);

	uint8					FindPrevLastIndex(void);
	uint8					FindNextFirstIndex(void);

	void					SetChildrensParent(void);

	size_t					CalculateNodeSize(void);
	size_t					CalculateDataBufferSize(CIndexTreeFileDataCallback* pcCallback);
	size_t					CalculateFileSize(CIndexTreeFileDataCallback* pcCallback);
	int						WriteToBuffer(void* pvBuffer, int iBufferSize, CIndexTreeFileDataCallback* pcCallback);

	bool					HasFile(void);
	bool					HasOnlyFileNodes(void);
	bool					HasChildWithFlags(uint8 uiFlags);
	bool					ConvertToFileNode(CIndexTreeNodeFile* pcNode);

	CIndexTreeNodeFile*		GetValidMemoryNode(int iIndex);

	bool					ValidateNodesEmpty(void);
	bool					ValidateParent(void);
	void					Print(CChars* psz, bool bHex);
	void					Dump(void);

protected:
	bool					Uncontain(uint8 uiIndex);
};


#endif // __INDEX_TREE_NODE_FILE_H__

