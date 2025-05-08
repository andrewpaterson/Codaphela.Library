#ifndef __BCD_RESULT_H__
#define __BCD_RESULT_H__
#include "BaseLib/PrimitiveTypes.h"


class CBCDResult
{
public:
    uint16  muiValue;
    bool    mbCarry;

    void Init(uint16 uiValue, bool bCarry);
};


#endif // __BCD_RESULT_H__

