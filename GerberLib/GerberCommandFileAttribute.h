#ifndef __GERBER_COMMAND_FILE_ATTRIBUTE_H__
#define __GERBER_COMMAND_FILE_ATTRIBUTE_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayChars.h"
#include "GerberCommandAttribute.h"


class CGerberCommandFileAttribute : public CGerberCommandAttribute
{
public:
	void	Init(size iNameLength);
	void	Kill(void);

	bool	IsFileAttribute(void);
};


#endif // __GERBER_COMMAND_FILE_ATTRIBUTE_H__

