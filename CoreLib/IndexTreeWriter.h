#ifndef __INDEX_TREE_WRITER_H__
#define __INDEX_TREE_WRITER_H__
#include "BaseLib/IndexTreeMemory.h"
#include "CoreLib/IndexTreeFile.h"
#include "IndexTreeNodeFile.h"
#include "IndexedFiles.h"


class CIndexTreeWriter
{
public:
	BOOL Write(CIndexTreeMemory* pcIndexTree, char* szDirectory);

protected:
	void RecurseAllocate(CIndexTreeNodeMemory* pcNode, CIndexTreeFile* pcFileTree, CIndexTreeNodeFile* pcFileNode);
	BOOL RecurseWrite(CIndexTreeFile* pcFileTree, CIndexTreeNodeFile* pcFileNode);
};

#endif //__INDEX_TREE_WRITER_H__

