#ifndef __SEQUENCE_FACTORY_H__
#define __SEQUENCE_FACTORY_H__
#include "Sequence.h"


class CSequenceFactory
{
public:
	static CSequence*	Create(const char* szDirectory, EDurableSync eSync, size uiBlockSize = SEQUENCE_BLOCK_SIZE);
};


#endif // __SEQUENCE_FACTORY_H__

