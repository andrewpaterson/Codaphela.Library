#ifndef __CONSTANT_OFFSET_H__
#define __CONSTANT_OFFSET_H__

public:
class ConstantOffset
extends AddressOffset
{
    private final int offset;

    public:
ConstantOffset(int offset)
    {
        if (offset < 0 || offset > 3)
        {
            throw new SimulatorException("Numeric Offset must be in the range 0...3.");
        }
        this->offset = offset;
    }

    /*@Override*/
        public:
uint16 GetOffset(CW65C816* pcCPU)
    {
        return offset;
    }

    /*@Override*/
        public:
void    Print(CChars* psz)
    {
        if (offset >= 0)
        {
            return "+" + offset;
        }
        else
        {
            return "" + offset;
        }
    }
};


#endif // __CONSTANT_OFFSET_H__

