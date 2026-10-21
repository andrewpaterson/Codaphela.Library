#ifndef __GERBER_COMMAND_STEP_AND_REPEAT_H__
#define __GERBER_COMMAND_STEP_AND_REPEAT_H__
#include "BaseLib/Number.h"
#include "GerberCommandCompound.h"


//%SRX..Y..I..J..*% to %SR*%.  The contained commands are repeated X by Y times, stepped by I and J.
class CGerberCommandStepAndRepeat : public CGerberCommandCompound
{
protected:
	uint32		muiXRepeats;
	uint32		muiYRepeats;
	CNumber		mcXStep;
	CNumber		mcYStep;

public:
	void		Init(uint32 uiXRepeats, uint32 uiYRepeats, CNumber* pcXStep, CNumber* pcYStep);

	bool		IsStepAndRepeat(void);

	uint32		GetXRepeats(void);
	uint32		GetYRepeats(void);
	CNumber*	GetXStep(void);
	CNumber*	GetYStep(void);
};


#endif // __GERBER_COMMAND_STEP_AND_REPEAT_H__

