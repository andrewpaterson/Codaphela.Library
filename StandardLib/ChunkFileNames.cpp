#include "ChunkFileNames.h"


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
void CChunkFileNames::Init(CAbstractFile* pcFile)
{
	CChunkFile::Init(pcFile);
	maszOpenChunkNames.Init(8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChunkFileNames::Kill(void)
{
	maszOpenChunkNames.Kill();
	CChunkFile::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChunkFileNames::FindChunkNamesMatching(CArrayString* paszOpenChunkNames, CArrayString* paszChunkNames)
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
BOOL CChunkFileNames::WriteUnmatchedChunkEnds(int iMatchingOpen)
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
BOOL CChunkFileNames::WriteNewUnmatchedChunks(int iMatchingOpen, CArrayString* paszChunkNames)
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
BOOL CChunkFileNames::WriteChunkBegin(char* szChunkName)
{
	CArrayString	aszChunkNames;
	int				iMatchingOpen;

	aszChunkNames.Init(8);
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
BOOL CChunkFileNames::WriteChunkEnd(void)
{
	WriteChunkEnd(mszLastChunkName.Text());
	mszLastChunkName.Kill();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFileNames::WriteClose(void)
{
	BOOL	bResult;

	bResult = WriteUnmatchedChunkEnds(0);
	bResult &= CChunkFile::WriteClose();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFileNames::WriteChunkBegin(void)
{
	return CChunkFile::WriteChunkBegin();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFileNames::WriteChunkEnd(char* szChunkName)
{
	return CChunkFile::WriteChunkEnd(szChunkName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFileNames::ReadChunkBegin(char* szChunkName)
{
	CArrayString	aszChunkNames;
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

	aszChunkNames.Init(8);
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
			iIndex = FindFirstChunkWithName(pszName->Text());
			abFoundFirst[iDepth] = TRUE;
		}
		else
		{
			iIndex = FindNextChunkWithName();
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
			CChunkFile::ReadChunkEnd();
		}
		else
		{
			bResult = CChunkFile::ReadChunkBegin(iIndex);
			if (iDepth == iLength-1)
			{
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
BOOL CChunkFileNames::ReadChunkEnd(void)
{
	int		i;

	if (mcChunkStack.NumElements() > 0)
	{
		for (i = 1; i < mcChunkStack.NumElements(); i++)
		{
			mcChunkStack.Get(i)->cChunkIndex.Kill();
		}
		mcChunkStack.SetUsedElements(1);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CChunkFileNames::StartNameIteration(SChunkFileNameIterator* psIter)
{
	psIter->Init();
	psIter->aiIndex.Add(0);
	return IterateName(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CChunkFileNames::IterateName(SChunkFileNameIterator* psIter)
{
	int		iNumChunks;
	CChars*	pszName;
	int*	piIndex;

	if (psIter->bDone)
	{
		return NULL;
	}

	iNumChunks = GetNumChunks();
	piIndex = psIter->aiIndex.Tail();

	if (iNumChunks == 0)
	{
		CChunkFile::ReadChunkEnd();
		psIter->aiIndex.Pop();

		piIndex = psIter->aiIndex.Tail();
		pszName = PrivateGetChunkName(*piIndex);
		psIter->szValue.Kill();
		psIter->szValue.Init(psIter->szFullName.Text(), 1);
		psIter->szFullName.RemoveFromEnd(1+pszName->Length());
		(*piIndex)++;
		return psIter->szValue.Text();
	}


	if (*piIndex >= iNumChunks)
	{
		CChunkFile::ReadChunkEnd();
		psIter->aiIndex.Pop();

		if (psIter->aiIndex.NumElements() == 0)
		{
			psIter->szFullName.Clear();
			psIter->bDone = TRUE;
			return NULL;
		}

		piIndex = psIter->aiIndex.Tail();
		pszName = PrivateGetChunkName(*piIndex);
		psIter->szFullName.RemoveFromEnd(1+pszName->Length());
		(*piIndex)++;
		return IterateName(psIter);
	}

	pszName = PrivateGetChunkName(*piIndex);
	if (pszName == NULL)
	{
		return FALSE;
	}

	psIter->szFullName.Append('/');
	psIter->szFullName.Append(pszName);

	CChunkFile::ReadChunkBegin(*piIndex);
	psIter->aiIndex.Add(0);
	return IterateName(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChunkFileNames::PrivateGetChunkName(int iIndex)
{
	int	iName;

	iName = GetChunkName(iIndex);
	if (iName == -1)
	{
		return NULL;
	}

	return mmsziNames.GetWithValue(iName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFileNames::StopIteration(SChunkFileNameIterator* psIter)
{
	psIter->Kill();

	CFileBasic::Seek(sizeof(CChunkFileHeader));
	return __PrivateReadChunkBegin();
}

