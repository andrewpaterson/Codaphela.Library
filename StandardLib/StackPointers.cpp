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

	pcStackPointer->Init(pcPointer);
	return pcStackPointer;
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

	pcLast = pcFirst->FindLast();
	pcStackPointer->Init(pcPointer);
	pcLast->SetNext(pcStackPointer);
	return pcStackPointer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CStackPointer* CStackPointers::FindUnused(void)
{
	int				i;
	CStackPointer*	pcPointer;
	int				iMidEnd;

	iMidEnd = miNumPointers - miLastUsed;

	for (i = miLastUsed; i < miNumPointers; i++)
	{
		pcPointer = &mpcMemory[i];
		if (!pcPointer->mbUsed)
		{
			miLastUsed = i;
			return pcPointer;
		}
	}

	for (i = 0; i < iMidEnd; i++)
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
