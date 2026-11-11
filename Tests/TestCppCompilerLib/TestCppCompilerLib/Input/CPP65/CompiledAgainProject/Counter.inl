//Not inline: the unit that includes this defines them, whether it uses them or not.
void CCounter::Init(void)
{
	miCount = 0;
}


int CCounter::Next(void)
{
	miCount = Twice(miCount) + 1;
	return miCount;
}


int Thrice(int i)
{
	return Twice(i) + i;
}

