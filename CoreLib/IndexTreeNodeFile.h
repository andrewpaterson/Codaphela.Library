#ifndef __INDEX_TREE_NODE_FILE_H__
#define __INDEX_TREE_NODE_FILE_H__
#include "BaseLib/FileIndex.h"
#include "BaseLib/IndexTreeNode.h"
#include "IndexTreeFileDataCallback.h"
#include "IndexTreeChildNode.h"


//Calculation to get to 32 bytes seems to be sizeof(CIndexTreeNode) 16 bytes + sizeof(CFileDataIndex) 8 bytes + sizeof(unsigned short) 2 bytes.
class CIndexTreeNodeFile : public CIndexTreeNode
{
private:
	CFileDataIndex			mcFileIndex;  //This index is not valid if the node is dirty; the node will need to be rewritten.

public:
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, CFileDataIndex cFileIndex, unsigned char uiIndexInParent);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, unsigned char uiIndexInParent);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, unsigned short uiDataSize, unsigned char uiIndexInParent);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, CFileDataIndex cFileIndex, unsigned char uiIndexInParent);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent);

	int						InitFromBuffer(void* pvBuffer, int iMaxBufferSize, CIndexTreeFileDataCallback* pcCallback);

	void					SetFileIndex(int iFile, unsigned int uiIndex);
	CFileDataIndex*			GetFileIndex(void);

	CIndexTreeChildNode*	Get(unsigned char uiIndex);
	BOOL					SetFile(unsigned char uiIndex, CFileDataIndex cFileNode);
	BOOL					SetMemory(unsigned char uiIndex, CIndexTreeNodeFile* pcNode);

	BOOL					ClearIndex(unsigned char uiIndex);
	BOOL					ClearIndexAndUncontain(unsigned char uiIndex);

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

	BOOL					HasFile(void);
	BOOL					HasOnlyFileNodes(void);
	BOOL					HasChildWithFlags(unsigned char uiFlags);
	BOOL					ConvertToFileNode(CIndexTreeNodeFile* pcNode);

	CIndexTreeNodeFile*		GetValidMemoryNode(int iIndex);

	BOOL					ValidateNodesEmpty(void);
	BOOL					ValidateParent(void);
	void					Print(CChars* psz, BOOL bHex);
	void					Dump(void);

protected:
	BOOL					Uncontain(unsigned char uiIndex);
};


#endif // __INDEX_TREE_NODE_FILE_H__

