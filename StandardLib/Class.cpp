#include "PrimitiveObject.h"
#include "Class.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClass::Init(void)
{
	muiSize = 0;
	muiType = 0;
	muiFlags = 0;

	macPointers.Init();
	macEmbeddedObjects.Init();
	macDatas.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClass::Kill(void)
{
	macPointers.Kill();
	macEmbeddedObjects.Kill();
	macDatas.Kill();

	muiFlags = 0;
	muiSize = 0;
	muiType = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClass::Pointer(CBaseObject* pcThis, CPointer* pcPointer)
{
	CPointerField*	pcPointerField;
	ptrdiff_t		iOffset;

	iOffset = (size_t)pcPointer - (size_t)pcThis;
	pcPointerField = macPointers.Add();
	pcPointerField->Init(iOffset);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClass::Primitive(CBaseObject* pcThis, CPrimitiveObject* pcPrimitive)
{
	CDataField*		pcDataField;
	ptrdiff_t		iOffset;

	iOffset = (size_t)pcPrimitive - (size_t)pcThis;

	pcDataField = macDatas.Add();
	pcDataField->Init(pcPrimitive->GetClassType(), iOffset);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClass::Embedded(CBaseObject* pcThis, CBaseObject* pcObject)
{
	CEmbeddedObjectField*	pcEmbeddedObjectField;
	ptrdiff_t				iOffset;

	iOffset = (size_t)pcObject- (size_t)pcThis;

	pcEmbeddedObjectField = macEmbeddedObjects.Add();
	pcEmbeddedObjectField->Init(pcObject->GetClassType(), iOffset);
}

