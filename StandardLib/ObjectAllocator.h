#ifndef __OBJECT_ALLOCATOR_H__
#define __OBJECT_ALLOCATOR_H__
#include "Pointer.h"


class CObjects;
class CObjectAllocator
{
protected:
	CObjects*	mpcObjects;
	BOOL		mbOverwriteExisting;

public:
	void				Init(CObjects* pcObjects, BOOL bOverwriteExisting);
	void				Kill(void);

	CPointerObject		Add(char* szClassName);
	CPointerObject		Add(char* szClassName, char* szObjectName);

	CPointerObject		Add(char* szClassName, OIndex oi);
	CPointerObject		Add(char* szClassName, char* szObjectName, OIndex oi);

	CPointerObject		Add(char* szClassName, OIndex oi, OIndex* poiExisting);
	CPointerObject		Add(char* szClassName, char* szObjectName, OIndex oi, OIndex* poiExisting);
};


#endif // __OBJECT_ALLOCATOR_H__
