#ifndef __EVICTED_LIST__
#define __EVICTED_LIST__
#include "BaseLib/ListVariable.h"
#include "EvictionCallback.h"


class CEvictedList : public CEvictionCallback
{
private:
	CListVariable	mcKeys;
	CListVariable	mcDatas;

public:
	void	Init(void);
	void	Kill(void);

	BOOL	NodeEvicted(unsigned char* pvKey, int iKeySize, void* pvData, int iDataSize);
	
	int		NumElements(void);
	void*	GetKey(int iIndex, int* piKeySize = NULL);
	void*	GetData(int iIndex, int* piDataSize = NULL);
	void	Clear(void);
};


#endif // __EVICTED_LIST__

