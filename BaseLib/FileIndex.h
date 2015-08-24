#ifndef __FILE_INDEX_H__
#define __FILE_INDEX_H__
#include "AbstractFile.h"


class CFileIndex
{
public:
	int			miFile;
	filePos		mulliFilePos;

	void Init(int iFile, filePos ulliFilePos);
	void Kill(void);
};


#endif // __FILE_INDEX_H__


