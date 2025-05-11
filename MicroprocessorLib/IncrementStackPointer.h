#ifndef __INCREMENT_STACK_COUNTER_H__
#define __INCREMENT_STACK_COUNTER_H__
#include "Operation.h"


class CIncrementStackPointer : public COperation
{
public:
	void	Init(void);

	void	Execute(CW65C816* pcCPU) override;

	void	Print(CChars* psz) override;
};


#endif // __INCREMENT_STACK_COUNTER_H__

