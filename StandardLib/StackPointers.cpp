#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/PointerFunctions.h"
#include "StackPointers.h"


CStackPointers	gcStackPointers;


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

	pcStackPointer = FindUnused();
	if (pcStackPointer)
	{
		pcStackPointer->Init(pcPointer);
		Add(pcStackPointer, pcFirst);
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
void CStackPointers::Add(CStackPointer* pcStackPointer, CStackPointer* pcFirst)
{
	CStackPointer*	pcLast;

	pcLast = pcFirst->FindLast();
	pcLast->SetNext(pcStackPointer);
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
void CStackPointers::ClearAllPointers(void)
{
	int				i;
	CStackPointer*	pcStackPointer;

	for (i = 0; i < miNumPointers; i++)
	{
		pcStackPointer = &mpcMemory[i];
		if (pcStackPointer->mbUsed)
		{
			pcStackPointer->ClearPointer();
		}
	}	
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CStackPointers::RemoveAll(CStackPointer* pcFirst)
{
	pcFirst->RemoveAll();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CStackPointer* CStackPointers::Remove(CStackPointer* pcFirst, CPointer* pcPointer)
{
	return pcFirst->Remove(pcPointer);
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
	size			iIndex;

	TestSetLastUsed(iLastUsed);
	pcUnused = FindUnused();
	iIndex = ((size)pcUnused - (size)mpcMemory)/sizeof(CStackPointer);

	return iIndex;
}

