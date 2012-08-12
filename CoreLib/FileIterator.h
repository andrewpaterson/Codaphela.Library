#ifndef __FILE_ITERATOR_H__
#define __FILE_ITERATOR_H__
#include "FileIteratorReturn.h"


//This iterator gives back every file, including duplicates.
class CFileIterator
{
public:
	int						miPackFileIndex;
	CPackFileIterator		mcPackFileIterator;
	CFileSystemIterator		mcFileSystemIterator;
	BOOL					mbMoveOn;
	BOOL					mbFileSystem;
	
	CFileIteratorReturn		mcCurrent;
	BOOL					mbHasCurrent;

	void					Init(void);
	void					Kill(void);

	CFileIteratorReturn*	GetCurrent(void);
	CFileIteratorReturn*	SetCurrent(EFileIteratorReturnType eType, CBaseFileNode* pcNode, int iFileRank, char* szOffset);
	CFileIteratorReturn*	SetCurrent(void);
};


#endif // __FILE_ITERATOR_H__

