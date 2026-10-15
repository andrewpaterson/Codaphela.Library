#ifndef __SEQUENCE_FACTORY_H__
#define __SEQUENCE_FACTORY_H__
#include "Sequence.h"


class CSequenceFactory
{
public:
	static CSequence*	Create(const char* szDirectory);
};


#endif // __SEQUENCE_FACTORY_H__

