#ifndef __INDEX_TREE_ITERATOR_H__
#define __INDEX_TREE_ITERATOR_H__
#include "IndexTree.h"


class CIndexTreeIterator
{
protected:
	char			macKey[MAX_KEY_SIZE];
	char			macData[MAX_DATA_SIZE];
	int				miType;
	BOOL			mbStarted;
	int				miKeySize;
	int				miDataSize;

public:
			void	Init(int iType);
	virtual void	Kill(void);

			int		GetDataSize(void);
			void*	GetData(void);
			int		GetKeySize(void);
			char*	GetKey(void);

	virtual BOOL	Iterate(void) =0;

protected:
			void	Clear(void);
};


#endif // __INDEX_TREE_ITERATOR_H__

