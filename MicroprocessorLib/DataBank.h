#ifndef __DATA_BANK_H__
#define __DATA_BANK_H__


public:
class DataBank
extends AddressOffset
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
        return pcCPU->GetState().getDataBank();
    }

    /*@Override*/
        public:
void    Print(CChars* psz)
    {
        return "DBR,";
    }
};


#endif // __DATA_BANK_H__

