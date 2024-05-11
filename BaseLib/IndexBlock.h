#ifndef __INDEX_BLOCK_H__
#define __INDEX_BLOCK_H__
#include "Malloc.h"
#include "IndexTreeMemory.h"


struct SIndexBlockNode
{
	void*	pvData;
	size		iDataSize;
};


class CIndexBlock;
class CIndexBlockMapvDataFreeCallback : public CDataFree 
{
private:
	CIndexBlock*	mpcIndexBlock;

public:
	void Init(CIndexBlock* pcIndexBlock);
	void DataWillBeFreed(void* pvData);
};


class CIndexBlock : public CMalloc
{
friend class CIndexBlockMapvDataFreeCallback;
protected:
	CIndexTreeMemory				mcIndex;
	CIndexBlockMapvDataFreeCallback	mcIndexCallback;
	CDataFree*						mpcDataFree;

public:
	void				Init(void);
	void				Init(CMallocator* pcMalloc, CIndexTreeConfig* pcConfig);
	void				Kill(void);

	bool				Get(void* pvKey, size iKeySize, void** ppvData, size* piDataSize);
	void*				Get(void* pvKey, size iKeySize);

	bool				GetLongestPartial(void* pvKey, size iKeySize, void** ppvData, size* piDataSize);
	void*				GetLongestPartial(void* pvKey, size iKeySize);

	void*				Put(void* pvKey, size iKeySize, size iDataSize);
	bool				Put(void* pvKey, size iKeySize, void* pvData, size iDataSize);

	bool				Remove(void* pvKey, size iKeySize);

	size				DataSize(void* pvKey, size iKeySize);

	bool				HasKey(void* pvKey, size iKeySize);

	size					NumElements(void);
	void				SetDataFreeCallback(CDataFree* pcDataFree);
	void				Dump(void);

	bool				StartIteration(SIndexTreeMemoryUnsafeIterator* psIterator, void** ppvData, size* puiDataSize, void* pvDestKey, size* puiKeySize, size uiMaxKeySize);
	bool				Iterate(SIndexTreeMemoryUnsafeIterator* psIterator, void** ppvData, size* puiDataSize, void* pvDestKey, size* puiKeySize, size uiMaxKeySize);

	bool				Write(CFileWriter* pcFileWriter);
	bool				Read(CFileReader* pcFileReader);

	bool				ValidateIndexTree(void);

protected:
	void				DataWillBeFreed(SIndexBlockNode* psNode);
};

#endif // __INDEX_BLOCK_H__

