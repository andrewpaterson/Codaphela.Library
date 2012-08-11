#ifndef __FILE_ITERATOR_H__
#define __FILE_ITERATOR_H__
#include "BaseLib/FileSystemIterator.h"
#include "BaseLib/PackFileIterator.h"


//This iterator gives back every file, including duplicates.
class CFileIterator
{
public:
	int							i;
	CPackFileIterator			cPackFileIterator;
	CFileSystemIterator			cFileSystemIterator;
	BOOL						mbMoveOn;
	BOOL						mbFileSystem;

	void Init(void);
	void Kill(void);
};


#endif // __FILE_ITERATOR_H__

