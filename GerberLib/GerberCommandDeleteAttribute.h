#ifndef __GERBER_COMMAND_DELETE_ATTRIBUTE_H__
#define __GERBER_COMMAND_DELETE_ATTRIBUTE_H__
#include "GerberCommandAttribute.h"


//An empty name deletes all attributes in the dictionary.
class CGerberCommandDeleteAttribute : public CGerberCommandAttribute
{
public:
	void	Init(size iNameLength);
	void	Kill(void);

	bool	IsDeleteAttribute(void);
	bool	IsDeleteAll(void);
};


#endif // __GERBER_COMMAND_DELETE_ATTRIBUTE_H__

