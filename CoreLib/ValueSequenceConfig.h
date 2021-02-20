#ifndef __VALUE_SEQUENCE_CONFIG_H__
#define __VALUE_SEQUENCE_CONFIG_H__
#include "SequenceConfig.h"


class CValueSequenceConfig : public CSequenceConfig
{
protected:
	char* mszDirectory;

public:
	void 								Init(char* szSubDirectory);
	void								Kill(void);

	char*								Getdirectory(void);

public:
	static CLifeInit<CSequenceConfig>	Create(char* szSubDirectory);
};




#endif // __VALUE_SEQUENCE_CONFIG_H__

