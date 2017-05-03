#ifndef __INDEX_TREE_WRITER_H__
#define __INDEX_TREE_WRITER_H__
#include "BaseLib/IndexTreeMemory.h"
#include "IndexTreeNodeFile.h"
#include "IndexedFiles.h"


class CIndexTreeWriter
{
public:
	BOOL Write(CIndexTreeMemory* pcIndexTree, char* szDirectory);
	BOOL Write(CIndexTreeNodeFile* pcNode, CIndexedFiles* pcIndexFiles);

protected:
	void WriteAll(CIndexTreeMemory* pcIndexTree, CIndexedFiles* pcIndexedFiles);
	void RecurseWriteAll(CIndexTreeNodeMemory* pcNode, CIndexedFiles* pcIndexedFiles);
};

#endif //__INDEX_TREE_WRITER_H__

