#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize.h"

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

  int x1,y1,n1;
  int x2,y2,n2;
  unsigned char *dataImg1 = stbi_load(imgPath1.c_str(), &x1, &y1, &n1, 0);
  unsigned char *dataImg2 = stbi_load(imgPath2.c_str(), &x2, &y2, &n2, 0);
//    // ... process data if not NULL ...
//    // ... x = width, y = height, n = # 8-bit components per pixel ...
//    // ... replace '0' with '1'..'4' to force that many components per pixel
//    // ... but 'n' will always be the number that it would have been if you said 0
 
 printf("img1: x=%d, y=%d, n=%d\n", x1, y1, n1);
 printf("img2: x=%d, y=%d, n=%d\n", x2, y2, n2);

 unsigned char *dataImg1256 = 0;
 unsigned char *dataImg2256 = 0;

 int maxwh1 = x1 > y1 ? x1 : y1;
 int maxwh2 = x2 > y2 ? x2 : y2;
 
 printf("maxwh1: %f, maxwh2: %f\n", maxwh1, maxwh2);

 float aspectr1 = (float)x1 / y1;
 float aspectr2 = (float)x2 / y2;

 
 printf("aspectr1: %f, aspectr2: %f\n", aspectr1, aspectr2);

 if (maxwh1 > 256) {
  printf("img1 needs resize\n");

  //stbir__calculate_memory
  //stbir__info
  int x1resized, y1resized, nch1resized;
  nch1resized = n1;
  x1resized = x1 > y1 ? 256 : (floor(256 * aspectr1));
  y1resized = x1 > y1 ? (floor(256 / aspectr1)) : 256;

  printf("init x1resized: %d, y1resized: %d\n", x1resized, y1resized);

  dataImg1256 = (unsigned char *)malloc(sizeof(unsigned char) * (x1resized * y1resized * n1));
  stbir_resize_uint8(dataImg1, x1, y1, 0, dataImg1256, x1resized, y1resized, 0, nch1resized);
  printf("stbir x1resized: %d, y1resized: %d\n", x1resized, y1resized);

  free(dataImg1256);
 }


 stbi_image_free(dataImg1);
 stbi_image_free(dataImg2);

 return 0;
}