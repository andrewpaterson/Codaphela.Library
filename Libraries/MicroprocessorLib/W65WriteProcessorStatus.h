#ifndef __WRITE_PROCESSOR_STATUS_H__
#define __WRITE_PROCESSOR_STATUS_H__
#include "W65DataOperation.h"


class CW65WriteProcessorStatus : public CW65DataOperation
{
protected:
	bool	mbBreak;

public:
	void	Init(void);
	void	Init(bool bBreak);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __WRITE_PROCESSOR_STATUS_H__

