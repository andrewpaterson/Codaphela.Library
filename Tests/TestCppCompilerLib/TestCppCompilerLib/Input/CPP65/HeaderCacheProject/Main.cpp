#include "Cache.h"


int Wide(void);
int Narrow(void);
int Narrow2(void);
int InnerFirst(void);
int Added(void);


int Inner(int x)
{
	return x + INNER_BASE;
}


int Scaled(int x)
{
	return x * CACHE_SCALE;
}


int main()
{
	//3 + 2 + 2 + (2 + 2) + 10 + 2.
	return Wide() + Narrow() + Narrow2() + InnerFirst() + Added() + Scaled(1);
}
