#ifndef __BRANCH_ADDRESS_H__
#define __BRANCH_ADDRESS_H__
#include "W65AddressOffset.h"


enum EBranchAddress
{
	BA_Origin,		//The address after the branch instruction.
	BA_PageCross,	//The page of the origin with the low byte of the target.
};


//After a branch has loaded the program counter with its target these give the addresses
//the processor puts on the bus for the extra page crossing cycle.  The offset is in DL.
class CW65BranchAddress : public CW65AddressOffset
{
protected:
	EBranchAddress	meAddress;

public:
    void    Init(void);
    void    Init(EBranchAddress eAddress);
    void    Kill(void) override;

    int32   GetOffset(CW65CCore* pcCPU) override;
    void    Print(CChars* psz) override;
};


#endif // __BRANCH_ADDRESS_H__

