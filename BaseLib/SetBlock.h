#ifndef __SET_BLOCK_H__
#define __SET_BLOCK_H__
#include "Malloc.h"
#include "DataCompare.h"
#include "DataFree.h"
#include "DataIO.h"
#include "SetNode.h"
#include "ArrayBlockSorted.h"


struct SSetIterator : SArraySortedIterator
{
};


class CSetBlock : public CMalloc, public CDataIO
{
friend struct SSNode;
protected:
	CArrayBlockSorted	mapArray;  //All that is stored in this array is the SSNode*.  
	size				miLargestDataSize;
	bool				mbOverwrite;
	CDataFree*			mpcDataFree;
	CDataIO*			mpcDataIO;
	DataCompare			mfCompare;

public:
	void				_Init(void);
	void				Init(bool bOverwrite = true);
	void				Init(CMallocator* pcMalloc, bool bOverwrite = true);
	void				Init(DataCompare fCompare, bool bOverwrite = true);
	void				Init(CMallocator* pcMalloc, DataCompare fCompare, bool bOverwrite = true);
	void				Init(CMallocator* pcMalloc, DataCompare fCompare, DataCompare fNodeCompare, bool bOverwrite);
	void				Kill(void);
	void				ReInit(void);

	void*				Get(void* pvData, size iDataSize);
	SSNode*				GetNode(void* pvData, size iDataSize);
	SSNode*				GetNode(SSetIterator* psIterator);

	void*				Put(void* pvData, size iDataSize);

	bool				Remove(void* pvData, size iDataSize);

	bool				Has(void* pvData, size iDataSize);

	size				NumElements(void);
	size				GetSortedSize(void);
	size				GetHoldingSize(void);

	CArrayBlockSorted*	GetArray(void);
	void				SetDataFreeCallback(CDataFree* pcDataFree);
	void				SetDataIOCallback(CDataIO* pcDataIO);

	bool				StartIteration(SSetIterator* psIterator, void** ppvData, size* piDataSize);
	bool				Iterate(SSetIterator* psIterator, void** ppvData, size* piDataSize);

	bool				Write(CFileWriter* pcFileWriter);
	bool				Read(CFileReader* pcFileReader);
	bool				Read(CFileReader* pcFileReader, DataCompare fCompare);
	bool				Read(CFileReader* pcFileReader, DataCompare fCompare, CDataIO* pcDataIO, CDataFree* pcDataFree);

	bool				WriteSetBlockHeader(CFileWriter* pcFileWriter);
	bool				ReadSetBlockHeader(CFileReader* pcFileReader, DataCompare fCompare, CDataIO* pcDataIO, CDataFree* pcDataFree);
	bool				WriteSetBlockElements(CFileWriter* pcFileWriter);
	bool				ReadSetBlockElements(CFileReader* pcFileReader);

	size				ByteSize(void);
	void				Dump(void);
	void				Pack(void);

public:
	SSNode*				WriteSizes(CFileWriter* pcFileWriter, size iIndex);
	SSNode*				ReadSizes(CFileReader* pcFileReader, size iIndex);

	static void*		GetData(SSNode* psNode);
	size				Sort(void);
	bool				IsSorted(void);

	SSNode*				AllocateNode(size iDataSize);
	void				FreeNode(SSNode* psNode);
	bool				GetInSorted(size iIndex, void** ppvData);
	bool				PutInSorted(size iIndex, void** ppvData, size uiDataSize);
	void*				GetDataInSorted(size iIndex);

protected:
	void				InsertHoldingIntoSorted(void);

	bool				WriteData(CFileWriter* pcFileWriter, void* pvData);
	bool				ReadData(CFileReader* pcFileReader, void* pvData);
};


#endif // __SET_BLOCK_H__

