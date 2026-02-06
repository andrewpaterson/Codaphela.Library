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
#ifndef __ARRAY_COMMON_UNKNOWN_H__
#define __ARRAY_COMMON_UNKNOWN_H__
#include "SetIterator.h"
#include "ArrayUnknownPtr.h"
#include "Unknown.h"


#define ARRAY_COMMOM_KILL_ELEMENT	0x0001
#define ARRAY_COMMOM_IS_PTR_SORTED	0x0002
#define ARRAY_COMMOM_UNIQUE_ONLY	0x0004
#define ARRAY_COMMOM_IGNORE_NULL	0x0008
#define ARRAY_COMMOM_PRESERVE_ORDER	0x0010
#define ARRAY_COMMOM_TYPE_KNOWN		0x0020


enum EArrayUnsetReturn
{
	AUR_NoChange,
	AUR_SetNull,
	AUR_MustRemove,
};


//This is called CArrayCommonUnknown because it *HAS* a _CArrayUnknownPtr not because it *IS* a the base class for unknown arrays (although it is that also).
class CArrayCommonUnknown : public CUnknown
{
CONSTRUCTABLE(CArrayCommonUnknown);
protected:
	uint16				miFlags;

private:
	CArrayUnknownPtr	mcArray;  //CArrayTemplate<CUnknown*>
	size				miNonNullElements;

	bool				PrivateIterate(SSetIterator* psIter, CUnknown** ppcUnknown);
	void				PrivateKill(void);

public:
	void 				Init(bool bTypeKnown, bool bKillElements, bool bUnique, bool bIgnoreNull, bool bPreserveOrder);
	void				Init(bool bTypeKnown, bool bKillElements, bool bUnique, bool bIgnoreNull, bool bPreserveOrder, size iChunkSize);
	void 				Kill(void);
	void 				ReInit(void);

	bool				Save(CFileWriter* pcFile);
	bool				Load(CFileReader* pcFile);

	size				NumElements(void);
	void				KillElements(bool bKill);
	void				UniqueElements(bool bUnique);
	void				IgnoreNullElements(bool bIgnoreNull);
	void				PreserveOrder(bool bPreserveOrder);
	void				Sort(void);
	bool				Contains(CUnknown* pcUnknown);
	size				Find(CUnknown* pcUnknown);
	void				CleanNullsIfNecessary(bool bCleanNullsIfNecessary);
	void				CleanNullsIfNecessary(void);
	void				TypeKnown(bool bTypeKnown);
	bool				Add(CUnknown* pcUnknown, bool bCleanNullsIfNecessary = true);
	bool				AddAll(CArrayCommonUnknown* pcSource);
	bool				Set(size iIndex, CUnknown* pcUnknown, bool bCleanNullsIfNecessary = true);
	bool				Insert(size iIndex, CUnknown* pcUnknown);
	bool				RemoveEnd(size iIndexInclusive);
	bool				Remove(CUnknown* pcUnknown);
	bool				Remove(size iIndex, bool bCleanNullsIfNecessary = true);
	bool				RemoveLast(bool bCleanNullsIfNecessary = true);
	bool				RemoveDuringIteration(SSetIterator* psIter);
	EArrayUnsetReturn	Unset(size iIndex);
	CUnknown*			First(void);
	CUnknown*			Last(void);
	CUnknown*			StartIteration(SSetIterator* psIter);
	CUnknown*			Iterate(SSetIterator* psIter);
	bool				IsKillElements(void);
	bool				IsEmpty(void);
	bool				IsNotEmpty(void);
	void				UnsafeSet(size iIndex, CUnknown* pcUnknown);
	CUnknown*			UnsafeGet(size iIndex);
	CUnknown**			UnsafeGetPointer(size iIndex);
	size				UnsafeNumElements(void);
	size				UnsafeNonNullElements(void);
	bool				LoadArrayHeader(CFileReader* pcFile, uint16* piFlags, size* piNumElements);
	bool				SaveArrayHeader(CFileWriter* pcFile);
	void				PostLoad(uint16 iFlags);
	void				SetChunkSize(size iChunkSize);

protected:
	virtual bool		LoadElement(CFileReader* pcFile, CUnknown** ppcUnknown);
	virtual bool		SaveElement(CFileWriter* pcFile, CUnknown* pcUnknown);
};


#endif // __ARRAY_COMMON_UNKNOWN_H__

