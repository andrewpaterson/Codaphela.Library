#ifndef __DECREMENT_STACK_POINTER_H__
#define __DECREMENT_STACK_POINTER_H__
#include "W65Operation.h"


class CW65DecrementStackPointer : public CW65Operation
{
public:
	void Init(void);

	void Execute(CW65CCore* pcCPU) override;
	bool IsInitialSide(void) override;

	void Print(CChars* psz) override;
};


#endif // __DECREMENT_STACK_POINTER_H__

