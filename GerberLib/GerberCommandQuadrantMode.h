#ifndef __GERBER_COMMAND_QUADRANT_MODE_H__
#define __GERBER_COMMAND_QUADRANT_MODE_H__
#include "GerberCommand.h"


enum EGerberQuadrantMode
{
	GQM_Unknown,
	GQM_Single,		//G74.  Deprecated.
	GQM_Multi,		//G75
};


class CGerberCommandQuadrantMode : public CGerberCommand
{
protected:
	EGerberQuadrantMode		meMode;

public:
	void				Init(EGerberCommandEnum eType, EGerberQuadrantMode eMode);
	void				Kill(void);

	bool				IsQuadrantMode(void);
	bool				IsSingleQuadrant(void);
	bool				IsMultiQuadrant(void);

	EGerberQuadrantMode	GetMode(void);
};


#endif // __GERBER_COMMAND_QUADRANT_MODE_H__

