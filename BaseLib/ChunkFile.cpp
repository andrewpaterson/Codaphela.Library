/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CChunkFile::Init(CAbstractFile* pcFile)
{
	CFileBasic::Init(MD5HashFile(pcFile));
	mbLastHashCheck = TRUE;
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
BOOL CChunkFile::WriteOpen(void)
{
	return WriteOpen(0);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFile::WriteOpen(int iUserID)
{
	mcChunkStack.Init();
	mmsziNames.Init(8, TRUE);
	msHeader.WriteInit(iUserID);
	miLastName = CFN_Error;

	ReturnOnFalse(CFileBasic::Open(EFM_Write_Create));
	ReturnOnFalse(WriteBasic(&msHeader, sizeof(CChunkFileHeader)));

	return WriteChunkBegin();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFile::WriteBasic(const void* pvSource, filePos iSize)
{
	filePos		iFilePos;

	iFilePos = CFileBasic::Write(pvSource, iSize, 1);
	return iFilePos == 1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFile::WriteClose(void)
{
	ReturnOnFalse(WriteChunkEnd(CFN_Chunk));
	((CMD5HashFile*)mpcFile)->ResetHash();
	ReturnOnFalse(WriteChunkNames());
	((CMD5HashFile*)mpcFile)->StopHashing();
	((CMD5HashFile*)mpcFile)->CopyDigestToDest(msHeader.macMD5Hash);
	CFileBasic::Seek(0);
	ReturnOnFalse(CFileBasic::WriteData(&msHeader, sizeof(CChunkFileHeader)));
	CFileBasic::Seek(0, EFSO_END);
	ReturnOnFalse(CFileBasic::Close());

	mcChunkStack.Kill();
	mmsziNames.Kill();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFile::ReadOpen(void)
{
	int		iResult;

	mcChunkStack.Init();
	mmsziNames.Init(8);
	msHeader.miChunkNamesPos = -1;
	miLastName = CFN_Error;

	ReturnOnFalse(CFileBasic::Open(EFM_Read));
	ReturnOnFalse(CFileBasic::Read(&msHeader, sizeof(CChunkFileHeader), 1));
	if (msHeader.miMagic != CHUNK_HEADER_MAGIC)
	{
		return FALSE;
	}
	
	((CMD5HashFile*)mpcFile)->StartHashing();
	ReturnOnFalse(ReadChunkNames());
	((CMD5HashFile*)mpcFile)->StopHashing();
	iResult = memcmp(((CMD5HashFile*)mpcFile)->msMD5Context.digest, msHeader.macMD5Hash, 16);
	if (iResult != 0)
	{
		return FALSE;
	}

	CFileBasic::Seek(sizeof(CChunkFileHeader));
	return __PrivateReadChunkBegin();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFile::ReadClose(void)
{
	int					i;
	BOOL				bResult;

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
BOOL CChunkFile::WriteChunkBegin(void)
{
	filePos					iFilePos;
	CChunkStackElement*		psElement;

	iFilePos = CFileBasic::GetFilePos();
	psElement = mcChunkStack.Tail();

	//This will only be NULL when WriteChunkBegin is called by WriteOpen.
	if (psElement)
	{
		psElement->bContainsChunks = 1;
		((CMD5HashFile*)mpcFile)->CopyContextToDest(&psElement->sMD5Context); //Save the files current MD5 hash to the chunk which contains the new chunk
	}

	psElement = mcChunkStack.Push();
	psElement->Init(iFilePos);
	ReturnOnFalse(WriteBasic(&psElement->sHeader, sizeof(CChunkHeader)));  //This write is ignored from a hashing point of view.

	((CMD5HashFile*)mpcFile)->StartHashing(); //Reset the files current MD5 hash for the new chunk.
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFile::WriteChunkEnd(char* szChunkName)
{
	int*	piName;
	int		iName;

	piName = mmsziNames.Get(szChunkName);
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
BOOL CChunkFile::WriteChunkEnd(int iChunkName)
{
	filePos					iFilePos;
	CChunkStackElement*		psElement;
	CChunkStackElement*		psParent;
	SChunkIndex*			psIndexParent;

	iFilePos = CFileBasic::GetFilePos();
	psElement = mcChunkStack.Tail();
	psParent = mcChunkStack.SafeGet(mcChunkStack.NumElements()-2);

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
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFile::WriteChunkNames(void)
{
	msHeader.miChunkNamesPos = CFileBasic::GetFilePos();

	return mmsziNames.Write(this);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFile::__PrivateReadChunkBegin(void)
{
	CChunkStackElement*		psElement;

	psElement = mcChunkStack.Push();
	psElement->iChunkHeaderPos = CFileBasic::GetFilePos();
	ReturnOnFalse(CFileBasic::Read(&psElement->sHeader, sizeof(CChunkHeader), 1));

	psElement->cChunkIndex.ReadInit();

	if (psElement->sHeader.miMagic != CHUNK_HEADER_MAGIC)
	{
		return FALSE;
	}

	if (psElement->sHeader.miChunkIndexPos != -1)
	{
		psElement->bContainsChunks = 1;
	}
	else
	{
		psElement->bContainsChunks = 0;
	}
	if (psElement->bContainsChunks)
	{
		__PrivateReadChunkIndex(psElement->sHeader.miChunkIndexPos, &psElement->cChunkIndex);
		CFileBasic::Seek(psElement->iChunkHeaderPos + sizeof(CChunkHeader), EFSO_SET);
	}
	((CMD5HashFile*)mpcFile)->StartHashing();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFile::__PrivateReadChunkIndex(filePos iIndexPos, CChunkIndex* pcIndex)
{
	CFileBasic::Seek(iIndexPos, EFSO_SET);
	return pcIndex->mcChunkIndices.Read(this);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CChunkFile::FindFirstChunkWithID(int iName)
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
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CChunkFile::FindFirstChunkWithName(char* szName)
{
	int*	piName;

	piName = mmsziNames.Get(szName);
	if (piName)
	{
		return FindFirstChunkWithID(*piName);
	}
	else
	{
		return -1;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CChunkFile::FindNextChunkWithName(void)
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
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CChunkFile::GetNumChunksWithID(int iName)
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
int CChunkFile::GetNumChunks(void)
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
int CChunkFile::GetChunkName(int iChunkNum)
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
BOOL CChunkFile::ReadChunkBegin(int iChunkNum)
{
	CChunkStackElement*		psElement;
	SChunkIndex*			psIndex;

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
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFile::ReadChunkEnd(void)
{
	CChunkStackElement*		psElement;
	filePos					iSeekPos;
	int						iResult;

	psElement = mcChunkStack.Tail();
	if (psElement)
	{
		((CMD5HashFile*)mpcFile)->StopHashing();
		iResult = memcmp(((CMD5HashFile*)mpcFile)->msMD5Context.digest, psElement->sHeader.macMD5Hash, 16);
		mbLastHashCheck = TRUE;
		if (iResult != 0)
		{
			mbLastHashCheck = FALSE;
		}

		if (psElement->bContainsChunks)
		{
			iSeekPos = psElement->sHeader.miChunkIndexPos;
			iSeekPos += psElement->cChunkIndex.mcChunkIndices.ByteSize() + sizeof(int)*2 + sizeof(int)*3 + sizeof(void*);
		}
		else
		{
			iSeekPos = psElement->iChunkHeaderPos;
			iSeekPos += psElement->sHeader.miChunkSize;
		}
		CFileBasic::Seek(iSeekPos, EFSO_SET);
		psElement->cChunkIndex.Kill();
		mcChunkStack.Pop();
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFile::ReadChunkEnd(int iNumChunksToEnd)
{
	int		i;
	BOOL	bResult;

	bResult = TRUE;
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
BOOL CChunkFile::ReadChunkNames(void)
{
	int*			piVal;
	int				iMax;
	BOOL			bResult;
	SMapIterator	sIter;

	CFileBasic::Seek(msHeader.miChunkNamesPos);

	bResult = mmsziNames.Read(this);
	if (!bResult)
	{
		return FALSE;
	}

	iMax = CFN_Error;
	bResult = mmsziNames.StartIteration(&sIter, NULL, (void**)&piVal);
	while(bResult)
	{
		if (*piVal > iMax)
		{
			iMax = *piVal;
		}
		bResult = mmsziNames.Iterate(&sIter, NULL, (void**)&piVal);
	}
	miLastName = iMax;
	return TRUE;
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
BOOL CChunkFile::ContainsChunks(void)
{
	CChunkStackElement*		psElement;

	psElement = mcChunkStack.Tail();
	if (psElement)
	{
		return psElement->bContainsChunks;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CChunkFile::StackDepth(void)
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

