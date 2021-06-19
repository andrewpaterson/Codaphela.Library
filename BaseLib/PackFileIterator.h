#ifndef __PACK_FILE_ITERATOR_H__
#define __PACK_FILE_ITERATOR_H__
#include "PackFilesGeneral.h"


struct SPackFileIteratorPosition
{
	CFileNodePackFileNode*	pcNode;
	int						iIndex;
};


typedef CArrayTemplate<SPackFileIteratorPosition>	CArrayPackFileIteratorPosition;


class CPackFileIterator
{
protected:
	CArrayPackFileIteratorPosition	macDepth;
	CFileNodePackFileNode*			mpcCurrent;

public:
	void							Init(void);
	void							Kill(void);

	void							Push(CFileNodePackFileNode* pcNode);
	SPackFileIteratorPosition*		Peek(void);
	void							Pop(void);

	void							SetCurrent(CFileNodePackFileNode* pcNode);
	CFileNodePackFileNode*			Current(void);
};


#endif // !__PACK_FILE_ITERATOR_H__

