#ifndef __BUS_CYCLE_H__
#define __BUS_CYCLE_H__
#include "BaseLib/Logger.h"
#include "BaseLib/ArrayTemplateEmbeddedPtr.h"
#include "ArrayOwnedPtr.h"
#include "StandardLib/Unknown.h"
#include "W65Address.h"
#include "W65AddressOffsetArray.h"
#include "W65Operation.h"
#include "W65DataOperation.h"


class CW65CCore;
class CW65BusCycle : public CUnknown
{
protected:
    CW65AddressOffsetArray     mapcAddressOffsets;
    CW65OperationArray         mapcOperations;
    CArrayTemplateEmbeddedPtr<CW65Operation, 2>  mapcConditions;  //The operations that can skip this cycle, usually none.
    CW65DataOperation*         mpcDataOperation;
    int16                   miCycle;

public:
    void                Init(CW65AddressOffsetArray* papcAddressOffsets, CW65OperationArray* papcOperations);
    void                Kill(void);

    void                GetAddress(CW65Address* pcDest, CW65CCore* pcCPU);
    CW65DataOperation*     GetDataOperation(void);
    size                GetDone8(void);
    size                GetDone16(void);
    bool                MustExecute(CW65CCore* pcCPU);
    void                ToAddressOffsetString(CChars* psz);
    void                ToOperationString(CChars* psz);
    void                ToDataString(CChars* psz);
    bool                IsFetchOpcode();
    bool                IsProgramAddress(void);
    void                SetCycle(int16 iCycle);
    int16               GetCycle(void);
    CW65OperationArray*    GetOperations(void);

private:
    void Validate(void);
};


typedef CArrayOwnedPtr<CW65BusCycle>	CW65BusCycleArray;


#endif // __BUS_CYCLE_H__

