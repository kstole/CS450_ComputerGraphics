//
//  utility_funcs.cpp
//  CS450 Final Project
//
//  Created by Kyler Stole on 12/1/16.
//  Copyright © 2016 Kyler Stole. All rights reserved.
//

#include "utility_funcs.hpp"


//float inline rerange(float val, float OldMin, float OldMax, float NewMin, float NewMax) {
//    return (((float)(val - OldMin) * (NewMax - NewMin)) / (float)(OldMax - OldMin)) + NewMin;
//}

#define gaussian(x, sigma) pow(M_E, -pow(x, 2) / (2 * pow(sigma, 2))) / (sigma * sqrt(2 * M_PI))


// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv(rgb);

void HsvRgb(float hsv[3], float rgb[3]) {
    // guarantee valid input:
    
    float h = hsv[0] / 60.f;
    while (h >= 6.)	h -= 6.;
    while (h <  0.) h += 6.;
    
    float s = hsv[1];
    if (s < 0.)
        s = 0.;
    if (s > 1.)
        s = 1.;
    
    float v = hsv[2];
    if (v < 0.)
        v = 0.;
    if (v > 1.)
        v = 1.;
    
    // if sat==0, then is a gray:
    
    if (s == 0.0) {
        rgb[0] = rgb[1] = rgb[2] = v;
        return;
    }
    
    // get an rgb from the hue itself:
    
    float i = floor(h);
    float f = h - i;
    float p = v * (1.f - s);
    float q = v * (1.f - s*f);
    float t = v * (1.f - (s * (1.f-f)));
    
    float r, g, b;			// red, green, blue
    switch ((int) i) {
        case 0:
            r = v;	g = t;	b = p;
            break;
            
        case 1:
            r = q;	g = v;	b = p;
            break;
            
        case 2:
            r = p;	g = v;	b = t;
            break;
            
        case 3:
            r = p;	g = q;	b = v;
            break;
            
        case 4:
            r = t;	g = p;	b = v;
            break;
            
        case 5:
            r = v;	g = p;	b = q;
            break;
            
        default:
            r = 0;  g = 0;  b = 0;
    }
    
    
    rgb[0] = r;
    rgb[1] = g;
    rgb[2] = b;
}

float Dot(float v1[3], float v2[3]) {
    return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

void Cross(float v1[3], float v2[3], float vout[3]) {
    float tmp[3];
    tmp[0] = v1[1]*v2[2] - v2[1]*v1[2];
    tmp[1] = v2[0]*v1[2] - v1[0]*v2[2];
    tmp[2] = v1[0]*v2[1] - v2[0]*v1[1];
    vout[0] = tmp[0];
    vout[1] = tmp[1];
    vout[2] = tmp[2];
}

float Unit(float vin[3], float vout[3]) {
    float dist = vin[0]*vin[0] + vin[1]*vin[1] + vin[2]*vin[2];
    if (dist > 0.0) {
        dist = sqrt( dist );
        vout[0] = vin[0] / dist;
        vout[1] = vin[1] / dist;
        vout[2] = vin[2] / dist;
    } else {
        vout[0] = vin[0];
        vout[1] = vin[1];
        vout[2] = vin[2];
    }
    return dist;
}

float* Array3(float a, float b, float c) {
    static float array[4];
    array[0] = a;
    array[1] = b;
    array[2] = c;
    array[3] = 1.;
    return array;
}

float* Array4(float a, float b, float c, float d) {
    static float array[4];
    array[0] = a;
    array[1] = b;
    array[2] = c;
    array[3] = d;
    return array;
}

float* BlendArray3(float factor, float array0[3], float array1[3]) {
    static float array[4];
    array[0] = factor * array0[0]  +  (1.f - factor) * array1[0];
    array[1] = factor * array0[1]  +  (1.f - factor) * array1[1];
    array[2] = factor * array0[2]  +  (1.f - factor) * array1[2];
    array[3] = 1.;
    return array;
}

float* MulArray3(float factor, float array0[3]) {
    static float array[4];
    array[0] = factor * array0[0];
    array[1] = factor * array0[1];
    array[2] = factor * array0[2];
    array[3] = 1.;
    return array;
}
