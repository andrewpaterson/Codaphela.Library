int Triple(int a, int b);
volatile int gi = 4;
int giPad00 = 0;
int giPad01 = 0;
int giPad02 = 0;
int giPad03 = 0;
int giPad04 = 0;
int giPad05 = 0;
int giPad06 = 0;
int giPad07 = 0;
int giPad08 = 0;
int giPad09 = 0;
int giPad10 = 0;
int giPad11 = 0;
int giPad12 = 0;
int giPad13 = 0;
int giPad14 = 0;
int giPad15 = 0;
int giPad16 = 0;
int giPad17 = 0;
int giPad18 = 0;
int giPad19 = 0;


int main(void)
{
	int r = Triple(gi, 5);
	return r;
}

int Twice(int x)
{
	int y = x + gi;
	y = Triple(y, 1);
	return y;
}
