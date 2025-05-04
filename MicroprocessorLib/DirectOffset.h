#ifndef __DIRECT_OFFSET_H__
#define __DIRECT_OFFSET_H__


public:
class DirectOffset
extends AddressOffset
{
    /*@Override*/
        public:
uint16 GetOffset(CW65C816* pcCPU)
    {
        return pcCPU->GetState().getDirectOffset();
    }

    /*@Override*/
        public:
void    Print(CChars* psz)
    {
        return "D0";
    }
};


#endif // __DIRECT_OFFSET_H__