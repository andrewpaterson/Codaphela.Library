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
#include "Unknowns.h"


#define INDEX_TREE_KILL_ELEMENT	0x0001
#define INDEX_TREE_NO_OVERWRITE	0x0002  //Set when Put must not replace an existing key.  Clear in indices saved before it existed, which always overwrote.


class CIndexUnknown : public CUnknown, public CDataFree, public CDataIO, public CIndexTreeDataSize
{
CONSTRUCTABLE(CIndexUnknown)
private:
	uint16					muiFlags;
	CIndexTreeMemory		mcIndex;

public:
	void		_Init(void);
	void		Init(bool bKillElements = true, bool bOverwriteExisting = true, EIndexKeyReverse eKeyReverse = IKR_No);
	void		Kill(void) override;
	void		ReInit(void);

	bool		Save(CFileWriter* pcFileWriter) override;
	bool		Load(CFileReader* pcFileReader) override;

	template<class M>
	M*			Put(uint8* pvKey, size iKeySize);  //Note this allocates M and returns a pointer to it.  Not a pointer to the pointer in the map.
	bool		Put(uint8* pvKey, size iKeySize, CUnknown* pcValue);
	template<class M>
	M*			Put(const char* szKey);
	bool		Put(const char* szKey, CUnknown* pcValue);
	CUnknown**	PutNode(uint8* pvKey, size iKeySize);
	CUnknown*	Get(const char* szKey);
	CUnknown*	Get(uint8* pvKey, size iKeySize);
	bool		Remove(const char* szKey);
	bool		Remove(uint8* pvKey, size iKeySize);
	size		Remove(CUnknown* pvData);

	size		NumElements(void);
	size		NonNullElements(void);
	bool 		IsKillElements(void);
	bool 		IsOverwriteExisting(void);
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
	void		FreeData(void* pvData) override;
	bool		WriteData(CFileWriter* pcFileWriter, void* pvData) override;
	bool		ReadData(CFileReader* pcFileReader, void* pvData) override;
	size		AdjustDataSize(void* pvValue, size iValueSize) override;
};


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CIndexUnknown::Put(uint8* pvKey, size iKeySize)
{
	M*	pv;

	if (iKeySize == 0)
	{
		return NULL;
	}
	pv = gcUnknowns.Add<M>();
	return gcUnknowns.KeptOrFreed(pv, Put(pvKey, iKeySize, pv));
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CIndexUnknown::Put(const char* szKey)
{
	if (StrEmpty(szKey))
	{
		return NULL;
	}
	return Put<M>((uint8*)szKey, StrLen(szKey));
}


#endif // __INDEX_UNKNOWN_H__

