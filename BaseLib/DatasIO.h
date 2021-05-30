#ifndef __DATAS_IO_H__
#define __DATAS_IO_H__
#include "MapStringBlock.h"


struct SDataTypeIO
{
	BOOL Save(CFileWriter* pcFile);
	BOOL Load(CFileReader* pcFile);
};


typedef BOOL (SDataTypeIO::* DataTypeIO_FileWriter)(CFileWriter*);
typedef BOOL (SDataTypeIO::* DataIO_FileReader)(CFileReader*);


struct SDataIO
{
	DataTypeIO_FileWriter	fWriter;
	DataIO_FileReader		fReader;
};


class CDatasIO
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
	DataIO_FileReader	Load(char* szClassName);
	template<class M>
	DataTypeIO_FileWriter	Save(void);
	template<class M>
	DataIO_FileReader	Load(void);

	SDataIO*				GetIO(char* szClassName);
	SDataIO*				GetIO(const char* szClassName);
	int						NumDataIOs(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CDatasIO::Add(char* szConstructorName)
{
	SDataIO*	psIO;
	BOOL(M::*				fSpecificClassFileSave)(CFileWriter*);
	BOOL(M::*				fSpecificClassFileLoad)(CFileReader*);

	fSpecificClassFileSave = &M::Save;
	fSpecificClassFileLoad = &M::Load;

	psIO = (SDataIO*)mcDataIOs.Put(szConstructorName, sizeof(SDataIO));
	psIO->fWriter = (DataTypeIO_FileWriter)fSpecificClassFileSave;
	psIO->fReader = (DataIO_FileReader)fSpecificClassFileLoad;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CDatasIO::Add(const char* szConstructorName)
{
	return Add<M>((char*)szConstructorName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CDatasIO::Add(void)
{
	M						m;
	SDataIO*	psIO;
	BOOL(M::*				fSpecificClassFileSave)(CFileWriter*);
	BOOL(M::*				fSpecificClassFileLoad)(CFileReader*);

	fSpecificClassFileSave = &M::Save;
	fSpecificClassFileLoad = &M::Load;

	psIO = (SDataIO*)mcDataIOs.Put(m.ClassName(), sizeof(SDataIO));
	psIO->fWriter = (DataTypeIO_FileWriter)fSpecificClassFileSave;
	psIO->fReader = (DataIO_FileReader)fSpecificClassFileLoad;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
DataTypeIO_FileWriter CDatasIO::Save(void)
{
	M m;

	return Save((char*)m.ClassName());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
DataIO_FileReader CDatasIO::Load(void)
{
	M m;

	return Load((char*)m.ClassName());
}


#endif // __DATAS_IO_H__

