//
//  glut_funcs.cpp
//  CS450 Final Project
//
//  Created by Kyler Stole on 12/1/16.
//  Copyright © 2016 Kyler Stole. All rights reserved.
//

#include "glut_funcs.hpp"

// what the glui package defines as true and false:
//const int GLUITRUE  = { true  };
//const int GLUIFALSE = { false };


// active mouse buttons (or them together):
const int LEFT   = { 4 };
const int MIDDLE = { 2 };
const int RIGHT  = { 1 };

char const* ColorNames[] = {
    "Red",
    "Yellow",
    "Green",
    "Cyan",
    "Blue",
    "Magenta",
    "White",
    "Black"
};


// the color definitions:
// this order must match the menu order
const GLfloat Colors[8][3] = {
    { 1., 0., 0. },		// red
    { 1., 1., 0. },		// yellow
    { 0., 1., 0. },		// green
    { 0., 1., 1. },		// cyan
    { 0., 0., 1. },		// blue
    { 1., 0., 1. },		// magenta
    { 1., 1., 1. },		// white
    { 0., 0., 0. },		// black
};
GLfloat White[] = {1., 1., 1., 1.};




// non-constant global variables:
GLuint	AxesList;				// list to hold the axes
GLuint  texDay, texLight, texMoon;

int		ActiveButton;			// current button that is down
bool	AxesOn;					// != 0 means to draw the axes
bool	DistortOn;              // global -- true means to distort the texture
bool    TextureOn;              // show texture or not
bool    DayMode;
bool	DebugOn;				// != 0 means to print debugging info
bool	DepthCueOn;				// != 0 means to use intensity depth cueing
bool    Frozen;                 // sets whether the scene is frozen

int		MainWindow;				// window id for main graphics window
float	Scale;					// scaling factor
int		WhichColor;				// index into Colors[ ]
int		WhichProjection;		// ORTHO or PERSP
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees
float   Time;                   // current time elapsed
float   TimeCycle;              // current time in animation cycle
float   SwitchCycle;

// MARK: - Menu Actions

void DoAxesMenu(int id) {
    AxesOn = id;
    
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

void DoDistortMenu(int id) {
    DistortOn = (id == Distort);
    TextureOn = (id != NoTex);
    
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

void DoColorMenu(int id) {
    WhichColor = id - RED;
    
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

void DoDebugMenu(int id) {
    DebugOn = id;
    
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

void DoDepthMenu(int id) {
    DepthCueOn = id;
    
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

// main menu callback:
void DoMainMenu(int id) {
    switch (id) {
        case RESET:
            Reset();
            break;
            
        case QUIT:
            // gracefully close out the graphics:
            // gracefully close the graphics window:
            // gracefully exit the program:
            glutSetWindow(MainWindow);
            glFinish();
            glutDestroyWindow(MainWindow);
            exit(0);
            break;
            
        default:
            fprintf(stderr, "Don't know what to do with Main Menu ID %d\n", id);
    }
    
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

void DoProjectMenu(int id) {
    WhichProjection = id;
    
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

// use glut to display a string of characters using a raster font:
void DoRasterString(float x, float y, float z, char const *s) {
    glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );
    
    char c;			// one character to print
    for(; (c = *s) != '\0'; s++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
}

// use glut to display a string of characters using a stroke font:
void DoStrokeString(float x, float y, float z, float ht, char const *s) {
    glPushMatrix();
    glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
    float sf = ht / (119.05f + 33.33f);
    glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
    char c;			// one character to print
    for (; (c = *s) != '\0'; s++) {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, c);
    }
    glPopMatrix();
}

// MARK: - GLUT functions

void (*AnimateFunc)() = NULL;

// called when the mouse button transitions down or up:
void MouseButton(int button, int state, int x, int y) {
    int b = 0;			// LEFT, MIDDLE, or RIGHT
    
    if (DebugOn)
        fprintf(stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y);
    
    // get the proper button bit mask:
    switch (button) {
        case GLUT_LEFT_BUTTON:
            b = LEFT;
            break;
            
        case GLUT_MIDDLE_BUTTON:
            b = MIDDLE;
            break;
            
        case GLUT_RIGHT_BUTTON:
            b = RIGHT;
            break;
            
        default:
            b = 0;
            fprintf(stderr, "Unknown mouse button: %d\n", button);
    }
    
    
    // button down sets the bit, up clears the bit:
    if (state == GLUT_DOWN) {
        Xmouse = x;
        Ymouse = y;
        ActiveButton |= b;		// set the proper bit
    } else {
        ActiveButton &= ~b;		// clear the proper bit
    }
}

// called when the mouse moves while a button is down:
void MouseMotion(int x, int y) {
    if (DebugOn)
        fprintf(stderr, "MouseMotion: %d, %d\n", x, y);
    
    
    int dx = x - Xmouse;		// change in mouse coords
    int dy = y - Ymouse;
    
    if (ActiveButton & LEFT) {
        Xrot += (ANGFACT*dy);
        Yrot += (ANGFACT*dx);
    }
    
    
    if (ActiveButton & MIDDLE) {
        Scale += SCLFACT * (float) (dx - dy);
        
        // keep object from turning inside-out or disappearing:
        if (Scale < MINSCALE)
            Scale = MINSCALE;
    }
    
    Xmouse = x;			// new current position
    Ymouse = y;
    
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

// called when user resizes the window:
void Resize(int width, int height) {
    float black[] = { 0, 0, 0, 0 };
    
    if (DebugOn)
        fprintf(stderr, "ReSize: %d, %d\n", width, height);
    
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, (float)width/height, 0.1, 1000);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 1, 3, 0, 1, 0, 0, 1, 0);
    glFogfv(GL_FOG_COLOR, black);
    glFogf(GL_FOG_START, 2.5);
    glFogf(GL_FOG_END, 4);
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);
//    glPointSize(point_size);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHT0);
    
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

// handle a change to the window's visibility:
void Visibility(int state) {
    if (DebugOn)
        fprintf(stderr, "Visibility: %d\n", state);
    if (state == GLUT_VISIBLE) {
        glutIdleFunc(AnimateFunc);
    } else {
        glutIdleFunc(NULL);
    }
}

// return the number of seconds since the start of the program:
float ElapsedSeconds() {
    // get # of milliseconds since the start of the program:
    int ms = glutGet(GLUT_ELAPSED_TIME);
    
    // convert it to seconds:
    return (float)ms / 1000.f;
}

// the stroke characters 'X' 'Y' 'Z' :
static float xx[] = {
    0.f, 1.f, 0.f, 1.f
};
static float xy[] = {
    -.5f, .5f, .5f, -.5f
};
static int xorder[] = {
    1, 2, -3, 4
};
static float yx[] = {
    0.f, 0.f, -.5f, .5f
};
static float yy[] = {
    0.f, .6f, 1.f, 1.f
};
static int yorder[] = {
    1, 2, 3, -2, 4
};
static float zx[] = {
    1.f, 0.f, 1.f, 0.f, .25f, .75f
};
static float zy[] = {
    .5f, .5f, -.5f, -.5f, 0.f, 0.f
};
static int zorder[] = {
    1, 2, 3, 4, -5, 6
};

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;
// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)
void Axes(float length) {
    // Draw axis lines
    glBegin(GL_LINE_STRIP);
    glVertex3f(length, 0., 0.);
    glVertex3f(0., 0., 0.);
    glVertex3f(0., length, 0.);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glVertex3f(0., 0., 0.);
    glVertex3f(0., 0., length);
    glEnd();
    
    
    // Set length fractions
    float fact = LENFRAC * length;
    float base = BASEFRAC * length;
    
    // Draw stroke X
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < 4; i++) {
        int j = xorder[i];
        if (j < 0) {
            glEnd();
            glBegin(GL_LINE_STRIP);
            j = -j;
        }
        j--;
        glVertex3f(base + fact*xx[j], fact*xy[j], 0.0);
    }
    glEnd();
    
    // Draw stroke Y
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < 5; i++) {
        int j = yorder[i];
        if (j < 0) {
            glEnd();
            glBegin(GL_LINE_STRIP);
            j = -j;
        }
        j--;
        glVertex3f(fact*yx[j], base + fact*yy[j], 0.0);
    }
    glEnd();
    
    // Draw stroke Z
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < 6; i++) {
        int j = zorder[i];
        if (j < 0) {
            glEnd();
            glBegin(GL_LINE_STRIP);
            j = -j;
        }
        j--;
        glVertex3f(0.0, fact*zy[j], base + fact*zx[j]);
    }
    glEnd();
    
}

// MARK: - Lighting

void SetMaterial(float r, float g, float b,  float shininess) {
    glMaterialfv( GL_BACK, GL_EMISSION, Array3( 0., 0., 0. ) );
    glMaterialfv( GL_BACK, GL_AMBIENT, MulArray3( .4f, White ) );
    glMaterialfv( GL_BACK, GL_DIFFUSE, MulArray3( 1., White ) );
    glMaterialfv( GL_BACK, GL_SPECULAR, Array3( 0., 0., 0. ) );
    glMaterialf ( GL_BACK, GL_SHININESS, 5.f );
    
    glMaterialfv( GL_FRONT, GL_EMISSION, Array3( 0., 0., 0. ) );
    glMaterialfv( GL_FRONT, GL_AMBIENT, Array3( r, g, b ) );
    glMaterialfv( GL_FRONT, GL_DIFFUSE, Array3( r, g, b ) );
    glMaterialfv( GL_FRONT, GL_SPECULAR, MulArray3( .8f, White ) );
    glMaterialf ( GL_FRONT, GL_SHININESS, shininess );
}


void SetPointLight(int ilight, float x, float y, float z,  float r, float g, float b) {
    glLightfv( ilight, GL_POSITION,  Array3( x, y, z ) );
    glLightfv( ilight, GL_AMBIENT,   Array3( 0., 0., 0. ) );
    glLightfv( ilight, GL_DIFFUSE,   Array3( r, g, b ) );
    glLightfv( ilight, GL_SPECULAR,  Array3( r, g, b ) );
    glLightf ( ilight, GL_CONSTANT_ATTENUATION, 1. );
    glLightf ( ilight, GL_LINEAR_ATTENUATION, 0. );
    glLightf ( ilight, GL_QUADRATIC_ATTENUATION, 0. );
    glEnable( ilight );
}


void SetSpotLight(int ilight, float x, float y, float z,  float xdir, float ydir, float zdir, float r, float g, float b) {
    glLightfv( ilight, GL_POSITION,  Array3( x, y, z ) );
    glLightfv( ilight, GL_SPOT_DIRECTION,  Array3(xdir,ydir,zdir) );
    glLightf(  ilight, GL_SPOT_EXPONENT, 1. );
    glLightf(  ilight, GL_SPOT_CUTOFF, 45. );
    glLightfv( ilight, GL_AMBIENT,   Array3( 0., 0., 0. ) );
    glLightfv( ilight, GL_DIFFUSE,   Array3( r, g, b ) );
    glLightfv( ilight, GL_SPECULAR,  Array3( r, g, b ) );
    glLightf ( ilight, GL_CONSTANT_ATTENUATION, 1. );
    glLightf ( ilight, GL_LINEAR_ATTENUATION, 0. );
    glLightf ( ilight, GL_QUADRATIC_ATTENUATION, 0. );
    glEnable( ilight );
}
