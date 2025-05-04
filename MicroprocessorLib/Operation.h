#ifndef __OPERATION_H__
#define __OPERATION_H__
#include "BusCycleParameter.h"

class CW65C816;
class COperation : public CBusCycleParameter
{
public:
    virtual void    Execute(CW65C816* pcCPU) =0;

            bool    IsOperation(void) override;
    virtual bool    MustExecute(CW65C816* pcCPU);
    virtual bool    IsAddress(void) override;
    virtual bool    IsData(void);
    virtual size    GetDone8(void);
    virtual size    GetDone16(void);
};


#endif // __OPERATION_H__

