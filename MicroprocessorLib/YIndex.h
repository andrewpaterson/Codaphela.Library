#ifndef __Y_INDEX_H__
#define __Y_INDEX_H__


public class YIndex
extends AddressOffset
{
    /*@Override*/
        public uint16 GetOffset(CW65C816* pcCPU)
    {
        return pcCPU->GetState().getY();
    }

    /*@Override*/
        public void    Print(CChars* psz)
    {
        return "Y";
    }
}


#endif // __Y_INDEX_H__

