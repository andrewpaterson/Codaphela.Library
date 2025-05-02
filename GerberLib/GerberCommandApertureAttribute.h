#ifndef __GERBER_COMMAND_APERTURE_ATTRIBUTE_H__
#define __GERBER_COMMAND_APERTURE_ATTRIBUTE_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayChars.h"
#include "GerberCommandAttribute.h"


class CGerberCommandApertureAttribute : public CGerberCommandAttribute
{
public:
	void	Init(size iNameLength);
	void	Kill(void);

	bool	IsApertureAttribute(void);
};


#endif // __GERBER_COMMAND_APERTURE_ATTRIBUTE_H__

