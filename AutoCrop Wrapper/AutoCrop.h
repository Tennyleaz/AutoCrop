#pragma once
#include "stdafx.h"

// �Q�Ϋe�m�B�z���w�q�A�w�qdllexport
#ifdef AUTOCROPWRAPPER_EXPORTS  
#define AUTOCROP_API __declspec(dllexport)   
#else  
#define AUTOCROP_API __declspec(dllimport)   
#endif  

// exported function
extern "C" AUTOCROP_API short AutoCrop(const WCHAR* wstrSourceFile);

// private utility functions
inline bool exists_test1(const std::wstring& name);
unsigned char *LoadBitmapFile(const wchar_t *filename, BITMAPINFOHEADER *bitmapInfoHeader, bool bSwapRG);
bool SaveImage(const std::wstring& szPathName, void * lpBits, long w, long h, unsigned int imageSize);