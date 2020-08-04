#ifndef __DATABASE_H__
#define __DATABASE_H__
#include "NamedIndexedData.h"


class CDatabase
{
protected:
	CNamedIndexedData			mcNamedIndexedData;
	CDurableFileController		mcFileController;
	CNamedIndexedDataConfig*	mpcConfig;

public:
	void	Init(char* szDirectory, CNamedIndexedDataConfig* pcConfig);
	void	Kill(void);
};


#endif // __DATABASE_H__

