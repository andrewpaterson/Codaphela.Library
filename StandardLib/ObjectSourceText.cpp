#include "ObjectSourceText.h"
#include "ObjectConverterText.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectSourceText::Init(CObjectConverter* pcConverter, CAbstractFile* pcFile, char* szFilename)
{
	CObjectSingleSource::Init(pcConverter, pcFile, szFilename);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectSourceText::Kill(void)
{
	CObjectSingleSource::Kill();
}

