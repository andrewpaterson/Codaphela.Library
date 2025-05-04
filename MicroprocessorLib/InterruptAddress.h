#ifndef __INTERRUPT_ADDRESS_H__
#define __INTERRUPT_ADDRESS_H__



public:
class InterruptAddress
extends AddressOffset
{
    private final InterruptVector interruptVector;

    public:
InterruptAddress(InterruptVector interruptVector)
    {
        this->interruptVector = interruptVector;
    }

    /*@Override*/
        public:
uint16 GetOffset(CW65C816* pcCPU)
    {
        return interruptVector.GetAddress(cpu);
    }

    /*@Override*/
        public:
void    Print(CChars* psz)
    {
        return "VA";
    }
}



#endif // __INTERRUPT_ADDRESS_H__

