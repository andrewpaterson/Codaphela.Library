#ifndef __GERBER_COMMAND_PLOT_MODE_H__
#define __GERBER_COMMAND_PLOT_MODE_H__
#include "BaseLib/Chars.h"
#include "GerberCommand.h"


enum EGerberPlotMode
{
	GPM_Unknown,
	GPM_Linear,
	GPM_CircularClockwise,
	GPM_CircularAnticlockwise,
};


class CGerberCommandPlotMode : public CGerberCommand
{
protected:
	EGerberPlotMode meMode;

public:
	void Init(EGerberCommandEnum eType, EGerberPlotMode eMode);
	void Kill(void);

	bool IsPlotMode(void);

	bool IsLinear(void);
	bool IsCircular(void);
	bool IsClockwise(void);
	bool IsAnticlockwise(void);
};


#endif // __GERBER_COMMAND_PLOT_MODE_H__

