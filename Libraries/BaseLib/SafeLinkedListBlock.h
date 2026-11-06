#ifndef __SAFE_LINKED_LIST_BLOCK_H__
#define __SAFE_LINKED_LIST_BLOCK_H__
#include "LinkedListBlock.h"
#include "StackMemory.h"
#include "Mutex.h"


class CSafeLinkedListBlock
{
private:
	CMutex				mcMutex;
	CLinkedListBlock	mcList;

public:
	void		Init(void);
	void		Init(CMallocator* pcMalloc);
	void		Kill(void);

	void*		InsertAfterTail(lsize uiDataSize);
	void*		InsertBeforeHead(lsize uiDataSize);
	void*		InsertBeforeNode(lsize uiDataSize, void* psPos);
	void*		InsertAfterNode(lsize uiDataSize, void* psPos); 
	void*		Add(lsize uiDataSize);

	lsize		ByteSize(void);

	bool		Write(CFileWriter* pcFileWriter);
	bool		Read(CFileReader* pcFileReader);

	void		InsertDetachedAfterTail(void* pvData);
	void		Detach(void* pvData);
	void*		DetachHead(void);
	void*		DetachTail(void);

	void* 		GetHead(void);
	void* 		GetTail(void);
	void* 		GetNext(void* pvData);
	void* 		GetPrev(void* pvData);

	void		RemoveTail(void);
	void		Remove(void* pvData);

	void		MoveToHead(void* pvData);
	void		MoveToTTail(void* pvData);


	size		NumElements(void);

	void		FreeDetached(void* pvData);

	void*		Get(size iNum);
	size		IndexOf(void* pvData);
	bool		IsInList(void* pvData);

	void		BubbleSort(DataCompare fCompare);
	void		InsertDetachedIntoSorted(DataCompare fCompare, void* pvData);
};


#endif // __SAFE_LINKED_LIST_BLOCK_H__

