#include <stdio.h>

const char* Shared(void);

int main(void) { printf("tool, %s\n", Shared()); return 0; }
