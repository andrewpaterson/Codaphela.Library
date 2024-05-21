/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/ArrayVoidPtr.h"
#include "BaseLib/MemoryAllocator.h"
#include "BaseLib/GlobalMemory.h"
#include "Unknown.h"
#include "Unknowns.h"


CUnknowns gcUnknowns;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CUnknowns::Init(char* szName, CConstructors* pcConstructors)
{
	CLifeInit<CMallocator> cLifeMalloc =  LifeAlloc<CMemoryAllocator, CMallocator>();
	Init(cLifeMalloc, szName, pcConstructors);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CUnknowns::Init(CLifeInit<CMallocator> cMalloc, char* szName, CConstructors* pcConstructors)
{
	mpcConstructors = pcConstructors;
	mpcConstructors->ValidateMemoryInitialised();

	cMalloc.ConfigureLife(&mcMallocLife, &mpcMalloc);

	mcIterables.Init();
	mszName.Init(szName);
	miNumElements = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CUnknowns::Kill(void)
{
	mszName.Kill();
	mcIterables.Kill();
	mcMallocLife.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CUnknown* CUnknowns::AddExisting(CUnknown* pcExisting, char(*pszDebug)[4])
{
	DebugName(pcExisting, pszDebug);

	pcExisting->SetUnknowns(this);
	miNumElements++;

	if (pcExisting->Iterable())
	{
		mcIterables.Add(pcExisting);
	}
	return pcExisting;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CUnknown* CUnknowns::Add(const char* szClassName)
{
	return Add(szClassName, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CUnknown* CUnknowns::Add(const char* szClassName, size uiAdditionalSize)
{
	CUnknown* pcUnknown;
	char(*acDebug)[4];

	if (StrEmpty(szClassName))
	{
		gcLogger.Error2(__METHOD__, " No constructor found for class with empty name.", NULL);
		return NULL;
	}

	pcUnknown = (CUnknown*)mpcConstructors->Construct(szClassName, mpcMalloc, uiAdditionalSize, &acDebug);
	if (pcUnknown)
	{
		return AddExisting(pcUnknown, acDebug);
	}
	else
	{
		gcLogger.Error2(__METHOD__, " No constructor found for class [", szClassName, "].", NULL);
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CUnknown* CUnknowns::AddFromHeader(CFileReader* pcFile)
{
	bool		bResult;
	size		iLength;
	char		sz[256];
	char*		psz;
	CUnknown*	pcUnknown;

	bResult = pcFile->ReadStringLength(&iLength);
	if (bResult != true)
	{
		return NULL;
	}

	if (iLength < 256)
	{
		bResult = pcFile->ReadStringChars(sz, iLength);
		if (bResult != true)
		{
			return NULL;
		}
		else
		{
			return Add(sz);
		}
	}
	else
	{
		psz = (char*)malloc(iLength+1);
		bResult = pcFile->ReadStringChars(psz, iLength);
		if (bResult != true)
		{
			free(psz);
			return NULL;
		}
		else
		{
			pcUnknown = Add(psz);
			free(psz);
			return pcUnknown;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CUnknowns::SaveUnknown(CFileWriter* pcFile, CUnknown* pcUnknown)
{
	ReturnOnFalse(pcUnknown->SaveHeader(pcFile));
	ReturnOnFalse(pcUnknown->Save(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CUnknowns::LoadUnknown(CFileReader* pcFile, CUnknown** ppcUnknown)
{
	CUnknown*		pcUnknown;

	pcUnknown = gcUnknowns.AddFromHeader(pcFile);
	if (!pcUnknown)
	{
		return false;
	}
	ReturnOnFalse(pcUnknown->Load(pcFile));
	*ppcUnknown = pcUnknown;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CUnknowns::Remove(CUnknown* pcUnknown)
{
	//This function will probably not be used.

	pcUnknown->Kill();
	//Unknowns kill will cause the list to remove the Unknown
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CUnknowns::RemoveInKill(CUnknown* pcUnknown)
{
	//Unknowns kill must call this last.
	//That means that anything inheriting from CUnknown must have a Kill that last calls CUnknown::Kill()
	if (pcUnknown)
	{
		if (pcUnknown->Iterable())
		{
			mcIterables.Remove(pcUnknown);
		}
		mpcMalloc->Free(pcUnknown);
		miNumElements--;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CUnknowns::RemoveInKill(CArrayUnknownPtr* papcObjectPts)
{
	size			i;
	CUnknown*		pcUnknown;
	CArrayVoidPtr	cArray;
	void**			pvData;
	size			iNumFreed;
	size			uiPtrs;

	if (papcObjectPts)
	{
		//Optimise this sometime later as iterables aren't used yet.
		uiPtrs = papcObjectPts->NumElements();
		for (i = 0; i < uiPtrs; i++)
		{
			pcUnknown = *papcObjectPts->Get(i);
			if (pcUnknown->Iterable())
			{
				mcIterables.Remove(pcUnknown);
			}
		}

		pvData = (void**)papcObjectPts->GetData();
		cArray.Fake(pvData, papcObjectPts->NumElements());

		iNumFreed = mpcMalloc->FreeMultiple(&cArray);
		miNumElements -= iNumFreed;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CUnknown* CUnknowns::StartIteration(char* szClassName, SIteratorUnknown* psIter)
{
	return mcIterables.StartIteration(szClassName, psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CUnknown* CUnknowns::Iterate(SIteratorUnknown* psIter)
{
	return mcIterables.Iterate(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CUnknowns::DebugName(CUnknown* pcUnknown, char (*pszDebug)[4])
{
	const char*	sz;
	char		szDebug[4];
	int			iLen;

	if (pszDebug)
	{
		sz = pcUnknown->ClassName();
		iLen = (int)strlen(sz);
		if (iLen >= 5)
		{
			szDebug[0] = sz[1];
			szDebug[1] = sz[2];
			szDebug[2] = sz[iLen - 2];
			szDebug[3] = sz[iLen - 1];
		}
		else
		{
			szDebug[0] = 0;
			szDebug[1] = 0;
			szDebug[2] = 0;
			szDebug[3] = 0;
			memcpy(szDebug, sz, iLen);
		}

		(*pszDebug)[0] = szDebug[0];
		(*pszDebug)[1] = szDebug[1];
		(*pszDebug)[2] = szDebug[2];
		(*pszDebug)[3] = szDebug[3];
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CUnknowns::DumpAddDetail(CUnknown* pcUnknown)
{
	SGeneralMemoryAllocation*	psAlloc;
	CChars				sz;

	psAlloc = GENERAL_MEMORY_GET_ALLOCATION(pcUnknown);
	if (!IsFreed(pcUnknown))
	{
		sz.Init("Size: ");
		sz.Append(psAlloc->uiSize);
		sz.AppendNewLine();
		sz.Append("AllocCount: ");
		sz.Append(psAlloc->uiAllocCount);
		sz.AppendNewLine();
		sz.Append("DebugName: ");
		sz.Append(psAlloc->szDebug, 4);
		sz.AppendNewLine();

		sz.Dump();
		sz.Kill();
	}
	else
	{
		sz.Init("Freed CUnknown.\n");
		sz.Append("AllocCount: ");
		sz.Append(psAlloc->uiAllocCount);
		sz.AppendNewLine();
		sz.Dump();
		sz.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CUnknowns::IsFreed(CUnknown* pcUnknown)
{
#ifdef _DEBUG
	SGeneralMemoryAllocation*	psAlloc;
	int							i;
	int*						piMem;

	psAlloc = GENERAL_MEMORY_GET_ALLOCATION(pcUnknown);
	for (i = 1; i < sizeof(SGeneralMemoryAllocation)/4; i++)
	{
		piMem = &((int*)psAlloc)[i];
		if (*piMem != 0xefefefef)
		{
			return false;
		}
	}
	return true;
#else // _DEBUG;

	//Actually we don't know.
	return false;
#endif // _DEBUG;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CUnknowns::NumElements(void)
{
	return miNumElements;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CUnknowns::GetIterableListsHeadNumElements(void)
{
	return mcIterables.mcIterableLists.GetHead()->mcFreeListPtrs.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void UnknownsInit(void)
{
	gcUnknowns.Init("Global", &gcConstructors);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void UnknownsKill(void)
{
	gcUnknowns.Kill();
}

