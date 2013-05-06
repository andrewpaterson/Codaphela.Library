#include "NamedIndexedBlock.h"
#include "NamedIndexedSorterSource.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamesIndexedSorterSource::Init(int iWidth, filePos iPosition)
{
	mpcCurrent = (CNamedIndexedBlock*)malloc(iWidth);
	memset_fast(mpcCurrent, 0, iWidth);

	miPosition = iPosition;
	miCount = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamesIndexedSorterSource::Kill(void)
{
	free(mpcCurrent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamesIndexedSorterSource::ReadNext(CFileBasic* pcFile, int iWidth)
{
	BOOL	bResult;

	bResult = pcFile->Seek(miPosition);
	if (!bResult)
	{
		return FALSE;
	}

	bResult = pcFile->ReadData(mpcCurrent, iWidth);
	miPosition += iWidth;

	if (bResult)
	{
		miCount++;
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
BOOL CNamesIndexedSorterSource::IsSmallerThan(CNamesIndexedSorterSource* pcOther)
{
	int		iResult;

	if (mpcCurrent->IsEmpty())
	{
		return FALSE;
	}
	else
	{
		if (pcOther == NULL)
		{
			return TRUE;
		}

		iResult = StrICmp(mpcCurrent->Name(), pcOther->mpcCurrent->Name());
		if (iResult < 0)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamesIndexedSorterSource::HasRemaining(int iBlockChunkSize)
{
	return miCount <= iBlockChunkSize;
}

