#ifndef __NOTE_THREE_H__
#define __NOTE_THREE_H__
#include "W65Operation.h"


class CW65NoteThree : public CW65Operation
{
public:
	void	Init(void);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __NOTE_THREE_H__

