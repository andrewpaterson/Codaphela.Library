#ifndef __THING_H__
#define __THING_H__
#include "Forward.h"
#include "Holder.h"


class CThing
{
protected:
	int		mi;

public:
	void	Set(int i);
	int		Twice(void);
	int		FromHolder(CHolder* pcHolder);
};


//Never used: stays unparsed in the units after the first.
class CUnused
{
public:
	int		Never(void) { return 7; }
};


#endif // __THING_H__
