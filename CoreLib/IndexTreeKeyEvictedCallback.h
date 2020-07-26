#ifndef __INDEX_TREE_KEY_EVICTED_CALLBACK_H__
#define __INDEX_TREE_KEY_EVICTED_CALLBACK_H__
#include "IndexTreeEvictionCallback.h"


class CIndexTreeKeyEvictedCallback : public CIndexTreeEvictionCallback
{
protected:
	void*					mpvKey; 
	int						miKeyLength;

public:
			void	Init(char* szKey);
			void	Init(void* pvKey, int iKeyLength);
			void	Kill(void);

			BOOL	IndexTreeNodeEvicted(void* pvKey, int iKeySize, void* pvData, int iDataSize);

	virtual void	Evicted(void* pvKey, int iKeySize, void* pvData, int iDataSize) =0;
};


#endif // __INDEX_TREE_KEY_EVICTED_CALLBACK_H__

