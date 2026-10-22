#ifndef __ABORT_VECTOR_H__
#define __ABORT_VECTOR_H__
#include "InterruptVector.h"


class CAbortVector : public CInterruptVector
{
public: 
    void    Init(void);

    uint16  GetAddress(CW65CCore* pcCPU) override;
};


#endif // __ABORT_VECTOR_H__

