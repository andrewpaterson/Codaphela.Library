#ifndef __DIRECT_PAGE_H__
#define __DIRECT_PAGE_H__


public:
class DirectPage
extends AddressOffset
{
    /*@Override*/
        public:
uint16 GetOffset(CW65C816* pcCPU)
    {
        return pcCPU->GetState().getDirectPage();
    }

    /*@Override*/
        public:
void    Print(CChars* psz)
    {
        return "D";
    }
};


#endif // __DIRECT_PAGE_H__

