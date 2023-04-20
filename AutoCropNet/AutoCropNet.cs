using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;

namespace AutoCropNet
{
    internal static class AutoCropNative
    {
        private static IntPtr ptrNewImageData;

        /// <summary>
        /// 
        /// </summary>
        /// <param name="bitmap"></param>
        /// <param name="result"></param>
        /// <returns></returns>
        /// <exception cref="ArgumentNullException"></exception>
        /// <exception cref="FormatException"></exception>
        /// <exception cref="DllNotFoundException"></exception>
        public static int AutoCrop(Bitmap bitmap, out Bitmap result)
        {
            result = null;
            if (bitmap == null)
            {
                throw new ArgumentNullException(nameof(bitmap));
            }
            if (bitmap.PixelFormat != PixelFormat.Format24bppRgb)
            {
                throw new FormatException("PixelFormat is not 24 bit RGB: " + bitmap.PixelFormat);
            }

            ushort wBitBerPixel = 24;
            int dwBytesPerLine = (bitmap.Width * wBitBerPixel / 8 + 3) / 4 * 4;  // 除4乘4 因為BMP stride是4的倍數
            int nSize = dwBytesPerLine * bitmap.Height;
            nSize += Marshal.SizeOf(typeof(NativeMethods.BITMAPINFOHEADER));

            try
            {
                // create memory
                IntPtr ptrHeader = Marshal.AllocHGlobal(nSize);
                IntPtr ptrImageData = ptrHeader + Marshal.SizeOf(typeof(NativeMethods.BITMAPINFOHEADER));

                // copy header
                NativeMethods.BITMAPINFOHEADER myHeader = new NativeMethods.BITMAPINFOHEADER();
                myHeader.biSize = (uint)Marshal.SizeOf(typeof(NativeMethods.BITMAPINFOHEADER));
                myHeader.biWidth = bitmap.Width;
                myHeader.biHeight = bitmap.Height;
                myHeader.biPlanes = 1;
                myHeader.biBitCount = wBitBerPixel;
                myHeader.biCompression = 0;
                myHeader.biSizeImage = (uint)nSize;
                myHeader.biXPelsPerMeter = 5906;  // ~96dpi
                myHeader.biYPelsPerMeter = 5906;  // ~96dpi
                myHeader.biClrUsed = 0;
                myHeader.biClrImportant = 0;
                Marshal.StructureToPtr(myHeader, ptrHeader, false);

                // copy image data
                Rectangle rect = new Rectangle(0, 0, bitmap.Width, bitmap.Height);
                BitmapData bitmapData = bitmap.LockBits(rect, ImageLockMode.ReadOnly, bitmap.PixelFormat);
                int length = bitmapData.Stride * bitmapData.Height;
                //Buffer.MemoryCopy(bitmapData.Scan0.ToPointer(), PtrBit.ToPointer(), length, length);  // need unsafe
                NativeMethods.CopyMemory(ptrImageData, bitmapData.Scan0, (uint)length);  // don't need unsafe
                bitmap.UnlockBits(bitmapData);
                bitmap.Dispose();
                bitmap = null;

                // do autocrop
                Bitmap outputBmp = null;
                int deskew = 1;
                NativeMethods.BITMAPPTR outBitmapPtr = new NativeMethods.BITMAPPTR();
                int nCropRtn = NativeMethods.autocrop(ref outBitmapPtr.pHeader, ptrHeader, deskew);
                if (nCropRtn > 0 && outBitmapPtr.pHeader != IntPtr.Zero)
                {
                    // move pointer to image data
                    NativeMethods.BITMAPINFOHEADER newHeader = Marshal.PtrToStructure<NativeMethods.BITMAPINFOHEADER>(outBitmapPtr.pHeader);
                    int BitCount = newHeader.biBitCount;
                    int nColorData = (BitCount <= 8) ? 1 << BitCount : 0;
                    outBitmapPtr.pQuad = outBitmapPtr.pHeader + Marshal.SizeOf(typeof(NativeMethods.BITMAPINFOHEADER));
                    outBitmapPtr.pBmp = outBitmapPtr.pQuad + nColorData;

                    // create new bitmap, create managed image data from unmanaged memory
                    int stride = (((newHeader.biWidth * newHeader.biBitCount) + 31) & ~31) >> 3;
                    nSize = stride * newHeader.biHeight;
                    FreeAll();
                    ptrNewImageData = Marshal.AllocHGlobal(nSize);
                    NativeMethods.CopyMemory(ptrNewImageData, outBitmapPtr.pBmp, (uint)nSize);  // don't need unsafe
                    outputBmp = new Bitmap(newHeader.biWidth, newHeader.biHeight, stride, PixelFormat.Format24bppRgb, ptrNewImageData);
                    //Marshal.FreeHGlobal(ptrNewImageData);  // cannot free now! this memory goes with bitmap
                }

                // release unmanaged memory
                NativeMethods.bmpfree(ref outBitmapPtr.pHeader);
                Marshal.FreeHGlobal(ptrHeader);
                GC.Collect();

                result = outputBmp;
                return nCropRtn;
            }
            catch (DllNotFoundException ex)
            {
                Console.WriteLine(ex);
                throw ex;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
                return 0;
            }
        }

        public static void FreeAll()
        {
            if (ptrNewImageData != IntPtr.Zero)
            {
                Marshal.FreeHGlobal(ptrNewImageData);
                ptrNewImageData = IntPtr.Zero;
            }
        }
    }
}
