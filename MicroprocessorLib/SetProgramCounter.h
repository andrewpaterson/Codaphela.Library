#ifndef __SET_PROGRAM_COUNTER_H__
#define __SET_PROGRAM_COUNTER_H__
#include "Operation.h"
#include "AddressOffsetArray.h"


class CSetProgramCounter : public COperation
{
protected:
	CAddressOffsetArray     mapcAddressOffsets;

public:
	void	Init(CAddressOffsetArray* papcAddressOffsets);
	void	Execute(CW65C816* pcCPU) override;
	void	Print(CChars* psz) override;
};


#endif // __SET_PROGRAM_COUNTER_H__

