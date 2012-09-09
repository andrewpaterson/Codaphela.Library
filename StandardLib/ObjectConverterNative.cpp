#include "BaseLib/ErrorTypes.h"
#include "BaseLib/ChunkFileFile.h"
#include "Unknowns.h"
#include "Objects.h"
#include "ObjectFileGeneral.h"
#include "SerialisedObjectReader.h"
#include "IndexGenerator.h"
#include "ObjectDeserialiser.h"
#include "HollowObjectGraphDeserialiser.h"
#include "ObjectConverterNative.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectConverterNative::Init(CIndexGenerator* pcIndexGenerator)
{
	mpcIndexGenerator = pcIndexGenerator;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectConverterNative::Kill(void)
{
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
CPointerObject CObjectConverterNative::Convert(CObjectSource* pcSource, char* szObjectName)
{
	CChunkFileFile						cFile;
	CPointerObject						cPointer;
	CFileBasic							cFileBasic;
	CHollowObjectGraphDeserialiser		cGraphDeserialiser;
	CObjectSourceChunked*				pcSourceChunked;
	CObjectSourceSimple*				pcSourceSimple;
	CObjectReader*						pcReader;

	if (pcSource->IsMultiSource() && pcSource->IsNative())
	{
		pcSourceChunked = (CObjectSourceChunked*)pcSource;
		pcReader = pcSourceChunked->GetReader();
	}
	else if (!pcSource->IsMultiSource() && pcSource->IsNative())
	{
		pcSourceSimple = (CObjectSourceSimple*)pcSource;
		pcReader = pcSourceSimple->GetReader();
	}
	else
	{
		return ONull;
	}

	cGraphDeserialiser.Init(pcReader, mpcIndexGenerator);
	cPointer = cGraphDeserialiser.Read(szObjectName);
	cGraphDeserialiser.Kill();

	return cPointer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectConverterNative::IsNative(void)
{
	return TRUE;
}

