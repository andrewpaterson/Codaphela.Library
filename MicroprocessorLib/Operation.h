#ifndef __OPERATION_H__
#define __OPERATION_H__
#include "BusCycleParameter.h"

class CW65CCore;
class COperation : public CBusCycleParameter
{
public:
            void    Init(void);

    virtual void    Execute(CW65CCore* pcCPU) =0;

            bool    IsOperation(void) override;
    virtual bool    MustExecute(CW65CCore* pcCPU);
    virtual bool    IsAddress(void) override;
    virtual bool    IsData(void);
    virtual size    GetDone8(void);
    virtual size    GetDone16(void);

    virtual bool    IsInitialSide(void) =0;
            bool    IsTrailingSide(void);
};


#endif // __OPERATION_H__

