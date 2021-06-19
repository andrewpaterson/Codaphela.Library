#ifndef __DATA_IO_H__
#define __DATA_IO_H__
#include "Define.h"
#include "FileIO.h"


struct SDataTypeIO
{
	BOOL Save(CFileWriter* pcFile);
	BOOL Load(CFileReader* pcFile);
};


typedef BOOL(SDataTypeIO::* DataIO_FileWriter)(CFileWriter*);
typedef BOOL(SDataTypeIO::* DataIO_FileReader)(CFileReader*);


struct SDataIO
{
	DataIO_FileWriter		fWriter;
	DataIO_FileReader		fReader;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL SaveMultiple(M* psThis, CFileWriter* pcFile, size_t uiCount)
{
	size_t	i;
	BOOL	bResult;

	bResult = TRUE;
	for (i = 0; i < uiCount; i++)
	{
		bResult |= psThis->Save(pcFile);
		psThis = psThis + sizeof(M);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL LoadMultiple(M* psThis, CFileReader* pcFile, size_t uiCount)
{
	size_t	i;
	BOOL	bResult;

	bResult = TRUE;
	for (i = 0; i < uiCount; i++)
	{
		bResult |= psThis->Load(pcFile);
		psThis = psThis + sizeof(M);
	}
	return bResult;
}


#endif // !__DATA_IO_H__

