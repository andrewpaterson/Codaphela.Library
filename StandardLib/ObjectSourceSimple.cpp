#include "Unknowns.h"
#include "ObjectSourceSimple.h"
#include "FileObjectReader.h"
#include "ObjectConverterNative.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectSourceSimple::Init(CObjectConverter* pcConverter, CAbstractFile* pcFile, char* szFileName)
{
	CObjectSingleSource::Init(pcConverter, pcFile, szFileName);
	mpcReader = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectSourceSimple::Kill(void)
{
	CObjectSingleSource::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectSourceSimple::IsSimple(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectSourceSimple::IsNative(void)
{
	return TRUE; 
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectSourceSimple::Contains(char* szFullName)
{
	return CObjectSingleSource::Contains(szFullName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectSourceSimple::Convert(char* szFullName)
{
	CBaseObject*	pvObject;

	mpcReader = UMalloc(CFileObjectReader);
	mpcReader->Init(NULL);

	pvObject = mpcConverter->Convert(this, szFullName);
	mpcReader->Kill();

	return pvObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CExternalObjectReader* CObjectSourceSimple::GetReader(void)
{
	return mpcReader;
}

