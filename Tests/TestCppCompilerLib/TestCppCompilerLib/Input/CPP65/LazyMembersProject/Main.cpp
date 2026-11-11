#include "Box.h"
#include "Scale.h"


int FirstValue(void);
int OtherValue(void);


int main()
{
	CBox<int>	cBox;
	CCounter	cCounter;
	CFour		cFour;

	cBox.Set(100);
	cCounter.Init();
	cCounter.Next();
	return FirstValue() + OtherValue() + cBox.Add(200) + cCounter.Next() * 1000 + Double(5) * 10000 + Triple(cFour.Value()) * 100000;
}
