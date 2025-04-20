#ifndef __GERBER_COMMAND_MODE_SET_H__
#define __GERBER_COMMAND_MODE_SET_H__
#include "BaseLib/Chars.h"
#include "GerberCommand.h"


enum EGerberMeasurementMode
{
	GM_Unknown,
	GM_Millimeters,
	GM_Inches,
};


class CGerberCommandMeasurementMode : public CGerberCommand
{
protected:
	EGerberMeasurementMode meMode;

public:
	void Init(EGerberMeasurementMode eMode);
	void Kill(void);

	bool IsMeasurementMode(void);
	bool IsMillimeters(void);
	bool IsInches(void);
};


#endif // __GERBER_COMMAND_MODE_SET_H__

