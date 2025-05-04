#ifndef __ADDRESS_BANK_H__
#define __ADDRESS_BANK_H__

public:
class AddressBank
extends AddressOffset
{
public:
    uint16 GetOffset(CW65C816* pcCPU) override;
    {
        return 0;
    }

    /*@Override*/
        public:
int GetBank(CW65C816* pcCPU)
    {
        return pcCPU->GetState().GetAddress().GetBank();
    }

    /*@Override*/
        public:
void    Print(CChars* psz)
    {
        return "AAB,";
    }
};


#endif // __ADDRESS_BANK_H__

