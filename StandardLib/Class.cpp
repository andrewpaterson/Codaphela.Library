#include "PrimitiveObject.h"
#include "BaseObject.h"
#include "Classes.h"
#include "Class.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClass::Init2(char* szClassName, CClasses* pcClasses)
{
	mszClassName.Init(szClassName);
	mpcClassesThisIn = pcClasses;

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

	mszClassName.Kill();
	mpcClassesThisIn = NULL;
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
	pcPointerField->Init(iOffset, this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClass::Primitive(CBaseObject* pcThis, CPrimitiveObject* pcPrimitive)
{
	CDataField*		pcDataField;
	ptrdiff_t		iOffset;
	CClass*			pcClass;

	iOffset = (size_t)pcPrimitive - (size_t)pcThis;

	pcDataField = macDatas.Add();
	pcClass = GetClass(pcPrimitive->GetClassType());
	pcDataField->Init(pcClass, iOffset, this);
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
	pcEmbeddedObjectField->Init(pcObject->GetClass(), iOffset, this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CClasses* CClass::GetClasses(void)
{
	return mpcClassesThisIn;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CClass* CClass::GetClass(EPrimitiveType eType)
{
	return mpcClassesThisIn->Get(eType);
}

