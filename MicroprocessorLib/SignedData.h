#ifndef __SIGNED_DATA_H__
#define __SIGNED_DATA_H__


public class SignedData
extends AddressOffset
{
    /*@Override*/
        public uint16 GetOffset(CW65C816* pcCPU)
    {
        return pcCPU->GetState().getData16BitOffset();
    }

    /*@Override*/
        public void    Print(CChars* psz)
    {
        return "R";
    }
};



#endif // __SIGNED_DATA_H__

