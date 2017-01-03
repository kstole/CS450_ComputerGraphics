//
//  particles.hpp
//  CS450 Final Project
//
//  Created by Kyler Stole on 12/3/16.
//  Copyright © 2016 Kyler Stole. All rights reserved.
//

#ifndef particles_hpp
#define particles_hpp

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
#include <math.h>
#include <stdlib.h>
#include <string.h>


#pragma GCC diagnostic ignored "-Wdeprecated-declarations"


void setSphereRadius(float rad);

void InitParticles();

void drawParticles();

void menustate(int state);
void reshape(int width, int height);

void DoParticleMenu(int id);

void idleParticles();

#endif /* particles_hpp */
