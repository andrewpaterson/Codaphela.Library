//Included first by InnerFirst.cpp, so the units that include Cache.h after it don't get it again: Cache.h depends on
//whether it was.  INNER_ADD is Added.cpp's own macro.
#pragma once
#ifndef INNER_ADD
#define INNER_ADD	0
#endif
#define INNER_BASE	(1 + INNER_ADD)


int Inner(int x);
