#ifndef __FILE_INDEX_H__
#define __FILE_INDEX_H__
#include "AbstractFile.h"


class CFilePosIndex
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


class CFileDataIndex
{
public:
	unsigned int	muiIndex;
	int				miFile;

	void Init(void);
	void Init(int iFile, unsigned int uiIndex);
	void Kill(void);

	BOOL HasFile(void);
	void SetIndex(int iFile, unsigned int uiIndex);
};


#endif // __FILE_INDEX_H__

