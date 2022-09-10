#ifndef __DATAS_IO_H__
#define __DATAS_IO_H__
#include "DataIO.h"
#include "MapStringBlock.h"


class CDatasIO
{
protected:
	CMapStringBlock		mcDataIOs;

public:
	void				Init(void);
	void				Kill(void);

	void				Add(const char* szFilename, DataIO_FileWriter fSpecificClassFileSave, DataIO_FileReader fSpecificClassFileLoad);
	template<class Class>
	void				Add(char* szClassName);
	template<class Class>
	void				Add(const char* szClassName);
	template<class Class>
	void				Add(void);

	DataIO_FileWriter	GetFileWriter(char* szClassName);
	DataIO_FileReader	GetFileReader(char* szClassName);
	template<class Class>
	DataIO_FileWriter	GetFileWriter(void);
	template<class Class>
	DataIO_FileReader	GetFileReader(void);

	SDataIO*			GetIO(char* szClassName);
	SDataIO*			GetIO(const char* szClassName);
	int					NumDataIOs(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class Class>
void CDatasIO::Add(char* szConstructorName)
{
	SDataIO*	psIO;
	bool(Class::*	fSpecificClassFileSave)(CFileWriter*);
	bool(Class::*	fSpecificClassFileLoad)(CFileReader*);

	fSpecificClassFileSave = &Class::Save;
	fSpecificClassFileLoad = &Class::Load;

	psIO = (SDataIO*)mcDataIOs.Put(szConstructorName, sizeof(SDataIO));
	psIO->fWriter = (DataIO_FileWriter)fSpecificClassFileSave;
	psIO->fReader = (DataIO_FileReader)fSpecificClassFileLoad;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class Class>
void CDatasIO::Add(const char* szConstructorName)
{
	return Add<Class>((char*)szConstructorName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class Class>
void CDatasIO::Add(void)
{
	Class			m;
	SDataIO*	psIO;
	bool(Class::*	fSpecificClassFileSave)(CFileWriter*);
	bool(Class::*	fSpecificClassFileLoad)(CFileReader*);

	fSpecificClassFileSave = &Class::Save;
	fSpecificClassFileLoad = &Class::Load;

	psIO = (SDataIO*)mcDataIOs.Put(m.ClassName(), sizeof(SDataIO));
	psIO->fWriter = (DataIO_FileWriter)fSpecificClassFileSave;
	psIO->fReader = (DataIO_FileReader)fSpecificClassFileLoad;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class Class>
DataIO_FileWriter CDatasIO::GetFileWriter(void)
{
	Class m;

	return GetFileWriter((char*)m.ClassName());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class Class>
DataIO_FileReader CDatasIO::GetFileReader(void)
{
	Class m;

	return GetFileReader((char*)m.ClassName());
}


#endif // !__DATAS_IO_H__

