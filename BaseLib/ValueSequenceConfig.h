#ifndef __VALUE_SEQUENCE_CONFIG_H__
#define __VALUE_SEQUENCE_CONFIG_H__
#include "SequenceConfig.h"


class CValueSequenceConfig : public CSequenceConfig
{
protected:
	const char* mszDirectory;

public:
	void 								Init(const char* szSubDirectory);
	void								Kill(void);

	const char*								Getdirectory(void);

public:
	static CLifeInit<CSequenceConfig>	Create(const char* szSubDirectory);
};




#endif // __VALUE_SEQUENCE_CONFIG_H__

