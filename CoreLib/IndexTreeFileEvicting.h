#ifndef __INDEX_TREE_FIL_EVICTING_H__
#define __INDEX_TREE_FIL_EVICTING_H__
#include "IndexTreeFile.h"


class CIndexTreeEvicting
{
private:
	CIndexTreeFile		mcIndexTree;
	size_t				muiCutoff;

	CCountingAllocator	mcCountingAllocator;
	
public:
	BOOL Init(CDurableFileController* pcDurableFileControl);
	BOOL Init(CDurableFileController* pcDurableFileControl, BOOL bWriteThrough);
	BOOL Init(CDurableFileController* pcDurableFileControl, CMallocator* pcMalloc, BOOL bWriteThrough);

	BOOL Kill(void);

	BOOL Get(void* pvKey, int iKeySize, void* pvObject, unsigned short* puiDataSize);
	BOOL Put(void* pvKey, int iKeySize, void* pvObject, unsigned short uiDataSize);
	BOOL Remove(void* pvKey, int iKeySize);
	BOOL HasKey(void* pvKey, int iKeySize);

};

#endif // __INDEX_TREE_FIL_EVICTING_H__

