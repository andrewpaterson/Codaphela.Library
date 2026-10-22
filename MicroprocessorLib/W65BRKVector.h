#ifndef __BRK_VECTOR_H__
#define __BRK_VECTOR_H__
#include "W65InterruptVector.h"


class CW65BRKVector : public CW65InterruptVector
{
public:
    void    Init(void);

    uint16  GetAddress(CW65CCore* pcCPU) override;
};


#endif // __BRK_VECTOR_H__

