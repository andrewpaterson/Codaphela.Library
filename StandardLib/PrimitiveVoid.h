#ifndef __PRIMITIVE_VOID_H__
#define __PRIMITIVE_VOID_H__
#include "BaseLib/Logger.h"
#include "Primitive.h"


class Void : public CPrimitiveObject
{
CONSTRUCTABLE(Void);
public:
	Void() : CPrimitiveObject(PT_void)
	{
	}

	BOOL Save(CFileWriter* pcFile)
	{
		return TRUE;
	}

	BOOL Load(CFileReader* pcFile)
	{
		return TRUE;
	}
};


#endif // !__PRIMITIVE_VOID_H__


