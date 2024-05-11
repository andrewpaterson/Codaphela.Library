#ifndef __DATA_IO_H__
#define __DATA_IO_H__
#include "Define.h"
#include "FileIO.h"


struct SDataTypeIO
{
	bool Save(CFileWriter* pcFile);
	bool Load(CFileReader* pcFile);
};


typedef bool(SDataTypeIO::* DataIO_FileWriter)(CFileWriter*);
typedef bool(SDataTypeIO::* DataIO_FileReader)(CFileReader*);


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
bool SaveMultiple(M* psThis, CFileWriter* pcFile, size uiCount)
{
	size	i;
	bool	bResult;

	bResult = true;
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
bool LoadMultiple(M* psThis, CFileReader* pcFile, size uiCount)
{
	size	i;
	bool	bResult;

	bResult = true;
	for (i = 0; i < uiCount; i++)
	{
		bResult |= psThis->Load(pcFile);
		psThis = psThis + sizeof(M);
	}
	return bResult;
}


#endif // __DATA_IO_H__

