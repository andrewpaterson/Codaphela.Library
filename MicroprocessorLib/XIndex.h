#ifndef __X_INDEX_H__
#define __X_INDEX_H__


public class XIndex
extends AddressOffset
{
    /*@Override*/
        public uint16 GetOffset(CW65C816* pcCPU)
    {
        return pcCPU->GetState().getX();
    }

    /*@Override*/
        public void    Print(CChars* psz)
    {
        return "X";
    }
}



#endif // __X_INDEX_H__
