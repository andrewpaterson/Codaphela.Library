#ifndef __CLASSES_H__
#define __CLASSES_H__
#include "BaseLib/MapStringTemplate.h"
#include "BaseLib/MapIntTemplate.h"
#include "BaseLib/FreeList.h"
#include "UnmanagedClasses.h"
#include "PrimitiveClasses.h"
#include "Class.h"


typedef CMapStringTemplate<CClass*> CMapClassesByName;
typedef CMapIntTemplate<CClass*>	CMapClassesByType;


class CObjects;
class CClasses
{
protected:
	CMapClassesByName	mmcpClassesByName;
	CMapClassesByType	mmcpClassesByType;
	CFreeList			maClasses;

	CUnmanagedClasses	mcUnmanaged;
	CPrimitiveClasses	mcPrimitive;

	uint32				muiCurrentClassType;

	CClass*				mpcPointer;

public:
	void				Init(CObjects* pcObjects);
	void				Kill(void);
	void				AddSystemClasses(void);

	CClass*				Get(char* szClassName);
	CClass*				Get(const char* szClassName);
	CClass*				Get(EPrimitiveType eType);
	CClass*				Get(uint32 iType);

	CClass*				GetValid(EPrimitiveType eType);

	CClass*				Add(char* szClassName, uint32 uiSize, EPrimitiveType eType);
	CClass*				Add(const char* szClassName, uint32 uiSize, EPrimitiveType eType);
	CClass*				Add(char* szClassName, uint32 uiSize);
	CClass*				Add(const char* szClassName, uint32 uiSize);

	CClass*				GetPointer(void);

	CUnmanagedClasses*	GetUnmanagedClasses(void);
	CPrimitiveClasses*	GetPrimitiveClasses(void);

	void				ValidateType(EPrimitiveType eType);
	template<class Class>
	void				AddConstructorAndIO(void);
	template<class Class>
	void				AddSystemClassAndConstructor(void);

protected:
	uint32				GetNextClassType(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class Class>
void CClasses::AddConstructorAndIO(void)
{			
	const char*		szClassName;
	SDataIO*		psIO;
	bool			bConstructor;
	CStackMemory<>	cStack;
	Class*			pcClass;

	pcClass = StackConstruct<Class>(&cStack);
	szClassName = pcClass->ClassName();
	cStack.Kill();

	psIO = gcDataTypesIO.GetIO(szClassName);
	if (!psIO)
	{
		gcDataTypesIO.Add<Class>();
	}

	bConstructor = gcConstructors.Contains(szClassName);
	if (!bConstructor)
	{
		gcConstructors.Add<Class>();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class Class>
void CClasses::AddSystemClassAndConstructor(void)
{
	bool			bConstructor;
	Class*			pvM;
	CClass*			pcClass;
	CStackMemory<>	cStack;
	const char*		szClassName;

	pvM = StackConstruct<Class>(&cStack);
	szClassName = pvM->ClassName();
	pcClass = pvM->CBaseObject::Class(this);
	pcClass->System();
	cStack.Kill();

	bConstructor = gcConstructors.Contains(szClassName);
	if (!bConstructor)
	{
		gcConstructors.Add<Class>();
	}
}


#endif // !__CLASSES_H__

