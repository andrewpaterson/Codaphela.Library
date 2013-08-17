#include "BaseLib/PointerFunctions.h"
#include "StackPointers.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CStackPointers::Init(int iNumPointers)
{
	int				i;
	CStackPointer*	pcStackPointer;

	mpcMemory = (CStackPointer*)malloc(sizeof(CStackPointer) * iNumPointers);
	miNumPointers = iNumPointers;

	miLastUsed = 0;

	for (i = 0; i < miNumPointers; i++)
	{
		pcStackPointer = &mpcMemory[i];
		pcStackPointer->Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CStackPointers::Kill(void)
{
	SafeFree(mpcMemory);
	mpcMemory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CStackPointer* CStackPointers::Add(CPointer* pcPointer)
{
	CStackPointer*	pcStackPointer;

	pcStackPointer = FindUnused();
	if (pcStackPointer)
	{
		pcStackPointer->Init(pcPointer);
		return pcStackPointer;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CStackPointer* CStackPointers::Add(CPointer* pcPointer, CStackPointer* pcFirst)
{
	CStackPointer*	pcStackPointer;
	CStackPointer*	pcLast;

	pcStackPointer = FindUnused();
	if (pcStackPointer)
	{
		pcLast = pcFirst->FindLast();
		pcStackPointer->Init(pcPointer);
		pcLast->SetNext(pcStackPointer);
		return pcStackPointer;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CStackPointer* CStackPointers::FindUnused(void)
{
	int				i;
	CStackPointer*	pcPointer;

	for (i = miLastUsed; i < miNumPointers; i++)
	{
		pcPointer = &mpcMemory[i];
		if (!pcPointer->mbUsed)
		{
			miLastUsed = i;
			return pcPointer;
		}
	}

	for (i = 0; i < miLastUsed; i++)
	{
		pcPointer = &mpcMemory[i];
		if (!pcPointer->mbUsed)
		{
			miLastUsed = i;
			return pcPointer;
		}
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CStackPointers::UsedPointers(void)
{
	int				i;
	CStackPointer*	pcStackPointer;
	int				iCount;

	iCount = 0;
	for (i = 0; i < miNumPointers; i++)
	{
		pcStackPointer = &mpcMemory[i];
		if (pcStackPointer->mbUsed)
		{
			iCount++;
		}
	}	

	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CStackPointers::Remove(CStackPointer* pcFirst)
{
	pcFirst->Remove();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CStackPointers::TestAdd(int iIndex)
{
	mpcMemory[iIndex].Init(NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CStackPointers::TestKill(int iIndex)
{
	mpcMemory[iIndex].Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CStackPointers::TestSetLastUsed(int iLastUsed)
{
	miLastUsed = iLastUsed;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CStackPointers::TestFindUnusedIndex(int iLastUsed)
{
	CStackPointer*	pcUnused;
	size_t			iIndex;

	TestSetLastUsed(iLastUsed);
	pcUnused = FindUnused();
	iIndex = ((size_t)pcUnused - (size_t)mpcMemory)/sizeof(CStackPointer);

	return iIndex;
}

