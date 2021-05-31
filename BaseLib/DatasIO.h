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

	template<class M>
	void				Add(char* szClassName);
	template<class M>
	void				Add(const char* szClassName);
	template<class M>
	void				Add(void);

	DataIO_FileWriter	GetFileWriter(char* szClassName);
	DataIO_FileReader	GetFileReader(char* szClassName);
	template<class M>
	DataIO_FileWriter	Save(void);
	template<class M>
	DataIO_FileReader	Load(void);

	SDataIO*			GetIO(char* szClassName);
	SDataIO*			GetIO(const char* szClassName);
	int					NumDataIOs(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CDatasIO::Add(char* szConstructorName)
{
	SDataIO*	psIO;
	BOOL(M::*	fSpecificClassFileSave)(CFileWriter*);
	BOOL(M::*	fSpecificClassFileLoad)(CFileReader*);

	fSpecificClassFileSave = &M::Save;
	fSpecificClassFileLoad = &M::Load;

	psIO = (SDataIO*)mcDataIOs.Put(szConstructorName, sizeof(SDataIO));
	psIO->fWriter = (DataIO_FileWriter)fSpecificClassFileSave;
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
	M			m;
	SDataIO*	psIO;
	BOOL(M::*	fSpecificClassFileSave)(CFileWriter*);
	BOOL(M::*	fSpecificClassFileLoad)(CFileReader*);

	fSpecificClassFileSave = &M::Save;
	fSpecificClassFileLoad = &M::Load;

	psIO = (SDataIO*)mcDataIOs.Put(m.ClassName(), sizeof(SDataIO));
	psIO->fWriter = (DataIO_FileWriter)fSpecificClassFileSave;
	psIO->fReader = (DataIO_FileReader)fSpecificClassFileLoad;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
DataIO_FileWriter CDatasIO::Save(void)
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

