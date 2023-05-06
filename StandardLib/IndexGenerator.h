#ifndef __INDEX_GENERATOR_H__
#define __INDEX_GENERATOR_H__
#include "BaseLib/Sequence.h"
#include "BaseLib/IndexedGeneral.h"


class CIndexGenerator
{
protected:
	CSequence	mcSequence;

public:
	void	Init(CLifeInit<CSequenceConfig> cConfig);
	void	Kill(void);
	OIndex	PopIndex(void);
	OIndex	PeekIndex(void);
};


#endif // !__INDEX_GENERATOR_H__

