//
//  BmpToTexture.cpp
//  CS450 Final Project
//
//  Created by Kyler Stole on 12/1/16.
//  Copyright © 2016 Kyler Stole. All rights reserved.
//

#include "BmpToTexture.hpp"

struct bmfh {
    short bfType;
    int bfSize;
    short bfReserved1;
    short bfReserved2;
    int bfOffBits;
} FileHeader;

struct bmih {
    int biSize;
    int biWidth;
    int biHeight;
    short biPlanes;
    short biBitCount;
    int biCompression;
    int biSizeImage;
    int biXPelsPerMeter;
    int biYPelsPerMeter;
    int biClrUsed;
    int biClrImportant;
} InfoHeader;

const int birgb = { 0 };




int ReadInt(FILE *fp) {
    unsigned char b3, b2, b1, b0;
    b0 = fgetc(fp);
    b1 = fgetc(fp);
    b2 = fgetc(fp);
    b3 = fgetc(fp);
    return (b3 << 24)  |  (b2 << 16)  |  (b1 << 8)  |  b0;
}

short ReadShort(FILE *fp) {
    unsigned char b1, b0;
    b0 = fgetc(fp);
    b1 = fgetc(fp);
    return (b1 << 8)  |  b0;
}

/**
 ** read a BMP file into a Texture:
 **/
unsigned char* BmpToTexture(char *filename, int *width, int *height) {
    int s, t, e;		// counters
    int numextra;		// # extra bytes each line in the file is padded with
    FILE *fp;
    unsigned char *texture;
    int nums, numt;
    unsigned char *tp;
    
    
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Cannot open Bmp file '%s'\n", filename);
        return NULL;
    }
    
    FileHeader.bfType = ReadShort(fp);
    
    // if bfType is not 0x4d42, the file is not a bmp:
    if (FileHeader.bfType != 0x4d42){
        fprintf(stderr, "Wrong type of file: 0x%0x\n", FileHeader.bfType);
        fclose(fp);
        return NULL;
    }
    
    FileHeader.bfSize = ReadInt(fp);
    FileHeader.bfReserved1 = ReadShort(fp);
    FileHeader.bfReserved2 = ReadShort(fp);
    FileHeader.bfOffBits = ReadInt(fp);
    
    
    InfoHeader.biSize = ReadInt(fp);
    InfoHeader.biWidth = ReadInt(fp);
    InfoHeader.biHeight = ReadInt(fp);
    
    nums = InfoHeader.biWidth;
    numt = InfoHeader.biHeight;
    
    InfoHeader.biPlanes = ReadShort(fp);
    InfoHeader.biBitCount = ReadShort(fp);
    InfoHeader.biCompression = ReadInt(fp);
    InfoHeader.biSizeImage = ReadInt(fp);
    InfoHeader.biXPelsPerMeter = ReadInt(fp);
    InfoHeader.biYPelsPerMeter = ReadInt(fp);
    InfoHeader.biClrUsed = ReadInt(fp);
    InfoHeader.biClrImportant = ReadInt(fp);
    
    // fprintf(stderr, "Image size found: %d x %d\n", ImageWidth, ImageHeight);
    
    texture = new unsigned char[ 3 * nums * numt ];
    if (texture == NULL) {
        fprintf(stderr, "Cannot allocate the texture array!\b");
        return NULL;
    }
    
    // extra padding bytes:
    numextra =  4*(((3*InfoHeader.biWidth)+3)/4) - 3*InfoHeader.biWidth;
    
    // we do not support compression:
    if (InfoHeader.biCompression != birgb) {
        fprintf(stderr, "Wrong type of image compression: %d\n", InfoHeader.biCompression);
        fclose(fp);
        return NULL;
    }
    
    rewind(fp);
    fseek(fp, 14+40, SEEK_SET);
    
    if (InfoHeader.biBitCount == 24) {
        for (t = 0, tp = texture; t < numt; t++) {
            for (s = 0; s < nums; s++, tp += 3)  {
                *(tp+2) = fgetc(fp);		// b
                *(tp+1) = fgetc(fp);		// g
                *(tp+0) = fgetc(fp);		// r
            }
            
            for (e = 0; e < numextra; e++)
                fgetc(fp);
        }
    }
    
    fclose(fp);
    
    *width = nums;
    *height = numt;
    return texture;
}
