#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>

#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
//C++17 specific stuff here
#include <filesystem>
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize.h"

#define CHANNEL_COUNT_RGB  3
#define CHANNEL_COUNT_RGBA 4

typedef unsigned char uchar;

int colorcount = 256;
int colorhistint = 30;
int colorhiststep = ceil((float)colorcount / colorhistint);
int rgbstep = 9; // 256 / 30 = 8.53 (9)

int hhstep = 12; //360 / 30 = 12
float hsstepf = 0.033; // 1.0 / 30 = 0.03333333
float hlstepf = 0.033; // 1.0 / 30 = 0.03333333

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
 float lf = (float)(minrgbf + maxrgbf) / 2.0;

 //Saturation
 float sf = 0.0;
 if (lf > 0)
  if (lf <= 0.5)
   sf = (maxrgbf - minrgbf) / (maxrgbf + minrgbf);
  else 
   sf = (maxrgbf - minrgbf) / (2.0 - maxrgbf - minrgbf);
 
 //Hue
 float hf = 0.0;
 if (sf > 0.0)
  if (maxrgbf == rf) hf = (gf - bf) / (maxrgbf - minrgbf);
  else if (maxrgbf == gf) hf = 2.0 + (bf - rf) / (maxrgbf - minrgbf);
  else if (maxrgbf == bf) hf = 4.0 + (rf - gf) / (maxrgbf - minrgbf);
 
 float hfdegrees = hf * 60.0;
 if (hfdegrees < 0) hfdegrees + 360.0;

 *h = hf;
 *s = sf;
 *l = lf;
}

int hf2degrees(float hf) {
 float hfdegrees = hf * 60.0;
 while (hfdegrees < 0) hfdegrees + 360.0;
 return hfdegrees;
}

void getpixelbyoffset(uchar *data, int offset, uchar *r, uchar *g, uchar *b/*, uchar *a*/) {
 //https://stackoverflow.com/questions/48235421/get-rgb-of-a-pixel-in-stb-image
 int channelcount = CHANNEL_COUNT_RGB/*A*/;
 uchar *pixeldata = data + offset * channelcount;
 *r = pixeldata[0];
 *g = pixeldata[1];
 *b = pixeldata[2];
 //*a = channelcount >= 4 ? pixeldata[3] : 0xff;
}

void getpixelbycoords(uchar *data, int i, int j, int y, uchar *r, uchar *g, uchar *b/*, uchar *a*/) {
 //https://stackoverflow.com/questions/48235421/get-rgb-of-a-pixel-in-stb-image
 int channelcount = CHANNEL_COUNT_RGB;
 int offset = (i + y * j);
 getpixelbyoffset(data, offset, r, g, b/*, a*/);
}

void calchistograms(uchar *dataimg, int w, int h, int *hrarr, int *hgarr, int *hbarr, int *hharr, int *hsarr, int *hlarr) {
 int x1 = w;
 int y1 = h;

 for (int i = 0; i < colorhistint; i++) hrarr[i] = 0;
 for (int i = 0; i < colorhistint; i++) hgarr[i] = 0;
 for (int i = 0; i < colorhistint; i++) hbarr[i] = 0;
 
 for (int i = 0; i < colorhistint; i++) hharr[i] = 0;
 for (int i = 0; i < colorhistint; i++) hsarr[i] = 0;
 for (int i = 0; i < colorhistint; i++) hlarr[i] = 0;
  
 for (int i = 0; i < x1; i++) {
  for (int j = 0; j < y1; j++) {
   uchar r, g, b;
   getpixelbycoords(dataimg, i, j, y1, &r, &g, &b);
  
   int hri = floor((float)r / colorhiststep);
   hrarr[hri]++;
   
   int hgi = floor((float)g / colorhiststep);
   hrarr[hgi]++;
   
   int hbi = floor((float)b / colorhiststep);
   hrarr[hbi]++;

   float hf = 0.0;
   float sf = 0.0;
   float lf = 0.0;
   rgb2hsl(r, g, b, &hf, &sf, &lf);
   
   int hfdegrees = hf2degrees(hf);
   int hhi = floor(hfdegrees / hhstep);
   hharr[hhi]++;
   
   int hsi = floor(sf / hsstepf);
   hsarr[hsi]++;
   
   int hli = floor(lf / hlstepf);
   hlarr[hli]++;

   //printf("R=%d, G=%d, B=%d, H=%d(%f), S=%f, L=%f\n", r, g, b, hfdegrees, hf, sf, lf);
  }
 }
}

float calcdist(int *harr1, int *harr2) {
 float sumf = 0.0;
 for (int i = 0; i < colorhistint; i++) {
  sumf += (float)pow(harr1[i] - harr2[i], 2);
 }
 float dist = sqrt(sumf);
 return dist;
}

uchar *resizeimgdataifbig(uchar *data, int x, int y, int n, int *xresized, int *yresized) {
 int x1 = x;
 int y1 = y;
 int n1 = n;

 uchar *dataimg1 = data;

 int maxwh1 = x1 > y1 ? x1 : y1;

 float aspectr1 = (float)x1 / y1;

 if (maxwh1 > 256) {

  //stbir__calculate_memory
  //stbir__info
  int x1resized, y1resized, nch1resized;
  nch1resized = n1;
  x1resized = x1 > y1 ? 256 : (floor(256 * aspectr1));
  y1resized = x1 > y1 ? (floor(256 / aspectr1)) : 256;

  //printf("init x1resized: %d, y1resized: %d\n", x1resized, y1resized);

  uchar *dataimg1256 = (uchar *)malloc(sizeof(uchar) * (x1resized * y1resized * n1));
  stbir_resize_uint8(dataimg1, x1, y1, 0, dataimg1256, x1resized, y1resized, 0, nch1resized);
  //printf("stbir x1resized: %d, y1resized: %d\n", x1resized, y1resized);
  x1 = x1resized;
  y1 = y1resized;

  *xresized = x1;
  *yresized = y1;

  return dataimg1256;
 }

 return 0;
}

int main() {
 std::string imgpath1;
 std::string imgpath2;

 std::getline(std::cin, imgpath1);
 std::getline(std::cin, imgpath2);

#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
 //C++17 specific stuff here
     
 std::filesystem::path imgfullpath1 = std::filesystem::canonical(imgpath1); 
 std::filesystem::path imgfullpath2 = std::filesystem::canonical(imgpath2); 
 //std::filesystem::path cwd = std::filesystem::current_path();

 imgpath1 = imgfullpath1.string();
 imgpath2 = imgfullpath2.string();
#endif

 int channelcount = CHANNEL_COUNT_RGB; //rgb
 
 int x1,y1,n1;
 int x1orig,y1orig,n1orig;

 int x2,y2,n2;
 int x2orig,y2orig,n2orig;

 unsigned char *dataimg1 = stbi_load(imgpath1.c_str(), &x1, &y1, &n1, channelcount);
 unsigned char *dataimg2 = stbi_load(imgpath2.c_str(), &x2, &y2, &n2, channelcount);
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

 unsigned char *dataimg1256 = resizeimgdataifbig(dataimg1, x1, y1, n1, &x1, &y1);
 unsigned char *dataimg2256 = resizeimgdataifbig(dataimg2, x2, y2, n2, &x2, &y2);
 
 unsigned char *dataimg1working = dataimg1256 ? dataimg1256 : dataimg1;
 unsigned char *dataimg2working = dataimg2256 ? dataimg2256 : dataimg2;

 int data1len = x1 * y1 * n1;
 int data2len = x2 * y2 * n2;

 int hrarr1[30];
 int hgarr1[30];
 int hbarr1[30];
 
 int hharr1[30];
 int hsarr1[30];
 int hlarr1[30];

 calchistograms(dataimg1working, x1, y1, hrarr1, hgarr1, hbarr1, hharr1, hsarr1, hlarr1);

 printf("histograms 1 calculated\n");

 int hrarr2[30];
 int hgarr2[30];
 int hbarr2[30];
 
 int hharr2[30];
 int hsarr2[30];
 int hlarr2[30];
  
 calchistograms(dataimg2working, x2, y2, hrarr2, hgarr2, hbarr2, hharr2, hsarr2, hlarr2);
 
 printf("histograms 2 calculated\n");

 float distarr[6];
 distarr[0] = calcdist(hrarr1, hrarr2);
 distarr[1] = calcdist(hgarr1, hgarr2);
 distarr[2] = calcdist(hbarr1, hbarr2);
 distarr[3] = calcdist(hharr1, hharr2);
 distarr[4] = calcdist(hsarr1, hsarr2);
 distarr[5] = calcdist(hlarr1, hlarr2);
 
 printf("dists calculated\n");

 printf("[\n");
 printf(" ");
 printf("dist(Ra,Rb), dist(Ga,Gb), dist(Ba,Bb), dist(Ha,Hb), dist(Sa,Sb), dist(La,Lb)\n");
 printf("]\n");
 printf("=\n");
 printf("[%f,%f,%f,%f,%f,%f]\n", 
  distarr[0], 
  distarr[1], 
  distarr[2], 
  distarr[3], 
  distarr[4], 
  distarr[5]
 );

 if (dataimg1256) stbi_image_free(dataimg1256);
 if (dataimg2256) stbi_image_free(dataimg2256);

 stbi_image_free(dataimg1);
 stbi_image_free(dataimg2);

 return 0;
}