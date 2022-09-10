#ifndef __INDEX_TREE_FILE_ITERATOR_H__
#define __INDEX_TREE_FILE_ITERATOR_H__
#include "BaseLib/IndexTreeIterator.h"
#include "IndexTreeFile.h"


#define INDEX_TREE_FILE_ITERATOR_TYPE 0x63847892


class CIndexTreeFileIterator : public CIndexTreeIterator
{
protected:
	SIndexTreeFileIterator	sIter;
	CIndexTreeFile*			mpcTree;

public:
	void	Init(CIndexTreeFile* pcIndexTree);
	void	Kill(void);

	bool	Iterate(void);
};


#endif // !__INDEX_TREE_FILE_ITERATOR_H__


