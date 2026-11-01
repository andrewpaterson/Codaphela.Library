#ifndef __ADDRESS_OFFSET_H__
#define __ADDRESS_OFFSET_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/Chars.h"
#include "W65BusCycleParameter.h"


class CW65Address;
class CW65AddressOffset : public CW65BusCycleParameter
{
protected:
    //Set by the classes that override GetBank and IsPageWrapped so the address calculation only calls them
    //when needed.  NewMalloc zeroes these for the classes that don't.
    bool    mbHasBank;
    bool    mbCanPageWrap;

public:
            void    Init(void);

            bool    IsOperation(void) override;
            bool    IsAddress(void) override;
    virtual uint8   GetBank(CW65CCore* pcCPU);
    virtual int32   GetOffset(CW65CCore* pcCPU) =0;
    virtual bool    IsConstantOffset(void);
    virtual bool    IsPageWrapped(CW65CCore* pcCPU);
    virtual bool    IsProgramCounter(void);
            bool    HasBank(void);
            bool    CanPageWrap(void);
};


#endif // __ADDRESS_OFFSET_H__

