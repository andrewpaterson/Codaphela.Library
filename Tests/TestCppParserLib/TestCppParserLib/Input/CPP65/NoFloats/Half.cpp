//A library function with floating point inside and none in its signature: a build with no floats
//(CCPP65Builder::SetNoFloats) that uses it fails to link, naming it.
int Half(int i)
{
	return (int)((float)i * 0.5f);
}
