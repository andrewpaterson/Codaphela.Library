#ifndef __VALUE_SEQUENCE_CONFIG_H__
#define __VALUE_SEQUENCE_CONFIG_H__
#include "SequenceConfig.h"


class CValueSequenceConfig : public CSequenceConfig
{
protected:
	const char* mszDirectory;
	EDurableSync	meSync;
	size			muiBlockSize;
	OIndex			moiFirst;
	OIndex			moiEnd;

public:
	void 								Init(const char* szSubDirectory, EDurableSync eSync, size uiBlockSize = SEQUENCE_BLOCK_SIZE);
	void 								Init(const char* szSubDirectory, EDurableSync eSync, OIndex oiFirst, OIndex oiEnd, size uiBlockSize = SEQUENCE_BLOCK_SIZE);
	void								Kill(void);

	const char*								Getdirectory(void);
	EDurableSync							GetSync(void);
	size									GetBlockSize(void);
	OIndex									GetFirstIndex(void);
	OIndex									GetEndIndex(void);

public:
	static CLifeInit<CSequenceConfig>	Create(const char* szSubDirectory, EDurableSync eSync, size uiBlockSize = SEQUENCE_BLOCK_SIZE);
	static CLifeInit<CSequenceConfig>	CreateRange(const char* szSubDirectory, EDurableSync eSync, OIndex oiFirst, OIndex oiEnd, size uiBlockSize = SEQUENCE_BLOCK_SIZE);  //Hands out indices only from oiFirst up to (not including) oiEnd.
};




#endif // __VALUE_SEQUENCE_CONFIG_H__

