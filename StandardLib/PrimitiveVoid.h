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

	bool Save(CFileWriter* pcFile)
	{
		return true;
	}

	bool Load(CFileReader* pcFile)
	{
		return true;
	}
};


#endif // __PRIMITIVE_VOID_H__


