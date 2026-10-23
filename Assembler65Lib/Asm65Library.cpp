/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela Assembler65Lib

Codaphela Assembler65Lib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela Assembler65Lib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela Assembler65Lib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/PackFiles.h"
#include "BaseLib/PackFile.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/PointerFunctions.h"
#include "Asm65Library.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Library::Init(const char* szName)
{
	mszName.Init(szName);
	mapcObjects.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Library::Kill(void)
{
	size			i;
	CAsm65Object*	pcObject;

	for (i = 0; i < mapcObjects.NumElements(); i++)
	{
		pcObject = mapcObjects.GetPtr(i);
		SafeKill(pcObject);
	}
	mapcObjects.Kill();
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CAsm65Library::GetName(void)
{
	return mszName.Text();
}


//////////////////////////////////////////////////////////////////////////
//
// An empty object for the caller to assemble into.
//
//////////////////////////////////////////////////////////////////////////
CAsm65Object* CAsm65Library::AddObject(const char* szName)
{
	CAsm65Object*	pcObject;

	pcObject = NewMalloc<CAsm65Object>();
	pcObject->Init(szName);
	mapcObjects.Add(pcObject);
	return pcObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CAsm65Library::NumObjects(void)
{
	return mapcObjects.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CAsm65Object* CAsm65Library::GetObject(size uiIndex)
{
	return mapcObjects.GetPtr(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// "Objects/3"
//
//////////////////////////////////////////////////////////////////////////
static void ObjectPackName(CChars* psz, size uiIndex)
{
	psz->Init("Objects/");
	psz->Append((uint32)uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// Writes the library as a PackFile.  Each object is written as a ChunkFile in
// memory first, then copied into the pack.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Library::Write(CAbstractFile* pcFile)
{
	CPackFiles		cPackFiles;
	CFileBasic		cBasic;
	CPackFile*		pcPackFile;
	CMemoryFile*	pcMemoryFile;
	CChars			szName;
	size			i;
	bool			bResult;

	if (!cPackFiles.Init(pcFile, PFM_Write))
	{
		return false;
	}

	bResult = false;
	pcPackFile = cPackFiles.WriteOpen("Library");
	if (pcPackFile)
	{
		cBasic.Init(pcPackFile);
		bResult = Asm65WriteMagic(&cBasic, ASM65_LIBRARY_MAGIC, ASM65_LIBRARY_VERSION);
		bResult &= mszName.WriteString(&cBasic);
		bResult &= cBasic.WriteInt32((uint32)mapcObjects.NumElements());
		bResult &= cBasic.Close();
		cBasic.Kill();
	}

	for (i = 0; bResult && (i < mapcObjects.NumElements()); i++)
	{
		pcMemoryFile = MemoryFile();
		pcMemoryFile->mbBasicFileMustFree = false;
		bResult = GetObject(i)->Write(pcMemoryFile);
		if (bResult)
		{
			ObjectPackName(&szName, i);
			pcPackFile = cPackFiles.WriteOpen(szName.Text());
			szName.Kill();
			bResult = pcPackFile != NULL;
		}
		if (bResult)
		{
			cBasic.Init(pcPackFile);
			bResult = cBasic.WriteData(pcMemoryFile->GetBufferPointer(), pcMemoryFile->GetBufferSize());
			bResult &= cBasic.Close();
			cBasic.Kill();
		}
		SafeKill(pcMemoryFile);
	}

	cPackFiles.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Reads a library written by Write into this (empty, initialised) library.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Library::Read(CAbstractFile* pcFile)
{
	CPackFiles		cPackFiles;
	CFileBasic		cBasic;
	CPackFile*		pcPackFile;
	CAsm65Object*	pcObject;
	CChars			szName;
	CArrayUint8		cBytes;
	uint32			uiCount;
	uint32			i;
	bool			bResult;

	if ((mapcObjects.NumElements() != 0) || !cPackFiles.Init(pcFile, PFM_Read))
	{
		return false;
	}

	bResult = false;
	pcPackFile = cPackFiles.ReadOpen("Library");
	if (pcPackFile)
	{
		cBasic.Init(pcPackFile);
		bResult = Asm65ReadMagic(&cBasic, ASM65_LIBRARY_MAGIC, ASM65_LIBRARY_VERSION);
		bResult = bResult && Asm65ReadText(&cBasic, &mszName);
		bResult = bResult && cBasic.ReadInt32(&uiCount);
		cBasic.Close();
		cBasic.Kill();
	}

	for (i = 0; bResult && (i < uiCount); i++)
	{
		ObjectPackName(&szName, i);
		pcPackFile = cPackFiles.ReadOpen(szName.Text());
		szName.Kill();
		if (pcPackFile == NULL)
		{
			bResult = false;
			continue;
		}

		cBytes.Init();
		cBasic.Init(pcPackFile);
		cBytes.Resize((size)cBasic.Size());
		bResult = (cBytes.NumElements() == 0) || cBasic.ReadData(cBytes.GetData(), cBytes.NumElements());
		cBasic.Close();
		cBasic.Kill();

		pcObject = AddObject("");
		bResult = bResult && pcObject->Read(MemoryFile(cBytes.GetData(), cBytes.NumElements()));
		cBytes.Kill();
	}

	cPackFiles.Kill();
	return bResult;
}
