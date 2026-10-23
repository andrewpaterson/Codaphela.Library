#ifndef __NOTE_DECIMAL_H__
#define __NOTE_DECIMAL_H__
#include "W65Operation.h"


class CW65NoteDecimal : public CW65Operation
{
public:
	void	Init(void);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	bool    MustExecute(CW65CCore* pcCPU) override;
	bool    IsConditional(void) override;
	void	Print(CChars* psz) override;
};


#endif // __NOTE_DECIMAL_H__

