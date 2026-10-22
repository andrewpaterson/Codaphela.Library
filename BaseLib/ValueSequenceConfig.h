#ifndef __VALUE_SEQUENCE_CONFIG_H__
#define __VALUE_SEQUENCE_CONFIG_H__
#include "SequenceConfig.h"


class CValueSequenceConfig : public CSequenceConfig
{
protected:
	const char* mszDirectory;
	EDurableSync	meSync;
	size			muiBlockSize;

public:
	void 								Init(const char* szSubDirectory, EDurableSync eSync, size uiBlockSize = SEQUENCE_BLOCK_SIZE);
	void								Kill(void);

	const char*								Getdirectory(void);
	EDurableSync							GetSync(void);
	size									GetBlockSize(void);

public:
	static CLifeInit<CSequenceConfig>	Create(const char* szSubDirectory, EDurableSync eSync, size uiBlockSize = SEQUENCE_BLOCK_SIZE);
};




#endif // __VALUE_SEQUENCE_CONFIG_H__

