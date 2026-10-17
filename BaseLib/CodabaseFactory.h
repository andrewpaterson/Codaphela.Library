#ifndef __CODABASE_FACTORY_H__
#define __CODABASE_FACTORY_H__
#include "Codabase.h"


class CCodabaseFactory
{
public:
	static CCodabase*	Create(const char* szDirectory, EIndexWriteThrough eWriteThrough, EDurableSync eSync);
};


#endif // __CODABASE_FACTORY_H__

