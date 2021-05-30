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


struct SDataIO
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

	DataTypeIO_FileWriter	Save(char* szClassName);
	DataTypeIO_FileReader	Load(char* szClassName);
	template<class M>
	DataTypeIO_FileWriter	Save(void);
	template<class M>
	DataTypeIO_FileReader	Load(void);

	SDataIO*				GetIO(char* szClassName);
	int						NumDataIOs(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CDataTypesIO::Add(char* szConstructorName)
{
	SDataIO*	psIO;
	BOOL(M::*				fSpecificClassFileSave)(CFileWriter*);
	BOOL(M::*				fSpecificClassFileLoad)(CFileReader*);

	fSpecificClassFileSave = &M::Save;
	fSpecificClassFileLoad = &M::Load;

	psIO = (SDataIO*)mcDataIOs.Put(szConstructorName, sizeof(SDataIO));
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
	SDataIO*	psIO;
	BOOL(M::*				fSpecificClassFileSave)(CFileWriter*);
	BOOL(M::*				fSpecificClassFileLoad)(CFileReader*);

	fSpecificClassFileSave = &M::Save;
	fSpecificClassFileLoad = &M::Load;

	psIO = (SDataIO*)mcDataIOs.Put(m.ClassName(), sizeof(SDataIO));
	psIO->fWriter = (DataTypeIO_FileWriter)fSpecificClassFileSave;
	psIO->fReader = (DataTypeIO_FileReader)fSpecificClassFileLoad;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
DataTypeIO_FileWriter CDataTypesIO::Save(void)
{
	M m;

	return Save((char*)m.ClassName());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
DataTypeIO_FileReader CDataTypesIO::Load(void)
{
	M m;

	return Load((char*)m.ClassName());
}


#endif // __DATA_TYPES_IO_H__

