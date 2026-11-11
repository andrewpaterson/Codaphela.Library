const char* GetName(void);
int GetCount(void);
int Next(void);


static int	siCount = 7;


int Other(void)
{
	static int	si = 20;

	return si;
}


//GetName, GetCount and Next name Name.cpp's own string, static global and static local: inlined here, they must
//not become this unit's string, siCount or si.
int main(void)
{
	const char*	sz;
	const char*	szName;

	sz = "Other";
	szName = GetName();
	if ((szName[0] != 'N') || (sz[0] != 'O'))
	{
		return 1;
	}
	if (GetCount() != 5)
	{
		return 2;
	}
	if ((Next() != 10) || (Next() != 11) || (Other() != 20))
	{
		return 3;
	}
	return siCount;
}
