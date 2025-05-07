#ifndef __WRITE_DATA_HIGH_H__
#define __WRITE_DATA_HIGH_H__
#include "DataOperation.h"


class CWriteDataHigh : public CDataOperation
{
public:
	void Execute(CW65C816* pcCPU) override;
	void Print(CChars* psz) override;
};


#endif // __WRITE_DATA_HIGH_H__

