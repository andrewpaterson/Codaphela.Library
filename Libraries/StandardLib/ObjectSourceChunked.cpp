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
#include "BaseLib/ChunkFileFile.h"
#include "Objects.h"
#include "ObjectConverterNative.h"
#include "ObjectFileGeneral.h"
#include "ChunkFileObjectReader.h"
#include "ObjectSourceChunked.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectSourceChunked::Init(CObjectConverter* pcConverter, CAbstractFile* pcFile, const char* szFilename)
{
	CObjectSource::Init(pcConverter, pcFile, szFilename);

	mcChunkFile.Init(pcFile);
	mcChunkFileFileSystem.Init(&mcChunkFile);
	mcNames.Init();

	ReturnOnFalse(mcChunkFile.ReadOpen());
	ReturnOnFalse(ReadNames());

	mpcReader = NULL;
	return mcChunkFile.StackDepth() == 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectSourceChunked::Kill(void)
{
	mcChunkFile.ReadClose();
	mcChunkFile.Kill();
	mcChunkFileFileSystem.Kill();

	mcNames.Kill();

	CObjectSource::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectSourceChunked::ReadNames(void)
{
	SChunkFilenameIterator	sIter;
	char*					szName;

	szName = mcChunkFileFileSystem.StartNameIteration(&sIter);
	while (szName)
	{
		if (!sIter.szValue.StartsWith(OBJECT_UNNAMED_FILE))
		{
			mcNames.Add(szName);
		}
		szName = mcChunkFileFileSystem.IterateName(&sIter);
	}
	mcChunkFileFileSystem.StopIteration(&sIter);


	mcNames.QuickSort(true);

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectSourceChunked::Convert(const char* szFullName)
{
	CBaseObject*	pvObject;

	mpcReader = UMalloc(CChunkFileObjectReader);
	mpcReader->Init(&mcChunkFileFileSystem);

	pvObject = mpcConverter->Convert(this, szFullName);
	mpcReader->Kill();

	return pvObject;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectSourceChunked::Contains(const char* szFullName)
{
	size	iIndex;

	iIndex = mcNames.FindInSorted(szFullName, false);
	return iIndex != ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CObjectSourceChunked::GetName(size iIndex)
{
	return mcNames.Get(iIndex)->Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CObjectSourceChunked::NumNames(void)
{
	return mcNames.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectSourceChunked::IsMultiSource(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectSourceChunked::IsNative(void)
{
	return true; 
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CExternalObjectReader* CObjectSourceChunked::GetReader(void)
{
	return mpcReader;
}
