#ifndef __SAFE_LINKED_LIST_BLOCK_H__
#define __SAFE_LINKED_LIST_BLOCK_H__
#include <mutex>
#include "BaseLib/LinkedListBlock.h"
#include "BaseLib/StackMemory.h"


class CSafeLinkedListBlock
{
private:
	std::mutex			m;
	CLinkedListBlock	c;

public:
	void		Init(void);
	void		Init(CMallocator* pcMalloc);
	void		Kill(void);

	void*		InsertAfterTail(unsigned int uiDataSize);
	void*		InsertBeforeHead(unsigned int uiDataSize);
	void*		InsertBeforeNode(unsigned int uiDataSize, void* psPos);
	void*		InsertAfterNode(unsigned int uiDataSize, void* psPos); 
	void*		Add(unsigned int uiDataSize);

	int			ByteSize(void);

	BOOL		Write(CFileWriter* pcFileWriter);
	BOOL		Read(CFileReader* pcFileReader);

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


	int			NumElements(void);

	void		FreeDetached(void* pvData);

	void*		Get(int iNum);
	int			IndexOf(void* pvData);
	BOOL		IsInList(void* pvData);

	void		BubbleSort(int(*fCompare)(const void*, const void*));
	void		InsertDetachedIntoSorted(int(*fCompare)(const void*, const void*), void* pvData);
};


#endif // __SAFE_LINKED_LIST_BLOCK_H__

