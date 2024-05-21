/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "ChunkFile.h"
#include "MD5HashFile.h"
#include "Logger.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CChunkFile::_Init(void)
{
	mcChunkStack.Init();
	mbLastHashCheck = false;
	mmsziNames._Init();
	memset(&msHeader, 0, sizeof(SChunkFileHeader));
	miLastName = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CChunkFile::Init(CAbstractFile* pcFile)
{
	CFileBasic::Init(MD5HashFile(pcFile));
	mbLastHashCheck = true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CChunkFile::Kill(void)
{
	CFileBasic::Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CChunkFile::WriteOpen(void)
{
	return WriteOpen(0);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CChunkFile::WriteOpen(chunkName iUserID)
{
	mcChunkStack.Init();
	mmsziNames.Init();
	msHeader.WriteInit(iUserID);
	miLastName = CFN_Error;

	if (!CFileBasic::Open(EFM_Write_Create))
	{
		return gcLogger.Error2(__METHOD__, " Could not create chunk file [", GetFilename(), "].", NULL);
	}

	if (!WriteBasic(&msHeader, sizeof(SChunkFileHeader)))
	{
		return gcLogger.Error2(__METHOD__, " Could not write chunk file [", GetFilename(), "] header.", NULL);
	}

	return WriteChunkBegin();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CChunkFile::WriteBasic(const void* pvSource, size uiSize)
{
	size		iFilePos;

	iFilePos = CFileBasic::Write(pvSource, uiSize, 1);
	return iFilePos == 1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CChunkFile::WriteClose(void)
{
	ReturnOnFalse(WriteChunkEnd(CFN_Chunk));
	((CMD5HashFile*)mpcFile)->ResetHash();
	ReturnOnFalse(WriteChunkNames());
	((CMD5HashFile*)mpcFile)->StopHashing();
	((CMD5HashFile*)mpcFile)->CopyDigestToDest(msHeader.macMD5Hash);
	CFileBasic::Seek(0);
	ReturnOnFalse(CFileBasic::WriteData(&msHeader, sizeof(SChunkFileHeader)));
	CFileBasic::Seek(0, EFSO_END);
	ReturnOnFalse(CFileBasic::Close());

	mcChunkStack.Kill();
	mmsziNames.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CChunkFile::ReadOpen(void)
{
	int		iResult;

	mcChunkStack.Init();
	mmsziNames.Init();
	msHeader.miChunkNamesPos = -1;
	miLastName = CFN_Error;

	ReturnOnFalse(CFileBasic::Open(EFM_Read));
	ReturnOnFalse(CFileBasic::Read(&msHeader, sizeof(SChunkFileHeader), 1));
	if (msHeader.miMagic != CHUNK_HEADER_MAGIC)
	{
		return false;
	}
	
	((CMD5HashFile*)mpcFile)->StartHashing();
	ReturnOnFalse(ReadChunkNames());
	((CMD5HashFile*)mpcFile)->StopHashing();
	iResult = memcmp(((CMD5HashFile*)mpcFile)->msMD5Context.digest, msHeader.macMD5Hash, 16);
	if (iResult != 0)
	{
		return false;
	}

	CFileBasic::Seek(sizeof(SChunkFileHeader));
	return __PrivateReadChunkBegin();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CChunkFile::ReadClose(void)
{
	size	i;
	bool	bResult;

	for (i = 0; i < mcChunkStack.NumElements(); i++)
	{
		mcChunkStack.Get(i)->cChunkIndex.Kill();
	}
	bResult = CFileBasic::Close();
	mcChunkStack.Kill();
	mmsziNames.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CChunkFile::WriteChunkBegin(void)
{
	filePos					iFilePos;
	CChunkStackElement*		psElement;

	iFilePos = CFileBasic::GetFilePos();
	psElement = mcChunkStack.Tail();

	//This will only be NULL when WriteChunkBegin is called by WriteOpen.
	if (psElement)
	{
		psElement->bContainsChunks = true;
		((CMD5HashFile*)mpcFile)->CopyContextToDest(&psElement->sMD5Context); //Save the files current MD5 hash to the chunk which contains the new chunk
	}

	psElement = mcChunkStack.Push();
	psElement->Init(iFilePos);
	if (!WriteBasic(&psElement->sHeader, sizeof(CChunkHeader)))  //This write is ignored from a hashing point of view.
	{
		return gcLogger.Error2(__METHOD__, " Could not write chunk file [", GetFilename(), "] chunk header.", NULL);
	}

	((CMD5HashFile*)mpcFile)->StartHashing(); //Reset the files current MD5 hash for the new chunk.
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CChunkFile::WriteChunkEnd(char* szChunkName)
{
	chunkName*	piName;
	chunkName	iName;

	piName = (chunkName*)mmsziNames.CMapStringTemplate::Get(szChunkName);
	if (!piName)
	{
		miLastName++;
		mmsziNames.Put(szChunkName, miLastName);
		iName = miLastName;
	}
	else
	{
		iName = *piName;
	}

	return WriteChunkEnd(iName);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CChunkFile::WriteChunkEnd(chunkName iChunkName)
{
	filePos					iFilePos;
	CChunkStackElement*		psElement;
	CChunkStackElement*		psParent;
	SChunkIndex*			psIndexParent;
	size					uiNumElements;

	iFilePos = CFileBasic::GetFilePos();
	psElement = mcChunkStack.Tail();
	uiNumElements = mcChunkStack.NumElements();
	if (uiNumElements > 1)
	{
		psParent = mcChunkStack.SafeGet(uiNumElements - 2);
	}
	else
	{
		psParent = NULL;
	}

	//How can there ever not be an element?
	if (psElement)
	{
		psElement->sHeader.miChunkSize = iFilePos - psElement->iChunkHeaderPos - sizeof(CChunkHeader);
		psElement->sHeader.miName = iChunkName;

		((CMD5HashFile*)mpcFile)->StopHashing();
		((CMD5HashFile*)mpcFile)->CopyDigestToDest(psElement->sHeader.macMD5Hash);
		if (psElement->bContainsChunks)
		{
			psElement->sHeader.miChunkIndexPos = iFilePos;
		}

		if (psParent)
		{
			psIndexParent = psParent->cChunkIndex.mcChunkIndices.Add();
			psIndexParent->Init(psElement->iChunkHeaderPos + sizeof(CChunkHeader), iChunkName, psElement->sHeader.miChunkSize);
		}

		CFileBasic::Seek(psElement->iChunkHeaderPos, EFSO_SET);
		ReturnOnFalse(WriteBasic(&psElement->sHeader, sizeof(CChunkHeader)));

		CFileBasic::Seek(0, EFSO_END);

		if (psElement->bContainsChunks)
		{
			ReturnOnFalse(psElement->cChunkIndex.mcChunkIndices.Write(this));
		}
		psElement->cChunkIndex.Kill();
		mcChunkStack.Pop();

		psElement = mcChunkStack.Tail();
		if (psElement)
		{
			((CMD5HashFile*)mpcFile)->StartHashing();
			((CMD5HashFile*)mpcFile)->CopyContextFromSource(&psElement->sMD5Context);
		}
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CChunkFile::WriteChunkNames(void)
{
	msHeader.miChunkNamesPos = CFileBasic::GetFilePos();

	return mmsziNames.Write(this);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CChunkFile::SeekStart(void)
{
	CFileBasic::Seek(sizeof(SChunkFileHeader));
	return __PrivateReadChunkBegin();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CChunkFile::__PrivateReadChunkBegin(void)
{
	CChunkStackElement*		psElement;
	bool					bResult;

	psElement = mcChunkStack.Push();
	psElement->iChunkHeaderPos = CFileBasic::GetFilePos();
	ReturnOnFalse(CFileBasic::Read(&psElement->sHeader, sizeof(CChunkHeader), 1));

	psElement->cChunkIndex.ReadInit();

	if (psElement->sHeader.miMagic != CHUNK_HEADER_MAGIC)
	{
		return false;
	}

	if (psElement->sHeader.miChunkIndexPos != -1)
	{
		psElement->bContainsChunks = true;
	}
	else
	{
		psElement->bContainsChunks = false;
	}
	if (psElement->bContainsChunks)
	{
		bResult = __PrivateReadChunkIndex(psElement->sHeader.miChunkIndexPos, &psElement->cChunkIndex);
		if (!bResult)
		{
			return false;
		}
		bResult = CFileBasic::Seek(psElement->iChunkHeaderPos + sizeof(CChunkHeader), EFSO_SET);
		if (!bResult)
		{
			return false;
		}
	}
	((CMD5HashFile*)mpcFile)->StartHashing();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CChunkFile::__PrivateReadChunkIndex(filePos iIndexPos, CChunkIndex* pcIndex)
{
	CFileBasic::Seek(iIndexPos, EFSO_SET);
	return pcIndex->mcChunkIndices.Read(this);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CChunkFile::FindFirstChunkWithID(chunkName iName)
{
	CChunkStackElement*		psElement;

	psElement = mcChunkStack.Tail();
	if (psElement)
	{
		if (psElement->bContainsChunks)
		{
			return psElement->cChunkIndex.FindFirstChunkWithName(iName);
		}
	}
	return CHUNK_FILE_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CChunkFile::FindFirstChunkWithName(char* szName)
{
	chunkName*	piName;

	piName = (chunkName*)mmsziNames.CMapStringTemplate::Get(szName);
	if (piName)
	{
		return FindFirstChunkWithID(*piName);
	}
	else
	{
		return CHUNK_FILE_NOT_FOUND;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CChunkFile::FindNextChunkWithName(void)
{
	CChunkStackElement*		psElement;

	psElement = mcChunkStack.Tail();
	if (psElement)
	{
		if (psElement->bContainsChunks)
		{
			return psElement->cChunkIndex.FindNextChunkWithName();
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CChunkFile::GetNumChunksWithID(chunkName iName)
{
	CChunkStackElement*		psElement;

	psElement = mcChunkStack.Tail();
	if (psElement)
	{
		if (psElement->bContainsChunks)
		{
			return psElement->cChunkIndex.GetNumChunksWithName(iName);
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CChunkFile::GetNumChunks(void)
{
	CChunkStackElement*		psElement;

	psElement = mcChunkStack.Tail();
	if (psElement)
	{
		if (psElement->bContainsChunks)
		{
			return psElement->cChunkIndex.mcChunkIndices.NumElements();
		}
	}
	return CFN_Error;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
chunkName CChunkFile::GetTailChunkNameIndex(size iChunkNum)
{
	SChunkIndex*			psIndex;
	CChunkStackElement*		psElement;

	psElement = mcChunkStack.Tail();
	if (psElement)
	{
		if (psElement->bContainsChunks)
		{
			psIndex = psElement->cChunkIndex.mcChunkIndices.SafeGet(iChunkNum);
			if (psIndex)
			{
				return psIndex->miName;
			}
		}
	}
	return CFN_Error;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CChunkFile::ReadChunkBegin(char* szName)
{
	chunkName				iChunkNum;
	CChunkStackElement*		psElement;
	SChunkIndex*			psIndex;

	iChunkNum = FindFirstChunkWithName(szName);
	psElement = mcChunkStack.Tail();
	if (psElement)
	{
		psIndex = psElement->cChunkIndex.mcChunkIndices.SafeGet(iChunkNum);
		if (psIndex)
		{
			CFileBasic::Seek(psIndex->miChunkDataPos - sizeof(CChunkHeader), EFSO_SET);
			return __PrivateReadChunkBegin();
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CChunkFile::ReadChunkBegin(size iChunkIndex)
{
	CChunkStackElement*		psElement;
	SChunkIndex*			psIndex;

	psElement = mcChunkStack.Tail();
	if (psElement)
	{
		psIndex = psElement->cChunkIndex.mcChunkIndices.SafeGet(iChunkIndex);
		if (psIndex)
		{
			CFileBasic::Seek(psIndex->miChunkDataPos - sizeof(CChunkHeader), EFSO_SET);
			return __PrivateReadChunkBegin();
		}
	}	
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CChunkFile::ReadChunkEnd(void)
{
	CChunkStackElement*		psElement;
	filePos					iSeekPos;
	int						iResult;

	psElement = mcChunkStack.Tail();
	if (psElement)
	{
		((CMD5HashFile*)mpcFile)->StopHashing();
		iResult = memcmp(((CMD5HashFile*)mpcFile)->msMD5Context.digest, psElement->sHeader.macMD5Hash, 16);
		mbLastHashCheck = true;
		if (iResult != 0)
		{
			mbLastHashCheck = false;
		}

		if (psElement->bContainsChunks)
		{
			iSeekPos = psElement->sHeader.miChunkIndexPos;
			iSeekPos += psElement->cChunkIndex.mcChunkIndices.ByteSize() + sizeof(size)*2 + sizeof(size)*3 + sizeof(void*);  //Howw is this caclulated?
		}
		else
		{
			iSeekPos = psElement->iChunkHeaderPos;
			iSeekPos += psElement->sHeader.miChunkSize;
		}
		CFileBasic::Seek(iSeekPos, EFSO_SET);
		psElement->cChunkIndex.Kill();
		mcChunkStack.Pop();
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CChunkFile::ReadChunkEnd(size iNumChunksToEnd)
{
	size		i;
	bool	bResult;

	bResult = true;
	for (i = 0; i < iNumChunksToEnd; i++)
	{
		bResult &= ReadChunkEnd();
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CChunkFile::ReadChunkNames(void)
{
	size*			piVal;
	chunkName		iMax;
	bool			bResult;
	SMapIterator	sIter;

	CFileBasic::Seek(msHeader.miChunkNamesPos);

	mmsziNames.Kill();
	bResult = mmsziNames.Read(this);
	if (!bResult)
	{
		return false;
	}

	iMax = CFN_Error;
	bResult = mmsziNames.StartIteration(&sIter, NULL, NULL, (void**)&piVal, NULL);
	while(bResult)
	{
		if (*piVal > iMax)
		{
			iMax = *piVal;
		}
		bResult = mmsziNames.Iterate(&sIter, NULL, NULL, (void**)&piVal, NULL);
	}
	miLastName = iMax;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CChunkFile::GetMD5Hash(void)
{
	CChunkStackElement*		psElement;

	psElement = mcChunkStack.Tail();
	if (psElement)
	{
		return psElement->sHeader.macMD5Hash;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CChunkFile::ContainsChunks(void)
{
	CChunkStackElement*		psElement;

	psElement = mcChunkStack.Tail();
	if (psElement)
	{
		return psElement->bContainsChunks;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CChunkFile::StackDepth(void)
{
	return mcChunkStack.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
filePos CChunkFile::ChunkSize(void)
{
	CChunkStackElement*		psElement;

	psElement = mcChunkStack.Tail();
	if (psElement)
	{
		return psElement->sHeader.miChunkSize;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
filePos CChunkFile::ChunkStart(void)
{
	CChunkStackElement*		psElement;

	psElement = mcChunkStack.Tail();
	if (psElement)
	{
		return psElement->iChunkHeaderPos + sizeof(CChunkHeader);
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CChunkFile::GetTailChunkNameForIndex(size iIndex)
{
	chunkName	iName;

	iName = GetTailChunkNameIndex(iIndex);
	if (iName == CHUNK_FILE_NOT_FOUND)
	{
		return NULL;
	}

	return mmsziNames.GetWithValue(iName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChunkFile::ReadChunkEndAll(void)
{
	size		i;

	if (mcChunkStack.NumElements() > 0)
	{
		for (i = 1; i < mcChunkStack.NumElements(); i++)
		{
			mcChunkStack.Get(i)->cChunkIndex.Kill();
		}
		mcChunkStack.SetUsedElements(1);
		return true;
	}
	else
	{
		return false;
	}
}

