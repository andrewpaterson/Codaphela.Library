#ifndef __FILE_INDEX_H__
#define __FILE_INDEX_H__
#include "AbstractFile.h"


//mulliFilePos is a direct offset into the file from its start in bytes.
class CFileDataIndex;
class CFilePosIndex
{
public:
	filePos			mulliFilePos;
	int				miFile;

public:
	void			Init(void);
	void			Init(int iFile, filePos ulliFilePos);
	void			Kill(void);

	BOOL			HasFile(void);
	void			SetIndex(int iFile, filePos ulliFilePos);

	CFileDataIndex	ToFileDataIndex(unsigned int muiDataSize);
};


//muiIndex is an offset into the file that needs to be multiplied by some data size.
class CFileDataIndex
{
public:
	unsigned int	muiIndex;
	int				miFile;

public:
	void Init(void);
	void Init(int iFile, unsigned int uiIndex);
	void Kill(void);

	BOOL HasFile(void);
	void SetIndex(int iFile, unsigned int uiIndex);
	void SetIndex(CFileDataIndex* pcSource);

	BOOL Equals(CFileDataIndex* pcOther);

	CFilePosIndex	ToFilePosIndex(unsigned int muiDataSize);
};


#endif // __FILE_INDEX_H__

