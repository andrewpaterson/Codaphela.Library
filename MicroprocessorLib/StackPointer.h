#ifndef __STACK_POINTER_H__
#define __STACK_POINTER_H__


public class StackPointer
extends AddressOffset
{
    /*@Override*/
        public uint16 GetOffset(CW65C816* pcCPU)
    {
        return pcCPU->GetState().getStackPointer();
    }

    /*@Override*/
        public void    Print(CChars* psz)
    {
        return "S";
    }
}


#endif // __STACK_POINTER_H__

