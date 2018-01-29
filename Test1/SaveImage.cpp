#include "stdafx.h"
#include "SaveImage.h"

const int bytesPerPixel = 3; /// red, green, blue
const int fileHeaderSize = 14;
const int infoHeaderSize = 40;

void generateBitmapImage(unsigned char *image, int height, int width, char* imageFileName) {

	unsigned char* fileHeader = createBitmapFileHeader(height, width);
	unsigned char* infoHeader = createBitmapInfoHeader(height, width);
	unsigned char padding[3] = { 0, 0, 0 };
	int paddingSize = (4 - (width*bytesPerPixel) % 4) % 4;

	FILE* imageFile = fopen(imageFileName, "wb");

	fwrite(fileHeader, 1, fileHeaderSize, imageFile);
	fwrite(infoHeader, 1, infoHeaderSize, imageFile);

	int i;
	for (i = 0; i < height; i++) {
		fwrite(image + (i*width*bytesPerPixel), bytesPerPixel, width, imageFile);
		fwrite(padding, 1, paddingSize, imageFile);
	}

	fclose(imageFile);
}

void generateBitmapImage(unsigned char *image, BITMAPINFOHEADER header, char* imageFileName) {

	unsigned char* fileHeader = createBitmapFileHeader(header.biHeight, header.biWidth);
	//unsigned char* infoHeader = createBitmapInfoHeader(height, width);
	unsigned char padding[3] = { 0, 0, 0 };
	int paddingSize = (4 - (header.biWidth*bytesPerPixel) % 4) % 4;

	FILE* imageFile = fopen(imageFileName, "wb");

	fwrite(fileHeader, 1, fileHeaderSize, imageFile);
	fwrite(&header, 1, sizeof(BITMAPINFOHEADER), imageFile);

	int i;
	for (i = 0; i < header.biHeight; i++) {
		fwrite(image + (i*header.biWidth*bytesPerPixel), bytesPerPixel, header.biWidth, imageFile);
		fwrite(padding, 1, paddingSize, imageFile);
	}

	fclose(imageFile);
}

unsigned char* createBitmapFileHeader(int height, int width) {
	int fileSize = fileHeaderSize + infoHeaderSize + bytesPerPixel*height*width;

	static unsigned char fileHeader[] = {
		0,0, /// signature
		0,0,0,0, /// image file size in bytes
		0,0,0,0, /// reserved
		0,0,0,0, /// start of pixel array
	};

	fileHeader[0] = (unsigned char)('B');
	fileHeader[1] = (unsigned char)('M');
	fileHeader[2] = (unsigned char)(fileSize);
	fileHeader[3] = (unsigned char)(fileSize >> 8);
	fileHeader[4] = (unsigned char)(fileSize >> 16);
	fileHeader[5] = (unsigned char)(fileSize >> 24);
	fileHeader[10] = (unsigned char)(fileHeaderSize + infoHeaderSize);

	return fileHeader;
}

unsigned char* createBitmapInfoHeader(int height, int width) {
	static unsigned char infoHeader[] = {
		0,0,0,0, /// header size
		0,0,0,0, /// image width
		0,0,0,0, /// image height
		0,0, /// number of color planes
		0,0, /// bits per pixel
		0,0,0,0, /// compression
		0,0,0,0, /// image size
		0,0,0,0, /// horizontal resolution
		0,0,0,0, /// vertical resolution
		0,0,0,0, /// colors in color table
		0,0,0,0, /// important color count
	};

	infoHeader[0] = (unsigned char)(infoHeaderSize);
	infoHeader[4] = (unsigned char)(width);
	infoHeader[5] = (unsigned char)(width >> 8);
	infoHeader[6] = (unsigned char)(width >> 16);
	infoHeader[7] = (unsigned char)(width >> 24);
	infoHeader[8] = (unsigned char)(height);
	infoHeader[9] = (unsigned char)(height >> 8);
	infoHeader[10] = (unsigned char)(height >> 16);
	infoHeader[11] = (unsigned char)(height >> 24);
	infoHeader[12] = (unsigned char)(1);
	infoHeader[14] = (unsigned char)(bytesPerPixel * 8);

	return infoHeader;
}

std::vector<char> readBMP(const std::string &file)
{
	static constexpr size_t HEADER_SIZE = 54;

	std::ifstream bmp(file, std::ios::binary);

	std::array<char, HEADER_SIZE> header;
	bmp.read(header.data(), header.size());

	auto fileSize = *reinterpret_cast<uint32_t *>(&header[2]);
	auto dataOffset = *reinterpret_cast<uint32_t *>(&header[10]);
	auto width = *reinterpret_cast<uint32_t *>(&header[18]);
	auto height = *reinterpret_cast<uint32_t *>(&header[22]);
	auto depth = *reinterpret_cast<uint16_t *>(&header[28]);

	std::cout << "fileSize: " << fileSize << std::endl;
	std::cout << "dataOffset: " << dataOffset << std::endl;
	std::cout << "width: " << width << std::endl;
	std::cout << "height: " << height << std::endl;
	std::cout << "depth: " << depth << "-bit" << std::endl;

	std::vector<char> img(dataOffset - HEADER_SIZE);
	bmp.read(img.data(), img.size());

	auto dataSize = ((width * 3 + 3) & (~3)) * height;
	img.resize(dataSize);
	bmp.read(img.data(), img.size());

	char temp = 0;

	/*for (auto i = dataSize - 4; i >= 0; i -= 3)
	{
		temp = img[i];
		img[i] = img[i + 2];
		img[i + 2] = temp;

		//std::cout << "R: " << int(img[i] & 0xff) << " G: " << int(img[i + 1] & 0xff) << " B: " << int(img[i + 2] & 0xff) << std::endl;
	}*/

	return img;
}