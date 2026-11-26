#ifndef __DATA_OPERATION_H__
#define __DATA_OPERATION_H__
#include "W65Operation.h"
#include "NotMemoryLock.h"


class CW65DataOperation : public CW65Operation
{
CONSTRUCTABLE(CW65DataOperation);
protected:
	bool	mbValidProgramAddress;
	bool	mbValidDataAddress;
	NotMemoryLock	mbNotMemoryLock;
	bool	mbRead;
	bool	mbNotVectorPull;
	bool	mbReady;

public:
	void Init(bool bValidProgramAddress, bool bValidDataAddress, NotMemoryLock bNotMemoryLock, bool bRead, bool bNotVectorPull);

	bool	IsData(void) const override;
	bool	IsRead(void) const;
	bool	IsValidProgramAddress(void) const;
	bool	IsValidDataAddress(void) const;
	bool	IsNotMemoryLock(void) const;
	bool	IsNotVectorPull(void) const;
	bool	IsReady(void) const;
	virtual bool	IsFetchOpcode(void) const;
};


#endif // __DATA_OPERATION_H__

