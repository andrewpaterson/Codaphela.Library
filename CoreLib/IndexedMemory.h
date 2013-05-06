/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __INDEXED_MEMORY_H__
#define __INDEXED_MEMORY_H__
#include "ArrayVariable.h"
#include "IndexedGeneral.h"
#include "TransactionIndexedData.h"


class CIndexedMemory : public CTransactionIndexedData
{
private:
	SOIndexIndex	asIndexCache[4];
	int				miNext;
	CArrayVariable	mcDatas;

public:
	void				Init(void);
	void				Kill(void);

	BOOL				Add(OIndex oi, void* pvData, unsigned int uiSize);
	BOOL				Get(OIndex oi, void* pvDest);
	BOOL				Set(OIndex oi, void* pvData, unsigned int uiSize);
	BOOL				Remove(OIndex oi);
	unsigned int		Size(OIndex oi);
	BOOL				AddRemoved(OIndex oi);

	SIndexedMemory*		GetIndexedData(int iIndexedDataIndex);
	int					NumDatas(void);
	void*				GetData(SIndexedMemory* psIndexedMemory);

	BOOL				TestOrder(void);
	void				Dump(void);
	BOOL				GetDetail(OIndex oi, void** ppvData, unsigned int* puiSize);

protected:
	BOOL				PrivateSetRemoved(OIndex oi, int iIndex);
	BOOL				PrivateAdd(OIndex oi, unsigned int uiSize, void** ppvData, int iIndex);
	BOOL				PrivateGetDetail(OIndex oi, void** ppvData, unsigned int* puiSize, int* piIndex);
	void				ClearIndexCache(void);
	BOOL				GetDirect(int iIndex, void** ppvData, unsigned int* puiSize);
	BOOL				GetNoCacheTest(OIndex oi, void** ppvData, unsigned int* puiSize, int* piIndex);
	void				SetIndexCache(OIndex oi, int iIndex);
};


#endif // __INDEXED_MEMORY_H__

