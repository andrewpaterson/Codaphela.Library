#ifndef __MAP_BLOCK2_H__
#define __MAP_BLOCK2_H__
#include "Malloc.h"
#include "IndexTreeMemory.h"


class CIndexBlock
{
protected:
	CIndexTreeMemory	mcIndex;

public:
	void				Init(void);
	void				Init(CIndexTreeConfig* pcConfig);
	void				Kill(void);

	BOOL				Get(void* pvKey, int iKeySize, void* pvDestData, size_t* puiDataSize, size_t uiMaxDataSize);

	BOOL				Put(void* pvKey, int iKeySize, void* pvData, size_t iDataSize);
	BOOL				Remove(void* pvKey, int iKeySize);

	int					NumElements(void);

	BOOL				StartIteration(SIndexTreeMemoryIterator* psIterator, void** pvKey, void** pvData);
	BOOL				Iterate(SIndexTreeMemoryIterator* psIterator, void** pvKey, void** pvData);

	BOOL				Write(CFileWriter* pcFileWriter);
	BOOL				Read(CFileReader* pcFileReader);
};

#endif // __MAP_BLOCK2_H__

