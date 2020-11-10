#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize.h"

#define CHANNEL_COUNT_RGB  3
#define CHANNEL_COUNT_RGBA 4

typedef unsigned char uchar;

void rgb2hsl(uchar r, uchar g, uchar b, float *h, float *s, float *l) {
 //https://www.niwa.nu/2013/05/math-behind-colorspace-conversions-rgb-hsl/

 float rf = (float)r / 255;
 float gf = (float)g / 255;
 float bf = (float)b / 255;

 float minrgbf = 1.0;
 if (rf < minrgbf) minrgbf = rf;
 if (gf < minrgbf) minrgbf = gf;
 if (bf < minrgbf) minrgbf = bf;

 float maxrgbf = 0.0;
 if (rf > maxrgbf) maxrgbf = rf;
 if (gf > maxrgbf) maxrgbf = gf;
 if (bf > maxrgbf) maxrgbf = bf;

 //Luminace
 float lf = (float)(minrgbf + maxrgbf) / 2;

 //Saturation
 float sf = 0.0;
 if (lf <= 0.5) {
  sf = (maxrgbf - minrgbf) / (maxrgbf + minrgbf);
 } else {
  sf = (maxrgbf - minrgbf) / (2.0 - maxrgbf - minrgbf);
 }
 
 //Hue
 float hf = 0.0;
 float hfdegrees = 0.0 * 60.0;
 if (sf > 0.0) {
  if (maxrgbf == rf) hf = (gf - bf) / (maxrgbf - minrgbf);
  if (maxrgbf == gf) hf = 2.0 + (bf - rf) / (maxrgbf - minrgbf);
  if (maxrgbf == bf) hf = 4.0 + (rf - gf) / (maxrgbf - minrgbf);
 }
 
 float hfdegrees = hf * 60.0;
 if (hfdegrees < 0) hfdegrees + 360.0;

 *h = hf;
 *s = sf;
 *l = lf;
}

void getpixelbyoffset(uchar *data, int offset, uchar *r, uchar *g, uchar *b/*, uchar *a*/) {
  //https://stackoverflow.com/questions/48235421/get-rgb-of-a-pixel-in-stb-image
  int channelCount = CHANNEL_COUNT_RGB/*A*/;
  unsigned bytePerPixel = channelCount;
  *r = offset[0];
  *g = offset[1];
  *b = offset[2];
  //*a = channelCount >= 4 ? pixelOffset[3] : 0xff;
}

void getpixelbycoords(uchar *data, int i, int j, int y, uchar *r, uchar *g, uchar *b/*, uchar *a*/) {
  //https://stackoverflow.com/questions/48235421/get-rgb-of-a-pixel-in-stb-image
  int channelCount = CHANNEL_COUNT_RGB;
  uchar *pixelOffset = data + (i + y * j) * channelCount;
  getpixelbyoffset(data, pixelOffset, r, g, b/*, a*/);
}

int main() {
 std::string imgPath1;
 std::string imgPath2;

 std::getline(std::cin, imgPath1);
 std::getline(std::cin, imgPath2);

 std::filesystem::path imgFullPath1 = std::filesystem::canonical(imgPath1); 
 std::filesystem::path imgFullPath2 = std::filesystem::canonical(imgPath2); 

 std::filesystem::path cwd = std::filesystem::current_path();
 imgPath1 = imgFullPath1.string();
 imgPath2 = imgFullPath2.string();

 std::cout << "img1: " << imgPath1 << std::endl;
 std::cout << "img2: " << imgPath2 << std::endl;

 int channelCount = CHANNEL_COUNT_RGB; //rgb
 
 int x1,y1,n1;
 int x1orig,y1orig,n1orig;

 int x2,y2,n2;
 int x2orig,y2orig,n2orig;

unsigned char *dataImg1 = stbi_load(imgPath1.c_str(), &x1, &y1, &n1, channelCount);
unsigned char *dataImg2 = stbi_load(imgPath2.c_str(), &x2, &y2, &n2, channelCount);
//    // ... process data if not NULL ...
//    // ... x = width, y = height, n = # 8-bit components per pixel ...
//    // ... replace '0' with '1'..'4' to force that many components per pixel
//    // ... but 'n' will always be the number that it would have been if you said 0
 
 printf("img1: x=%d, y=%d, n=%d\n", x1, y1, n1);
 printf("img2: x=%d, y=%d, n=%d\n", x2, y2, n2);

 x1orig = x1;
 x2orig = x2;

 y1orig = y1;
 y2orig = y2;

 n1orig = n1;
 n2orig = n2;

 unsigned char *dataImg1256 = 0;
 unsigned char *dataImg2256 = 0;

 int maxwh1 = x1 > y1 ? x1 : y1;
 int maxwh2 = x2 > y2 ? x2 : y2;
 
 printf("maxwh1: %f, maxwh2: %f\n", maxwh1, maxwh2);

 float aspectr1 = (float)x1 / y1;
 float aspectr2 = (float)x2 / y2;

 
 printf("aspectr1: %f, aspectr2: %f\n", aspectr1, aspectr2);

 if (maxwh1 > 256) {

  //stbir__calculate_memory
  //stbir__info
  int x1resized, y1resized, nch1resized;
  nch1resized = n1;
  x1resized = x1 > y1 ? 256 : (floor(256 * aspectr1));
  y1resized = x1 > y1 ? (floor(256 / aspectr1)) : 256;

  //printf("init x1resized: %d, y1resized: %d\n", x1resized, y1resized);

  dataImg1256 = (unsigned char *)malloc(sizeof(unsigned char) * (x1resized * y1resized * n1));
  stbir_resize_uint8(dataImg1, x1, y1, 0, dataImg1256, x1resized, y1resized, 0, nch1resized);
  //printf("stbir x1resized: %d, y1resized: %d\n", x1resized, y1resized);
  x1 = x1resized;
  y1 = y1resized;
 }
 
 if (maxwh2 > 256) {

  int x2resized, y2resized, nch2resized;
  nch2resized = n2;
  x2resized = x2 > y2 ? 256 : (floor(256 * aspectr2));
  y2resized = x2 > y2 ? (floor(256 / aspectr2)) : 256;

  dataImg2256 = (unsigned char *)malloc(sizeof(unsigned char) * (x2resized * y2resized * n2));
  stbir_resize_uint8(dataImg2, x2, y2, 0, dataImg2256, x2resized, y2resized, 0, nch2resized);

  x2 = x2resized;
  y2 = y2resized;
 }
 
 unsigned char *dataImg1Working = dataImg1256 ? dataImg1256 : dataImg1;
 unsigned char *dataImg2Working = dataImg2256 ? dataImg2256 : dataImg2;

 int data1len = x1 * y1 * n1;
 int data2len = x2 * y2 * n2;

 int colorcount = 256;
 int colorhistint = 30;
 int colorhiststep = ceil((float)colorcount / colorhistint);

 int hrarr[256]; for (int i = 0; i < 256; i++) hrarr[i] = 0;
 int hgarr[256]; for (int i = 0; i < 256; i++) hgarr[i] = 0;
 int hbarr[256]; for (int i = 0; i < 256; i++) hbarr[i] = 0;
 
 int hharr[256]; for (int i = 0; i < 256; i++) hharr[i] = 0;
 int hsarr[256]; for (int i = 0; i < 256; i++) hsarr[i] = 0;
 int hlarr[256]; for (int i = 0; i < 256; i++) hlarr[i] = 0;
  
 for (int i = 0; i < x1; i++) {
  for (int j = 0; j < y1; j++) {
   uchar r, g, b;
   getpixelbycoords(dataImg1Working, i, j, y1, r, g, b);
  
   int hri = floor((float)r / colorhiststep);
   hrarr[hri]++;
   
   int hgi = floor((float)g / colorhiststep);
   hrarr[hgi]++;
   
   int hbi = floor((float)b / colorhiststep);
   hrarr[hbi]++;
  }
 }

 if (dataImg1256) stbi_image_free(dataImg1256);
 if (dataImg2256) stbi_image_free(dataImg2256);

 stbi_image_free(dataImg1);
 stbi_image_free(dataImg2);

 return 0;
}