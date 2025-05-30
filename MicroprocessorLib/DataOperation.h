#ifndef __DATA_OPERATION_H__
#define __DATA_OPERATION_H__
#include "Operation.h"
#include "NotMemoryLock.h"


class CDataOperation : public COperation
{
CONSTRUCTABLE(CDataOperation);
protected:
    bool            mbValidProgramAddress;
    bool            mbValidDataAddress;
    NotMemoryLock   mbNotMemoryLock;
    bool            mbRead;
    bool            mbNotVectorPull;
    bool            mbReady;

public:
            void Init(bool bValidProgramAddress, bool bValidDataAddress, NotMemoryLock bNotMemoryLock, bool bRead, bool bNotVectorPull);

            bool IsData(void) override;
            bool IsRead(void);
            bool IsValidProgramAddress(void);
            bool IsValidDataAddress(void);
            bool IsNotMemoryLock(void);
            bool IsNotVectorPull(void);
            bool IsReady(void);
    virtual bool IsFetchOpcode(void);
};


#endif // __DATA_OPERATION_H__

