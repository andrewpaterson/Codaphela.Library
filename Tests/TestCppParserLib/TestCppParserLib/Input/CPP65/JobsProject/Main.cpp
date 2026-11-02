//Calls UseOuter (Middle.cpp), only exported when Middle.cpp is compiled again.
int UseOuter(int x);


int main()
{
	return UseOuter(5);
}
