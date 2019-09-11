#include "stdafx.h"
#include "AutoCrop.h"
#include "autocrop_LIB.h"
#include "XBitMap.h"
#include <fstream>

// Gdi+ headers
/*#include <objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")*/

// returns: 0=file not exist or fail to read, 1=success, 2=fail to auto crop, 3=fail to save file, 4=fail to load DLL
AUTOCROP_API short AutoCrop(const WCHAR* pwstr)
{
	// https://stackoverflow.com/questions/874551/stdstring-in-c
	// https://stackoverflow.com/questions/4608876/c-sharp-dllimport-with-c-boolean-function-not-returning-correctly
	// Convert C# string to c
	std::wstring wstrSourceFile(pwstr);
	std::wcout << wstrSourceFile << std::endl;	

	// Check if file exist
	if (!exists_test1(wstrSourceFile))
		return 0;

	// Load the file
	// First we get a BITMAPINFOHEADER
	BITMAPINFOHEADER bitmapInfoHeader;
	unsigned char *bitmapData;
	bitmapData = LoadBitmapFile(wstrSourceFile.c_str(), &bitmapInfoHeader);
	if (bitmapData == nullptr)
		return 0;

	// Because LoadBitmapFile() seems not very correct, we only use its header.
	// We will get the data using GDI+
	// Initialize GDI+.
	/*GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// Read the same file again using GDI+
	Bitmap * bmp = new Bitmap(wstrSourceFile.c_str());
	if (bmp == nullptr)
		return 0;

	// Take out its raw RGB data.
	BitmapData pbitmapData;
	bmp->LockBits(&Rect(0, 0, bmp->GetWidth(), bmp->GetHeight()), ImageLockModeWrite, PixelFormat24bppRGB, &pbitmapData);
	int dataSize = bitmapInfoHeader.biSizeImage;*/

	// Make a XBitmap object for input.
	XBitmap	bmpImage = XBitmap::XBitmap();
	bmpImage.CreateImage(bitmapInfoHeader.biWidth, bitmapInfoHeader.biHeight, 24, bitmapData, bitmapInfoHeader.biSizeImage);
	//bmp->UnlockBits(&pbitmapData);
	free(bitmapData);

	// un-init GDI+
	//GdiplusShutdown(gdiplusToken);

	// Do auto chop
	int nDeskew = 1;
	BITMAPPTR bitmapptrout;
	bitmapptrout.pHeader = nullptr;
	short nCropRtn = autocrop(&(bitmapptrout.pHeader), &(bmpImage.pInfo->bmiHeader), nDeskew);
	if (nCropRtn >= 0 && bitmapptrout.pHeader != nullptr)
	{
		int BitCount = bitmapptrout.pHeader->biBitCount;
		int nColorData = (BitCount <= 8) ? 1 << BitCount : 0;
		bitmapptrout.pQuad = (RGBQUAD *)(bitmapptrout.pHeader + 1);
		bitmapptrout.pBmp = (PBYTE)(bitmapptrout.pQuad + nColorData);
		bmpImage.CopyFrom(bitmapptrout);
		bmpfree(&bitmapptrout.pHeader);

		// Save to new file name
		std::wstring szPathName = wstrSourceFile.substr(0, (wstrSourceFile.length() - 4)) + L"_a.bmp";
		bool saveRtn = SaveImage(szPathName, (void*)bmpImage.pBit, bmpImage.pInfo->bmiHeader.biWidth, bmpImage.pInfo->bmiHeader.biHeight, bmpImage.pInfo->bmiHeader.biSizeImage);
		if (saveRtn)
		{
			std::cout << "success!" << std::endl;
			return 1;
		}
		else
		{
			std::cout << "save fail!" << std::endl;
			return 3;
		}
	}
	else
	{
		std::cout << "fail!" << std::endl;
		return 2;
	}	
}

inline bool exists_test1(const std::wstring& name) {
	if (FILE *file = _wfopen(name.c_str(), L"r+")) {
		fclose(file);
		return true;
	}
	else {
		return false;
	}
}

unsigned char *LoadBitmapFile(const wchar_t *filename, BITMAPINFOHEADER *bitmapInfoHeader, bool bSwapRG)
{
	FILE *filePtr; //our file pointer
	BITMAPFILEHEADER bitmapFileHeader; //our bitmap file header
	unsigned char *bitmapImage;  //store image data
	int imageIdx = 0;  //image index counter
	unsigned char tempRGB;  //our swap variable	

							//open filename in read binary mode
	filePtr = _wfopen(filename, L"rb");
	if (filePtr == NULL)
		return NULL;

	//read the bitmap file header
	fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);

	//verify that this is a bmp file by check bitmap id
	if (bitmapFileHeader.bfType != 0x4D42)
	{
		fclose(filePtr);
		return NULL;
	}

	//read the bitmap info header
	fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);

	//check if 24-bit bmp
	if (bitmapInfoHeader->biBitCount != 24)
	{
		std::cout << "Unsupported BMP format.";
		fclose(filePtr);
		return NULL;
	}

	//move file point to the begging of bitmap data
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

	//allocate enough memory for the bitmap image data
	int wBitBerPixel = 24;
	int padding_out = (4 - (bitmapInfoHeader->biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
	// Change size and image size = ([RGB] * width + padding) * height
	bitmapInfoHeader->biSizeImage = ((sizeof(RGBTRIPLE) * bitmapInfoHeader->biWidth) + padding_out) * abs(bitmapInfoHeader->biHeight);
	bitmapImage = (unsigned char*)malloc(bitmapInfoHeader->biSizeImage);

	//verify memory allocation
	if (!bitmapImage)
	{
		free(bitmapImage);
		fclose(filePtr);
		return NULL;
	}

	//read in the bitmap image data
	fread(bitmapImage, bitmapInfoHeader->biSizeImage, 1, filePtr);

	//make sure bitmap image data was read
	if (bitmapImage == NULL)
	{
		fclose(filePtr);
		return NULL;
	}

	//swap the r and b values to get RGB (bitmap is BGR)
	if (bSwapRG)
	{
		/*for (imageIdx = 0;imageIdx < bitmapInfoHeader->biSizeImage;imageIdx += 3) // fixed semicolon
		{
			tempRGB = bitmapImage[imageIdx];
			bitmapImage[imageIdx] = bitmapImage[imageIdx + 2];
			bitmapImage[imageIdx + 2] = tempRGB;
		}*/
	}

	//close file and return bitmap iamge data
	fclose(filePtr);
	return bitmapImage;
}

// szPathName : Specifies the pathname        -> the file path to save the image
// lpBits    : Specifies the bitmap bits      -> the buffer (content of the) image
// w    : Specifies the image width
// h    : Specifies the image height
bool SaveImage(const std::wstring& szPathName, void * lpBits, long w, long h, unsigned int imageSize)
{
	// Create a new file for writing
	std::ofstream pFile(szPathName, std::ios_base::binary);
	if (!pFile.is_open())
	{
		return false;
	}

	BITMAPINFOHEADER bmih;
	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = w;
	bmih.biHeight = h;
	bmih.biPlanes = 1;
	bmih.biBitCount = 24;
	bmih.biCompression = BI_RGB;
	bmih.biSizeImage = imageSize;//w * h * 3;

	BITMAPFILEHEADER bmfh;
	int nBitsOffset = sizeof(BITMAPFILEHEADER) + bmih.biSize;
	LONG lImageSize = bmih.biSizeImage;
	LONG lFileSize = nBitsOffset + lImageSize;
	bmfh.bfType = 'B' + ('M' << 8);
	bmfh.bfOffBits = nBitsOffset;
	bmfh.bfSize = lFileSize;
	bmfh.bfReserved1 = bmfh.bfReserved2 = 0;

	// Write the bitmap file header
	pFile.write((const char*)&bmfh, sizeof(BITMAPFILEHEADER));
	UINT nWrittenFileHeaderSize = pFile.tellp();

	// And then the bitmap info header
	pFile.write((const char*)&bmih, sizeof(BITMAPINFOHEADER));
	UINT nWrittenInfoHeaderSize = pFile.tellp();

	// Finally, write the image data itself
	//-- the data represents our drawing
	pFile.write((const char*)lpBits, imageSize);
	UINT nWrittenDIBDataSize = pFile.tellp();
	pFile.close();

	return true;
}