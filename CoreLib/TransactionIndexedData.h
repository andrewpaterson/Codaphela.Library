/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#ifndef __TRANSACTION_INDEXED_DATA_H__
#define __TRANSACTION_INDEXED_DATA_H__
#include "BaseLib/Bool.h"
#include "IndexedGeneral.h"


struct SIndexedMemory
{
	OIndex			oi;
	unsigned int	uiSize;

	BOOL	IsRemoved(void);
};


class CTransactionIndexedData
{
public:
	virtual void				Kill(void) =0;

	virtual BOOL				Add(OIndex oi, void* pvData, unsigned int uiSize) =0;
	virtual BOOL				Get(OIndex oi, void* pvDest) =0;
	virtual BOOL				Set(OIndex oi, void* pvData, unsigned int uiSize) =0;
	virtual BOOL				Remove(OIndex oi) =0;
	virtual unsigned int		Size(OIndex oi) =0;
	virtual BOOL				AddRemoved(OIndex oi) =0;
	virtual SIndexedMemory*		GetIndexedData(int iIndexedDataIndex) =0;
	virtual int					NumDatas(void) =0;
	virtual void*				GetData(SIndexedMemory* psIndexedMemory) =0;
	virtual void				Dump(void) =0;
	virtual BOOL				GetDetail(OIndex oi, void** ppvData, unsigned int* puiSize) =0;
};


#endif // __TRANSACTION_INDEXED_DATA_H__
