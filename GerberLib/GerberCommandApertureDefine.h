#ifndef __GERBER_COMMAND_APERTURE_DEFINE_H__
#define __GERBER_COMMAND_APERTURE_DEFINE_H__
#include "BaseLib/Chars.h"
#include "BaseLib/Number.h"
#include "BaseLib/ArrayTemplate.h"
#include "GerberNumbers.h"
#include "GerberCommand.h"


class CGerberCommandApertureDefine : public CGerberCommand
{
protected:
	uint32				muiDCode;
	CChars				mszTemplate;
	CArrayGerberNumber	macParameters;

public:
	void		Init(uint32 uiDCode, size iTemplateLength);
	void		Kill(void);

	bool		IsApertureDefine(void);

	uint32		GetDCode(void);
	void		SetDCode(uint32 uiDCode);
	char*		TemplateText(void);
	void		SetTemplate(const char* szTemplate);

	CNumber*	AddParameter(CNumber* pcParameter);
	size		NumParameters(void);
	CNumber*	GetParameter(size uiIndex);
};


#endif // __GERBER_COMMAND_APERTURE_DEFINE_H__

