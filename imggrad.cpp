#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "./../stb/stb_image.h"

int main() {
 std::string imgPath1;
 std::string imgPath2;

 std::getline(std::cin, imgPath1);
 std::getline(std::cin, imgPath2);

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

 stbi_image_free(dataImg1);
 stbi_image_free(dataImg2);

 return 0;
}