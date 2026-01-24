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
#ifndef __TRACKER_TEMPLATE_H__
#define __TRACKER_TEMPLATE_H__
#include "BaseLib/ChunkFile.h"
#include "StandardHeader.h"
#include "ArrayUnknownTemplate.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class T>
class CTrackerTemplate
{
protected:
	bool			mbIndexAndIDSame;
	CChars			mszClassName;
	CArrayUnknownTemplate<T>	mcArray;

public:
	void 	Init(void);
	void 	Init(bool bIndexAndIDSame, int iType);
	void 	Kill(void);
	int		Num(void);
	T*		Add(void);
	int		Load(CFileReader* pcSourceFile);
	void	Save(CFileWriter* pcDestFile);

	T*		Copy(T* p);
	T*		GetWithID(int iID);
	T*		Get(int iElementNum);
	void	Remove(int iElementNum);
	int		GetNextUniqueID(void);
	T*		CopyWithID(int iID);
	T*		GetOrAdd(char* szName);
	T*		GetWithName(char* szName);
	T*		AddAll(CArrayUnknownTemplate<T>* pcArray);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class T>
void CTrackerTemplate<T>::Init(void)
{
	Init(true, -1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class T>
void CTrackerTemplate<T>::Init(bool bIndexAndIDSame, int iType)
{
	T	q;

	mcArray.Init();
	mbIndexAndIDSame = bIndexAndIDSame;
	mszClassName.Init(q.ClassName());
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class T>
void CTrackerTemplate<T>::Kill(void)
{
	mcArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//Me
template <class T>
int CTrackerTemplate<T>::Num(void)
{
	return mcArray.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class T>
T* CTrackerTemplate<T>::Add(void)
{
	T*		pHelper;
	int		iID;

	iID = GetNextUniqueID();
	pHelper = mcArray.Add();

	//Get rid of this.
	pHelper->Init();  //Assumes there is always a default init function that takes no parameters.
						
	pHelper->SetID(iID);
	return pHelper;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class T>
T* CTrackerTemplate<T>::GetWithID(int iID)
{
	T*		aHelper;
	int		i;

	if (mbIndexAndIDSame)
	{
		return mcArray.Get(iID);
	}

	
	for (i = 0; i < mcArray.NumElements(); i++)
	{
		aHelper = mcArray.Get(i);
		if (iID == aHelper->GetID())
		{
			return aHelper;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class T>
T* CTrackerTemplate<T>::Get(int iElementNum)
{
	return mcArray.Get(iElementNum);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class T>
int CTrackerTemplate<T>::Load(CFileReader* pcSourceFile)
{
	int		iChunkNum;
	T*		pcHelper;
	int		iFirstIndex;
	int		iIndex;
	T		cTemp;

	iFirstIndex = Num();
	iChunkNum = pcSourceFile->FindFirstChunkWithName(cTemp.ClassName());
	iIndex = 0;
	while (iChunkNum != -1)
	{
		pcHelper = (T*)mcArray.Add();
	
		if (!pcHelper->LoadSpecific(pcSourceFile, iChunkNum))
		{
			return -1;
		}

		if (pcHelper->GetID() != iIndex)
		{
			mbIndexAndIDSame = false;
		}

		iChunkNum = pcSourceFile->FindNextChunkWithName();
		iIndex++;
	}

	return iFirstIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class T>
void CTrackerTemplate<T>::Save(CFileWriter* pcDestFile)
{
	T*		pcHelper;
	int		i;

	for (i = 0; i < Num(); i++)
	{
		pcHelper = Get(i);
		pcHelper->Save(pcDestFile);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class T>
void CTrackerTemplate<T>::Remove(int iElementNum)
{
	T*		pHelper;
	
	pHelper = Get(iElementNum);

	if (pHelper)
	{
		mcArray.Remove(iElementNum);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class T>
int CTrackerTemplate<T>::GetNextUniqueID(void)
{
	T*		pHelper;
	int		i;
	int		iID;

	if (mbIndexAndIDSame)
	{
		return mcArray.NumElements();
	}

	iID = 0;
	for (i = 0; i < Num(); i++)
	{
		pHelper = Get(i);
		if (iID <= pHelper->GetID())
		{
			iID = pHelper->GetID() + 1;
		}
	}
	return iID;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class T>
T* CTrackerTemplate<T>::Copy(T* p)
{
	T* q;

	if (p)
	{
		q = Add();
		q->Copy(p);
		return q;
	}
	return NULL;
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class T>
T* CTrackerTemplate<T>::GetWithName(char* szName)
{
	int		i;
	T*		pcGroup;

	for (i = 0; i < mcArray.NumElements(); i++)
	{
		pcGroup = mcArray.Get(i);
		if (pcGroup->Is(szName))
		{
			return pcGroup;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class T>
T* CTrackerTemplate<T>::GetOrAdd(char* szName)
{
	T*		q;

	q = GetWithName(szName);
	if (q)
	{
		return q;
	}
	else
	{
		q = Add();
		q->Init(szName);
		return q;
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class T>
T* CTrackerTemplate<T>::AddAll(CArrayUnknownTemplate<T>* pcArray)
{
	int		i;
	T*		q;
	int		iID;

	iID = GetNextUniqueID();
	mcArray.AddAll(pcArray);

	for (i = 0; i < pcArray->NumElements(); i++)
	{
		q = pcArray->Get(i);
		q->SetID(iID);
		iID++;
	}
}


#endif // __TRACKER_TEMPLATE_H__

