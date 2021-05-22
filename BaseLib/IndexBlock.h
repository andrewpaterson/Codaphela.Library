#ifndef __MAP_BLOCK2_H__
#define __MAP_BLOCK2_H__
#include "Malloc.h"
#include "IndexTreeMemory.h"


struct SIndexBlockNode
{
	void*	pvData;
	int		iDataSize;
};


class CIndexBlock : public CMalloc
{
protected:
	CIndexTreeMemory	mcIndex;
	DataFree			mDataFree;

public:
	void				Init(void);
	void				Init(CMallocator* pcMalloc, CIndexTreeConfig* pcConfig);
	void				Kill(void);

	BOOL				Get(void* pvKey, int iKeySize, void** ppvData, int* piDataSize);
	void*				Get(void* pvKey, int iKeySize);

	void*				Put(void* pvKey, int iKeySize, int iDataSize);
	BOOL				Put(void* pvKey, int iKeySize, void* pvData, int iDataSize);

	BOOL				Remove(void* pvKey, int iKeySize);

	size_t				DataSize(void* pvKey, int iKeySize);

	BOOL				HasKey(void* pvKey, int iKeySize);

	int					NumElements(void);
	void				SetDataFreeCallback(DataFree fDataFree);
	void				Dump(void);

	BOOL				StartIteration(SIndexTreeMemoryUnsafeIterator* psIterator, void** ppvData, size_t* puiDataSize, void* pvDestKey, size_t* puiKeySize, size_t uiMaxKeySize);
	BOOL				Iterate(SIndexTreeMemoryUnsafeIterator* psIterator, void** ppvData, size_t* puiDataSize, void* pvDestKey, size_t* puiKeySize, size_t uiMaxKeySize);

	BOOL				Write(CFileWriter* pcFileWriter);
	BOOL				Read(CFileReader* pcFileReader);
};

#endif // __MAP_BLOCK2_H__

