#ifndef __DECREMENT_STACK_POINTER_H__
#define __DECREMENT_STACK_POINTER_H__
#include "Operation.h"


class CDecrementStackPointer : public COperation
{
public:
	void Init(void);

	void Execute(CW65C816* pcCPU) override;
	bool IsInitialSide(void) override;

	void Print(CChars* psz) override;
};


#endif // __DECREMENT_STACK_POINTER_H__

