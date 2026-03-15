#ifndef __MAP_BLOCK_H__
#define __MAP_BLOCK_H__
#include "Malloc.h"
#include "DataCompare.h"
#include "DataFree.h"
#include "DataIO.h"
#include "MapNode.h"
#include "ArrayBlockSorted.h"


struct SMapIterator : SArraySortedIterator
{
};


class CMapBlock;
class CMapBlockDataIO : public CDataIO
{
private:
	CMapBlock*	mpcMap;

public:
	void Init(CMapBlock* pcMap);
};


class CMapBlock : public CMalloc, public CDataIO
{
friend struct SMNode;
protected:
	CArrayBlockSorted	mapArray;  //All that is stored in this array is the SMNode*.  
	size				miLargestKeySize;
	bool				mbOverwrite;
	CDataFree*			mpcDataFree;
	CDataIO*			mpcDataIO;
	DataCompare			mfKeyCompare;

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
	size				GetSortedSize(void);
	size				GetHoldingSize(void);

	CArrayBlockSorted*	GetArray(void);
	void				SetDataFreeCallback(CDataFree* pcDataFree);
	void				SetDataIOCallback(CDataIO* pcDataIO);

	bool				StartIteration(SMapIterator* psIterator, void** ppvKey, size* piKeySize, void** ppvData, size* piDataSize);
	bool				Iterate(SMapIterator* psIterator, void** ppvKey, size* piKeySize, void** ppvData, size* piDataSize);

	bool				Write(CFileWriter* pcFileWriter);
	bool				Read(CFileReader* pcFileReader);
	bool				Read(CFileReader* pcFileReader, DataCompare fKeyCompare);
	bool				Read(CFileReader* pcFileReader, DataCompare fKeyCompare, CDataIO* pcDataIO);

	size				ByteSize(void);
	void				Dump(void);
	void				Pack(void);

public:
	bool				WriteExceptData(CFileWriter* pcFileWriter);
	bool				ReadExceptData(CFileReader* pcFileReader, DataCompare fKeyCompare);
	SMNode*				WriteSizes(CFileWriter* pcFileWriter, size iIndex);
	SMNode*				ReadSizes(CFileReader* pcFileReader, size iIndex);

	void*				GetValue(SMNode* psNode);
	void*				GetKey(SMNode* psNode);
	static void			RemapKeyAndData(SMNode* psNode, void** ppvKey, void** ppvData);
	void				Sort(void);

protected:
	SMNode*				AllocateNode(size iKeySize, size iDataSize);
	void				FreeNode(SMNode* psNode);

	void				InsertHoldingIntoSorted(void);
	void				GetInSorted(size iIndex, void** ppvKey, void** ppvData);
	SMNode*				GetNode(void* pvKey, size iKeySize);

	bool				WriteData(CFileWriter* pcFileWriter, void* pvData);
	bool				ReadData(CFileReader* pcFileReader, void* pvData);
};


#endif // __MAP_BLOCK_H__

