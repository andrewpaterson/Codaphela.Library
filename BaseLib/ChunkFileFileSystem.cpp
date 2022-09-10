#include "ChunkFileFileSystem.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SChunkFilenameIterator::Init(void)
{
	szFullName.Init();
	aiIndex.Init();
	bDone = false;
	szValue.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SChunkFilenameIterator::Kill(void)
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
bool CChunkFileFileSystem::WriteUnmatchedChunkEnds(int iMatchingOpen)
{
	int			i;
	CChars*		pszOpenName;

	for (i = maszOpenChunkNames.NumElements()-1; i >= iMatchingOpen; i--)
	{
		pszOpenName = maszOpenChunkNames.Get(i);
		ReturnOnFalse(WriteChunkEnd(pszOpenName->Text()));
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChunkFileFileSystem::WriteNewUnmatchedChunks(int iMatchingOpen, CArrayChars* paszChunkNames)
{
	int			i;
	CChars*		pszOpenName;

	for (i = iMatchingOpen; i < paszChunkNames->NumElements(); i++)
	{
		pszOpenName = paszChunkNames->Get(i);
		ReturnOnFalse(WriteChunkBegin());
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChunkFileFileSystem::WriteChunkBegin(char* szChunkName)
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
bool CChunkFileFileSystem::WriteChunkEnd(void)
{
	WriteChunkEnd(mszLastChunkName.Text());
	mszLastChunkName.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChunkFileFileSystem::ReadOpen(void)
{
	return mpcChunkFile->ReadOpen();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChunkFileFileSystem::ReadClose(void)
{
	return mpcChunkFile->ReadClose();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChunkFileFileSystem::WriteOpen(void)
{
	return mpcChunkFile->WriteOpen();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChunkFileFileSystem::WriteOpen(int iUserID)
{
	return mpcChunkFile->WriteOpen(iUserID);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChunkFileFileSystem::WriteClose(void)
{
	bool	bResult;

	bResult = WriteUnmatchedChunkEnds(0);
	bResult &= mpcChunkFile->WriteClose();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChunkFileFileSystem::WriteChunkBegin(void)
{
	return mpcChunkFile->WriteChunkBegin();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChunkFileFileSystem::WriteChunkEnd(char* szChunkName)
{
	return mpcChunkFile->WriteChunkEnd(szChunkName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChunkFileFileSystem::ReadChunkBegin(char* szChunkName)
{
	CArrayChars	aszChunkNames;
	int				iDepth;
	CChars*			pszName;
	int				iIndex;
	bool*			abFoundFirst;
	int				iLength;
	bool			bResult;

	if (!szChunkName)
	{
		return false;
	}

	aszChunkNames.Init();
	aszChunkNames.Split(szChunkName, '/');

	iLength = aszChunkNames.NumElements();
	if (iLength == 0)
	{
		aszChunkNames.Kill();
		return false;
	}

	abFoundFirst = (bool*)malloc(sizeof(bool) * iLength);
	memset_fast(abFoundFirst, 0, sizeof(bool) * iLength);

	for (iDepth = 0;;)
	{
		pszName = aszChunkNames.Get(iDepth);
		if (!abFoundFirst[iDepth])
		{
			iIndex = mpcChunkFile->FindFirstChunkWithName(pszName->Text());
			abFoundFirst[iDepth] = true;
		}
		else
		{
			iIndex = mpcChunkFile->FindNextChunkWithName();
		}

		if (iIndex == -1)
		{
			abFoundFirst[iDepth] = false;
			iDepth--;

			if (iDepth == -1)
			{
				ReadChunkEnd();
				aszChunkNames.Kill();
				free(abFoundFirst);
				return false;
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
				return true;
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
bool CChunkFileFileSystem::ReadChunkEnd(void)
{
	return mpcChunkFile->ReadChunkEndAll();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CChunkFileFileSystem::StartNameIteration(SChunkFilenameIterator* psIter)
{
	psIter->Init();
	psIter->aiIndex.Add(0);
	return IterateName(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CChunkFileFileSystem::IterateName(SChunkFilenameIterator* psIter)
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
			psIter->bDone = true;
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
		return false;
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
bool CChunkFileFileSystem::StopIteration(SChunkFilenameIterator* psIter)
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

