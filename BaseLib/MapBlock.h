#ifndef __MAP_BLOCK_H__
#define __MAP_BLOCK_H__
#include "Malloc.h"
#include "DataCallback.h"
#include "MapNode.h"
#include "ArrayBlockSorted.h"


struct SMapIterator : SArraySortedIterator
{
};


class CMapBlock : public CMalloc
{
protected:
	CArrayBlockSorted	mapArray;  //All that is stored in this array is the SMNode*.  
	int					miLargestKeySize;
	BOOL				mbOverwrite;
	DataFree			mDataFree;

public:
	DataCompare			fKeyCompare;

public:
	void				Init(BOOL bOverwrite = TRUE);
	void				Init(CMallocator* pcMalloc, BOOL bOverwrite = TRUE);
	void				Init(DataCompare fKeyCompare, BOOL bOverwrite = TRUE);
	void				Init(CMallocator* pcMalloc, DataCompare fKeyCompare, BOOL bOverwrite = TRUE);
	void				Init(CMallocator* pcMalloc, DataCompare fKeyCompare, DataCompare fCompare, BOOL bOverwrite);
	void				Kill(void);

	BOOL				Get(void* pvKey, int iKeySize, void** ppvData, int* piDataSize);
	void*				Get(void* pvKey, int iKeySize);

	void*				Put(void* pvKey, int iKeySize, int iDataSize);
	BOOL				Put(void* psKey, int iKeySize, void* pvData, int iDataSize);

	BOOL				Remove(void* pvKey, int iKeySize);

	size_t				DataSize(void* pvKey, int iKeySize);

	BOOL				HasKey(void* pvKey, int iKeySize);

	int					NumElements(void);
	CArrayBlockSorted*	GetArray(void);
	void				SetDataFreeCallback(DataFree fDataFree);

	BOOL				StartIteration(SMapIterator* psIterator, void** ppvKey, int* piKeySize, void** ppvData, int* piDataSize);
	BOOL				Iterate(SMapIterator* psIterator, void** ppvKey, int* piKeySize, void** ppvData, int* piDataSize);

	BOOL				Write(CFileWriter* pcFileWriter);
	BOOL				Read(CFileReader* pcFileReader);
	BOOL				Read(CFileReader* pcFileReader, DataCompare fKeyCompare);

	void				FinaliseSorted(void);
	size_t				ByteSize(void);
	void				Dump(void);

protected:
	BOOL				WriteExceptData(CFileWriter* pcFileWriter);
	BOOL				ReadExceptData(CFileReader* pcFileReader, DataCompare fKeyCompare);
	void*				WriteKey(CFileWriter* pcFileWriter, int iIndex, int* piDataSize);
	void*				ReadKey(CFileReader* pcFileReader, int iIndex, int* piDataSize);

	SMNode*				AllocateNode(int iKeySize, int iDataSize, void** ppvKey, void** ppvData);
	void				RemapKeyAndData(SMNode* psNode, void** ppvKey, void** ppvData);

	void				InsertHoldingIntoSorted(void);
	void				GetInSorted(int iIndex, void** ppvKey, void** ppvData);
	SMNode*				GetNode(void* pvKey, int iKeySize);
	void*				GetData(SMNode* psNode);
};


#endif // __MAP_BLOCK_H__

