#ifndef __INDEX_TREE_ITERATOR_H__
#define __INDEX_TREE_ITERATOR_H__
#include "IndexTree.h"


class CIndexTreeIterator
{
protected:
	uint8			macKey[MAX_KEY_SIZE];
	char			macData[MAX_DATA_SIZE];
	int				miType;
	bool			mbStarted;
	size			miKeySize;
	size			miDataSize;

public:
			void	Init(int iType);
	virtual void	Kill(void);

			size	GetDataSize(void);
			void*	GetData(void);
			size	GetKeySize(void);
			uint8*	GetKey(void);

	virtual bool	Iterate(void) =0;

protected:
			void	Clear(void);
};


#endif // __INDEX_TREE_ITERATOR_H__

