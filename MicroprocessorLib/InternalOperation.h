#ifndef __INTERNAL_OPERATION_H__
#define __INTERNAL_OPERATION_H__
#include "DataOperation.h"


class CInternalOperation : public CDataOperation
{
public:
	void Execute(CW65C816* pcCPU) override;
	void Print(CChars* psz) override;
};


#endif // __INTERNAL_OPERATION_H__

