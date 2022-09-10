#ifndef __INDEX_TREE_NODE_FILE_H__
#define __INDEX_TREE_NODE_FILE_H__
#include "BaseLib/FileIndex.h"
#include "BaseLib/IndexTreeNode.h"
#include "IndexTreeFileDataCallback.h"
#include "IndexTreeChildNode.h"


//Calculation to get to 32 bytes seems to be sizeof(CIndexTreeNode) 16 bytes + sizeof(CFileDataIndex) 8 bytes + sizeof(uint16) 2 bytes.
class CIndexTreeNodeFile : public CIndexTreeNode
{
private:
	CFileDataIndex			mcFileIndex;  //This index is not valid if the node is dirty; the node will need to be rewritten.

public:
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, CFileDataIndex cFileIndex, unsigned char uiIndexInParent);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, unsigned char uiIndexInParent);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, uint16 uiDataSize, unsigned char uiIndexInParent);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, CFileDataIndex cFileIndex, unsigned char uiIndexInParent);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent);

	int						InitFromBuffer(void* pvBuffer, int iMaxBufferSize, CIndexTreeFileDataCallback* pcCallback);

	void					SetFileIndex(int iFile, unsigned int uiIndex);
	CFileDataIndex*			GetFileIndex(void);

	CIndexTreeChildNode*	Get(unsigned char uiIndex);
	bool					SetFile(unsigned char uiIndex, CFileDataIndex cFileNode);
	bool					SetMemory(unsigned char uiIndex, CIndexTreeNodeFile* pcNode);

	bool					ClearIndex(unsigned char uiIndex);
	bool					ClearIndexAndUncontain(unsigned char uiIndex);

	CIndexTreeChildNode*	GetNode(int i);
	CIndexTreeChildNode*	GetNodes(void);
	CIndexTreeChildNode*	GetFirstNode(void);
	int						NumValidIndexes(void);
	int						NumValidMemoryIndexes(void);

	void					RemapChildNodes(CIndexTreeNodeFile* pcOldNode, CIndexTreeNodeFile* pcNewNode);

	unsigned char			FindPrevLastIndex(void);
	unsigned char			FindNextFirstIndex(void);

	void					SetChildrensParent(void);

	size_t					CalculateNodeSize(void);
	size_t					CalculateDataBufferSize(CIndexTreeFileDataCallback* pcCallback);
	size_t					CalculateFileSize(CIndexTreeFileDataCallback* pcCallback);
	int						WriteToBuffer(void* pvBuffer, int iBufferSize, CIndexTreeFileDataCallback* pcCallback);

	bool					HasFile(void);
	bool					HasOnlyFileNodes(void);
	bool					HasChildWithFlags(unsigned char uiFlags);
	bool					ConvertToFileNode(CIndexTreeNodeFile* pcNode);

	CIndexTreeNodeFile*		GetValidMemoryNode(int iIndex);

	bool					ValidateNodesEmpty(void);
	bool					ValidateParent(void);
	void					Print(CChars* psz, bool bHex);
	void					Dump(void);

protected:
	bool					Uncontain(unsigned char uiIndex);
};


#endif // !__INDEX_TREE_NODE_FILE_H__

