static int	siCount = 5;


const char* GetName(void)
{
	return "Name";
}


int GetCount(void)
{
	return siCount;
}


int Next(void)
{
	static int	si = 10;

	return si++;
}
