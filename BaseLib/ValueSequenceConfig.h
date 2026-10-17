#ifndef __VALUE_SEQUENCE_CONFIG_H__
#define __VALUE_SEQUENCE_CONFIG_H__
#include "SequenceConfig.h"


class CValueSequenceConfig : public CSequenceConfig
{
protected:
	const char* mszDirectory;
	EDurableSync	meSync;

public:
	void 								Init(const char* szSubDirectory, EDurableSync eSync);
	void								Kill(void);

	const char*								Getdirectory(void);
	EDurableSync							GetSync(void);

public:
	static CLifeInit<CSequenceConfig>	Create(const char* szSubDirectory, EDurableSync eSync);
};




#endif // __VALUE_SEQUENCE_CONFIG_H__

