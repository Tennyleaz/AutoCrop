

#ifdef __cplusplus
#define XXEXPORT extern "C" __declspec (dllexport)
#else
#define XXEXPORT __declspec (dllexport)
#endif


XXEXPORT int CALLBACK autocrop(BITMAPINFOHEADER**, BITMAPINFOHEADER*,int);
XXEXPORT void CALLBACK bmpfree(BITMAPINFOHEADER** ppbmp);
XXEXPORT void CALLBACK LineFilter(BITMAPINFOHEADER* in);



