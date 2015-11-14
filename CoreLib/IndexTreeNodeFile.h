#ifndef __INDEX_TREE_NODE_FILE_H__
#define __INDEX_TREE_NODE_FILE_H__
#include "BaseLib/FileIndex.h"


//
//  Bytes 0     [muiFirstIndex.....muiLastIndex]       255  
//  


class CIndexTreeNodeFile
{
private:
	CIndexTreeNodeFile*		mpcParent;
	unsigned char			muiFirstIndex;
	unsigned char			muiLastIndex;
	unsigned char			muiDataSize;
	CFileIndex				mcFileIndex;

	 
};


#endif // __INDEX_TREE_NODE_FILE_H__

