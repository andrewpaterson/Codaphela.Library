#ifndef __ADDRESS_OFFSET_H__
#define __ADDRESS_OFFSET_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/Chars.h"
#include "BusCycleParameter.h"


class CAddress;
class CAddressOffset : public CBusCycleParameter
{
public:
            void    Init(void);
            bool    IsOperation(void) override;
            bool    IsAddress(void) override;
    virtual uint8   GetBank(CW65C816* pcCPU);
    virtual int32   GetOffset(CW65C816* pcCPU) =0;
    virtual bool    IsConstantOffset(void);
};


#endif // __ADDRESS_OFFSET_H__

