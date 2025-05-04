#ifndef __PROGRAM_BANK_H__
#define __PROGRAM_BANK_H__

public class ProgramBank
extends AddressOffset
{
    /*@Override*/
        public uint16 GetOffset(CW65C816* pcCPU)
    {
        return 0;
    }

    /*@Override*/
        public int GetBank(CW65C816* pcCPU)
    {
        return pcCPU->GetState().getProgramCounter().GetBank();
    }

    /*@Override*/
        public void    Print(CChars* psz)
    {
        return "PBR,";
    }
};


#endif // __PROGRAM_BANK_H__

