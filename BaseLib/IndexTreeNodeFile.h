#ifndef __INDEX_TREE_NODE_FILE_H__
#define __INDEX_TREE_NODE_FILE_H__


class CIndexTreeNodeFile
{
private:
	CIndexTreeNodeFile*		mpcParent;
	unsigned char			muiFirstIndex;
	unsigned char			muiLastIndex;
	unsigned char			muiDataSize;
	int						miFileId;


};


#endif // __INDEX_TREE_NODE_FILE_H__

