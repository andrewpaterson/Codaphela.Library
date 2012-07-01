#ifndef __BERKELEY_DB_H__
#define __BERKELEY_DB_H__
#include <db_cxx.h>


class CBerkeleyDB
{
public:
	Db*		mpDb;

	void Init(void);
	void Kill(void);
};


#endif // __BERKELEY_DB_H__
