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
#include "BaseLib/FileUtil.h"
#include "ChunkFileObjectWriter.h"
#include "ChunkFileSystemObjectReader.h"
#include "ExternalObjectDeserialiser.h"
#include "ExternalObjectSerialiser.h"
#include "ObjectFileGeneral.h"
#include "Objects.h"
#include "NamedObjectFile.h"


//////////////////////////////////////////////////////////////////////////
//
// szFilename as the folder it's in and its name less the object files'
// extension, which is how the objects' reader and writer are told of a
// file.
//
//////////////////////////////////////////////////////////////////////////
static void NamedObjectFileSplit(const char* szFilename, CChars* pszDirectory, CChars* pszName)
{
	CFileUtil	cFileUtil;
	CChars		szExtension;

	pszDirectory->Init(szFilename);
	cFileUtil.RemoveLastFromPath(pszDirectory);
	if (pszDirectory->Empty())
	{
		pszDirectory->Append(".");
	}
	pszName->Init();
	cFileUtil.GetFileName(pszName, szFilename);

	szExtension.Init(".");
	szExtension.Append(OBJECT_FILE_EXTENSION);
	if (pszName->EndsWithIgnoreCase(szExtension.Text()))
	{
		pszName->RemoveFromEnd(szExtension.Length());
	}
	szExtension.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool WriteNamedObjectFile(CBaseObject* pcObject, const char* szFilename)
{
	CExternalObjectSerialiser	cSerialiser;
	CChunkFileObjectWriter		cWriter;
	CChars						szDirectory;
	CChars						szName;
	bool						bResult;

	NamedObjectFileSplit(szFilename, &szDirectory, &szName);
	cWriter.Init(szDirectory.Text(), "", szName.Text());
	cSerialiser.Init(&cWriter);
	bResult = cSerialiser.Write(pcObject);
	cSerialiser.Kill();
	cWriter.Kill();
	szName.Kill();
	szDirectory.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* ReadNamedObjectFile(const char* szFilename, const char* szObjectName)
{
	CExternalObjectDeserialiser		cDeserialiser;
	CChunkFileSystemObjectReader	cReader;
	CBaseObject*					pcObject;
	CFileUtil						cFileUtil;
	CChars							szDirectory;
	CChars							szName;
	CChars							szFile;

	NamedObjectFileSplit(szFilename, &szDirectory, &szName);
	szFile.Init(szDirectory);
	cFileUtil.AppendToPath(&szFile, szName.Text());
	szFile.Append(".");
	szFile.Append(OBJECT_FILE_EXTENSION);

	pcObject = NULL;
	if (cFileUtil.Exists(szFile.Text()))
	{
		cReader.Init(szDirectory.Text(), szName.Text());
		cDeserialiser.Init(&cReader, false, &gcObjects);
		pcObject = cDeserialiser.Read(szObjectName);
		cDeserialiser.Kill();
		cReader.Kill();
	}
	szFile.Kill();
	szName.Kill();
	szDirectory.Kill();
	return pcObject;
}

