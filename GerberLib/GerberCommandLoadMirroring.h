#ifndef __GERBER_COMMAND_LOAD_MIRRORING_H__
#define __GERBER_COMMAND_LOAD_MIRRORING_H__
#include "BaseLib/Chars.h"
#include "GerberCommand.h"


enum EGerberMirroring
{
	GLM_Unknown,
	GLM_None,
	GLM_X,
	GLM_Y,
	GLM_XY,
};


class CGerberCommandLoadMirroring : public CGerberCommand
{
protected:
	EGerberMirroring	meMirroring;

public:
	void Init(EGerberMirroring eMirroring);
	void Kill(void);

	bool IsLoadMirroring(void);

	bool IsNone(void);
	bool IsX(void);
	bool IsY(void);
	bool IsXY(void);
};


#endif // __GERBER_COMMAND_LOAD_MIRRORING_H__

