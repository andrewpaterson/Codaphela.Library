#ifndef __EXECUTE_IF_16_BIT_H__
#define __EXECUTE_IF_16_BIT_H__
#include "Operation.h"


class CExecuteIf16Bit : public COperation
{
public:
	void Execute(CW65C816* pcCPU) override;
	void Print(CChars* psz) override;
};


#endif // __EXECUTE_IF_16_BIT_H__

