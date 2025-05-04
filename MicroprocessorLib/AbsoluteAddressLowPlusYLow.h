#ifndef __ABSOLUTE_ADDRESS_LOW_PLUS_Y_LOW_H__
#define __ABSOLUTE_ADDRESS_LOW_PLUS_Y_LOW_H__


class AbsoluteAddressLowPlusYLow
extends AddressOffset
{
    /*@Override*/
        public:
uint16 GetOffset(CW65C816* pcCPU)
    {
        return pcCPU->GetState().GetAddressOffsetY();
    }

    /*@Override*/
        public:
void    Print(CChars* psz)
    {
        return "(AAL+YL)";
    }
};


#endif // __ABSOLUTE_ADDRESS_LOW_PLUS_Y_LOW_H__

