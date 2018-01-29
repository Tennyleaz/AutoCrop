#pragma once
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <iterator>
void generateBitmapImage(unsigned char *image, int height, int width, char* imageFileName);
void generateBitmapImage(unsigned char *image, BITMAPINFOHEADER header, char* imageFileName);
unsigned char* createBitmapFileHeader(int height, int width);
unsigned char* createBitmapInfoHeader(int height, int width);
std::vector<char> readBMP(const std::string &file);