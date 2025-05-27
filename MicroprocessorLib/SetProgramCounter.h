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
	void	Kill(void) override;

	void	Execute(CW65C816* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __SET_PROGRAM_COUNTER_H__

