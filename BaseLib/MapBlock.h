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
friend struct SMNode;
protected:
	CArrayBlockSorted	mapArray;  //All that is stored in this array is the SMNode*.  
	int					miLargestKeySize;
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

	bool				Get(void* pvKey, int iKeySize, void** ppvData, int* piDataSize);
	void*				Get(void* pvKey, int iKeySize);

	void*				Put(void* pvKey, int iKeySize, int iDataSize);
	bool				Put(void* psKey, int iKeySize, void* pvData, int iDataSize);

	bool				Remove(void* pvKey, int iKeySize);

	size_t				DataSize(void* pvKey, int iKeySize);

	bool				HasKey(void* pvKey, int iKeySize);

	int					NumElements(void);
	CArrayBlockSorted*	GetArray(void);
	void				SetDataFreeCallback(CDataFree* pcDataFree);

	bool				StartIteration(SMapIterator* psIterator, void** ppvKey, int* piKeySize, void** ppvData, int* piDataSize);
	bool				Iterate(SMapIterator* psIterator, void** ppvKey, int* piKeySize, void** ppvData, int* piDataSize);

	bool				Write(CFileWriter* pcFileWriter);
	bool				Read(CFileReader* pcFileReader);
	bool				Read(CFileReader* pcFileReader, DataCompare fKeyCompare);

	void				FinaliseSorted(void);
	size_t				ByteSize(void);
	void				Dump(void);

protected:
	bool				WriteExceptData(CFileWriter* pcFileWriter);
	bool				ReadExceptData(CFileReader* pcFileReader, DataCompare fKeyCompare);
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

