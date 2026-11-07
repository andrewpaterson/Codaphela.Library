/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
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
void CObjectConverterNative::Init(CObjects* pcObjects)
{
	CObjectConverter::Init(pcObjects);
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
	return OBJECT_FILE_EXTENSION;
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
// A chunked file starts with its type.  A basic file starts with the
// object file extension and then its type (CMultiFileObjectWriter).
//
//////////////////////////////////////////////////////////////////////////
int32 CObjectConverterNative::ReadFileType(CFileBasic* pcFile)
{
	char	szExtension[4];
	int32	iFileType;

	if (!pcFile->ReadData(szExtension, 4))
	{
		return 0;
	}
	if (memcmp(szExtension, OBJECT_FILE_EXTENSION, 4) != 0)
	{
		memcpy(&iFileType, szExtension, 4);
		return iFileType;
	}
	if (!pcFile->ReadInt32(&iFileType))
	{
		return 0;
	}
	return iFileType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CObjectSource* CObjectConverterNative::CreateSource(CAbstractFile* pcFile, const char* szFilename)
{
	CObjectSourceChunked*	pcSourceChunked;
	CObjectSourceSimple*	pcSourceSimple;
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
	iFileType = ReadFileType(&cFile);
	cFile.Close();

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

	cGraphDeserialiser.Init(pcReader, true, mpcObjects);
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

