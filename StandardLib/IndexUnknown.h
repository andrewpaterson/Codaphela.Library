/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#ifndef __INDEX_UNKNOWN_H__
#define __INDEX_UNKNOWN_H__
#include "BaseLib/IndexTreeMemory.h"
#include "Unknown.h"


#define INDEX_TREE_KILL_ELEMENT	0x0001


class CIndexUnknown : public CUnknown, public CDataFree, public CDataIO, public CIndexTreeDataSize
{
CONSTRUCTABLE(CIndexUnknown)
friend class CIndexUnknownDataFree;
private:
	uint16					muiFlags;
	CIndexTreeMemory		mcIndex;

public:
	void		Init(bool bKillElements = true, bool bOverwriteExisting = true, EIndexKeyReverse eKeyReverse = IKR_No);
	void		Kill(void);
	void		ReInit(void);

	bool		Save(CFileWriter* pcFileWriter);
	bool		Load(CFileReader* pcFileReader);

	template<class M>
	M*			Put(uint8* pvKey, size iKeySize);  //Note this allocates M and returns a pointer to it.  Not a pointer to the pointer in the map.
	bool		Put(uint8* pvKey, size iKeySize, CUnknown* pcValue);
	template<class M>
	M*			Put(char* szKey);
	bool		Put(char* szKey, CUnknown* pcValue);
	CUnknown**	PutNode(uint8* pvKey, size iKeySize);
	CUnknown*	Get(char* szKey);
	CUnknown*	Get(uint8* pvKey, size iKeySize);
	bool		Remove(char* szKey);
	bool		Remove(uint8* pvKey, size iKeySize);
	size		Remove(CUnknown* pvData);

	size		NumElements(void);
	size		NonNullElements(void);
	bool 		IsKillElements(void);
	bool 		IsEmpty(void);
	bool 		IsNotEmpty(void);

	bool		StartIteration(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, CUnknown** ppvData);
	bool		Iterate(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, CUnknown** ppvData);
	bool		StartIteration(SIndexTreeMemoryUnsafeIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, CUnknown** ppvData);
	bool		Iterate(SIndexTreeMemoryUnsafeIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, CUnknown** ppvData);
	bool		StartIteration(SIndexTreeMemoryUnsafeIterator* psIterator, CUnknown** ppvData);
	bool		Iterate(SIndexTreeMemoryUnsafeIterator* psIterator, CUnknown** ppvData);

	bool		WriteIndexUnknownHeader(CFileWriter* pcFileWriter);
	size		ReadIndexUnknownHeader(CFileReader* pcFileReader);
	size		ReadIndexUnknownHeader(CFileReader* pcFileReader, CDataIO* pcDataIO, CIndexTreeDataSize* pcDataSize, CDataFree* pcDataFree);

protected:
	void		FreeData(void* pvData);
	bool		WriteData(CFileWriter* pcFileWriter, void* pvData);
	bool		ReadData(CFileReader* pcFileReader, void* pvData);
	size		AdjustDataSize(void* pvValue, size iValueSize);
};


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CIndexUnknown::Put(uint8* pvKey, size iKeySize)
{
	M*		pv;
	bool	bResult;

	if (iKeySize > 0)
	{
		pv = gcUnknowns.Add<M>();
		bResult = Put(pvKey, iKeySize, pv);
		if (bResult)
		{
			return pv;
		}
		else
		{
			//Nasty, but if we allocated (and didn't initialise) the object then it must be freed but not killed.
			gcUnknowns.RemoveInKill(pv);
			return NULL;
		}
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CIndexUnknown::Put(char* szKey)
{
	M*		pv;
	bool	bResult;
	size	iKeySize;

	iKeySize = strlen(szKey);
	if (iKeySize > 0)
	{
		pv = gcUnknowns.Add<M>();
		bResult = Put(pvKey, iKeySize, pv);
		if (bResult)
		{
			return pv;
		}
		else
		{
			//Nasty, but if we allocated (and didn't initialise) the object then it must be freed but not killed.
			gcUnknowns.RemoveInKill(pv);
			return NULL;
		}
	}
	else
	{
		return NULL;
	}
}


#endif // __INDEX_UNKNOWN_H__

