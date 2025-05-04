#ifndef __NEW_PROGRAM_BANK_H__
#define __NEW_PROGRAM_BANK_H__
#include "AddressOffset.h"


class CNewProgramBank : public:
    CAddressOffset
    {
        /*@Override*/
            public:
    uint16 GetOffset(CW65C816* pcCPU)
        {
            return 0;
        }

        /*@Override*/
            public:
    int GetBank(CW65C816* pcCPU)
        {
            return pcCPU->GetState().getNewProgramCounter().GetBank();
        }

        /*@Override*/
            public:
    void    Print(CChars* psz)
        {
            return "New_PBR,";
        }
    };



#endif // __NEW_PROGRAM_BANK_H__

