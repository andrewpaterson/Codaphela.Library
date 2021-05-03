#ifndef __CLASSES_H__
#define __CLASSES_H__
#include "BaseLib/MapStringTemplate.h"
#include "Class.h"


typedef CMapStringTemplate<CClass> CMapClasses;


class CClasses
{
protected:
	CMapClasses		macClasses;
	CClass*			mpcVoidClass;

public:
	void Init(void);
	void Kill(void);

	CClass* Get(char* szClassName);
	CClass* Get(const char* szClassName);
	CClass* Get(EPrimitiveType eType);

	CClass* Add(char* szClassName);
	CClass* Add(const char* szClassName);

	CClass* GetVoidPtrClass(void);
};


#endif // __CLASSES_H__

