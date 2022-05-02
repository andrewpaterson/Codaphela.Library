#include "StringHelper.h"
#include "Number.h"
#include "NumberControl.h"
#include "FloatPrinter.h"


int8	giPow2DigitsToPow10Digits[64] = { 0,1,1,1,2,2,2,3,3,3,4,4,4,4,5,5,5,6,6,6,7,7,7,7,8,8,8,9,9,9,10,10,10,10,11,11,11,12,12,12,13,13,13,13,14,14,14,15,15,15,16,16,16,16,17,17,17,18,18,18,19,19,19,19 };
char	szCommonLogarithmOfTwo[] = "0.3010299956639811952137388947244930267653"; 		//X.Init(2, 2, 40) X.CommonLogarithm();


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int GetPow2DigitsToPow10Digits(int iPow2)
{
	if (iPow2 < 0)
	{
		return 0;
	}
	else if (iPow2 < 64)
	{
		return giPow2DigitsToPow10Digits[iPow2];
	}
	else
	{
		CNumber		cPow2;
		CNumber		cLog2;
		CNumber		cNum;

		cLog2.Init(szCommonLogarithmOfTwo, 2, 40);
		cPow2.Init(&cLog2);
		cNum.Init(iPow2);
		cPow2.Multiply(&cNum);
		cPow2.Ceiling();
		return cPow2.IntValue();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void FloatToString(char* szDest, int iDestLength, float f)
{
	unsigned char*	pui;
	int				iMantissa;
	int				iSign;
	int				iExponent;
	int				iValue;

	pui = (unsigned char*)&f;
	iExponent = (*((int*)pui) & 0x7f800000) >> 23;
	iMantissa = *((int*)pui) & 0x7fffff;
	iSign = (*((int*)pui) & 0x80000000) == 0x80000000;

	if (iExponent == 0)
	{

	}
	else if (iExponent == 255)
	{

	}
	else
	{
		iExponent = iExponent - 127;  //Convert by exponent bias.
		iValue = (iMantissa | 0x800000);  //Add implied [1.fraction].

		int			iWholeDigits;
		CNumber		cExponent;
		CNumber		cTwo;
		int			iLeftMost;

		cExponent.Init(iExponent);
		cTwo.Init(2, 1, 0);

		if (iExponent > 29)
		{
			CNumber*	pcResult;
			CNumber*	pcTwoPower;

			iWholeDigits = GetPow2DigitsToPow10Digits(iExponent);
			pcResult = gcNumberControl.Add(iWholeDigits, 0);
			pcTwoPower = gcNumberControl.Add(iWholeDigits, 0);

			pcTwoPower->Init(2, iWholeDigits, 0);
			pcTwoPower->Power(&cExponent);

			pcResult->Zero(iWholeDigits, 0);

			int iDigit = 0x800000;
			for (int i = 0; i < 24; i++)
			{
				if (iValue & iDigit)
				{
					pcResult->Add(pcTwoPower);
				}
				pcTwoPower->Divide(&cTwo);
				iDigit >>= 1;
			}

			iLeftMost = pcResult->GetFirstNonZeroDigit();
			pcResult->RoundSignificant(9);
			pcResult->Dump();

			gcNumberControl.Remove(2);
		}

	}
}

