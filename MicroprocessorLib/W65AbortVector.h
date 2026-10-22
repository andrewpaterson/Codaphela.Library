#ifndef __ABORT_VECTOR_H__
#define __ABORT_VECTOR_H__
#include "W65InterruptVector.h"


class CW65AbortVector : public CW65InterruptVector
{
public: 
    void    Init(void);

    uint16  GetAddress(CW65CCore* pcCPU) override;
};


#endif // __ABORT_VECTOR_H__

