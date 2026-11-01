#ifndef __INDEX_TREE_ITERATOR_H__
#define __INDEX_TREE_ITERATOR_H__
#include "IndexTree.h"


class CIndexTreeIterator
{
protected:
	CArrayTemplateEmbedded<uint8, INDEX_TREE_ITERATOR_KEY_SIZE>	macKey;
	char			macData[MAX_DATA_SIZE];
	uint32			miType;
	bool			mbStarted;
	size			miKeySize;
	size			miDataSize;

public:
			void	Init(uint32 iType);
	virtual void	Kill(void);

			size	GetDataSize(void);
			void*	GetData(void);
			size	GetKeySize(void);
			uint8*	GetKey(void);

	virtual bool	Iterate(void) =0;

protected:
			void	Clear(void);
			void	SetKey(uint8* pvKey, size uiKeySize);
};


#endif // __INDEX_TREE_ITERATOR_H__

