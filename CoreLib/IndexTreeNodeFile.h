#ifndef __INDEX_TREE_NODE_FILE_H__
#define __INDEX_TREE_NODE_FILE_H__
#include "BaseLib/FileIndex.h"
#include "BaseLib/IndexTreeNode.h"
#include "IndexTreeFileCallback.h"
#include "IndexTreeChildNode.h"


//Calculation to get to 32 bytes seems to be sizeof(CIndexTreeNode) 16 bytes + sizeof(CFileDataIndex) 8 bytes + sizeof(unsigned short) 2 bytes.
class CIndexTreeNodeFile : public CIndexTreeNode
{
private:
	CFileDataIndex			mcFileIndex;  //This index is not valid if the node is dirty; the node will need to br rewritten.
	unsigned short			muiFileDataSize;  //This is the size of the node in the file (if it exists).  If the node is dirty then muiDataSize contains the current size.

	unsigned char			maucPadding[4];  //Padding.

public:
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, CFileDataIndex cFileIndex, unsigned char uiIndexInParent);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, unsigned char uiIndexInParent);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, unsigned short uiDataSize, unsigned char uiIndexInParent);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, CFileDataIndex cFileIndex, unsigned char uiIndexInParent);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent);

	int						InitFromBuffer(void* pvBuffer, int iMaxBufferSize, CIndexTreeFileCallback* pcCallback);

	void					SetFileIndex(int iFile,  unsigned int uiIndex);
	CFileDataIndex*			GetFileIndex(void);

	CIndexTreeChildNode*	Get(unsigned char uiIndex);
	void									Set(unsigned char uiIndex, CIndexTreeNodeFile* pcNode);
	void					Set(unsigned char uiIndex, CFileDataIndex cFileNode);

	void					Clear(unsigned char uiIndex);
	BOOL					ClearAndUncontain(unsigned char uiIndex);

	CIndexTreeChildNode*	GetNode(int i);
	CIndexTreeChildNode*	GetNodes(void);
	CIndexTreeChildNode*	GetFirstNode(void);
	int						NumValidIndexes(void);
	int						NumMemoryIndexes(void);

	unsigned short			GetFileDataSize(void);

	void					Contain(unsigned char uiIndex);
	BOOL					Uncontain(unsigned char uiIndex);

	void					RemapChildNodes(CIndexTreeNodeFile* pcOldNode, CIndexTreeNodeFile* pcNewNode);

	unsigned char			FindPrevLastIndex(void);
	unsigned char			FindNextFirstIndex(void);

	void					SetChildrensParent(void);
	void					ClearDirtyNodeWithPath(void);

	int						CalculateNodeSize(void);
	int						CalculateDataBufferSize(CIndexTreeFileCallback* pcCallback);
	int						CalculateFileSize(CIndexTreeFileCallback* pcCallback);
	int						WriteToBuffer(void* pvBuffer, int iBufferSize, CIndexTreeFileCallback* pcCallback);

	BOOL					HasFile(void);
	BOOL					HasOnlyFileNodes(void);
	BOOL					HasChildWithFlags(unsigned char uiFlags);
	BOOL					ConvertToFileNode(CIndexTreeNodeFile* pcNode);

	CIndexTreeNodeFile*		GetValidMemoryNode(int iIndex);

	BOOL					ValidateNodesEmpty(void);
	void					Print(CChars* psz, BOOL bHex);
	void					Dump(void);
};


#endif // __INDEX_TREE_NODE_FILE_H__

