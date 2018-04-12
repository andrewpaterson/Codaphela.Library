#ifndef __FILE_INDEX_H__
#define __FILE_INDEX_H__
#include "AbstractFile.h"


class CFileIndex
{
public:
	filePos		mulliFilePos;
	int			miFile;

	void Init(void);
	void Init(int iFile, filePos ulliFilePos);
	void Kill(void);

	BOOL HasFile(void);
	void SetIndex(int iFile, filePos ulliFilePos);
};


#endif // __FILE_INDEX_H__

