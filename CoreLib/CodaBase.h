#ifndef __CODA_BASE_H__
#define __CODA_BASE_H__
#include "NamedIndexedData.h"


class CCodaBase
{
private:
	CNamedIndexedData			mcNamedIndexData;
	CDurableFileController		mcController;
	CNamedIndexedDataConfig		mcConfig;

public:
	void Init(CDurableFileController* pcController, size_t uiDataCacheSize, size_t uiIndexCacheSize, size_t uiNamedCacheSize);
	void Kill(void);
};


#endif // __CODA_BASE_H__

