#ifndef __INDEX_GENERATOR_H__
#define __INDEX_GENERATOR_H__
#include "CoreLib/IndexedGeneral.h"


class CIndexGenerator
{
protected:
	OIndex	moiNext;

public:
	void	Init(void);
	void	Kill(void);
	OIndex	PopIndex(void);
	OIndex	PeekIndex(void);

protected:
	void	StepNext(void);
};


#endif // __INDEX_GENERATOR_H__

