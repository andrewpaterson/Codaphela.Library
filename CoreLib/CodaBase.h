#ifndef __CODABASE_H__
#define __CODABASE_H__
#include "NamedIndexedData.h"


class CCodabase
{
protected:
	CNamedIndexedData			mcNamedIndexedData;
	CDurableFileController		mcFileController;

public:
	void	Init(char* szDirectory, CLifeInit<CIndexedDataConfig> cIndexConfig, CLifeInit<CNamedIndexesConfig> cNamedConfig);
	void	Kill(void);

	BOOL	ValidateConfigInitialised(void);
	BOOL	ValidateConfigKilled(void);
};


#endif // __CODABASE_H__

