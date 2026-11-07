#include "BaseLib/ErrorTypes.h"
#include "BaseLib/ChunkFileFile.h"
#include "Unknowns.h"
#include "Objects.h"
#include "ObjectFileGeneral.h"
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
const char* CObjectConverterNative::GetFileExtension(void)
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
CObjectSource* CObjectConverterNative::CreateSource(CAbstractFile* pcFile, const char* szFilename)
{
	CObjectSourceChunked*	pcSourceChunked;
	CObjectSourceSimple*	pcSourceSimple;
	bool					bResult;
	CFileBasic				cFile;
	int32					iFileType;

	if ((pcFile == NULL) || (szFilename == NULL))
	{
		return NULL;
	}

	//The file type decides the source.  cFile is closed but not killed: killing it can free pcFile, which the source keeps.
	cFile.Init(pcFile);
	if (!cFile.Open(EFM_Read))
	{
		return NULL;
	}
	bResult = cFile.ReadInt32(&iFileType);
	cFile.Close();
	if (!bResult)
	{
		return NULL;
	}

	if (iFileType == CHUNKED_OBJECT_FILE)
	{
		pcSourceChunked = UMalloc(CObjectSourceChunked);
		if (!pcSourceChunked->Init(this, pcFile, szFilename))
		{
			pcSourceChunked->Kill();
			return NULL;
		}
		return pcSourceChunked;
	}
	else if (iFileType == BASIC_OBJECT_FILE)
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
CBaseObject* CObjectConverterNative::Convert(CObjectSource* pcSource, const char* szObjectName)
{
	CBaseObject*					pcObject;
	CExternalObjectDeserialiser		cGraphDeserialiser;
	CExternalObjectReader*			pcReader;

	pcReader = pcSource->GetReader();
	if (pcReader == NULL)
	{
		return NULL;
	}

	cGraphDeserialiser.Init(pcReader, true, &gcObjects);
	pcObject = cGraphDeserialiser.Read(szObjectName);
	mcIndexRemap.InsertArrayAfterEnd(cGraphDeserialiser.GetArrayIndexNewOld());
	cGraphDeserialiser.Kill();
	return pcObject;
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

