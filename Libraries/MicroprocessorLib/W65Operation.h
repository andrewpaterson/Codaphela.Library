#ifndef __OPERATION_H__
#define __OPERATION_H__
#include "ArrayOwnedPtr.h"
#include "W65BusCycleParameter.h"

class CW65CCore;
class CW65Operation : public CW65BusCycleParameter
{
public:
	void    Init(void);

	virtual void    Execute(CW65CCore* pcCPU) =0;

	bool			IsOperation(void) const override;
	virtual bool	MustExecute(CW65CCore* pcCPU);
	virtual bool	IsConditional(void) const;
	virtual bool	IsAddress(void) const override;
	virtual bool	IsData(void) const;
	virtual size	GetDone8(void) const;
	virtual size	GetDone16(void) const;

	virtual bool	IsInitialSide(void) const =0;
	bool			IsTrailingSide(void);
};


typedef CArrayOwnedPtr<CW65Operation>	CW65OperationArray;


#endif // __OPERATION_H__

