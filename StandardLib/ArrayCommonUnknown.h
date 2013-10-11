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
#include "ArrayUnknownPtr.h"
#include "Unknown.h"


#define ARRAY_COMMOM_KILL_ELEMENT	0x0001
#define ARRAY_COMMOM_IS_PTR_SORTED	0x0002
#define ARRAY_COMMOM_UNIQUE_ONLY	0x0004
#define ARRAY_COMMOM_IGNORE_NULL	0x0008
#define ARRAY_COMMOM_PRESERVE_ORDER	0x0010
#define ARRAY_COMMOM_TYPE_KNOWN		0x0020


#define ARRAY_COMMOM_CHUNK_SIZE	16


struct SSetIterator
{
	int		iIndex;
	BOOL	bStepFirst;
};


enum EArrayUnsetReturn
{
	AUR_NoChange,
	AUR_SetNull,
	AUR_MustRemove,
};


//This is called CArrayCommonUnknown because it *HAS* a _CArrayUnknownPtr not because it *IS* a the base class for unknown arrays (although it is that also).
class CArrayCommonUnknown : public CUnknown
{
BASE_FUNCTIONS(CArrayCommonUnknown);
protected:
	int					miFlags;

private:
	CArrayUnknownPtr	mcArray;
	int					miNonNullElements;

	BOOL				PrivateIterate(SSetIterator* psIter, CUnknown** ppcUnknown);
	void				PrivateKill(void);

public:
	void 				Init(BOOL bTypeKnown, BOOL bKillElements, BOOL bUnique, BOOL bIgnoreNull, BOOL bPreserveOrder, int iChunkSize);
	void 				Kill(void);
	void 				ReInit(void);
	BOOL				Save(CFileWriter* pcFile);
	BOOL				Load(CFileReader* pcFile);

	int					NumElements(void);
	void				KillElements(BOOL bKill);
	void				UniqueElements(BOOL bUnique);
	void				IgnoreNullElements(BOOL bIgnoreNull);
	void				PreserveOrder(BOOL bPreserveOrder);
	void				Sort(void);
	BOOL				Contains(CUnknown* pcUnknown);
	int					Find(CUnknown* pcUnknown);
	void				CleanNullsIfNecessary(BOOL bCleanNullsIfNecessary);
	void				CleanNullsIfNecessary(void);
	void				TypeKnown(BOOL bTypeKnown);
	BOOL				Add(CUnknown* pcUnknown, BOOL bCleanNullsIfNecessary = TRUE);
	BOOL				AddAll(CArrayCommonUnknown* pcSource);
	BOOL				Set(int iIndex, CUnknown* pcUnknown, BOOL bCleanNullsIfNecessary = TRUE);
	BOOL				Insert(int iIndex, CUnknown* pcUnknown);
	BOOL				RemoveEnd(int iIndexInclusive);
	BOOL				Remove(CUnknown* pcUnknown);
	BOOL				Remove(int iIndex, BOOL bCleanNullsIfNecessary = TRUE);
	BOOL				RemoveLast(BOOL bCleanNullsIfNecessary = TRUE);
	void				RemoveDuringIteration(SSetIterator* psIter);
	EArrayUnsetReturn	Unset(int iIndex);
	CUnknown*			First(void);
	CUnknown*			Last(void);
	CUnknown*			StartIteration(SSetIterator* psIter);
	CUnknown*			Iterate(SSetIterator* psIter);
	BOOL				IsKillElements(void);
	BOOL				IsEmpty(void);
	BOOL				IsNotEmpty(void);
	void				UnsafeSet(int iIndex, CUnknown* pcUnknown);
	CUnknown*			UnsafeGet(int iIndex);
	CUnknown**			UnsafeGetPointer(int iIndex);
	int					UnsafeNumElements(void);
	int					UnsafeNonNullElements(void);
	BOOL				LoadArrayHeader(CFileReader* pcFile, int* piFlags, int* piNumElements);
	BOOL				SaveArrayHeader(CFileWriter* pcFile);
	void				PostLoad(int iFlags);

protected:
	virtual BOOL	LoadElement(CFileReader* pcFile, CUnknown** ppcUnknown);
	virtual BOOL	SaveElement(CFileWriter* pcFile, CUnknown* pcUnknown);
};


#endif // __ARRAY_COMMON_UNKNOWN_H__

