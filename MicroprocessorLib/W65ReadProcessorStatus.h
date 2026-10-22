#ifndef __READ_PROCESSOR_STATUS_H__
#define __READ_PROCESSOR_STATUS_H__
#include "W65DataOperation.h"


class CW65ReadProcessorStatus : public CW65DataOperation
{
public:
	void	Init(void);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __READ_PROCESSOR_STATUS_H__

