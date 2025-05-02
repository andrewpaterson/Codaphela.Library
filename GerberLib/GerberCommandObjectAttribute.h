#ifndef __GERBER_COMMAND_OBJECT_ATTRIBUTE_H__
#define __GERBER_COMMAND_OBJECT_ATTRIBUTE_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayChars.h"
#include "GerberCommandAttribute.h"


class CGerberCommandObjectAttribute : public CGerberCommandAttribute
{
public:
	void	Init(size iNameLength);
	void	Kill(void);

	bool	IsObjectAttribute(void);
};


#endif // __GERBER_COMMAND_OBJECT_ATTRIBUTE_H__

