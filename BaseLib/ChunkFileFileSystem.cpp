#include "ChunkFileFileSystem.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SChunkFileNameIterator::Init(void)
{
	szFullName.Init();
	aiIndex.Init();
	bDone = FALSE;
	szValue.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SChunkFileNameIterator::Kill(void)
{
	szValue.Kill();
	aiIndex.Kill();
	szFullName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChunkFileFileSystem::_Init(void)
{
	maszOpenChunkNames._Init();
	mszLastChunkName._Init();
	mpcChunkFile = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChunkFileFileSystem::Init(CChunkFile* pcChunkFile)
{
	mpcChunkFile = pcChunkFile;
	maszOpenChunkNames.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChunkFileFileSystem::Kill(void)
{
	maszOpenChunkNames.Kill();
	mpcChunkFile = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChunkFileFileSystem::FindChunkNamesMatching(CArrayChars* paszOpenChunkNames, CArrayChars* paszChunkNames)
{
	int				i;
	CChars*			pszNewName;
	CChars*			pszOpenName;

	for (i = 0; i < paszChunkNames->NumElements() && i < paszOpenChunkNames->NumElements(); i++)
	{
		pszNewName = paszChunkNames->Get(i);
		pszOpenName = paszOpenChunkNames->Get(i);
		if (!pszNewName->EqualsIgnoreCase(pszOpenName->Text()))
		{
			break;
		}
	}
	return i;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFileFileSystem::WriteUnmatchedChunkEnds(int iMatchingOpen)
{
	int			i;
	CChars*		pszOpenName;

	for (i = maszOpenChunkNames.NumElements()-1; i >= iMatchingOpen; i--)
	{
		pszOpenName = maszOpenChunkNames.Get(i);
		ReturnOnFalse(WriteChunkEnd(pszOpenName->Text()));
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFileFileSystem::WriteNewUnmatchedChunks(int iMatchingOpen, CArrayChars* paszChunkNames)
{
	int			i;
	CChars*		pszOpenName;

	for (i = iMatchingOpen; i < paszChunkNames->NumElements(); i++)
	{
		pszOpenName = paszChunkNames->Get(i);
		ReturnOnFalse(WriteChunkBegin());
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFileFileSystem::WriteChunkBegin(char* szChunkName)
{
	CArrayChars	aszChunkNames;
	int				iMatchingOpen;

	aszChunkNames.Init();
	aszChunkNames.Split(szChunkName, '/');
	mszLastChunkName.Init(aszChunkNames.Tail()->Text());
	aszChunkNames.RemoveTail();

	iMatchingOpen = FindChunkNamesMatching(&maszOpenChunkNames, &aszChunkNames);

	ReturnOnFalse(WriteUnmatchedChunkEnds(iMatchingOpen));

	ReturnOnFalse(WriteNewUnmatchedChunks(iMatchingOpen, &aszChunkNames));

	maszOpenChunkNames.ReInit();
	maszOpenChunkNames.Copy(&aszChunkNames);
	aszChunkNames.Kill();

	return WriteChunkBegin();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFileFileSystem::WriteChunkEnd(void)
{
	WriteChunkEnd(mszLastChunkName.Text());
	mszLastChunkName.Kill();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFileFileSystem::ReadOpen(void)
{
	return mpcChunkFile->ReadOpen();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFileFileSystem::ReadClose(void)
{
	return mpcChunkFile->ReadClose();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFileFileSystem::WriteOpen(void)
{
	return mpcChunkFile->WriteOpen();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFileFileSystem::WriteOpen(int iUserID)
{
	return mpcChunkFile->WriteOpen(iUserID);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFileFileSystem::WriteClose(void)
{
	BOOL	bResult;

	bResult = WriteUnmatchedChunkEnds(0);
	bResult &= mpcChunkFile->WriteClose();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFileFileSystem::WriteChunkBegin(void)
{
	return mpcChunkFile->WriteChunkBegin();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFileFileSystem::WriteChunkEnd(char* szChunkName)
{
	return mpcChunkFile->WriteChunkEnd(szChunkName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFileFileSystem::ReadChunkBegin(char* szChunkName)
{
	CArrayChars	aszChunkNames;
	int				iDepth;
	CChars*			pszName;
	int				iIndex;
	BOOL*			abFoundFirst;
	int				iLength;
	BOOL			bResult;

	if (!szChunkName)
	{
		return FALSE;
	}

	aszChunkNames.Init();
	aszChunkNames.Split(szChunkName, '/');

	iLength = aszChunkNames.NumElements();
	if (iLength == 0)
	{
		aszChunkNames.Kill();
		return FALSE;
	}

	abFoundFirst = (BOOL*)malloc(sizeof(BOOL) * iLength);
	memset_fast(abFoundFirst, 0, sizeof(BOOL) * iLength);

	for (iDepth = 0;;)
	{
		pszName = aszChunkNames.Get(iDepth);
		if (!abFoundFirst[iDepth])
		{
			iIndex = mpcChunkFile->FindFirstChunkWithName(pszName->Text());
			abFoundFirst[iDepth] = TRUE;
		}
		else
		{
			iIndex = mpcChunkFile->FindNextChunkWithName();
		}

		if (iIndex == -1)
		{
			abFoundFirst[iDepth] = FALSE;
			iDepth--;

			if (iDepth == -1)
			{
				ReadChunkEnd();
				aszChunkNames.Kill();
				free(abFoundFirst);
				return FALSE;
			}
			mpcChunkFile->ReadChunkEnd();
		}
		else
		{
			bResult = mpcChunkFile->ReadChunkBegin(iIndex);
			if (iDepth == iLength-1)
			{
				aszChunkNames.Kill();
				free(abFoundFirst);
				return TRUE;
			}
			iDepth++;
		}
	}

	aszChunkNames.Kill();
	free(abFoundFirst);

	return WriteChunkBegin();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFileFileSystem::ReadChunkEnd(void)
{
	return mpcChunkFile->ReadChunkEndAll();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CChunkFileFileSystem::StartNameIteration(SChunkFileNameIterator* psIter)
{
	psIter->Init();
	psIter->aiIndex.Add(0);
	return IterateName(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CChunkFileFileSystem::IterateName(SChunkFileNameIterator* psIter)
{
	int		iNumChunks;
	char*	szName;
	int*	piIndex;
	int		iLength;

	if (psIter->bDone)
	{
		return NULL;
	}

	iNumChunks = mpcChunkFile->GetNumChunks();
	piIndex = psIter->aiIndex.Tail();

	if (iNumChunks == 0)
	{
		mpcChunkFile->ReadChunkEnd();
		psIter->aiIndex.Pop();

		piIndex = psIter->aiIndex.Tail();
		szName = mpcChunkFile->GetTailChunkNameForIndex(*piIndex);
		iLength = strlen(szName);
		psIter->szValue.Kill();
		psIter->szValue.Init(psIter->szFullName.Text(), 1);
		psIter->szFullName.RemoveFromEnd(1 + iLength);
		(*piIndex)++;
		return psIter->szValue.Text();
	}


	if (*piIndex >= iNumChunks)
	{
		mpcChunkFile->ReadChunkEnd();
		psIter->aiIndex.Pop();

		if (psIter->aiIndex.NumElements() == 0)
		{
			psIter->szFullName.Clear();
			psIter->bDone = TRUE;
			return NULL;
		}

		piIndex = psIter->aiIndex.Tail();
		szName = mpcChunkFile->GetTailChunkNameForIndex(*piIndex);
		iLength = strlen(szName);
		psIter->szFullName.RemoveFromEnd(1 + iLength);
		(*piIndex)++;
		return IterateName(psIter);
	}

	szName = mpcChunkFile->GetTailChunkNameForIndex(*piIndex);
	if (szName == NULL)
	{
		return FALSE;
	}

	psIter->szFullName.Append('/');
	psIter->szFullName.Append(szName);

	mpcChunkFile->ReadChunkBegin(*piIndex);
	psIter->aiIndex.Add(0);
	return IterateName(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFileFileSystem::StopIteration(SChunkFileNameIterator* psIter)
{
	psIter->Kill();

	return mpcChunkFile->SeekStart();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChunkFile* CChunkFileFileSystem::GetChunkFile(void)
{
	return mpcChunkFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CChunkFileFileSystem::Write(const void* pvSource, filePos iSize, filePos iCount)
{
	return mpcChunkFile->Write(pvSource, iSize, iCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CChunkFileFileSystem::Read(void* pvDest, filePos iSize, filePos iCount)
{
	return mpcChunkFile->Read(pvDest, iSize, iCount);
}

