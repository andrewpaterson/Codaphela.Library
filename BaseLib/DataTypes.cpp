#include "DataTypes.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
EPrimitiveType IntPrimitive(void)
{
	if (sizeof(int) == 2)
	{
		return PT_int16;
	}
	else if (sizeof(int) == 4)
	{
		return PT_int32;
	}
	else if (sizeof(int) == 8)
	{
		return PT_int64;
	}
	else if (sizeof(int) == 1)
	{
		return PT_int8;
	}
	return PT_Undefined;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
EPrimitiveType SizePrimitive(void)
{
	if (sizeof(size) == 2)
	{
		return PT_uint16;
	}
	else if (sizeof(size) == 4)
	{
		return PT_uint32;
	}
	else if (sizeof(size) == 8)
	{
		return PT_uint64;
	}
	return PT_Undefined;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
EPrimitiveType UIntPrimitive(void)
{
	if (sizeof(uint) == 2)
	{
		return PT_uint16;
	}
	else if (sizeof(uint) == 4)
	{
		return PT_uint32;
	}
	else if (sizeof(uint) == 8)
	{
		return PT_uint64;
	}
	else if (sizeof(uint) == 1)
	{
		return PT_uint8;
	}
	return PT_Undefined;
}

