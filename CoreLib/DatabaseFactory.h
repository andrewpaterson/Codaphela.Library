#ifndef __DATABASE_FACTORY_H__
#define __DATABASE_FACTORY_H__
#include "Database.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
class CDatabaseFactory
{
public:
	static CDatabase*	Create(char* szDirectory, EIndexWriteThrough eWriteThrough);
};


#endif // __DATABASE_FACTORY_H__

