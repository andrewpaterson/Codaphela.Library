#include <windows.h>
#include <GdiPlus.h>
#include <GdiPlusHeaders.h>
#include "GDIPlusWriter.h"


using namespace Gdiplus;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool GetEncoderClsid(EImageType eImageType, CLSID* psID)
{
	OLECHAR				szID[MAX_PATH];
	bool				bFound;
	UINT 				num;    // number of image encoders
	UINT 				size;   // size, in bytes, of the image encoder array
	ImageCodecInfo* pImageCodecInfo;

	// How many encoders are there?
	// How big (in bytes) is the array of all ImageCodecInfo obects?
	GetImageEncodersSize(&num, &size);

	// Create a buffer large enough to hold the array of ImageCodecInfo
	// objects that will be returned by GetImageEncoders.
	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));

	// GetImageEncoders creates an array of ImageCodecInfo objects
	// and copies that array into a previously allocated buffer. 
	// The third argument, imageCodecInfos, is a pointer to that buffer. 
	GetImageEncoders(num, size, pImageCodecInfo);

	memset(psID, 0, sizeof(CLSID));
	//These are the only types that can be written.  Stop looking for a tga writer.
	switch (eImageType)
	{
	case IT_BMP:
		wcscpy(szID, L"image/bmp");
		break;
	case IT_PNG:
		wcscpy(szID, L"image/png");
		break;
	case IT_JPG:
		wcscpy(szID, L"image/jpeg");
		break;
	case IT_GIF:
		wcscpy(szID, L"image/gif");
		break;
	case IT_TIFF:
		wcscpy(szID, L"image/tiff");
		break;
	default:
		wcscpy(szID, L"NoTFouNd");
	}

	// Display the graphics file format (MimeType)
	// for each ImageCodecInfo object.
	bFound = false;
	for (UINT j = 0; j < num; ++j)
	{
		if (_wcsicmp(szID, pImageCodecInfo[j].MimeType) == 0)
		{
			memcpy(psID, &pImageCodecInfo[j].Clsid, sizeof(CLSID));
			bFound = true;
			break;
		}
	}

	free(pImageCodecInfo);
	return bFound;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void WriteGDIPlusImage(Ptr<CImage> pcImage, char* szFilename, EImageType eImageType)
{
	// Initialize GDI+.
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	BitmapData			bitmapData;
	char* pixels;
	int					iFormat;
	CLSID				Clsid;
	OLECHAR				wszPath[MAX_PATH + 1];
	CImage				cImageExport;
	Status				sResult;
	CImageCopier		cCopier;

	if (GetEncoderClsid(eImageType, &Clsid))
	{
		MultiByteToWideChar(CP_ACP, 0, szFilename, -1, wszPath, MAX_PATH);

		iFormat = PixelFormatMax;
		if (pcImage->HasChannels(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_ZERO))
		{
			iFormat = PixelFormat32bppARGB;
			cImageExport.Init(pcImage->GetWidth(), pcImage->GetHeight(), (void*)1, PT_uint8, IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, IMAGE_OPACITY, CHANNEL_ZERO);
		}
		else if (pcImage->HasChannels(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_ZERO))
		{
			iFormat = PixelFormat24bppRGB;
			cImageExport.Init(pcImage->GetWidth(), pcImage->GetHeight(), (void*)1, PT_uint8, IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, CHANNEL_ZERO);
		}
		else if (pcImage->HasChannels(IMAGE_DIFFUSE_GREY, CHANNEL_ZERO))
		{
			iFormat = PixelFormat16bppGrayScale;
			cImageExport.Init(pcImage->GetWidth(), pcImage->GetHeight(), (void*)1, PT_uint16, IMAGE_DIFFUSE_GREY, CHANNEL_ZERO);
		}
		else
		{
			goto Shutdown;
		}

		Bitmap bitmap(pcImage->GetWidth(), pcImage->GetHeight(), iFormat);

		sResult = bitmap.LockBits(NULL, Gdiplus::ImageLockModeWrite, iFormat, &bitmapData);
		pixels = (char*)bitmapData.Scan0;

		cImageExport.SetData(pixels);
		cCopier.Init(pcImage, &cImageExport);
		cCopier.Copy(0, 0);
		cCopier.Kill();

		cImageExport.Kill();

		sResult = bitmap.UnlockBits(&bitmapData);
		sResult = bitmap.Save(wszPath, &Clsid, NULL);
	}

	//This must be the absolute last line...  not even a return can come after it :(
Shutdown:
	GdiplusShutdown(gdiplusToken);
}

