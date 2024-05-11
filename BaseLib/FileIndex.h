#ifndef __FILE_INDEX_H__
#define __FILE_INDEX_H__
#include "AbstractFile.h"


//mulliFilePos is a direct offset into the file from its start in bytes.
class CFileDataIndex;
class CFilePosIndex
{
public:
	filePos			mulliFilePos;
	uint32			miFile;

public:
	void			Init(void);
	void			Init(uint32 iFile, filePos ulliFilePos);
	void			Kill(void);

	bool			HasFile(void);
	void			SetIndex(uint32 iFile, filePos ulliFilePos);

	CFileDataIndex	ToFileDataIndex(uint32 muiDataSize);
};


//muiIndex is an offset into the file that needs to be multiplied by some data size.
class CFileDataIndex
{
public:
	uint32	muiIndex;
	uint32	miFile;

public:
	void			Init(void);
	void			Init(uint32 iFile, uint32 uiIndex);
	void			Kill(void);

	bool			HasFile(void);
	void			SetIndex(uint32 iFile, uint32 uiIndex);
	void			SetIndex(CFileDataIndex* pcSource);

	bool			Equals(CFileDataIndex* pcOther);

	CFilePosIndex	ToFilePosIndex(uint32 muiDataSize);
};


#endif // __FILE_INDEX_H__

