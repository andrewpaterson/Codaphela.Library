#ifndef __FILE_NODE_ITERATOR_H__
#define __FILE_NODE_ITERATOR_H__
#include "FileSystemGeneral.h"


struct SFileSystemIteratorPosition
{
	CSystemFileNode*	pcNode;
	int					iIndex;
};


typedef CArrayTemplate<SFileSystemIteratorPosition>	CArrayFileSystemIteratorPosition;


class CFileNodeIterator
{
protected:
	CArrayFileSystemIteratorPosition	macDepth;
	CSystemFileNode*					mpcCurrent;

public:
	void							Init(void);
	void							Kill(void);

	void							Push(CSystemFileNode* pcNode);
	SFileSystemIteratorPosition*	Peek(void);
	void							Pop(void);

	void							SetCurrent(CSystemFileNode* pcNode);
	CFileNodeSystemFile*			Current(void);
};


#endif // __FILE_NODE_ITERATOR_H__

