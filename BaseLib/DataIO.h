#ifndef __DATA_IO_H__
#define __DATA_IO_H__
#include "Define.h"
#include "FileIO.h"


struct SDataTypeIO
{
	bool Save(CFileWriter* pcFileWriter);
	bool Load(CFileReader* pcFileReader);
};


typedef bool(SDataTypeIO::* DataIO_FileWriter)(CFileWriter*);
typedef bool(SDataTypeIO::* DataIO_FileReader)(CFileReader*);


struct SDataIO
{
	DataIO_FileWriter		fWriter;
	DataIO_FileReader		fReader;
};


class CDataIO
{
public:
	virtual bool WriteData(CFileWriter* pcFileWriter, void* pvData) =0;
	virtual bool ReadData(CFileReader* pcFileReader, void* pvData) =0;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool SaveMultiple(M* psThis, CFileWriter* pcFileWriter, size uiCount)
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
bool LoadMultiple(M* psThis, CFileReader* pcFileReader, size uiCount)
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

