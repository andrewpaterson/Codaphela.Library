#ifndef __GERBER_COMMAND_LOAD_ROTATION_H__
#define __GERBER_COMMAND_LOAD_ROTATION_H__
#include "BaseLib/Chars.h"
#include "BaseLib/Number.h"
#include "GerberCommand.h"


class CGerberCommandLoadRotation : public CGerberCommand
{
protected:
	CNumber		mcRotation;

public:
	void		Init(CNumber* pcRotation);
	void		Kill(void);

	bool		IsLoadRotation(void);

	CNumber*	GetRotation(void);
};


#endif // __GERBER_COMMAND_LOAD_ROTATION_H__

