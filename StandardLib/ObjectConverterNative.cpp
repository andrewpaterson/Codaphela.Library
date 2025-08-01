#include "BaseLib/ErrorTypes.h"
#include "BaseLib/ChunkFileFile.h"
#include "Unknowns.h"
#include "Objects.h"
#include "ObjectFileGeneral.h"
#include "IndexGenerator.h"
#include "ObjectReader.h"
#include "ExternalObjectDeserialiser.h"
#include "ObjectConverterNative.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectConverterNative::Init(void)
{
	mcIndexRemap.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectConverterNative::Kill(void)
{
	mcIndexRemap.Kill();
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
bool CObjectConverterNative::IsFor(CAbstractFile* pcFile)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CObjectSource* CObjectConverterNative::CreateSource(CAbstractFile* pcFile, char* szFilename)
{
	CObjectSourceChunked*	pcSourceChunked;
	CObjectSourceSimple*	pcSourceSimple;
	bool					bResult;
	CFileBasic				cFile;
	int						c;

	if ((!pcFile) || (!szFilename))
	{
		return NULL;
	}

	cFile.Init(pcFile);
	bResult = cFile.Open(EFM_Read);
	if (!bResult)
	{
		return NULL;
	}

	bResult = cFile.ReadInt32(&c);
	if (!bResult)
	{
		return NULL;
	}

	if (c == CHUNKED_OBJECT_FILE)
	{
		pcSourceChunked = UMalloc(CObjectSourceChunked);
		bResult = pcSourceChunked->Init(this, pcFile, szFilename);
		if (!bResult)
		{
			return NULL;
		}
		return pcSourceChunked;
	}
	else if (c == BASIC_OBJECT_FILE)
	{
		pcSourceSimple = UMalloc(CObjectSourceSimple);
		pcSourceSimple->Init(this, pcFile, szFilename);
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
	CChunkFileFile					cFile;
	CBaseObject*					pvObject;
	CFileBasic						cFileBasic;
	CExternalObjectDeserialiser		cGraphDeserialiser;
	CExternalObjectReader*			pcReader;

	//This if statement should be calling a virtual method instead.
	pcReader = pcSource->GetReader();
	if (!pcReader)
	{
		return NULL;
	}

	cGraphDeserialiser.Init(pcReader, true, &gcObjects);
	pvObject = cGraphDeserialiser.Read(szObjectName);

	mcIndexRemap.InsertArrayAfterEnd(cGraphDeserialiser.GetArrayIndexNewOld());

	cGraphDeserialiser.Kill();

	return pvObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectConverterNative::IsNative(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CObjectConverterNative::TestGetNewIndexFromOld(OIndex oiOld)
{
	size			i;
	CIndexNewOld*	pcRemap;
	size			uiIndices;

	uiIndices = mcIndexRemap.NumElements();
	for (i = 0; i < uiIndices; i++)
	{
		pcRemap = mcIndexRemap.Get(i);
		if (pcRemap->moiOld == oiOld)
		{
			return pcRemap->moiNew;
		}
	}
	return INVALID_O_INDEX;
}

