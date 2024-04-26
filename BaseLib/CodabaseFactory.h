#ifndef __CODABASE_FACTORY_H__
#define __CODABASE_FACTORY_H__
#include "Codabase.h"


class CCodabaseFactory
{
public:
	static CCodabase*	Create(char* szDirectory, EIndexWriteThrough eWriteThrough);
};


#endif // __CODABASE_FACTORY_H__

