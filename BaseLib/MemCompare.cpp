#include "MemCompare.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ErrorNames(const char* szName1, const char* szName2, CChars* psz1, CChars* psz2)
{
	psz1->Append(szName1);
	psz2->Append(szName2);
	if (psz1->Length() > psz2->Length())
	{
		psz2->Append(' ', psz1->Length() - psz2->Length());
	}
	else if (psz2->Length() > psz1->Length())
	{
		psz2->Append(' ', psz2->Length() - psz1->Length());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Difference(CChars* psz1, filePos iPosition, char c)
{
	psz1->Append(" 0x");
	psz1->AppendHexHiLo(&iPosition, 8);
	psz1->Append(":[0x");
	psz1->AppendHexHiLo(&c, 1);
	psz1->Append("]");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool MemCompare(const char* szName1, const char* szName2, CChars* psz1, CChars* psz2, void* pvMem1, void* pvMem2, filePos iLength1, filePos iLength2)
{
	int				i;
	unsigned char* pcMem1;
	unsigned char* pcMem2;
	int				iCount;

	if (iLength1 != iLength2)
	{
		psz1->Append(szName1);
		psz1->Append(" size [");
		psz1->Append(iLength1);
		psz1->Append("]");

		psz2->Append(szName2);
		psz2->Append(" size [");
		psz2->Append(iLength2);
		psz2->Append("]");

		return false;
	}

	//Can't compare files that don't exist.
	if ((iLength1 == 0) && (iLength2 == 0))
	{
		return true;
	}

	pcMem1 = (unsigned char*)pvMem1;
	pcMem2 = (unsigned char*)pvMem2;
	iCount = 0;
	for (i = 0; i < iLength1; i++)
	{
		if (pcMem1[i] != pcMem2[i])
		{
			if (iCount == 0)
			{
				ErrorNames(szName1, szName2, psz1, psz2);
			}

			if (iCount < 16)
			{
				Difference(psz1, i, pcMem1[i]);
				Difference(psz2, i, pcMem2[i]);
			}

			iCount++;
		}
	}

	if (iCount >= 16)
	{
		psz1->Append(" ... ");
		psz1->Append(iCount - 16);
		psz1->Append(" more differences");

		psz2->Append(" ... ");
		psz2->Append(iCount - 16);
		psz2->Append(" more differences");
	}

	return iCount == 0;
}

