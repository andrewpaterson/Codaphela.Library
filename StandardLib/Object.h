/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#ifndef __OBJECT_H__
#define __OBJECT_H__
#include "Integer.h"
#include "Float.h"
#include "Character.h"
#include "PrimitiveVoid.h"
#include "BaseObject.h"


class CObject : public CBaseObject
{
template<class M>
friend class Ptr;
friend class CPointer;
friend class CObjects;
friend class CExternalObjectDeserialiser;

CONSTRUCTABLE(CObject);
protected:
	CArrayTemplateEmbedded<CPointer*, 5>		mapPointers;  //Pointers in this object.  
	CArrayTemplateEmbedded<CBaseObject*, 3>		mapEmbedded;  //Objects embedded in this object

public:
						CObject();
	void				Allocate(CObjects* pcObjects);
	void				Class(void);
	BOOL				IsCollection(void);
	BOOL				IsObject(void);
	void				SetPointerTosExpectedDistToRoot(int iDistToRoot);

	CPointer*			Pointer(CPointer* pcPointer, char* szFieldName);
	void				Embedded(CBaseObject* pcObject, char* szFieldName);

	BOOL				IsDirty(void);
	int					GetEmbeddedIndex(CEmbeddedObject* pcEmbedded);
	uint16				GetNumEmbedded(void);
	CEmbeddedObject*	GetEmbeddedObject(uint16 iIndex);
	CBaseObject*		Dehollow(void);
	int					NumHeapFroms(void);
	int					NumStackFroms(void);
	void				SetFlag(int iFlag, int iFlagValue);
	void				GetHeapFroms(CArrayTemplateEmbeddedBaseObjectPtr* papcFroms);
	void				GetStackFroms(CArrayTypedPointerPtr* papcFroms);
	CBaseObject*		GetClosestFromToRoot(void);
	CBaseObject*		GetClosestFromToStack(void);
	int					NumPointerTos(void);
	int					BaseNumPointerTos(void);
	void				GetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos);
	void				BaseGetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos);
	BOOL				ContainsPointerTo(CEmbeddedObject* pcEmbedded);
	void				CollectAndClearPointerTosInvalidDistToRootObjects(CDistCalculatorParameters* pcParameters);
	void				BaseCollectAndClearPointerTosInvalidDistToRootObjects(CDistCalculatorParameters* pcParameters);
	int					CollectDetachedFroms(CDistCalculatorParameters* pcParameters);

	int					GetFieldPointerToIndex(CPointer* pcFieldPointer);
	int					GetNumFieldPointerTos(void);
	CPointer*			GetFieldPointerTo(int iIndex);

	void				ValidatePointerTos(void);
	void				ValidateConsistency(void);

public:
	void				Primitive(Int8* pcPrimitive, char* szFieldName);
	void				Primitive(UInt8* pcPrimitive, char* szFieldName);
	void				Primitive(Int16* pcPrimitive, char* szFieldName);
	void				Primitive(UInt16* pcPrimitive, char* szFieldName);
	void				Primitive(Int32* pcPrimitive, char* szFieldName);
	void				Primitive(UInt32* pcPrimitive, char* szFieldName);
	void				Primitive(Int64* pcPrimitive, char* szFieldName);
	void				Primitive(UInt64* pcPrimitive, char* szFieldName);
	void				Primitive(Bool* pcPrimitive, char* szFieldName);
	void				Primitive(Float32* pcPrimitive, char* szFieldName);
	void				Primitive(Float64* pcPrimitive, char* szFieldName);
	void				Primitive(Char8* pcPrimitive, char* szFieldName);
	void				Primitive(Char16* pcPrimitive, char* szFieldName);

	void				UnmanagedInt(int8* pc, char* szFieldName);
	void				UnmanagedInt(int8* pac, size_t uiLength, char* szFieldName);
	void				UnmanagedInt(int16* pi, char* szFieldName);
	void				UnmanagedInt(int16* pai, size_t uiLength, char* szFieldName);
	void				UnmanagedInt(int32* pi, char* szFieldName);
	void				UnmanagedInt(int32* pai, size_t uiLength, char* szFieldName);
	void				UnmanagedInt(int64* pi, char* szFieldName);
	void				UnmanagedInt(int64* pai, size_t uiLength, char* szFieldName);
	void				UnmanagedInt(uint8* pc, char* szFieldName);
	void				UnmanagedInt(uint8* pac, size_t uiLength, char* szFieldName);
	void				UnmanagedInt(uint16* pi, char* szFieldName);
	void				UnmanagedInt(uint16* pai, size_t uiLength, char* szFieldName);
	void				UnmanagedInt(uint32* pi, char* szFieldName);
	void				UnmanagedInt(uint32* pai, size_t uiLength, char* szFieldName);
	void				UnmanagedInt(uint64* pi, char* szFieldName);
	void				UnmanagedInt(uint64* pai, size_t uiLength, char* szFieldName);
	void				UnmanagedFloat(float32* pf, char* szFieldName);
	void				UnmanagedFloat(float32* paf, size_t uiLength, char* szFieldName);
	void				UnmanagedFloat(float64* pf, char* szFieldName);
	void				UnmanagedFloat(float64* paf, size_t uiLength, char* szFieldName);
	void				UnmanagedChar(char8* pc, char* szFieldName);
	void				UnmanagedChar(char8* pac, size_t uiLength, char* szFieldName);
	void				UnmanagedChar(char16* pc, char* szFieldName);
	void				UnmanagedChar(char16* pac, size_t uiLength, char* szFieldName);

	void				UnmanagedString(CChars* pcChars, char* szFieldName);
	void				UnmanagedNumber(CNumber* pcNumber, char* szFieldName);
	void				UnmanagedDate(CDate* pcDate, char* szFieldName);

	void				UnmanagedData(void* pv, size_t uiSizeof, char* szFieldName);

protected:
	void				EmbedFields(void);
	void				InternalFree(void);
	void				FreePointers(void);
	void				RemovePointerTo(CEmbeddedObject* pcTo);
	void				RemoveAllPointerTosDontKill(void);
	void				RemoveAllPointerTos(void);
	void				RemoveAllHeapFroms(void);
	void				RemoveAllStackFroms(void);
	CBaseObject*		GetClosestFromForCanFindRoot(void);
	int					RemapPointerTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew);
	void				UpdateAttachedEmbeddedObjectPointerTosDistToRoot(CDistCalculatorParameters* pcParameters, int iExpectedDist);
	void				BaseUpdateAttachedEmbeddedObjectPointerTosDistToRoot(CDistCalculatorParameters* pcParameters, int iExpectedDist);
	int					CalculateDistToRootFromPointedFroms(int iDistToRoot);
	void				SetPointedTosDistToRoot(int iDistToRoot);
	BOOL				SetDistToRoot(int iDistToRoot);
	void				SetDistToStack(int iDistToStack);
	BOOL				RecurseGetEmbeddedIndex(CEmbeddedObject* pcTest, int* piIndex);
	CEmbeddedObject*	RecurseGetEmbeddedObject(int iIndex, int* iCount);
	BOOL				RecurseGetFieldPointerToIndex(CPointer* pcTest, int* piIndex);
	CPointer*			RecurseGetFieldPointerTo(int iIndex, int* piCount);
	void				BaseValidatePointerTos(void);
	void				ValidateEmbeddedConsistency(void);
	BOOL				IsDistToRootValid(void);
	void				EmbedPoinerFields(void);
	void				EmbedEmbeddedObjectFields(void);
};


#endif // __OBJECT_H__

