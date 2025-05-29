#ifndef __BUS_CYCLE_H__
#define __BUS_CYCLE_H__
#include "BaseLib/Logger.h"
#include "StandardLib/Unknown.h"
#include "Address.h"
#include "AddressOffsetArray.h"
#include "OperationArray.h"
#include "DataOperation.h"


class CW65C816;
class CBusCycle : public CUnknown
{
protected:
    CAddressOffsetArray     mapcAddressOffsets;
    COperationArray         mapcOperations;
    CDataOperation*         mpcDataOperation;
    int16                   miCycle;

public:
    void                Init(CAddressOffsetArray* papcAddressOffsets, COperationArray* papcOperations);
    void                Kill(void);

    void                GetAddress(CAddress* pcDest, CW65C816* pcCPU);
    CDataOperation*     GetDataOperation(void);
    size                GetDone8(void);
    size                GetDone16(void);
    bool                MustExecute(CW65C816* pcCPU);
    void                ToAddressOffsetString(CChars* psz);
    void                ToOperationString(CChars* psz);
    void                ToDataString(CChars* psz);
    bool                IsFetchOpcode();
    void                SetCycle(int16 iCycle);
    int16               GetCycle(void);
    COperationArray*    GetOperations(void);

private:
    void Validate(void);
};


#endif // __BUS_CYCLE_H__

