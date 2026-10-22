#ifndef __READ_DIRECT_OFFSET_H__
#define __READ_DIRECT_OFFSET_H__
#include "W65DataOperation.h"


class CW65ReadDirectOffset : public CW65DataOperation
{
public:
	void	Init(NotMemoryLock bNotMemoryLock);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __READ_DIRECT_OFFSET_H__

