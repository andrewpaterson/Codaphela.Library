#ifndef __READ_PROCESSOR_STATUS_H__
#define __READ_PROCESSOR_STATUS_H__
#include "DataOperation.h"


class CReadProcessorStatus : public CDataOperation
{
public:
	void Execute(CW65C816* pcCPU) override;
	void Print(CChars* psz) override;
};


#endif // __READ_PROCESSOR_STATUS_H__

