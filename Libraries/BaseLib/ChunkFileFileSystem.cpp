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
size CChunkFileFileSystem::FindChunkNamesMatching(CArrayChars* paszOpenChunkNames, CArrayChars* paszChunkNames)
{
	size			i;
	CChars*			pszNewName;
	CChars*			pszOpenName;
	size			uiChunkNames;
	size			uiOpenChunkNames;

	uiChunkNames = paszChunkNames->NumElements();
	uiOpenChunkNames = paszOpenChunkNames->NumElements();

	for (i = 0; (i < uiChunkNames) && (i < uiOpenChunkNames); i++)
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
bool CChunkFileFileSystem::WriteUnmatchedChunkEnds(size iMatchingOpen)
{
	size		i;
	CChars*		pszOpenName;

	i = maszOpenChunkNames.NumElements();
	if ((i != 0) && (i > iMatchingOpen))
	{
		do
		{
			i--;
			pszOpenName = maszOpenChunkNames.Get(i);
			ReturnOnFalse(WriteChunkEnd(pszOpenName->Text()));
		}
		while ((i != 0) && (i > iMatchingOpen));
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChunkFileFileSystem::WriteNewUnmatchedChunks(size iMatchingOpen, CArrayChars* paszChunkNames)
{
	size		i;
	size		uiChunkNames;

	uiChunkNames = paszChunkNames->NumElements();
	for (i = iMatchingOpen; i < uiChunkNames; i++)
	{
		ReturnOnFalse(WriteChunkBegin());
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChunkFileFileSystem::WriteChunkBegin(const char* szChunkName)
{
	CArrayChars		aszChunkNames;
	size			iMatchingOpen;

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
	maszOpenChunkNames.ReInit();
	return mpcChunkFile->ReadOpen();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChunkFileFileSystem::ReadClose(void)
{
	maszOpenChunkNames.ReInit();
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
bool CChunkFileFileSystem::WriteOpen(uint32 iUserID)
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
bool CChunkFileFileSystem::WriteChunkEnd(const char* szChunkName)
{
	return mpcChunkFile->WriteChunkEnd(szChunkName);
}


//////////////////////////////////////////////////////////////////////////
//
// Opens the chunk at the path szChunkName ("Container/Chunk").  The chunks
// containing the last one read are still open: those on the way to this
// one are kept and searched first, so reading many chunks from one
// container reads its index once.  Chunks can share a name (a container
// written more than once), so if the chunk isn't under them every path is
// searched from the start.
//
//////////////////////////////////////////////////////////////////////////
bool CChunkFileFileSystem::ReadChunkBegin(const char* szChunkName)
{
	CArrayChars	aszChunkNames;
	size		uiKeep;
	bool		bFound;

	if (StrEmpty(szChunkName))
	{
		return false;
	}

	aszChunkNames.Init();
	aszChunkNames.Split(szChunkName, '/');
	if (aszChunkNames.NumElements() == 0)
	{
		aszChunkNames.Kill();
		return false;
	}

	uiKeep = NumOpenOnPath(&aszChunkNames);
	ReadOpenChunksEnd(uiKeep);
	bFound = ReadChunkBeginFrom(&aszChunkNames, uiKeep);
	if (!bFound && (uiKeep > 0))
	{
		ReadOpenChunksEnd(0);
		bFound = ReadChunkBeginFrom(&aszChunkNames, 0);
	}

	if (bFound)
	{
		aszChunkNames.RemoveTail();
		maszOpenChunkNames.ReInit();
		maszOpenChunkNames.Copy(&aszChunkNames);
	}
	aszChunkNames.Kill();
	return bFound;
}


//////////////////////////////////////////////////////////////////////////
//
// How many of the open chunks are the first containers on the path.
//
//////////////////////////////////////////////////////////////////////////
size CChunkFileFileSystem::NumOpenOnPath(CArrayChars* paszChunkNames)
{
	size	i;
	size	uiContainers;
	size	uiOpen;

	uiContainers = paszChunkNames->NumElements() - 1;
	uiOpen = maszOpenChunkNames.NumElements();
	for (i = 0; (i < uiContainers) && (i < uiOpen); i++)
	{
		if (!paszChunkNames->Get(i)->Equals(maszOpenChunkNames.Get(i)))
		{
			break;
		}
	}
	return i;
}


//////////////////////////////////////////////////////////////////////////
//
// Ends every chunk open for reading but the first uiKeep containers (and
// the file's own).
//
//////////////////////////////////////////////////////////////////////////
void CChunkFileFileSystem::ReadOpenChunksEnd(size uiKeep)
{
	while (mpcChunkFile->StackDepth() > uiKeep + 1)
	{
		mpcChunkFile->ReadChunkEnd();
	}
	while (maszOpenChunkNames.NumElements() > uiKeep)
	{
		maszOpenChunkNames.RemoveTail();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Searches for the chunk at the path below the uiDepth chunks open on it,
// trying every chunk with each name.  If it isn't found only those chunks
// are open.
//
//////////////////////////////////////////////////////////////////////////
bool CChunkFileFileSystem::ReadChunkBeginFrom(CArrayChars* paszChunkNames, size uiDepth)
{
	size		uiFrom;
	size		uiLength;
	size		uiIndex;
	CChars*		pszName;
	bool*		abFoundFirst;

	uiFrom = uiDepth;
	uiLength = paszChunkNames->NumElements();
	abFoundFirst = (bool*)malloc((lsize)sizeof(bool) * uiLength);
	memset(abFoundFirst, 0, (lsize)sizeof(bool) * uiLength);

	for (;;)
	{
		pszName = paszChunkNames->Get(uiDepth);
		if (!abFoundFirst[uiDepth])
		{
			uiIndex = mpcChunkFile->FindFirstChunkWithName(pszName->Text());
			abFoundFirst[uiDepth] = true;
		}
		else
		{
			uiIndex = mpcChunkFile->FindNextChunkWithName();
		}

		if (uiIndex == ARRAY_ELEMENT_NOT_FOUND)
		{
			abFoundFirst[uiDepth] = false;
			if (uiDepth == uiFrom)
			{
				free(abFoundFirst);
				return false;
			}
			uiDepth--;
			mpcChunkFile->ReadChunkEnd();
		}
		else
		{
			mpcChunkFile->ReadChunkBegin(uiIndex);
			if (uiDepth == uiLength - 1)
			{
				free(abFoundFirst);
				return true;
			}
			uiDepth++;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Ends the chunk ReadChunkBegin opened.  The chunks containing it are left
// open for the next.
//
//////////////////////////////////////////////////////////////////////////
bool CChunkFileFileSystem::ReadChunkEnd(void)
{
	if (mpcChunkFile->StackDepth() <= maszOpenChunkNames.NumElements() + 1)
	{
		return false;
	}
	return mpcChunkFile->ReadChunkEnd();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CChunkFileFileSystem::StartNameIteration(SChunkFilenameIterator* psIter)
{
	ReadOpenChunksEnd(0);
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
	size		iNumChunks;
	char*		szName;
	uint*		piIndex;
	size		iLength;

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
		iLength = StrLen(szName);
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
		iLength = StrLen(szName);
		psIter->szFullName.RemoveFromEnd(1 + iLength);
		(*piIndex)++;
		return IterateName(psIter);
	}

	szName = mpcChunkFile->GetTailChunkNameForIndex(*piIndex);
	if (szName == NULL)
	{
		return NULL;
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
lsize CChunkFileFileSystem::Write(const void* pvSource, lsize iSize, lsize iCount)
{
	return mpcChunkFile->Write(pvSource, iSize, iCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CChunkFileFileSystem::Read(void* pvDest, lsize iSize, lsize iCount)
{
	return mpcChunkFile->Read(pvDest, iSize, iCount);
}

