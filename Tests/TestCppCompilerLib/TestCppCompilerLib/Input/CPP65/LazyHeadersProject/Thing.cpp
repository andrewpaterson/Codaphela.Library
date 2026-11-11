//Members defined outside the class that a forward declaration and a friend declared before it.
#include "Thing.h"


void CThing::Set(int i)
{
	mi = i;
}


int CThing::Twice(void)
{
	return mi * 2;
}


int CThing::FromHolder(CHolder* pcHolder)
{
	return pcHolder->miValue;
}
