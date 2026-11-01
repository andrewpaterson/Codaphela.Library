#ifndef __SORTED_TEST_OBJECT_H__
#define __SORTED_TEST_OBJECT_H__


class CSortedTestObject
{
public:
	int		miValue;
	char	mszThreeChars[4];

	void Init(int iValue, const char* szThreeChars);
};


int CompareSortedTestObject(const void* arg1, const void* arg2);


#endif // __SORTED_TEST_OBJECT_H__

