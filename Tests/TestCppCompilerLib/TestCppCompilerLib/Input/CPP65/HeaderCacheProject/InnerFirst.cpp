#include "Inner.h"
#include "Cache.h"


int InnerFirst(void)
{
	return CacheScale() + Inner(1);
}
