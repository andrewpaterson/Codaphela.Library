//Outer calls Twice (Last.cpp), so it can't be exported until First.cpp is compiled again with Twice inlined.
int Twice(int x);


int Outer(int x)
{
	return Twice(x) + 1;
}
