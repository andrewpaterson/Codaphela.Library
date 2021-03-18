#include "BaseLib/ErrorTypes.h"
#include "BaseLib/ChunkFileFile.h"
#include "Unknowns.h"
#include "Objects.h"
#include "ObjectFileGeneral.h"
#include "IndexGenerator.h"
#include "ObjectDeserialiser.h"
#include "ExternalObjectDeserialiser.h"
#include "ObjectConverterNative.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectConverterNative::Init(void)
{
	mcDependentObjects.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectConverterNative::Kill(void)
{
	mcDependentObjects.Kill();
	CObjectConverter::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CObjectConverterNative::GetFileExtension(void)
{
	return "DRG";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectConverterNative::IsFor(CAbstractFile* pcFile)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CObjectSource* CObjectConverterNative::CreateSource(CAbstractFile* pcFile, char* szFileName)
{
	CObjectSourceChunked*	pcSourceChunked;
	CObjectSourceSimple*	pcSourceSimple;
	BOOL					bResult;
	CFileBasic				cFile;
	int						c;

	if ((!pcFile) || (!szFileName))
	{
		return NULL;
	}

	cFile.Init(pcFile);
	bResult = cFile.Open(EFM_Read);
	if (!bResult)
	{
		return NULL;
	}

	bResult = cFile.ReadInt(&c);
	if (!bResult)
	{
		return NULL;
	}

	if (c == CHUNKED_OBJECT_FILE)
	{
		pcSourceChunked = UMalloc(CObjectSourceChunked);
		bResult = pcSourceChunked->Init(this, pcFile, szFileName);
		if (!bResult)
		{
			return NULL;
		}
		return pcSourceChunked;
	}
	else if (c == BASIC_OBJECT_FILE)
	{
		pcSourceSimple = UMalloc(CObjectSourceSimple);
		pcSourceSimple->Init(this, pcFile, szFileName);
		return pcSourceSimple;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectConverterNative::Convert(CObjectSource* pcSource, char* szObjectName)
{
	CChunkFileFile				cFile;
	CBaseObject*				pvObject;
	CFileBasic					cFileBasic;
	CExternalObjectDeserialiser	cGraphDeserialiser;
	CObjectReader*				pcReader;

	//This if statement should be calling a virtual method instead.
	pcReader = pcSource->GetReader();
	if (!pcReader)
	{
		return NULL;
	}

	mcDependentObjects.Kill();
	mcDependentObjects.Init();

	cGraphDeserialiser.Init(pcReader, TRUE, &gcObjects, &mcDependentObjects, gcObjects.GetMemory());
	pvObject = cGraphDeserialiser.Read(szObjectName);
	cGraphDeserialiser.Kill();

	return pvObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectConverterNative::IsNative(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CObjectConverterNative::TestGetNewIndexFromOld(OIndex oiOld)
{
	return mcDependentObjects.GetNewIndexFromOld(oiOld);
}
