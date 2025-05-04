#ifndef __DATA_OPERATION_H__
#define __DATA_OPERATION_H__
#include "Operation.h"


class CDataOperation : public COperation
{
protected:
    bool mbValidProgramAddress;
    bool mbValidDataAddress;
    bool mbNotMemoryLock;
    bool mbRead;
    bool mbNotVectorPull;
    bool mbReady;

public:
            void Init(bool bValidProgramAddress, bool bValidDataAddress, bool bNotMemoryLock, bool bRead, bool bNotVectorPull);
            bool IsData(void) override;
            bool IsRead(void);
            bool IsValidProgramAddress(void);
            bool IsValidDataAddress(void);
            bool IsNotMemoryLock(void);
            bool IsNotVectorPull(void);
            bool IsReady(void);
    virtual bool IsFetchOpCode(void);
};


#endif // __DATA_OPERATION_H__

