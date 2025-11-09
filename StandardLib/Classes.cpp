#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "Array.h"
#include "Set.h"
#include "Root.h"
#include "ArrayObject.h"
#include "String.h"
#include "PointerContainer.h"
#include "Classes.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClasses::Init(CObjects* pcObjects)
{
	DataIOValidate();
	TypesValidate();

	muiCurrentClassType = 0;

	maClasses.Init(sizeof(CClass));
	mmcpClassesByName.Init();
	mmcpClassesByType.Init();

	mcUnmanaged.Init(this);

	muiCurrentClassType = CLASS_TYPES - 1;

	AddSystemClasses();
	mcPrimitive.Init(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClasses::Kill(void)
{
	SFreeListIterator	sIter;
	CClass*				pcClass;

	mcUnmanaged.Kill();

	mmcpClassesByName.Kill();
	mmcpClassesByType.Kill();
	
	pcClass = (CClass*)maClasses.StartIteration(&sIter);
	while (pcClass)
	{
		pcClass->Kill();
		pcClass = (CClass*)maClasses.Iterate(&sIter);
	}
	maClasses.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClasses::AddSystemClasses(void)
{
	CClass*			pcClass;

	pcClass = Add("Pointer", sizeof(CPointer));
	pcClass->System();
	pcClass->Complete();

	AddSystemClassAndConstructor<CSetObject>();
	AddSystemClassAndConstructor<CSet<>>();
	AddSystemClassAndConstructor<CArrayObject>();
	AddSystemClassAndConstructor<CArray<>>();
	AddSystemClassAndConstructor<CRoot>();
	AddSystemClassAndConstructor<CString>();
	AddSystemClassAndConstructor<CPointerContainer>();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CClass* CClasses::Add(char* szClassName, uint32 uiSize, EPrimitiveType eType)
{
	CClass* pcClass;

	pcClass = (CClass*)maClasses.Add();
	pcClass->Init(szClassName, uiSize, eType, this);
	mmcpClassesByName.Put(szClassName, &pcClass);
	mmcpClassesByType.Put(eType, &pcClass);

	return pcClass;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CClass* CClasses::Add(const char* szClassName, uint32 uiSize, EPrimitiveType eType)
{
	return Add((char*)szClassName, uiSize, eType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CClass* CClasses::Add(char* szClassName, uint32 uiSize)
{
	return Add(szClassName, uiSize, (EPrimitiveType)GetNextClassType());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CClass* CClasses::Add(const char* szClassName, uint32 uiSize)
{
	return Add((char*)szClassName, uiSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CClass* CClasses::Get(char* szClassName)
{
	CClass** ppcClass;

	ppcClass = mmcpClassesByName.Get(szClassName);
	if (ppcClass)
	{
		return *ppcClass;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CClass* CClasses::Get(const char* szClassName)
{
	return Get((char*)szClassName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CClass* CClasses::Get(EPrimitiveType eType)
{
	CClass** ppcClass;

	ppcClass = mmcpClassesByType.Get(eType);
	if (ppcClass)
	{
		return *ppcClass;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CClass* CClasses::GetValid(EPrimitiveType eType)
{
	CClass** ppcClass;

	ppcClass = mmcpClassesByType.Get(eType);
	if (ppcClass)
	{
		return *ppcClass;
	}
	ValidateType(eType);
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CClass* CClasses::Get(uint32 iType)
{
	return Get((EPrimitiveType)iType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CUnmanagedClasses* CClasses::GetUnmanagedClasses(void)
{
	return &mcUnmanaged;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPrimitiveClasses* CClasses::GetPrimitiveClasses(void)
{
	return &mcPrimitive;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CClass* CClasses::GetPointer(void)
{
	return mpcPointer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CClasses::GetNextClassType(void)
{
	muiCurrentClassType++;
	return muiCurrentClassType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClasses::ValidateType(EPrimitiveType eType)
{
	const char* szName;

	if (eType < NUM_PRIMITIVE_TYPES)
	{
		if (gbTypeNames)
		{
			szName = gcTypeNames.GetPrettyName(eType);
			if (!szName)
			{
				szName = "";
			}
			gcLogger.Error2(__METHOD__, " Classes does not contain unmanaged primitive class with ID [", IntToString(eType), "] and name [", szName, "].", NULL);
		}
		else
		{
			gcLogger.Error2(__METHOD__, " Classes does not contain unmanaged primitive class with ID [", IntToString(eType), "].", NULL);
		}
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Classes does not contain class with ID [", IntToString(eType), "].", NULL);
	}
	return;
}

