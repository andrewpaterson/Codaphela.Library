#ifndef __GERBER_COMMAND_LOAD_POLARITY_H__
#define __GERBER_COMMAND_LOAD_POLARITY_H__
#include "BaseLib/Chars.h"
#include "GerberCommand.h"


enum EGerberPolarity
{
	GP_Unknown,
	GP_Dark,
	GP_Clear,
};


class CGerberCommandLoadPolarity : public CGerberCommand
{
protected:
	EGerberPolarity mePolarity;

public:
	void Init(EGerberPolarity ePolarity);
	void Kill(void);

	bool IsLoadPolarity(void);

	bool IsDark(void);
	bool IsClear(void);
};


#endif // __GERBER_COMMAND_MODE_SET_H__


