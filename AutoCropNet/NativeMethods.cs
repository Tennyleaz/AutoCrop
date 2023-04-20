using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace AutoCropNet
{
    internal static class NativeMethods
    {
        [StructLayout(LayoutKind.Sequential)]
        public struct BITMAPINFO
        {
            /// BITMAPINFOHEADER->tagBITMAPINFOHEADER
            public BITMAPINFOHEADER bmiHeader;

            /// RGBQUAD[1]
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 1, ArraySubType = UnmanagedType.Struct)]
            public RGBQUAD[] bmiColors;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct BITMAPINFOHEADER
        {
            /// DWORD->unsigned int
            public uint biSize;

            /// LONG->int
            public int biWidth;

            /// LONG->int
            public int biHeight;

            /// WORD->unsigned short
            public ushort biPlanes;

            /// WORD->unsigned short
            public ushort biBitCount;

            /// DWORD->unsigned int
            public uint biCompression;

            /// DWORD->unsigned int
            public uint biSizeImage;

            /// LONG->int
            public int biXPelsPerMeter;

            /// LONG->int
            public int biYPelsPerMeter;

            /// DWORD->unsigned int
            public uint biClrUsed;

            /// DWORD->unsigned int
            public uint biClrImportant;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct RGBQUAD
        {
            /// BYTE->unsigned char
            public byte rgbBlue;

            /// BYTE->unsigned char
            public byte rgbGreen;

            /// BYTE->unsigned char
            public byte rgbRed;

            /// BYTE->unsigned char
            public byte rgbReserved;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct BITMAPPTR
        {
            /// PBITMAPINFOHEADER->tagBITMAPINFOHEADER*
            public IntPtr pHeader;

            /// RGBQUAD*
            public IntPtr pQuad;

            /// PBYTE->BYTE*
            public IntPtr pBmp;
        }

        /// Return Type: int
        ///param0: BITMAPINFOHEADER**
        ///param1: BITMAPINFOHEADER*
        ///param2: int
        [DllImport("atocropLIB.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern int autocrop(ref IntPtr outputPtr, IntPtr inputHeader, int isDeskew);

        [DllImport("atocropLIB.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern int bmpfree(ref IntPtr pHeader);

        [DllImport("kernel32.dll", EntryPoint = "CopyMemory", SetLastError = false)]
        public static extern void CopyMemory(IntPtr dest, IntPtr src, uint count);
    }
}
