#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/PointerFunctions.h"
#include "StackPointers.h"


CStackPointers	gcStackPointers;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CStackPointers::Init(size iNumPointers)
{
	size				i;
	CStackPointer*	pcStackPointer;

	mpcMemory = (CStackPointer*)malloc(sizeof(CStackPointer) * iNumPointers);
	miAllocatedPointers = iNumPointers;

	miLastUsed = 0;

	for (i = 0; i < miAllocatedPointers; i++)
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
	ClearAllPointers();
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
CStackPointer* CStackPointers::Add(CCollection* pcCollection)
{
	CStackPointer* pcStackPointer;

	pcStackPointer = FindUnused();
	if (pcStackPointer)
	{
		pcStackPointer->Init(pcCollection);
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
CStackPointer* CStackPointers::Add(CCollection* pcCollection, CStackPointer* pcFirst)
{
	CStackPointer* pcStackPointer;

	pcStackPointer = FindUnused();
	if (pcStackPointer)
	{
		pcStackPointer->Init(pcCollection);
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
	size				i;
	CStackPointer*	pcPointer;

	for (i = miLastUsed; i < miAllocatedPointers; i++)
	{
		pcPointer = &mpcMemory[i];
		if (!pcPointer->IsUsed())
		{
			miLastUsed = i;
			return pcPointer;
		}
	}

	for (i = 0; i < miLastUsed; i++)
	{
		pcPointer = &mpcMemory[i];
		if (!pcPointer->IsUsed())
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
size CStackPointers::NumElements(void)
{
	size			i;
	CStackPointer*	pcStackPointer;
	size			uiCount;

	uiCount = 0;
	for (i = 0; i < miAllocatedPointers; i++)
	{
		pcStackPointer = &mpcMemory[i];
		if (pcStackPointer->IsUsed())
		{
			uiCount++;
		}
	}	

	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SStackPointer* CStackPointers::Get(size uiIndex)
{
	size			i;
	CStackPointer*	pcStackPointer;
	size			uiCount;

	uiCount = 0;
	for (i = 0; i < miAllocatedPointers; i++)
	{
		pcStackPointer = &mpcMemory[i];
		if (pcStackPointer->IsUsed())
		{
			if (uiCount == uiIndex)
			{
				return pcStackPointer->Get();
			}
			uiCount++;
		}
	}	

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CStackPointers::ClearAllPointers(void)
{
	size				i;
	CStackPointer*	pcStackPointer;

	for (i = 0; i < miAllocatedPointers; i++)
	{
		pcStackPointer = &mpcMemory[i];
		if (pcStackPointer->IsUsed())
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
CStackPointer* CStackPointers::Remove(CStackPointer* pcFirst, CCollection* pcPointer)
{
	return pcFirst->Remove(pcPointer);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CStackPointers::TestAdd(size iIndex)
{
	mpcMemory[iIndex].Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CStackPointers::TestKill(size iIndex)
{
	mpcMemory[iIndex].Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CStackPointers::TestSetLastUsed(size iLastUsed)
{
	miLastUsed = iLastUsed;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CStackPointers::TestFindUnusedIndex(size iLastUsed)
{
	CStackPointer*	pcUnused;
	size			iIndex;

	TestSetLastUsed(iLastUsed);
	pcUnused = FindUnused();
	iIndex = ((size)pcUnused - (size)mpcMemory)/sizeof(CStackPointer);

	return iIndex;
}

