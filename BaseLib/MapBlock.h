#ifndef __MAP_BLOCK_H__
#define __MAP_BLOCK_H__
#include "ArrayBlockSorted.h"
#include "Malloc.h"


class CMapBlock;
struct SMNode
{
	int			iKeySize;
	int			iDataSize;
	CMapBlock*	pcMapBlock;
};


struct SMapIterator : SArraySortedIterator
{
};


typedef int(*CompareFunc)(const void*, const void*);


//I think CMapBlock must memory leak a lot.  SMNodes are allocated but never freed when overwritten.
class CMapBlock : public CMalloc
{
protected:
	CArrayBlockSorted	mapArray;
	int					miLargestKeySize;
	BOOL				mbOverwrite;

public:
	int					(*fKeyCompare)(const void*, const void*);

public:
	void				Init(int(*fKeyCompare)(const void*, const void*), BOOL bOverwrite = TRUE);
	void				Init(CMallocator* pcMalloc, int(*fKeyCompare)(const void*, const void*), BOOL bOverwrite);
	void				Kill(void);

	BOOL				Get(void* pvKey, void** ppvData, int* piDataSize);
	void*				Get(void* pvKey);

	void*				Put(void* pvKey, int iKeySize, int iDataSize);
	BOOL				Put(void* psKey, int iKeySize, void* pvData, int iDataSize);
	BOOL				Remove(void* pvKey);

	int					NumElements(void);
	CArrayBlockSorted*	GetArray(void);

	BOOL				StartIteration(SMapIterator* psIterator, void** pvKey, void** pvData);
	BOOL				Iterate(SMapIterator* psIterator, void** pvKey, void** pvData);

	BOOL				WriteExceptData(CFileWriter* pcFileWriter);
	BOOL				ReadExceptData(CFileReader* pcFileReader, int(*fKeyCompare)(const void*, const void*));
	void*				WriteKey(CFileWriter* pcFileWriter, int iIndex, int* piDataSize);
	void*				ReadKey(CFileReader* pcFileReader, int iIndex, int* piDataSize);
	BOOL				Write(CFileWriter* pcFileWriter);
	BOOL				Read(CFileReader* pcFileReader, int(*fKeyCompare)(const void*, const void*));

	void				InsertHoldingIntoSorted(void);
	void				GetInSorted(int iIndex, void** ppvKey, void** ppvData);

protected:
	SMNode*				AllocateNode(int iKeySize, int iDataSize, void** ppvKey, void** ppvData);
	void				RemapKeyAndData(SMNode* psNode, void** ppvKey, void** ppvData);
};


#endif // __MAP_BLOCK_H__

