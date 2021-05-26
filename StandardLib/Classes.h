#ifndef __CLASSES_H__
#define __CLASSES_H__
#include "BaseLib/MapStringTemplate.h"
#include "BaseLib/MapIntTemplate.h"
#include "BaseLib/FreeList.h"
#include "PrimitiveClasses.h"
#include "Class.h"


typedef CMapStringTemplate<CClass*> CMapClassesByName;
typedef CMapIntTemplate<CClass*>	CMapClassesByType;


class CClasses
{
protected:
	CMapClassesByName	mmcpClassesByName;
	CMapClassesByType	mmcpClassesByType;
	CFreeList			maClasses;

	CPrimitiveClasses	mcPrimitives;

	uint32				muiCurrentClassType;

public:
	void				Init(void);
	void				Kill(void);

	CClass*				Get(char* szClassName);
	CClass*				Get(const char* szClassName);
	CClass*				Get(EPrimitiveType eType);
	CClass*				Get(uint32 iType);

	CClass*				Add(char* szClassName, uint32 uiSize, EPrimitiveType eType);
	CClass*				Add(const char* szClassName, uint32 uiSize, EPrimitiveType eType);
	CClass*				Add(char* szClassName);
	CClass*				Add(const char* szClassName);

	CPrimitiveClasses*	GetPrimitiveClasses(void);

protected:
	uint32		GetNextClassType(void);
};


#endif // __CLASSES_H__

