#pragma once
#include "stdafx.h"

//#include <vfw.h>
typedef RGBQUAD *PRGBQUAD;
typedef struct _BITMAPPTR
{
	// The members of the BITMAPINFOHEADER, BITMAPV4HEADER, or BITMAPV5HEADER structure 
	// specify the width and height of the bitmap, in pixels; 
	// the color format (count of color planes and color bits-per-pixel) of the display device on which the bitmap was created; 
	// whether the bitmap data was compressed before storage and the type of compression used; 
	// the number of bytes of bitmap data; the resolution of the display device on which the bitmap was created; 
	// and the number of colors represented in the data.
	PBITMAPINFOHEADER pHeader;
	// The RGBQUAD structures specify the RGB intensity values for each of the colors in the device's palette.
	RGBQUAD *pQuad;
	// The number of bits in the color-index array equals the number of pixels times the number of bits needed to index the RGBQUAD structures.
	PBYTE pBmp;
} BITMAPPTR, *PBITMAPPTR;



class XBitmap
{
public:
	void Inverse();
	void CopyFrom(BITMAPPTR bmap);
	void FreeBitmap();
	DWORD GetBytesPerLine();

	void ConvertRGB(BYTE* pByte, int nSize);
	void PushBitForA4S(BYTE* plBuff, DWORD dwLineCount);
	void PushBitReverse(BYTE *plBuff, DWORD dwLineCount);
	void PushBit(BYTE* plBuff, DWORD dwLineCount);
	void PushBitEX(BYTE* plBuff, DWORD dwLineCount);
	void PushBitStart();
	WORD GetBitPerPixel();
	DWORD GetHeight();
	DWORD GetWidth();
	bool IsValidate();
	void Crop(int nWidth, int nHeight);
	XBitmap();
	virtual ~XBitmap();
	void PushDownTopBitDataFixAndRBChange(int nRow, BYTE* pByte, int nSize);
	void PushDownTopBitDataFix(int nRow, BYTE* pByte, int nSize);
	void PushDownTopBitData(int nRow, BYTE* pByte, int nSize);
	void PushBitData(int nRow, BYTE* pByte, int nSize);
	void CreateImage(DWORD dwWidth, DWORD dwHeight, WORD wBitBerPixel);
	void CreateImage(DWORD dwWidth, DWORD dwHeight, WORD wBitBerPixel, unsigned char * pdata, int dataSize);
	//private:

	// pInfo has a BITMAPINFOHEADER, a RGBQUAD
	BITMAPINFO*	pInfo;
	BYTE*		pBit;
	DWORD		m_dwCurRow;
};