#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "BaseLib/Logger.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/LeakDetector.h"
#include "StandardLib/Objects.h"
#include "SupportLib/ImageChannel.h"
#include "SupportLib/ImageReader.h"
#include "SupportLib/SFTQuantiser.h"
#include "SupportLib/SFTWriter.h"


//PNGToSFT [--colours <most>] [--merge <gap>] <out.sft> <in.png> [<in.png> ...]
//
//Makes an SFT container (SupportLib's SFTCommon.h) of the pictures (PNG, BMP or any other SupportLib reads), in that
//order, with one palette for all of them before them: their own colours if there are no more than --colours (256 if it
//isn't given, 255 when any picture has a pixel whose opacity is below half, which is transparent), else that many
//chosen by median cut.  A picture with a transparent pixel is a transparent image, its runs no more than --merge
//pixels apart (0 if it isn't given) one mixed run.  The HCP65's SFTImageMake makes the images into images to blit.
#define PNG_TO_SFT_USAGE	"PNGToSFT [--colours <most>] [--merge <gap>] <out.sft> <in.png> [<in.png> ...]\n"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool Convert(char* szOutput, char** aszInputs, int iNumInputs, uint16 uiMaxColours, lsize uiMergeGap)
{
	CArrayImage		acImages;
	CSFTQuantised	cQuantised;
	Ptr<CImage>		pImage;
	bool			bResult;
	int				i;

	acImages.Init();
	bResult = true;
	for (i = 0; i < iNumInputs; i++)
	{
		pImage = ReadImage(aszInputs[i]);
		if (pImage.IsNull())
		{
			printf("%s: can't be read\n", aszInputs[i]);
			bResult = false;
			break;
		}
		acImages.Add(pImage);
	}
	if (bResult)
	{
		bResult = QuantiseSFT(&acImages, uiMaxColours, &cQuantised);
		if (!bResult)
		{
			printf("The pictures need red, green and blue, and no more than 65535 pixels across or down.\n");
		}
	}
	if (bResult)
	{
		bResult = SaveSFTQuantised(&cQuantised, szOutput, uiMergeGap);
		if (bResult)
		{
			printf("%s: %d images, %d colours from index %d\n", szOutput, (int)cQuantised.muiNumImages, (int)cQuantised.msColours.uiCount, (int)cQuantised.msColours.uiFirst);
		}
		else
		{
			printf("%s: can't be written\n", szOutput);
		}
		cQuantised.Kill();
	}
	pImage = NULL;
	acImages.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int __cdecl main(int argc, char* argv[])
{
	int		i;
	int		iColours;
	int		iMerge;
	bool	bResult;

	iColours = 0;
	iMerge = 0;
	for (i = 1; i < argc - 1; i += 2)
	{
		if (strcmp(argv[i], "--colours") == 0)
		{
			iColours = atoi(argv[i + 1]);
		}
		else if (strcmp(argv[i], "--merge") == 0)
		{
			iMerge = atoi(argv[i + 1]);
		}
		else
		{
			break;
		}
	}
	if ((argc - i < 2) || (iColours < 0) || (iColours > 256) || (iMerge < 0))
	{
		printf(PNG_TO_SFT_USAGE);
		return 1;
	}

	gcLogger.Init((const char*)NULL);
	TypesInit();
	TypeConverterInit();
	MemoryInit();
	DataIOInit();
	ObjectsInit();
	ImageChannelDescriptorInit();

	bResult = Convert(argv[i], &argv[i + 1], argc - i - 1, (uint16)iColours, (lsize)iMerge);

	ImageChannelDescriptorKill();
	ObjectsKill(false);
	DataIOKill();
	MemoryKill();
	TypeConverterKill();
	TypesKill();
	LeakDetection();
	gcLogger.Kill();
	return bResult ? 0 : 1;
}
