#ifndef __RESTORE_ABORT_VALUES_H__
#define __RESTORE_ABORT_VALUES_H__
#include "Operation.h"


class CRestoreAbortValues : public COperation
{
public:
	void	Init(void);
	void	Execute(CW65C816* pcCPU) override;
	void	Print(CChars* psz) override;
};


#endif // __RESTORE_ABORT_VALUES_H__

