#ifndef __INDEX_BLOCK_H__
#define __INDEX_BLOCK_H__
#include "BaseLib/PrimitiveTypes.h"
#include "Malloc.h"
#include "IndexTreeMemory.h"


struct SIndexBlockNode
{
	void*	pvData;
	size	iDataSize;
};


class CIndexBlock;
class CIndexBlockDataFree : public CDataFree
{
private:
	CIndexBlock*	mpcIndexBlock;

public:
	void Init(CIndexBlock* pcIndexBlock);
	void FreeData(void* pvData);
};


class CIndexBlock : public CDataIO, public CIndexTreeDataSize
{
friend class CIndexBlockDataFree;
protected:
	CIndexTreeMemory		mcIndex;
	CIndexBlockDataFree		mcIndexFree;

public:
	void				Init(void);
	void				Init(CLifeInit<CMallocator> cMalloc);
	void				Init(CIndexTreeConfig* pcConfig);
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

	size				NumElements(void);
	void				SetDataFreeCallback(CDataFree* pcDataFree);
	void				Dump(void);

	bool				StartIteration(SIndexTreeMemoryUnsafeIterator* psIterator, void** ppvData, size* puiDataSize, void* pvDestKey, size* puiKeySize, size uiMaxKeySize);
	bool				Iterate(SIndexTreeMemoryUnsafeIterator* psIterator, void** ppvData, size* puiDataSize, void* pvDestKey, size* puiKeySize, size uiMaxKeySize);

	bool				Write(CFileWriter* pcFileWriter);
	bool				Read(CFileReader* pcFileReader);

	bool				WriteData(CFileWriter* pcFileWriter, void* pvData);
	bool				ReadData(CFileReader* pcFileReader, void* pvData);

	bool				ValidateIndexTree(void);

protected:
	size				AdjustDataSize(void* pvValue, size iValueSize);

	void*				Malloc(size uiSize);
	void*				Realloc(void* pv, size iMemSize);
	void				Free(void* pv);
};

#endif // __INDEX_BLOCK_H__

