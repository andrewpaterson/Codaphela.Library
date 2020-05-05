#ifndef __CODA_BASE_H__
#define __CODA_BASE_H__
#include "NamedIndexedData.h"


class CCodaBase
{
private:
	CNamedIndexedData	mcNamedIndexData;

public:
	void Init(void);
	void Kill(void);
};

#endif // __CODA_BASE_H__

