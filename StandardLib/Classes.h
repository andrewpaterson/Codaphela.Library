#ifndef __CLASSES_H__
#define __CLASSES_H__
#include "BaseLib/MapStringTemplate.h"
#include "BaseLib/MapIntTemplate.h"
#include "BaseLib/FreeList.h"
#include "UnmanagedClasses.h"
#include "Class.h"


typedef CMapStringTemplate<CClass*> CMapClassesByName;
typedef CMapIntTemplate<CClass*>	CMapClassesByType;


class CClasses
{
protected:
	CMapClassesByName	mmcpClassesByName;
	CMapClassesByType	mmcpClassesByType;
	CFreeList			maClasses;

	CUnmanagedClasses	mcUnmanaged;

	uint32				muiCurrentClassType;

	CClass*				mpcPointer;

public:
	void				Init(void);
	void				Kill(void);
	void				AddSystemClasses(void);

	CClass*				Get(char* szClassName);
	CClass*				Get(const char* szClassName);
	CClass*				Get(EPrimitiveType eType);
	CClass*				Get(uint32 iType);

	CClass*				Add(char* szClassName, uint32 uiSize, EPrimitiveType eType);
	CClass*				Add(const char* szClassName, uint32 uiSize, EPrimitiveType eType);
	CClass*				Add(char* szClassName, uint32 uiSize);
	CClass*				Add(const char* szClassName, uint32 uiSize);

	CClass*				GetPointer(void);

	CUnmanagedClasses*	GetUnmanagedClasses(void);

protected:
	uint32		GetNextClassType(void);
};


#endif // __CLASSES_H__

