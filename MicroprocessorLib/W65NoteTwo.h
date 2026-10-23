#ifndef __NOTE_TWO_H__
#define __NOTE_TWO_H__
#include "W65Operation.h"


class CW65NoteTwo : public CW65Operation
{
public:
	void	Init(void);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	bool    MustExecute(CW65CCore* pcCPU) override;
	bool    IsConditional(void) override;
	void	Print(CChars* psz) override;

};


#endif // __NOTE_TWO_H__

