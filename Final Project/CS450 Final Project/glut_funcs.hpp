//
//  glut_funcs.hpp
//  CS450 Final Project
//
//  Created by Kyler Stole on 12/1/16.
//  Copyright © 2016 Kyler Stole. All rights reserved.
//

#ifndef glut_funcs_hpp
#define glut_funcs_hpp

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
#include <cstdlib>
#include "utility_funcs.hpp"

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"


// the escape key:
#define ESCAPE		0x1b

// multiplication factors for input interaction:
//  (these are known from previous experience)
const float ANGFACT = { 1. };
const float SCLFACT = { 0.005f };

// minimum allowable scale factor:
const float MINSCALE = { 0.05f };

// which projection:
enum Projections {
    ORTHO,
    PERSP
};

// distortion values:
enum DistortTypes {
    NoTex,
    NoDistort,
    Distort
};

// which button:
enum ButtonVals {
    RESET,
    QUIT
};

// the color numbers:
// this order must match the radio button order
enum Colors {
    RED,
    YELLOW,
    GREEN,
    CYAN,
    BLUE,
    MAGENTA,
    WHITE,
    BLACK
};
extern GLfloat White[];
extern const GLfloat Colors[8][3];
extern char const* ColorNames[];

extern int		ActiveButton;			// current button that is down
extern GLuint	AxesList;				// list to hold the axes
extern bool		AxesOn;					// != 0 means to draw the axes
extern bool     DistortOn;              // global -- true means to distort the texture
extern bool     TextureOn;              // show texture or not
extern bool     DayMode;
extern bool		DebugOn;				// != 0 means to print debugging info
extern bool		DepthCueOn;				// != 0 means to use intensity depth cueing
extern GLuint   texDay, texLight, texMoon;
extern int		MainWindow;				// window id for main graphics window
extern float	Scale;					// scaling factor
extern int		WhichColor;				// index into Colors[ ]
extern int		WhichProjection;		// ORTHO or PERSP
extern int		Xmouse, Ymouse;			// mouse values
extern float	Xrot, Yrot;				// rotation angles in degrees
extern float    Time;                   // current time elapsed
extern float    TimeCycle;              // current time in animation cycle
extern float    SwitchCycle;
extern bool     Frozen;                 // sets whether the scene is frozen

void	Animate();
void	Display();
extern void    (*AnimateFunc)();

void	DoAxesMenu(int);
void    DoDistortMenu(int);
void	DoColorMenu(int);
void	DoDepthMenu(int);
void	DoDebugMenu(int);
void	DoMainMenu(int);
void	DoProjectMenu(int);
void	DoRasterString(float, float, float, char const *);
void	DoStrokeString(float, float, float, float, char const *);

float	ElapsedSeconds();
void	InitGraphics();
void    InitTextures();
void	InitLists();
void	InitMenus();
void	Keyboard(unsigned char, int, int);
void	MouseButton(int, int, int, int);
void	MouseMotion(int, int);
void	Reset();
void	Resize(int, int);
void	Visibility(int);

float ElapsedSeconds();

void	Axes(float);

void SetMaterial(float r, float g, float b,  float shininess);
void SetPointLight(int ilight, float x, float y, float z,  float r, float g, float b);
void SetSpotLight(int ilight, float x, float y, float z,  float xdir, float ydir, float zdir, float r, float g, float b);

#endif /* glut_funcs_hpp */
