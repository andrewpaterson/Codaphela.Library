#ifndef __DATA_TYPES_IO_H__
#define __DATA_TYPES_IO_H__
#include "MapStringBlock.h"


struct SDataTypeIO
{
	BOOL Save(CFileWriter* pcFile);
	BOOL Load(CFileReader* pcFile);
};


typedef BOOL (SDataTypeIO::* DataTypeIO_FileWriter)(CFileWriter*);
typedef BOOL (SDataTypeIO::* DataTypeIO_FileReader)(CFileReader*);


struct SDataTypeIOPointers
{
	DataTypeIO_FileWriter	fWriter;
	DataTypeIO_FileReader	fReader;
};


class CDataTypesIO
{
protected:
	CMapStringBlock		mcDataIOs;

public:
	void					Init(void);
	void					Kill(void);

	template<class M>
	void					Add(char* szClassName);
	template<class M>
	void					Add(const char* szClassName);
	template<class M>
	void					Add(void);

	DataTypeIO_FileWriter	GetSave(char* szClassName);
	DataTypeIO_FileReader	GetLoad(char* szClassName);
	template<class M>
	DataTypeIO_FileWriter	GetSave(void);
	template<class M>
	DataTypeIO_FileReader	GetLoad(void);

	int						NumDataIOs(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CDataTypesIO::Add(char* szConstructorName)
{
	SDataTypeIOPointers*	psIO;
	BOOL(M::*				fSpecificClassFileSave)(CFileWriter*);
	BOOL(M::*				fSpecificClassFileLoad)(CFileReader*);

	fSpecificClassFileSave = &M::Save;
	fSpecificClassFileLoad = &M::Load;

	psIO = (SDataTypeIOPointers*)mcDataIOs.Put(szConstructorName, sizeof(SDataTypeIOPointers));
	psIO->fWriter = (DataTypeIO_FileWriter)fSpecificClassFileSave;
	psIO->fReader = (DataTypeIO_FileReader)fSpecificClassFileLoad;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CDataTypesIO::Add(const char* szConstructorName)
{
	return Add<M>((char*)szConstructorName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CDataTypesIO::Add(void)
{
	M						m;
	SDataTypeIOPointers*	psIO;
	BOOL(M::*				fSpecificClassFileSave)(CFileWriter*);
	BOOL(M::*				fSpecificClassFileLoad)(CFileReader*);

	fSpecificClassFileSave = &M::Save;
	fSpecificClassFileLoad = &M::Load;

	psIO = (SDataTypeIOPointers*)mcDataIOs.Put(m.ClassName(), sizeof(SDataTypeIOPointers));
	psIO->fWriter = (DataTypeIO_FileWriter)fSpecificClassFileSave;
	psIO->fReader = (DataTypeIO_FileReader)fSpecificClassFileLoad;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
DataTypeIO_FileWriter CDataTypesIO::GetSave(void)
{
	M m;

	return GetSave((char*)m.ClassName());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
DataTypeIO_FileReader CDataTypesIO::GetLoad(void)
{
	M m;

	return GetLoad((char*)m.ClassName());
}


#endif // __DATA_TYPES_IO_H__
