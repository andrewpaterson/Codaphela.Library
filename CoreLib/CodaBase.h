#ifndef __CODABASE_H__
#define __CODABASE_H__
#include "NamedIndexedData.h"


class CCodabase
{
protected:
	CNamedIndexedData			mcNamedIndexedData;
	CDurableFileController		mcFileController;
	CNamedIndexedDataConfig*	mpcConfig;

public:
	void	Init(char* szDirectory, CNamedIndexedDataConfig* pcConfig);
	void	Kill(void);
};


#endif // __CODABASE_H__

