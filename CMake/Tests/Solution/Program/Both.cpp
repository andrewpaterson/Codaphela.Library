#include "Where.h"
#include <stdio.h>

void Say(const char* szMain) { printf("%s main, %s library, %s\n", szMain, Where(), Shared()); }
