//The same names as the program's, but static: each unit has its own.
static int	giValue = 20;


static int Value(void)
{
	return 2;
}


int Other(void)
{
	return Value() + giValue;
}

