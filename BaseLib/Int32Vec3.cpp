#include "Numbers.h"
#include "Int32Vec3.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SInt32Vec3::Print(CChars* psx, int iWholeNumbers)
{
	CChars		szzz;
	int			iWidth;

	if (iWholeNumbers == -1)
	{
		iWholeNumbers = WholeNumbers();
		iWholeNumbers++;
		if (iWholeNumbers < 2)
		{
			iWholeNumbers = 2;
		}
	}

	iWidth = iWholeNumbers + 1;

	psx->Append('[');
	szzz.Init();
	szzz.Append(x);
	szzz.RightAlign(' ', iWidth);
	psx->Append(szzz);
	szzz.Kill();
	psx->Append(',');
	szzz.Init();
	szzz.Append(y);
	szzz.RightAlign(' ', iWidth);
	psx->Append(szzz);
	szzz.Kill();
	psx->Append(',');
	szzz.Init();
	szzz.Append(z);
	szzz.RightAlign(' ', iWidth);
	psx->Append(szzz);
	szzz.Kill();
	psx->Append(']');
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int SInt32Vec3::WholeNumbers(void)
{
	int i[3];

	i[0] = CountDigits(x);
	i[1] = CountDigits(y);
	i[2] = CountDigits(z);

	return LargestInt(i, 3);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 Int32Vec3Dot(const SInt32Vec3* pV1, const SInt32Vec3* pV2)
{
	return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec3* Int32Vec3Cross(SInt32Vec3* pOut, const SInt32Vec3* pV1, const SInt32Vec3* pV2)
{
	SInt32Vec3 v;

	v.x = pV1->y * pV2->z - pV1->z * pV2->y;
	v.y = pV1->z * pV2->x - pV1->x * pV2->z;
	v.z = pV1->x * pV2->y - pV1->y * pV2->x;

	*pOut = v;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec3* Int32Vec3Add(SInt32Vec3* pOut, const SInt32Vec3* pV1, const SInt32Vec3* pV2)
{
	pOut->x = pV1->x + pV2->x;
	pOut->y = pV1->y + pV2->y;
	pOut->z = pV1->z + pV2->z;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec3* Int32Vec3Subtract(SInt32Vec3* pOut, const SInt32Vec3* pV1, const SInt32Vec3* pV2)
{
	pOut->x = pV1->x - pV2->x;
	pOut->y = pV1->y - pV2->y;
	pOut->z = pV1->z - pV2->z;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec3* Int32Vec3Minimize(SInt32Vec3* pOut, const SInt32Vec3* pV1, const SInt32Vec3* pV2)
{
	pOut->x = pV1->x < pV2->x ? pV1->x : pV2->x;
	pOut->y = pV1->y < pV2->y ? pV1->y : pV2->y;
	pOut->z = pV1->z < pV2->z ? pV1->z : pV2->z;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec3* Int32Vec3Maximize(SInt32Vec3* pOut, const SInt32Vec3* pV1, const SInt32Vec3* pV2)
{
	pOut->x = pV1->x > pV2->x ? pV1->x : pV2->x;
	pOut->y = pV1->y > pV2->y ? pV1->y : pV2->y;
	pOut->z = pV1->z > pV2->z ? pV1->z : pV2->z;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec3* Int32Vec3Scale(SInt32Vec3 *pOut, const SInt32Vec3 *pV, float32 s)
{
	pOut->x = (int32)(pV->x * s);
	pOut->y = (int32)(pV->y * s);
	pOut->z = (int32)(pV->z * s);
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec3* Int32Vec3Lerp(SInt32Vec3 *pOut, const SInt32Vec3 *pV1, const SInt32Vec3 *pV2, float32 s)
{
	pOut->x = (int32)(pV1->x + s * (pV2->x - pV1->x));
	pOut->y = (int32)(pV1->y + s * (pV2->y - pV1->y));
	pOut->z = (int32)(pV1->z + s * (pV2->z - pV1->z));
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Int32Vec3Swap(SInt32Vec3* ps1, SInt32Vec3* ps2)
{
	SInt32Vec3 temp;

	temp = *ps2;
	*ps2 = *ps1;
	*ps1 = temp;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Int32Vec3MinMax(SInt32Vec3* psMin, SInt32Vec3* psMax, SInt32Vec3* asIn, int iInStride, int iNumPoints)
{
	int			i;
	SInt32Vec3*	psIn;

	if (iNumPoints > 0)
	{
		psIn = asIn;
		*psMin = *psIn;
		*psMax = *psIn;

		for (i = 1; i < iNumPoints; i++)
		{
			psIn = (SInt32Vec3*)RemapSinglePointer(asIn, i * iInStride);

			if (psIn->x < psMin->x)
			{
				psMin->x = psIn->x;
			}
			if (psIn->y < psMin->y)
			{
				psMin->y = psIn->y;
			}
			if (psIn->z < psMin->z)
			{
				psMin->z = psIn->z;
			}

			if (psIn->x > psMax->x)
			{
				psMax->x = psIn->x;
			}
			if (psIn->y > psMax->y)
			{
				psMax->y = psIn->y;
			}
			if (psIn->z > psMax->z)
			{
				psMax->z = psIn->z;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Int32Vec3InterpolatePosition(SInt32Vec3* psVecDest, const SInt32Vec3* psVec1, const SInt32Vec3* psVec2, float32 fWeight)
{
	SInt32Vec3	sVec1;
	SInt32Vec3	sVec2;

	Int32Vec3Scale(&sVec1, psVec1, fWeight);
	Int32Vec3Scale(&sVec2, psVec2, 1.0f - fWeight);
	Int32Vec3Add(psVecDest, &sVec1, &sVec2);
}

