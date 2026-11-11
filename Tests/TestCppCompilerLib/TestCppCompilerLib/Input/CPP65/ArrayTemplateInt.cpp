#include "BaseLib/ArrayTemplate.h"
#include "ArrayTemplateProject/ArrayBlockStub.h"


int main()
{
	CArrayTemplate<int>	aiValues;
	int*				pi;
	int					iTotal;
	size				i;

	aiValues.Init();
	for (i = 0; i < 10; i++)
	{
		pi = aiValues.Add();
		*pi = (int)i * 3;
	}
	if (aiValues.NumElements() != 10 || aiValues.ElementSize() != sizeof(int))
	{
		return -1;
	}

	iTotal = 0;
	for (i = 0; i < aiValues.NumElements(); i++)
	{
		iTotal += *aiValues.Get(i);
	}
	aiValues[1] = 100;
	iTotal += aiValues[2] + *aiValues.Tail() + aiValues[1];
	aiValues.Kill();
	return iTotal;
}
