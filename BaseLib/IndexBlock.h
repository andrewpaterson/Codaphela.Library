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

	bool				Get(uint8* pvKey, size iKeySize, void** ppvData, size* piDataSize);
	void*				Get(uint8* pvKey, size iKeySize);

	bool				GetLongestPartial(uint8* pvKey, size iKeySize, void** ppvData, size* piDataSize);
	void*				GetLongestPartial(uint8* pvKey, size iKeySize);

	void*				Put(uint8* pvKey, size iKeySize, size iDataSize);
	bool				Put(uint8* pvKey, size iKeySize, void* pvData, size iDataSize);

	bool				Remove(uint8* pvKey, size iKeySize);

	size				DataSize(uint8* pvKey, size iKeySize);

	bool				HasKey(uint8* pvKey, size iKeySize);

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

