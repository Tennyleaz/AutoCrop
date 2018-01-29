// Test1.cpp : 定義主控台應用程式的進入點。
//

#include "stdafx.h"
#include <stdlib.h> 
#include <iostream>
//#include "Windows.h"
#include <string>
#include <vector>
#include <fstream>
#include "Wingdi.h"
#include "autocrop_LIB.h"
#include "XBitmap.h"

// Gdiplus 
#pragma comment( lib, "gdiplus.lib" ) 
#include <gdiplus.h> 

using namespace Gdiplus;
using namespace std;

unsigned char *LoadBitmapFile(char *filename, BITMAPINFOHEADER *bitmapInfoHeader, bool bSwapRG);
//bool MySaveFile(HGLOBAL hDIB, wstring pszFileName);
//bool MySaveFile(unsigned char * hDIB, wstring pszFileName, PBITMAPINFOHEADER pBitmapInfoHeader);
HGLOBAL LoadDIB(LPCTSTR sBMPFile);
bool SaveImage(const std::string& szPathName, const std::vector<char>& lpBits, long w, long h);
bool SaveImage(const std::wstring& szPathName, void * lpBits, long w, long h, unsigned int imageSize);

int main()
{
	// get the bmp header
	BITMAPINFOHEADER bitmapInfoHeader;
	unsigned char *bitmapData;
	bitmapData = LoadBitmapFile("WorldCardX150621.bmp", &bitmapInfoHeader, true);	

	// Initialize GDI+.
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;	
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// use GDI+ to get rgb array
	Bitmap * bmp = new Bitmap(L"WorldCardX150621.bmp");
	BitmapData pbitmapData;
	bmp->LockBits(&Rect(0, 0, bmp->GetWidth(), bmp->GetHeight()), ImageLockModeWrite, PixelFormat24bppRGB, &pbitmapData);
	int dataSize = bitmapInfoHeader.biSizeImage;//(pbitmapData.Stride) * pbitmapData.Height;

	// test
	wstring szorigional = L"mypic03.bmp";
	SaveImage(szorigional, pbitmapData.Scan0, bmp->GetWidth(), bmp->GetHeight(), dataSize);
	bmp->UnlockBits(&pbitmapData);

	// create custom class
	XBitmap	bmpImage = XBitmap::XBitmap();
	bmpImage.CreateImage(bitmapInfoHeader.biWidth, bitmapInfoHeader.biHeight, 24, (unsigned char *)pbitmapData.Scan0, dataSize);

	GdiplusShutdown(gdiplusToken);

	// test
	/*string szPathName = "mypic04.bmp";
	SaveImage(szPathName, (void*)bmpImage.pBit, bmpImage.pInfo->bmiHeader.biWidth, bmpImage.pInfo->bmiHeader.biHeight, dataSize);*/

	// do auto crop
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

		wstring szPathName = L"result.bmp";
		bool saveRtn = SaveImage(szPathName, (void*)bmpImage.pBit, bmpImage.pInfo->bmiHeader.biWidth, bmpImage.pInfo->bmiHeader.biHeight, bmpImage.pInfo->bmiHeader.biSizeImage);
		if (saveRtn)
			cout << "success!" << endl;
		else
			cout << "save fail!" << endl;
	}
	else
		cout << "fail!" << endl;

	free(bitmapData);
	system("pause");
    return 0;
}

unsigned char *LoadBitmapFile(char *filename, BITMAPINFOHEADER *bitmapInfoHeader, bool bSwapRG)
{
	FILE *filePtr; //our file pointer
	BITMAPFILEHEADER bitmapFileHeader; //our bitmap file header
	unsigned char *bitmapImage;  //store image data
	int imageIdx = 0;  //image index counter
	unsigned char tempRGB;  //our swap variable	

							//open filename in read binary mode
	filePtr = fopen(filename, "rb");
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

	//move file point to the begging of bitmap data
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

	//allocate enough memory for the bitmap image data
	int wBitBerPixel = 24;
	int padding_out = (4 - (bitmapInfoHeader->biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
	// Change size and image size
	bitmapInfoHeader->biSizeImage = ((sizeof(RGBTRIPLE) * bitmapInfoHeader->biWidth) + padding_out) * abs(bitmapInfoHeader->biHeight);
	DWORD dwBytesPerLine = (bitmapInfoHeader->biWidth * wBitBerPixel / 8 + 3) / 4 * 4;
	unsigned long imageSize = dwBytesPerLine * bitmapInfoHeader->biHeight;
	bitmapImage = /*new unsigned char[bitmapInfoHeader->biSizeImage];*/ (unsigned char*)malloc(bitmapInfoHeader->biSizeImage);

	//verify memory allocation
	if (!bitmapImage)
	{
		/*delete [] bitmapImage;*/ free(bitmapImage);
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
		for (imageIdx = 0;imageIdx < bitmapInfoHeader->biSizeImage;imageIdx += 3) // fixed semicolon
		{
			tempRGB = bitmapImage[imageIdx];
			bitmapImage[imageIdx] = bitmapImage[imageIdx + 2];
			bitmapImage[imageIdx + 2] = tempRGB;
		}
	}

	//close file and return bitmap iamge data
	fclose(filePtr);
	return bitmapImage;
}

// szPathName : Specifies the pathname        -> the file path to save the image
// lpBits    : Specifies the bitmap bits      -> the buffer (content of the) image
// w    : Specifies the image width
// h    : Specifies the image height
bool SaveImage(const std::string& szPathName, const std::vector<char>& lpBits, long w, long h)
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
	bmih.biSizeImage = w * h * 3;

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
	pFile.write(&lpBits[0], lpBits.size());
	UINT nWrittenDIBDataSize = pFile.tellp();
	pFile.close();

	return true;
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

bool MySaveFile(HGLOBAL hDIB, wstring pszFileName)
{
	// 來源:
	// https://www.leadtools.com/help/leadtools/v19/main/api/leadtoolsbasicdatatypes.html	

	DWORD             dwSize;
	HANDLE            filePtr;
	unsigned char*    pDIB;
	BITMAPFILEHEADER  bitmapFileHeader;
	PBITMAPINFOHEADER pBitmapInfoHeader;  // *PBITMAPINFOHEADER = BITMAPINFOHEADER
	SIZE_T            zDIBsize;

	zDIBsize = GlobalSize(hDIB);
	bitmapFileHeader.bfType = 0x4d42; // "BM" start of file; The file type; must be BM.
	bitmapFileHeader.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) + zDIBsize);  // The size, in bytes, of the bitmap file.
	bitmapFileHeader.bfReserved1 = 0;  // Reserved; must be zero.
	bitmapFileHeader.bfReserved2 = 0;  // Reserved; must be zero.
	pDIB = (unsigned char *)GlobalLock(hDIB);
	pBitmapInfoHeader = (PBITMAPINFOHEADER)pDIB;

	// Compute the offset to the array of color indices. 
	// bfOffBits: The offset, in bytes, from the beginning of the BITMAPFILEHEADER structure to the bitmap bits.
	bitmapFileHeader.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) +
		pBitmapInfoHeader->biSize + pBitmapInfoHeader->biClrUsed * sizeof(RGBQUAD);

	// Create and save file.
	filePtr = CreateFile(pszFileName.c_str(), GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (filePtr == 0)
		return false;
	WriteFile(filePtr, &bitmapFileHeader, sizeof(BITMAPFILEHEADER), &dwSize, NULL);
	WriteFile(filePtr, pDIB, (unsigned int)zDIBsize, &dwSize, NULL);
	CloseHandle(filePtr);
	GlobalUnlock(hDIB);
	return true;
}

// LoadDIBSectionFromFile - Creates a DIB section from BMP file
// lpszFileName			- Name of the BMP file
// ppvBits			- to receive address of bitmap bits
// hSection		- optional handle to a file mapping object
// dwOffset		- offset to the bitmap bit values within hSection
HBITMAP LoadDIBSectionFromFile(LPCTSTR lpszFileName, LPVOID *ppvBits,
	HANDLE hSection, DWORD dwOffset)
{
	LPVOID lpBits;
	CFile file;
	if (!file.Open(lpszFileName, CFile::modeRead))
		return NULL;

	BITMAPFILEHEADER bmfHeader;
	long nFileLen;

	nFileLen = file.GetLength();

	// Read file header
	if (file.Read((LPSTR)&bmfHeader, sizeof(bmfHeader)) != sizeof(bmfHeader))
		return NULL;

	// File type should be 'BM'
	if (bmfHeader.bfType != ((WORD)('M' << 8) | 'B'))
		return NULL;

	BITMAPINFO *pbmInfo;
	pbmInfo = (BITMAPINFO *)::GlobalAlloc(GMEM_FIXED,
		sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 256);
	if (pbmInfo == NULL)
		return NULL;

	// Read the BITMAPINFO
	file.Read(pbmInfo, sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 256);

	BITMAPINFO &bmInfo = *pbmInfo;

	HBITMAP hBmp = CreateDIBSection(NULL, pbmInfo, DIB_RGB_COLORS, &lpBits,
		hSection, dwOffset);

	LPBYTE  lpDIBBits;              // Pointer to DIB bits
	int nColors = bmInfo.bmiHeader.biClrUsed ? bmInfo.bmiHeader.biClrUsed :
		1 << bmInfo.bmiHeader.biBitCount;

	if (bmInfo.bmiHeader.biBitCount > 8)
		lpDIBBits = (LPBYTE)((LPDWORD)(bmInfo.bmiColors +
			bmInfo.bmiHeader.biClrUsed) +
			((bmInfo.bmiHeader.biCompression == BI_BITFIELDS) ? 3 : 0));
	else
		lpDIBBits = (LPBYTE)(bmInfo.bmiColors + nColors);

	int nOffset = sizeof(BITMAPFILEHEADER) + (lpDIBBits - (LPBYTE)pbmInfo);
	file.Seek(nOffset, CFile::begin);
	file.Write((LPSTR)lpBits, nFileLen - nOffset); //bmInfo.biSizeImage ); 

	::GlobalFree(pbmInfo);

	if (ppvBits)
		*ppvBits = lpBits;

	return hBmp;
}

// Simple function to load a BMP file into a DIB. This assumes that the file is smaller than UINT_MAX.
HGLOBAL LoadDIB(LPCTSTR sBMPFile)
{
	CFile file;
	if (!file.Open(sBMPFile, CFile::modeRead))
		return NULL;

	BITMAPFILEHEADER bmfHeader;
	ULONGLONG nFileLen;

	nFileLen = file.GetLength();

	if (nFileLen > UINT_MAX)
	{
		cout << "File is too big to load with a single read";
		return NULL;
	}

	// Read file header and ignore
	if (file.Read((LPSTR)&bmfHeader, sizeof(bmfHeader)) != sizeof(bmfHeader))
		return NULL;

	// File type should be ‘BM'
	if (bmfHeader.bfType != ((WORD)('M' << 8) | 'B')) return NULL;

	HGLOBAL hDIB = ::GlobalAlloc(GMEM_FIXED, (SIZE_T)nFileLen);
	if (hDIB == 0)
		return NULL;
	
	// Read the remainder of the bitmap file.
	if (file.Read((LPSTR)hDIB, (UINT)nFileLen-sizeof(BITMAPFILEHEADER)) !=
		(UINT)nFileLen-sizeof(BITMAPFILEHEADER))
	{
		::GlobalFree(hDIB);
		return NULL;
	}

	return hDIB;
}

/*bool MySaveFile(unsigned char * hDIB, wstring pszFileName, PBITMAPINFOHEADER pBitmapInfoHeader)
{
	// 來源:
	// https://www.leadtools.com/help/leadtools/v19/main/api/leadtoolsbasicdatatypes.html	

	DWORD             dwSize;
	HANDLE            filePtr;
	unsigned char*    pDIB;
	BITMAPFILEHEADER  bitmapFileHeader;
	//PBITMAPINFOHEADER pBitmapInfoHeader;  // *PBITMAPINFOHEADER = BITMAPINFOHEADER
	SIZE_T            zDIBsize;

	zDIBsize = GlobalSize(hDIB);
	bitmapFileHeader.bfType = 0x4d42; // "BM" start of file; The file type; must be BM.
	bitmapFileHeader.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) + zDIBsize);  // The size, in bytes, of the bitmap file.
	bitmapFileHeader.bfReserved1 = 0;  // Reserved; must be zero.
	bitmapFileHeader.bfReserved2 = 0;  // Reserved; must be zero.
	pDIB = (unsigned char *)GlobalLock(hDIB);
	pBitmapInfoHeader = (PBITMAPINFOHEADER)pDIB;

	// Compute the offset to the array of color indices. 
	// bfOffBits: The offset, in bytes, from the beginning of the BITMAPFILEHEADER structure to the bitmap bits.
	bitmapFileHeader.bfOffBits = 
		(DWORD) sizeof(BITMAPFILEHEADER) +
		pBitmapInfoHeader->biSize + 
		pBitmapInfoHeader->biClrUsed * sizeof(RGBQUAD);

	// Create and save file.
	filePtr = CreateFile(pszFileName.c_str(), GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (filePtr == 0)
		return false;
	WriteFile(filePtr, &bitmapFileHeader, sizeof(BITMAPFILEHEADER), &dwSize, NULL);
	WriteFile(filePtr, pDIB, (unsigned int)zDIBsize, &dwSize, NULL);
	CloseHandle(filePtr);
	GlobalUnlock(hDIB);
	return true;
}*/

