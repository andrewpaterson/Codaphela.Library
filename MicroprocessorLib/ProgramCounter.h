#ifndef __PROGRAM_COUNTER_H__
#define __PROGRAM_COUNTER_H__


public class ProgramCounter
extends AddressOffset
{
    public ProgramCounter()
    {
        super();
    }

    /*@Override*/
        public uint16 GetOffset(CW65C816* pcCPU)
    {
        return pcCPU->GetState().getProgramCounter().GetOffset();
    }

    /*@Override*/
        public void    Print(CChars* psz)
    {
        return "PC";
    }
};


#endif // __PROGRAM_COUNTER_H__

