#include "stdafx.h"
#include "XBitMap.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

XBitmap::XBitmap()
{
	pBit = NULL;
	pInfo = NULL;
}

XBitmap::~XBitmap()
{
	FreeBitmap();
}
void XBitmap::ConvertRGB(BYTE* pByte, int nSize)
{
	if (GetBitPerPixel() == 24)
	{
		//		ASSERT(nSize %3 == 0);
		int nCount = nSize / 3;
		for (int i = 0; i < nCount; i++)
		{
			BYTE bRed = *(pByte + i * 3);
			BYTE bGeen = *(pByte + i * 3 + 1);
			BYTE bBlue = *(pByte + i * 3 + 2);
			*(pByte + i * 3) = bBlue;
			*(pByte + i * 3 + 2) = bRed;
		}
	}

}
void XBitmap::PushDownTopBitDataFixAndRBChange(int nRow, BYTE* pByte, int nSize)
{
	DWORD dwBytesPerLine = GetBytesPerLine();

	if (pBit != NULL && pInfo != NULL)
	{
		int nRealRow = pInfo->bmiHeader.biHeight - 1 - nRow;
		if (nRealRow >= 0 && nRealRow * dwBytesPerLine + nSize <  pInfo->bmiHeader.biSizeImage)
		{
			ConvertRGB(pByte, nSize);
			memcpy((pBit + nRealRow * dwBytesPerLine), pByte, nSize);
		}
	}
}
void XBitmap::PushDownTopBitDataFix(int nRow, BYTE* pByte, int nSize)
{
	DWORD dwBytesPerLine = GetBytesPerLine();

	if (pBit != NULL && pInfo != NULL)
	{
		int nRealRow = pInfo->bmiHeader.biHeight - 1 - nRow;
		if (nRealRow >= 0 && nRealRow * dwBytesPerLine + nSize <  pInfo->bmiHeader.biSizeImage)
		{
			memcpy((pBit + nRealRow * dwBytesPerLine), pByte, nSize);
		}
	}
}
void XBitmap::PushDownTopBitData(int nRow, BYTE* pByte, int nSize)
{
	if (pBit != NULL && pInfo != NULL)
	{
		int nRealRow = pInfo->bmiHeader.biHeight - 1 - nRow;
		if (nRealRow >= 0 && nRealRow * pInfo->bmiHeader.biWidth * pInfo->bmiHeader.biBitCount / 8 + nSize <  pInfo->bmiHeader.biSizeImage)
		{
			memcpy((pBit + nRealRow * GetWidth() * GetBitPerPixel() / 8), pByte, nSize);
		}
	}
}
void XBitmap::PushBitData(int nRow, BYTE* pByte, int nSize)
{
	if (pBit != NULL && pInfo != NULL)
	{
		if (nRow * pInfo->bmiHeader.biWidth * pInfo->bmiHeader.biBitCount / 8 + nSize <  pInfo->bmiHeader.biSizeImage)
		{
			memcpy((pBit + nRow * pInfo->bmiHeader.biWidth * pInfo->bmiHeader.biBitCount / 8), pByte, nSize);
		}
	}
}
void XBitmap::Crop(int nWidth, int nHeight)
{

	int nColor = pInfo->bmiHeader.biBitCount / 8;
	int nNewSize = nWidth * nHeight * nColor;
	int nPicHeight = pInfo->bmiHeader.biHeight;
	int nPicWidth = pInfo->bmiHeader.biWidth;
	if (nWidth > nPicWidth || nHeight > nPicHeight)
	{
		return;
	}
	BYTE* pNewBit = new BYTE[nNewSize];
	for (int i = 0; i < nHeight; i++)
	{
		int nPicIdx = nPicHeight - i - 1;
		BYTE* pSrc = pBit + (nPicWidth * nPicIdx) * nColor;
		int nIdx = nHeight - i - 1;
		BYTE* pTarget = pNewBit + (nWidth * nIdx) *nColor;
		memcpy(pTarget, pSrc, nWidth * nColor);
	}
	memcpy(pBit, pNewBit, nNewSize);
	delete[]pNewBit;

	pInfo->bmiHeader.biWidth = nWidth;
	pInfo->bmiHeader.biHeight = nHeight;
	pInfo->bmiHeader.biSizeImage = nNewSize;
	/*	if( pBit != NULL && pInfo != NULL )
	{
	int nWidth = pInfo->bmiHeader.biWidth;
	int nHeight = pInfo->bmiHeader.biHeight;
	int nLeft = pInfo->bmiHeader.biWidth -1 ;
	int nBtn = pInfo->bmiHeader.biHeight -1 ;
	int nColor = pInfo->bmiHeader.biBitCount / 8 ;
	if( rcCrop.left < 0 || rcCrop.top < 0 || rcCrop.bottom >  nBtn || rcCrop.left > nLeft || pBit == NULL )
	return;
	int nNewSize = rcCrop.Width() * rcCrop.Height() * nColor;
	BYTE* pNewBit = new BYTE[ nNewSize ];
	memset(pNewBit,0xFF,nNewSize);
	for( int i = 0 ; i <= nBtn ; i++)
	{
	if( i >= rcCrop.top && i < rcCrop.bottom )
	{
	int nIdx = nHeight - i - 1;
	int nIdxNew = rcCrop.bottom - rcCrop.top - (i - rcCrop.top) - 1 ;

	BYTE* pSrc = pBit + ( nWidth * nIdx + rcCrop.left ) * nColor;
	BYTE* pTar = pNewBit + nWidth *nIdxNew * nColor;
	memcpy( pTar   , pSrc, (rcCrop.right - rcCrop.left) * nColor );
	}
	}

	memcpy(pBit,pNewBit,nNewSize);
	delete []pNewBit;

	pInfo->bmiHeader.biWidth = rcCrop.Width();
	pInfo->bmiHeader.biHeight = rcCrop.Height();
	pInfo->bmiHeader.biSizeImage = nNewSize;
	}*/
}

bool XBitmap::IsValidate()
{
	if (pBit != NULL && pInfo != NULL)
	{
		return true;
	}
	else
		return false;
}

void XBitmap::CreateImage(DWORD dwWidth, DWORD dwHeight, WORD wBitBerPixel)
{
	FreeBitmap();
	DWORD dwBytesPerLine = (dwWidth * wBitBerPixel / 8 + 3) / 4 * 4;
	int nSize = dwBytesPerLine * dwHeight;
	int biClrUsed = 0;
	if (wBitBerPixel <= 1)
	{
		biClrUsed = 2;
	}
	else
	{
		if (wBitBerPixel <= 8)
		{
			biClrUsed = 256;
		}
		else
		{
			biClrUsed = 0;
		}
	}
	int nColors = biClrUsed ? biClrUsed : 1 << wBitBerPixel;

	if (nColors <= 256)
		nSize = nSize + sizeof(RGBQUAD) * nColors + sizeof(BITMAPINFOHEADER);
	else
		nSize = nSize + sizeof(BITMAPINFOHEADER);
	pInfo = (BITMAPINFO*) new  BYTE[nSize];
	if (nColors <= 256)
		pBit = ((BYTE*)pInfo) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * nColors;
	else
		pBit = ((BYTE*)pInfo) + sizeof(BITMAPINFOHEADER);
	pInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pInfo->bmiHeader.biWidth = dwWidth;
	pInfo->bmiHeader.biHeight = dwHeight;
	pInfo->bmiHeader.biPlanes = 1;
	pInfo->bmiHeader.biBitCount = wBitBerPixel;
	pInfo->bmiHeader.biCompression = 0;
	pInfo->bmiHeader.biSizeImage = dwBytesPerLine * dwHeight;
	pInfo->bmiHeader.biXPelsPerMeter = 5906;
	pInfo->bmiHeader.biYPelsPerMeter = 5906;

	if (wBitBerPixel <= 1)
	{
		pInfo->bmiHeader.biClrUsed = 0;
		for (int i = 0; i < nColors; i++)
		{
			pInfo->bmiColors[i].rgbBlue = i * 255;
			pInfo->bmiColors[i].rgbGreen = i * 255;
			pInfo->bmiColors[i].rgbRed = i * 255;
			pInfo->bmiColors[i].rgbReserved = 0;
		}

	}
	else
	{
		if (wBitBerPixel <= 8)
		{
			pInfo->bmiHeader.biClrUsed = 256;
			for (int i = 0; i < nColors; i++)
			{
				pInfo->bmiColors[i].rgbBlue = i;
				pInfo->bmiColors[i].rgbGreen = i;
				pInfo->bmiColors[i].rgbRed = i;
				pInfo->bmiColors[i].rgbReserved = 0;
			}
		}
		else
		{
			pInfo->bmiHeader.biClrUsed = 0;
		}
	}

	pInfo->bmiHeader.biClrImportant = BI_RGB;
	memset(pBit, 0xFF, pInfo->bmiHeader.biSizeImage);
}

void XBitmap::CreateImage(DWORD dwWidth, DWORD dwHeight, WORD wBitBerPixel, unsigned char * pdata, int dataSize)
{
	FreeBitmap();
	DWORD dwBytesPerLine = (dwWidth * wBitBerPixel / 8 + 3) / 4 * 4;
	int nSize = dwBytesPerLine * dwHeight;
	int biClrUsed = 0;
	if (wBitBerPixel <= 1)
	{
		biClrUsed = 2;
	}
	else
	{
		if (wBitBerPixel <= 8)
		{
			biClrUsed = 256;
		}
		else
		{
			biClrUsed = 0;
		}
	}
	int nColors = biClrUsed ? biClrUsed : 1 << wBitBerPixel;

	if (nColors <= 256)
		nSize = nSize + sizeof(RGBQUAD) * nColors + sizeof(BITMAPINFOHEADER);
	else
		nSize = nSize + sizeof(BITMAPINFOHEADER);
	pInfo = (BITMAPINFO*) new  BYTE[nSize];
	if (nColors <= 256)
		pBit = ((BYTE*)pInfo) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * nColors;
	else
		pBit = ((BYTE*)pInfo) + sizeof(BITMAPINFOHEADER);
	pInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pInfo->bmiHeader.biWidth = dwWidth;
	pInfo->bmiHeader.biHeight = dwHeight;
	pInfo->bmiHeader.biPlanes = 1;
	pInfo->bmiHeader.biBitCount = wBitBerPixel;
	pInfo->bmiHeader.biCompression = 0;
	pInfo->bmiHeader.biSizeImage = dwBytesPerLine * dwHeight;
	pInfo->bmiHeader.biXPelsPerMeter = 5906;
	pInfo->bmiHeader.biYPelsPerMeter = 5906;

	if (wBitBerPixel <= 1)
	{
		pInfo->bmiHeader.biClrUsed = 0;
		for (int i = 0; i < nColors; i++)
		{
			pInfo->bmiColors[i].rgbBlue = i * 255;
			pInfo->bmiColors[i].rgbGreen = i * 255;
			pInfo->bmiColors[i].rgbRed = i * 255;
			pInfo->bmiColors[i].rgbReserved = 0;
		}

	}
	else
	{
		if (wBitBerPixel <= 8)
		{
			pInfo->bmiHeader.biClrUsed = 256;
			for (int i = 0; i < nColors; i++)
			{
				pInfo->bmiColors[i].rgbBlue = i;
				pInfo->bmiColors[i].rgbGreen = i;
				pInfo->bmiColors[i].rgbRed = i;
				pInfo->bmiColors[i].rgbReserved = 0;
			}
		}
		else
		{
			pInfo->bmiHeader.biClrUsed = 0;
		}
	}

	pInfo->bmiHeader.biClrImportant = BI_RGB;
	memcpy(pBit, pdata, dataSize);
}

DWORD XBitmap::GetWidth()
{
	if (pInfo == 0)
		return 0;
	return pInfo->bmiHeader.biWidth;
}

DWORD XBitmap::GetHeight()
{
	if (pInfo == 0)
		return 0;
	return pInfo->bmiHeader.biHeight;
}

WORD XBitmap::GetBitPerPixel()
{
	if (pInfo == 0)
		return 0;
	return pInfo->bmiHeader.biBitCount;
}

void XBitmap::PushBitStart()
{
	m_dwCurRow = 0;
}


void XBitmap::PushBitReverse(BYTE *plBuff, DWORD dwLineCount)
{
	if (m_dwCurRow + dwLineCount > GetHeight())
		return;

	for (int i = m_dwCurRow; i < m_dwCurRow + dwLineCount; i++)
	{
		PushDownTopBitDataFix(2 * m_dwCurRow + dwLineCount - i - 1, plBuff + GetBytesPerLine() *(i - m_dwCurRow), GetBytesPerLine());
	}
	m_dwCurRow = m_dwCurRow + dwLineCount;
}

void XBitmap::PushBitForA4S(BYTE *plBuff, DWORD dwLineCount)
{
	if (m_dwCurRow + dwLineCount > GetHeight())
	{
		if (m_dwCurRow < GetHeight())
			dwLineCount = GetHeight() - m_dwCurRow;
		else
			return;
	}
	for (int i = m_dwCurRow; i < m_dwCurRow + dwLineCount; i++)
	{
		PushDownTopBitDataFixAndRBChange(i, plBuff + (i - m_dwCurRow)  * GetWidth() * GetBitPerPixel() / 8, GetBytesPerLine());
	}
	m_dwCurRow = m_dwCurRow + dwLineCount;
}
void XBitmap::PushBit(BYTE *plBuff, DWORD dwLineCount)
{
	if (m_dwCurRow + dwLineCount > GetHeight())
	{
		if (m_dwCurRow < GetHeight())
			dwLineCount = GetHeight() - m_dwCurRow;
		else
			return;
	}
	for (int i = m_dwCurRow; i < m_dwCurRow + dwLineCount; i++)
	{

		PushDownTopBitDataFix(i, plBuff + (i - m_dwCurRow)  * GetWidth() * GetBitPerPixel() / 8, GetBytesPerLine());
	}
	m_dwCurRow = m_dwCurRow + dwLineCount;
}


void XBitmap::PushBitEX(BYTE *plBuff, DWORD dwLineCount)
{
	if (m_dwCurRow + dwLineCount > GetHeight())
	{
		//		ASSERT(FALSE);
		if (m_dwCurRow < GetHeight())
			dwLineCount = GetHeight() - m_dwCurRow;
		else
			return;
	}
	for (int i = m_dwCurRow; i < m_dwCurRow + dwLineCount; i++)
	{
		INT XX = GetWidth() * GetBitPerPixel() / 8;
		INT YY = GetBytesPerLine();

		XX = (i - m_dwCurRow) * (GetWidth() * GetBitPerPixel() / 8);
		YY = (i - m_dwCurRow) *(GetWidth() * GetBitPerPixel() / 8 + 3) / 4 * 4;
		PushDownTopBitDataFix(i, plBuff + XX, GetBytesPerLine());
		//		PushDownTopBitDataFix(i,plBuff +    ( i - m_dwCurRow)  * GetWidth() * GetBitPerPixel() / 8 , GetWidth() * GetBitPerPixel() / 8);
	}
	m_dwCurRow = m_dwCurRow + dwLineCount;
}

DWORD XBitmap::GetBytesPerLine()
{
	return (GetWidth() * GetBitPerPixel() / 8 + 3) / 4 * 4;
}

void XBitmap::FreeBitmap()
{
	if (pInfo)
	{
		delete[]pInfo;
		pInfo = NULL;
		pBit = NULL;
	}
}

void XBitmap::CopyFrom(BITMAPPTR bmap)
{
	DWORD dwBytesPerLine = (bmap.pHeader->biWidth * bmap.pHeader->biBitCount / 8 + 3) / 4 * 4;
	int nSize = dwBytesPerLine * bmap.pHeader->biHeight;
	int biClrUsed = 0;
	if (bmap.pHeader->biBitCount <= 1)
	{
		biClrUsed = 2;
	}
	else
	{
		if (bmap.pHeader->biBitCount <= 8)
		{
			biClrUsed = 256;
		}
		else
		{
			biClrUsed = 0;
		}
	}
	int nColors = biClrUsed ? biClrUsed : 1 << bmap.pHeader->biBitCount;

	if (nColors <= 256)
		nSize = nSize + sizeof(RGBQUAD) * nColors + sizeof(BITMAPINFOHEADER);
	else
		nSize = nSize + sizeof(BITMAPINFOHEADER);
	this->FreeBitmap();
	pInfo = (BITMAPINFO*) new  BYTE[nSize];
	if (nColors <= 256)
		pBit = ((BYTE*)pInfo) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * nColors;
	else
		pBit = ((BYTE*)pInfo) + sizeof(BITMAPINFOHEADER);
	//	memcpy(&(pInfo->bmiHeader),bmap.pHeader,sizeof(BITMAPINFOHEADER));
	pInfo->bmiHeader.biSizeImage = dwBytesPerLine * bmap.pHeader->biHeight;
	//	memset(pBit,0xFF,pInfo->bmiHeader.biSizeImage);
	memcpy(pBit, bmap.pBmp, pInfo->bmiHeader.biSizeImage);

	pInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pInfo->bmiHeader.biWidth = bmap.pHeader->biWidth;
	pInfo->bmiHeader.biHeight = bmap.pHeader->biHeight;
	pInfo->bmiHeader.biPlanes = bmap.pHeader->biPlanes;
	pInfo->bmiHeader.biBitCount = bmap.pHeader->biBitCount;
	pInfo->bmiHeader.biCompression = 0;
	//	pInfo->bmiHeader.biSizeImage = dwBytesPerLine * dwHeight;
	pInfo->bmiHeader.biXPelsPerMeter = 5906;
	pInfo->bmiHeader.biYPelsPerMeter = 5906;

	if (bmap.pHeader->biBitCount <= 1)
	{
		pInfo->bmiHeader.biClrUsed = 0;
		for (int i = 0; i < nColors; i++)
		{
			pInfo->bmiColors[i].rgbBlue = i * 255;
			pInfo->bmiColors[i].rgbGreen = i * 255;
			pInfo->bmiColors[i].rgbRed = i * 255;
			pInfo->bmiColors[i].rgbReserved = 0;
		}
	}
	else
	{
		if (bmap.pHeader->biBitCount <= 8)
		{
			pInfo->bmiHeader.biClrUsed = 256;
			for (int i = 0; i < nColors; i++)
			{
				pInfo->bmiColors[i].rgbBlue = i;
				pInfo->bmiColors[i].rgbGreen = i;
				pInfo->bmiColors[i].rgbRed = i;
				pInfo->bmiColors[i].rgbReserved = 0;
			}
		}
		else
		{
			pInfo->bmiHeader.biClrUsed = 0;
		}
	}

	pInfo->bmiHeader.biClrImportant = BI_RGB;
}

void XBitmap::Inverse()
{
	int biClrUsed = 0;
	if (pInfo->bmiHeader.biBitCount <= 1)
	{
		biClrUsed = 2;
	}
	else
	{
		if (pInfo->bmiHeader.biBitCount <= 8)
		{
			biClrUsed = 256;
		}
		else
		{
			biClrUsed = 0;
		}
	}
	int nColors = biClrUsed ? biClrUsed : 1 << pInfo->bmiHeader.biBitCount;
	if (pInfo->bmiHeader.biBitCount == 24)
	{
		DWORD dwBytesPerLine = GetBytesPerLine();
		for (int i = 0;i < GetHeight(); i++)
		{
			BYTE* pLine = pBit + i * dwBytesPerLine;
			for (int j = 0; j< GetWidth(); j++)
			{
				BYTE* pRed = pLine + j * 3 + 0;
				BYTE* pG = pLine + j * 3 + 1;
				BYTE* pB = pLine + j * 3 + 2;
				*pRed = 0XFF - *pRed;
				*pG = 0XFF - *pG;
				*pB = 0XFF - *pB;
			}

		}
	}
	else
	{
		if (pInfo->bmiHeader.biBitCount == 8)
		{
			DWORD dwBytesPerLine = GetBytesPerLine();
			for (int i = 0;i < GetHeight(); i++)
			{
				BYTE* pLine = pBit + i * dwBytesPerLine;
				for (int j = 0; j< GetWidth(); j++)
				{
					BYTE* pColor = pLine + j;
					*pColor = 0XFF - *pColor;
				}

			}
		}
	}

}

