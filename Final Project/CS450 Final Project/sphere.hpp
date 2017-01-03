//
//  sphere.hpp
//  CS450 Final Project
//
//  Created by Kyler Stole on 12/1/16.
//  Copyright © 2016 Kyler Stole. All rights reserved.
//

#ifndef sphere_hpp
#define sphere_hpp

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
#include "utility_funcs.hpp"
#include "glut_funcs.hpp"

extern int bounceMult;

void MjbSphere(float rad, int slices, int stacks, float** spec);

#endif /* sphere_hpp */
