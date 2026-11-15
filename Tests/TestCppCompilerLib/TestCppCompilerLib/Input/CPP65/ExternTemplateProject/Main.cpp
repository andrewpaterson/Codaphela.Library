#include "Counter.h"


int main(void)
{
	CCounter<SNarrow>		cNarrow;
	CCounter<SWide>			cWide;
	CTally<int, SNarrow>	cTally;
	CCounter<SNarrow>*		pcNarrow;
	CCounter<SWide>*		pcWide;
	int						iResult;

	cNarrow.Init();
	cNarrow.Add(70);
	cNarrow.Add(70);
	cWide.Init();
	cWide.Add(700);
	cWide.Add(200);
	cTally.Init();
	cTally.mTotal = 0;
	cTally.Tally(5);
	cTally.Tally(6);
	pcNarrow = &cNarrow;
	pcWide = &cWide;

	iResult = cNarrow.Get();					//100, clamped
	iResult += cWide.Get();						//900
	iResult += cTally.mTotal * 100;				//1100
	iResult += cTally.Get() * 10;				//20
	iResult += pcNarrow->Kind();				//1
	iResult += pcWide->Kind() * 2;				//4
	iResult += cNarrow.BufferSize();			//10 + 2 + 100
	iResult += cWide.BufferSize();				//100 + 20 + 1000
	iResult += Twice(7);						//14
	iResult += Twice((short)3);					//6
	return iResult;								//3377
}

