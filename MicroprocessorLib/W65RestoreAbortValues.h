#ifndef __RESTORE_ABORT_VALUES_H__
#define __RESTORE_ABORT_VALUES_H__
#include "W65Operation.h"


class CW65RestoreAbortValues : public CW65Operation
{
public:
	void	Init(void);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __RESTORE_ABORT_VALUES_H__

