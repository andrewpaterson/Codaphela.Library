#ifndef __ADDRESS_OFFSET_H__
#define __ADDRESS_OFFSET_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BusCycleParameter.h"


class CAddress;
class CAddressOffset : public CBusCycleParameter
{
public:
    bool            IsOperation(void) override;
    bool            IsAddress(void) override;
    virtual uint8   GetBank(CW65C816* pcCPU) =0;
    virtual uint16  GetOffset(CW65C816* pcCPU) =0;
};


#endif // __ADDRESS_OFFSET_H__

