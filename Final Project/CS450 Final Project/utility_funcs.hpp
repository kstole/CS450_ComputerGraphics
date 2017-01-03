//
//  utility_funcs.hpp
//  CS450 Final Project
//
//  Created by Kyler Stole on 12/1/16.
//  Copyright © 2016 Kyler Stole. All rights reserved.
//

#ifndef utility_funcs_hpp
#define utility_funcs_hpp

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#else
#include <GLUT/glut.h>
#endif

#include <stdio.h>
#include <cmath>

#define rerange(val, OldMin, OldMax, NewMin, NewMax) (((float)(val - OldMin) * (NewMax - NewMin)) / (float)(OldMax - OldMin)) + NewMin

void HsvRgb(float hsv[3], float rgb[3]);

float Dot(float v1[3], float v2[3]);
void Cross(float v1[3], float v2[3], float vout[3]);
float Unit(float vin[3], float vout[3]);

float* Array3(float a, float b, float c);
float* Array4(float a, float b, float c, float d);
float* BlendArray3(float factor, float array0[3], float array1[3]);
float* MulArray3(float factor, float array0[3]);

#endif /* utility_funcs_hpp */
