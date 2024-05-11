#ifndef __MAP_BLOCK_H__
#define __MAP_BLOCK_H__
#include "Malloc.h"
#include "DataCompare.h"
#include "DataFree.h"
#include "MapNode.h"
#include "ArrayBlockSorted.h"


struct SMapIterator : SArraySortedIterator
{
};


class CMapBlock : public CMalloc
{
friend struct SMNode;
protected:
	CArrayBlockSorted	mapArray;  //All that is stored in this array is the SMNode*.  
	size					miLargestKeySize;
	bool				mbOverwrite;
	CDataFree*			mpcDataFree;
	DataCompare			fKeyCompare;

public:
	void				_Init(void);
	void				Init(bool bOverwrite = true);
	void				Init(CMallocator* pcMalloc, bool bOverwrite = true);
	void				Init(DataCompare fKeyCompare, bool bOverwrite = true);
	void				Init(CMallocator* pcMalloc, DataCompare fKeyCompare, bool bOverwrite = true);
	void				Init(CMallocator* pcMalloc, DataCompare fKeyCompare, DataCompare fCompare, bool bOverwrite);
	void				Kill(void);

	bool				Get(void* pvKey, size iKeySize, void** ppvData, size* piDataSize);
	void*				Get(void* pvKey, size iKeySize);

	void*				Put(void* pvKey, size iKeySize, size iDataSize);
	bool				Put(void* psKey, size iKeySize, void* pvData, size iDataSize);

	bool				Remove(void* pvKey, size iKeySize);

	size				DataSize(void* pvKey, size iKeySize);

	bool				HasKey(void* pvKey, size iKeySize);

	size				NumElements(void);
	CArrayBlockSorted*	GetArray(void);
	void				SetDataFreeCallback(CDataFree* pcDataFree);

	bool				StartIteration(SMapIterator* psIterator, void** ppvKey, size* piKeySize, void** ppvData, size* piDataSize);
	bool				Iterate(SMapIterator* psIterator, void** ppvKey, size* piKeySize, void** ppvData, size* piDataSize);

	bool				Write(CFileWriter* pcFileWriter);
	bool				Read(CFileReader* pcFileReader);
	bool				Read(CFileReader* pcFileReader, DataCompare fKeyCompare);

	void				FinaliseSorted(void);
	size				ByteSize(void);
	void				Dump(void);

protected:
	bool				WriteExceptData(CFileWriter* pcFileWriter);
	bool				ReadExceptData(CFileReader* pcFileReader, DataCompare fKeyCompare);
	void*				WriteKey(CFileWriter* pcFileWriter, size iIndex, size* piDataSize);
	void*				ReadKey(CFileReader* pcFileReader, size iIndex, size* piDataSize);

	SMNode*				AllocateNode(size iKeySize, size iDataSize, void** ppvKey, void** ppvData);
	void				RemapKeyAndData(SMNode* psNode, void** ppvKey, void** ppvData);

	void				InsertHoldingIntoSorted(void);
	void				GetInSorted(size iIndex, void** ppvKey, void** ppvData);
	SMNode*				GetNode(void* pvKey, size iKeySize);
	void*				GetData(SMNode* psNode);
};


#endif // __MAP_BLOCK_H__

