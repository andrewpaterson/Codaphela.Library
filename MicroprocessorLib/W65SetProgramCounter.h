#ifndef __SET_PROGRAM_COUNTER_H__
#define __SET_PROGRAM_COUNTER_H__
#include "W65Operation.h"
#include "W65AddressOffsetArray.h"


class CW65SetProgramCounter : public CW65Operation
{
protected:
	CW65AddressOffsetArray     mapcAddressOffsets;

public:
	void	Init(CW65AddressOffsetArray* papcAddressOffsets);
	void	Kill(void) override;

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __SET_PROGRAM_COUNTER_H__

