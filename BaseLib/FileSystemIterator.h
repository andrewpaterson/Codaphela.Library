#ifndef __FILE_SYSTEM_ITERATOR_H__
#define __FILE_SYSTEM_ITERATOR_H__
#include "FileSystemGeneral.h"


struct SFileSystemIteratorPosition
{
	CSystemFileNode*	pcNode;
	int					iIndex;
};


typedef CArrayTemplate<SFileSystemIteratorPosition>	CArrayFileSystemIteratorPosition;


class CFileSystemIterator
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
	CSystemFileNode*				Current(void);
};


#endif // __FILE_SYSTEM_ITERATOR_H__

