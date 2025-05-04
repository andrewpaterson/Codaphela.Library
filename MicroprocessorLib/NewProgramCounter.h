#ifndef __NEW_PROGRAM_COUNTER_H__
#define __NEW_PROGRAM_COUNTER_H__

class NewProgramCounter
extends AddressOffset
{
    /*@Override*/
        public uint16 GetOffset(CW65C816* pcCPU)
    {
        return pcCPU->GetState().getNewProgramCounter().GetOffset();
    }

    /*@Override*/
        public void    Print(CChars* psz)
    {
        return "New_PC";
    }
}


#endif // __NEW_PROGRAM_COUNTER_H__

