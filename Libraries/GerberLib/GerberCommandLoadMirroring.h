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
	void	Init(EGerberMirroring eMirroring);
	void	Kill(void);

	bool IsLoadMirroring(void) const;

	bool	IsNone(void) const;
	bool	IsX(void) const;
	bool	IsY(void) const;
	bool	IsXY(void) const;

	EGerberMirroring	GetMirroring(void) const;
};


#endif // __GERBER_COMMAND_LOAD_MIRRORING_H__

