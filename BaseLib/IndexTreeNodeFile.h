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
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, size uiFirstIndex, size uiLastIndex, CFileDataIndex cFileIndex, size uiIndexInParent);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, size uiFirstIndex, size uiLastIndex, size uiIndexInParent);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, size uiFirstIndex, size uiLastIndex, size uiDataSize, size uiIndexInParent);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, CFileDataIndex cFileIndex, size uiIndexInParent);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, size uiIndexInParent);

	size					InitFromBuffer(void* pvBuffer, size iMaxBufferSize, CIndexTreeFileDataCallback* pcCallback);

	void					SetFileIndex(uint32 iFile, uint32 uiIndex);
	CFileDataIndex*			GetFileIndex(void);

	CIndexTreeChildNode*	Get(size uiIndex);
	bool					SetFile(size uiIndex, CFileDataIndex cFileNode);
	bool					SetMemory(size uiIndex, CIndexTreeNodeFile* pcNode);

	bool					ClearIndex(size uiIndex);
	bool					ClearIndexAndUncontain(size uiIndex);

	CIndexTreeChildNode*	GetNode(size i);
	CIndexTreeChildNode*	GetNodes(void);
	CIndexTreeChildNode*	GetFirstNode(void);
	size					NumValidIndexes(void);
	size					NumValidMemoryIndexes(void);

	void					RemapChildNodes(CIndexTreeNodeFile* pcOldNode, CIndexTreeNodeFile* pcNewNode);

	size					FindPrevLastIndex(void);
	size					FindNextFirstIndex(void);

	void					SetChildrensParent(void);

	size					CalculateNodeSize(void);
	size					CalculateDataBufferSize(CIndexTreeFileDataCallback* pcCallback);
	size					CalculateFileSize(CIndexTreeFileDataCallback* pcCallback);
	size					WriteToBuffer(void* pvBuffer, size iBufferSize, CIndexTreeFileDataCallback* pcCallback);

	bool					HasFile(void);
	bool					HasOnlyFileNodes(void);
	bool					HasChildWithFlags(size uiFlags);
	bool					ConvertToFileNode(CIndexTreeNodeFile* pcNode);

	CIndexTreeNodeFile*		GetValidMemoryNode(size iIndex);

	bool					ValidateNodesEmpty(void);
	bool					ValidateParent(void);
	void					Print(CChars* psz, bool bHex);
	void					Dump(void);

protected:
	bool					Uncontain(size uiIndex);
};


#endif // __INDEX_TREE_NODE_FILE_H__

