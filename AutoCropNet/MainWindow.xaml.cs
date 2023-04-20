using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using Microsoft.Win32;

namespace AutoCropNet
{
    /// <summary>
    /// MainWindow.xaml 的互動邏輯
    /// </summary>
    public partial class MainWindow : Window
    {
        private string fileName;

        public MainWindow()
        {
            InitializeComponent();
        }

        private void BtnOpen_OnClick(object sender, RoutedEventArgs e)
        {
            OpenFileDialog dialog = new OpenFileDialog();
            dialog.InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.MyPictures);
            dialog.Filter = "Image Files|*.BMP;*.PNG;*.JPG;*.GIF";
            if (dialog.ShowDialog() == true)
            {
                try
                {
                    fileName = dialog.FileName;
                    using (FileStream fs = new FileStream(fileName, FileMode.Open))
                    {
                        BitmapImage imageIn = new BitmapImage();
                        imageIn.BeginInit();
                        imageIn.StreamSource = fs;
                        imageIn.CacheOption = BitmapCacheOption.OnLoad;
                        imageIn.EndInit();
                        imageIn.Freeze();
                        imageSource.Source = imageIn;
                        imageTarget.Source = null;
                        tbSource.Text = $"Source image {imageIn.PixelWidth}x{imageIn.PixelHeight}";
                        tbTarget.Text = "";
                    }
                }
                catch (Exception ex)
                {
                    MessageBox.Show(this, ex.ToString());
                }
            }
        }

        private void BtnAutoCrop_OnClick(object sender, RoutedEventArgs e)
        {
            try
            {
                using (FileStream fs = new FileStream(fileName, FileMode.Open))
                {
                    using (Bitmap bitmap = new Bitmap(fs))
                    {
                        int nRtn = AutoCropNative.AutoCrop(bitmap, out Bitmap result);
                        if (nRtn > 0)
                        {
                            IntPtr handle = result.GetHbitmap();
                            BitmapSource resultImage = Imaging.CreateBitmapSourceFromHBitmap(handle, IntPtr.Zero, Int32Rect.Empty, BitmapSizeOptions.FromEmptyOptions());
                            resultImage.Freeze();
                            imageTarget.Source = resultImage;
                            result.Dispose();
                            tbTarget.Text = $"Result image {resultImage.PixelWidth}x{resultImage.PixelHeight}";
                        }
                        else
                        {
                            MessageBox.Show(this, "AutoCrop failed, return=" + nRtn, "AutoCrop");
                        }
                    }
                }
            }
            catch (FormatException ex)
            {
                MessageBox.Show(this, ex.Message, "FormatException");
            }
            catch (DllNotFoundException ex)
            {
                MessageBox.Show(this, ex.Message, "DllNotFoundException");
            }
            catch (ArgumentNullException ex)
            {
                MessageBox.Show(this, ex.Message, "ArgumentNullException");
            }
            catch (Exception ex)
            {
                MessageBox.Show(this, ex.ToString());
            }
            finally
            {
                AutoCropNative.FreeAll();
            }
        }
    }
}
