//	Final Project: Music Visualizer
//
//	The objective is to create a 3D music visualizer.
//
//	The left mouse button does rotation
//	The middle mouse button does scaling
//	The user interface allows:
//		1. The axes to be turned on and off
//		2. The color of the axes to be changed
//		3. Debugging to be turned on and off
//		4. Depth cueing to be turned on and off
//		5. The projection to be changed
//		6. The transformations to be reset
//		7. The program to quit
//  The keyboard allows:
//      m. Toggle music
//      s. Toggle stage
//      p. Toggle particles
//      v. Toggle visualizer
//      r. Toggle rotation
//      0,1,2. Toggle lights
//
//	Author:			Kyler Stole


/* --- Note: Requires the FMOD low level API to be installed --- */

#include <stdio.h>
#include <cstdlib>
#include <ctype.h>
#include <cmath>

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

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include "fmod_funcs.hpp"
#include "glut_funcs.hpp"
#include "utility_funcs.hpp"
#include "sphere.hpp"
#include "particles.hpp"
#include "BmpToTexture.hpp"

// title of these windows:
const char *WINDOWTITLE = { "OpenGL / Final Project -- Kyler Stole" };
const char *GLUITITLE   = { "User Interface Window" };
// initial window size:
#define INIT_WINDOW_SIZE 600

// animation cycle time
#define MS_IN_THE_ANIMATION_CYCLE 2000

// fog parameters:
const GLfloat FOGCOLOR[4] = {.0, .0, .0, 1.};
const GLenum  FOGMODE     = {GL_LINEAR};
const GLfloat FOGDENSITY  = {0.30f};
const GLfloat FOGSTART    = {1.5};
const GLfloat FOGEND      = {4.};

struct xyz {
    float x;
    float y;
    float z;
};

bool VisualizerOn;
bool ParticlesOn;
bool StageOn;
bool    Light0On;
bool    Light1On;
bool    Light2On;
bool RotateOn;

// window background color (rgba):
const GLfloat BACKCOLOR[] = { 0., 0., 0., 1. };

// line width for the axes:
#define AXES_WIDTH      3.

// sphere parameters:
#define SPHERE_RADIUS   1
#define SPHERE_SLICES   100
#define SPHERE_STACKS   50


// MARK: - Main
int main(int argc, char *argv[]) {
    // turn on the glut package:
    // (do this before checking argc and argv since it might
    // pull some command line arguments out)
    glutInit(&argc, argv);
    
    InitFMOD(SPHERE_SLICES);
    InitGraphics();
    InitTextures(); // import textures
    InitLists(); // display structures that will not change
    InitParticles();
    setSphereRadius(SPHERE_RADIUS);
    
    Reset(); // init global vars used by Display() (and post redisplay)
    
    InitMenus(); // builds right-click menu
    
    // draw the scene once and wait for some interaction:
    // (this will never return)
    glutSetWindow(MainWindow);
    glutMainLoop();
    
    return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display() from here -- let glutMainLoop() do it

void Animate() {
    // put animation stuff in here -- change some global variables
    // for Display() to find:
    int ms = glutGet(GLUT_ELAPSED_TIME);                    // milliseconds
    int ms2 = ms % (4*MS_IN_THE_ANIMATION_CYCLE);
    Time = (float)ms / (float)1000;
    ms %= MS_IN_THE_ANIMATION_CYCLE;
    TimeCycle = (float)ms / (float)MS_IN_THE_ANIMATION_CYCLE;    // [0., 1.]
    
    SwitchCycle = (float)ms2 / ((float)MS_IN_THE_ANIMATION_CYCLE*4);
    if (SwitchCycle < 0.5) DayMode = true;
    else DayMode = false;
    
    idleParticles();
    
    // force a call to Display() next time it is convenient:
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

void drawPlatformOld() {
    glPushMatrix();
    
    glBegin(GL_QUADS);
    glColor3ub(0, 128, 128);
    glVertex3f(-2, -2, -2);
    glVertex3f(-2, -2, 2);
    glVertex3f(2, -2, 2);
    glVertex3f(2, -2, -2);
    glEnd();
    
    glPopMatrix();
}

#define STAGE_LEFT      -2
#define STAGE_RIGHT     2
#define STAGE_HEIGHT    -2
#define STAGE_RES       10

void drawStage(float **spec) {
    if (!spec) return;
    
    glPushMatrix();

    glColor3ub(0, 128, 128);
    
    float divs = (float)(STAGE_RIGHT - STAGE_LEFT) / STAGE_RES;
    for (float x = STAGE_LEFT; x < STAGE_RIGHT; x += divs) {
        float newX = rerange(x, STAGE_LEFT, STAGE_RIGHT, -M_PI, M_PI);
        float xBulge =  (cosf(newX) + 1) * spec[1][5];
        glBegin(GL_TRIANGLE_STRIP);
        for (float z = STAGE_LEFT; z < STAGE_RIGHT; z += divs) {
            float newZ = rerange(z, STAGE_LEFT, STAGE_RIGHT, -M_PI, M_PI);
            float zBulge =  (cosf(newZ) + 1) * spec[0][5];
            float y = STAGE_HEIGHT - xBulge * zBulge * 80;
            glVertex3f(x, y, z);
            glVertex3f(x+divs, y, z);
        }
        glEnd();
    }
    
    glPopMatrix();
}

// draw the complete scene:

void Display() {
    if (DebugOn)
        fprintf(stderr, "Display\n");
    
    
    // set which window we want to do the graphics into:
    glutSetWindow(MainWindow);
    
    
    // erase the background:
    glDrawBuffer(GL_BACK);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    
    
    // specify shading to be smooth:
    glShadeModel(GL_SMOOTH);
    
    
    // set the viewport to a square centered in the window:
    GLsizei vx = glutGet(GLUT_WINDOW_WIDTH);
    GLsizei vy = glutGet(GLUT_WINDOW_HEIGHT);
    GLsizei v = vx < vy ? vx : vy;			// minimum dimension
    GLint xl = (vx - v) / 2;
    GLint yb = (vy - v) / 2;
    glViewport(xl, yb,  v, v);
    
    
    // set the viewing volume:
    // remember that the Z clipping  values are actually
    // given as DISTANCES IN FRONT OF THE EYE
    // USE gluOrtho2D() IF YOU ARE DOING 2D !
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (WhichProjection == ORTHO)
        glOrtho(-3., 3.,     -3., 3.,     0.1, 1000.);
    else
        gluPerspective(90., 1.,	0.1, 1000.);
    
    
    // place the objects into the scene:
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    
    // set the eye position, look-at position, and up-vector:
    gluLookAt(0., 0., 3.,     0., 0., 0.,     0., 1., 0.);
    
    
    xyz l1pos = {2, 2, 2};
    glPushMatrix();
    glTranslatef(l1pos.x, l1pos.y, l1pos.z);
    glColor3f(0., 0.5, 0.5);
    //glutSolidSphere(0.05, 50, 50);
    SetPointLight(GL_LIGHT0, l1pos.x, l1pos.y, l1pos.z, 0., 0.5, 0.5);
    glPopMatrix();
    
    // rotate the scene:
    glRotatef((GLfloat)Yrot, 0., 1., 0.);
    glRotatef((GLfloat)Xrot, 1., 0., 0.);
    
    
    // uniformly scale the scene:
    if (Scale < MINSCALE)
        Scale = MINSCALE;
    glScalef((GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale);
    
    
    // set the fog parameters:
    if (DepthCueOn) {
        glFogi(GL_FOG_MODE, FOGMODE);
        glFogfv(GL_FOG_COLOR, FOGCOLOR);
        glFogf(GL_FOG_DENSITY, FOGDENSITY);
        glFogf(GL_FOG_START, FOGSTART);
        glFogf(GL_FOG_END, FOGEND);
        glEnable(GL_FOG);
    } else {
        glDisable(GL_FOG);
    }
    
    // color the scene
    glColor3fv(&Colors[WhichColor][0]);
    
    // possibly draw the axes:
    if (AxesOn)
        glCallList(AxesList);
    
    
    // since we are using glScalef(), be sure normals get unitized:
    glEnable(GL_NORMALIZE);
    
    // glut solids
    
    xyz l2pos = {(float)cos(Time), -1, (float)sin(Time)};
    glPushMatrix();
    glTranslatef(l2pos.x, l2pos.y, l2pos.z);
    glColor3f(1., 1., 1.);
    //glutSolidSphere(0.05, 50, 50);
    SetPointLight(GL_LIGHT2, l2pos.x, l2pos.y, l2pos.z, 1., 1., 1.);
    glPopMatrix();
    
    xyz l0pos = {0, 0.3, -1.5};
    glPushMatrix();
    glTranslatef(l0pos.x, l0pos.y, l0pos.z);
    glColor3f(0.73, 0.29, 0.31);
    //glutSolidSphere(0.05, 50, 50);
    SetSpotLight(GL_LIGHT1, l0pos.x, l0pos.y, l0pos.z, -l0pos.x, -l0pos.y, -l0pos.z, .73, .29, .31);
    glPopMatrix();
    
    (Light0On) ? glEnable(GL_LIGHT0) : glDisable(GL_LIGHT0);
    (Light1On) ? glEnable(GL_LIGHT1) : glDisable(GL_LIGHT1);
    (Light2On) ? glEnable(GL_LIGHT2) : glDisable(GL_LIGHT2);
    
    float **spec = freq_analysis(SPHERE_SLICES);
    
    glEnable(GL_LIGHTING);
    
    /* Visualizers */
    glPushMatrix();
    glShadeModel(GL_SMOOTH);
    SetMaterial(1., 0.7, 1., 50);
    glColor3ub(51, 205, 225);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    if (TextureOn) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texDay);
    }

    if (RotateOn) glRotatef(TimeCycle*360, 0., 1., 0.);
    if (VisualizerOn) MjbSphere(SPHERE_RADIUS, SPHERE_SLICES, SPHERE_STACKS, spec);
    
    if (TextureOn) glDisable(GL_TEXTURE_2D);
    
    glDisable(GL_LIGHTING);
    glPopMatrix();
    
    /* Particles */
    if (ParticlesOn) drawParticles();
    
    /* Stage */
    if (StageOn) drawStage(spec);
    
    
    // draw some gratuitous text that just rotates on top of the scene:
    //    glDisable(GL_DEPTH_TEST);
    //    glColor3f(0., 1., 1.);
    //    DoRasterString(0., 1., 0., "Wha!!!");
    
    
    // draw some gratuitous text that is fixed on the screen:
    //
    // the projection matrix is reset to define a scene whose
    // world coordinate system goes from 0-100 in each axis
    //
    // this is called "percent units", and is just a convenience
    //
    // the modelview matrix is reset to identity as we don't
    // want to transform these coordinates
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0., 100., 0., 100.);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor3f(1., 1., 1.);
    DoRasterString(5., 5., 0., "Kyler Stole - CS 450 - Final Project");
    
    
    // swap the double-buffered framebuffers:
    glutSwapBuffers();
    
    
    // be sure the graphics buffer has been sent:
    // note: be sure to use glFlush() here, not glFinish() !
    glFlush();
}

void DoBulgeMenu(int id) {
    bounceMult = id;
    
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}


// initialize the glui window:
void InitMenus() {
    glutSetWindow(MainWindow);
    
    int numColors = sizeof(Colors) / (3 * sizeof(int));
    int colormenu = glutCreateMenu(DoColorMenu);
    for (int i = 0; i < numColors; i++)
        glutAddMenuEntry(ColorNames[i], i);
    
    int axesmenu = glutCreateMenu(DoAxesMenu);
    glutAddMenuEntry("Off",  0);
    glutAddMenuEntry("On",   1);
    
    int distortmenu = glutCreateMenu(DoDistortMenu);
    glutAddMenuEntry("No texture",                  NoTex);
    glutAddMenuEntry("Texture without distortion",  NoDistort);
    glutAddMenuEntry("Texture with distortion",     Distort);
    
    int depthcuemenu = glutCreateMenu(DoDepthMenu);
    glutAddMenuEntry("Off",  0);
    glutAddMenuEntry("On",   1);
    
    int debugmenu = glutCreateMenu(DoDebugMenu);
    glutAddMenuEntry("Off",  0);
    glutAddMenuEntry("On",   1);
    
    int projmenu = glutCreateMenu(DoProjectMenu);
    glutAddMenuEntry("Orthographic",  ORTHO);
    glutAddMenuEntry("Perspective",   PERSP);
    
    int particlemenu = glutCreateMenu(DoParticleMenu);
    glutAddMenuEntry("[-] Less flow", '-');
    glutAddMenuEntry("[+] More flow", '+');
    
    int bulgemenu = glutCreateMenu(DoBulgeMenu);
    glutAddMenuEntry("2", 2);
    glutAddMenuEntry("4", 4);
    glutAddMenuEntry("6", 6);
    glutAddMenuEntry("8", 8);
    
    glutCreateMenu(DoMainMenu);
    glutAddSubMenu(  "Axes",          axesmenu);
    glutAddSubMenu(  "Distortion",    distortmenu);
    glutAddSubMenu(  "Colors",        colormenu);
    glutAddSubMenu(  "Depth Cue",     depthcuemenu);
    glutAddSubMenu(  "Projection",    projmenu);
    glutAddSubMenu(  "Particles",     particlemenu);
    glutAddSubMenu(  "Bulge",         bulgemenu);
    glutAddMenuEntry("Reset",         RESET);
    glutAddSubMenu(  "Debug",         debugmenu);
    glutAddMenuEntry("Quit",          QUIT);
    
    // attach the pop-up menu to the right mouse button:
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

// import textures
void InitTextures() {
    int width, height, level, ncomps, border;
    level = 0;
    ncomps = 3;
    border = 0;
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    glGenTextures(1, &texDay);
    glGenTextures(1, &texLight);
    glGenTextures(1, &texMoon);
    
    width = 1024;
    height = 512;
    unsigned char* Texture0 = BmpToTexture((char*)"worldtex.bmp", &width, &height);
    
    glBindTexture(GL_TEXTURE_2D, texDay);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, level, ncomps, width, height, border, GL_RGB, GL_UNSIGNED_BYTE, Texture0);
    
//    width = 2048;
//    height = 1024;
//    unsigned char* Texture1 = BmpToTexture((char*)"night_world.bmp", &width, &height);
//    
//    glBindTexture(GL_TEXTURE_2D, texLight);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexImage2D(GL_TEXTURE_2D, level, ncomps, width, height, border, GL_RGB, GL_UNSIGNED_BYTE, Texture1);
//    
//    width = 4096;
//    height = 2048;
//    unsigned char* Texture2 = BmpToTexture((char*)"moon.bmp", &width, &height);
//    
//    glBindTexture(GL_TEXTURE_2D, texMoon);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexImage2D(GL_TEXTURE_2D, level, ncomps, width, height, border, GL_RGB, GL_UNSIGNED_BYTE, Texture2);
    
    
    glMatrixMode(GL_TEXTURE);
}


// initialize the glut and OpenGL libraries:
//	also setup display lists and callback functions
void InitGraphics() {
    // request the display modes:
    // ask for red-green-blue-alpha color, double-buffering, and z-buffering:
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    
    // set the initial window configuration:
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(INIT_WINDOW_SIZE, INIT_WINDOW_SIZE);
    
    // open the window and set its title:
    MainWindow = glutCreateWindow(WINDOWTITLE);
    glutSetWindowTitle(WINDOWTITLE);
    
    // set the framebuffer clear values:
    glClearColor(BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3]);
    
    // set lights
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, MulArray3(.3f, White));
    
    // setup the callback functions:
    // DisplayFunc -- redraw the window
    // ReshapeFunc -- handle the user resizing the window
    // KeyboardFunc -- handle a keyboard input
    // MouseFunc -- handle the mouse button going down or up
    // MotionFunc -- handle the mouse moving with a button down
    // PassiveMotionFunc -- handle the mouse moving with a button up
    // VisibilityFunc -- handle a change in window visibility
    // EntryFunc	-- handle the cursor entering or leaving the window
    // SpecialFunc -- handle special keys on the keyboard
    // SpaceballMotionFunc -- handle spaceball translation
    // SpaceballRotateFunc -- handle spaceball rotation
    // SpaceballButtonFunc -- handle spaceball button hits
    // ButtonBoxFunc -- handle button box hits
    // DialsFunc -- handle dial rotations
    // TabletMotionFunc -- handle digitizing tablet motion
    // TabletButtonFunc -- handle digitizing tablet button hits
    // MenuStateFunc -- declare when a pop-up menu is in use
    // TimerFunc -- trigger something to happen a certain time from now
    // IdleFunc -- what to do when nothing else is going on
    
    glutSetWindow(MainWindow);
    glutDisplayFunc(Display);
    glutReshapeFunc(Resize);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(Keyboard);
    glutMouseFunc(MouseButton);
    glutMotionFunc(MouseMotion);
    glutPassiveMotionFunc(NULL);
    glutVisibilityFunc(Visibility);
    glutEntryFunc(NULL);
    glutSpecialFunc(NULL);
    glutSpaceballMotionFunc(NULL);
    glutSpaceballRotateFunc(NULL);
    glutSpaceballButtonFunc(NULL);
    glutButtonBoxFunc(NULL);
    glutDialsFunc(NULL);
    glutTabletMotionFunc(NULL);
    glutTabletButtonFunc(NULL);
    glutMenuStateFunc(menustate);
    glutTimerFunc(-1, NULL, 0);
    AnimateFunc = Animate;
    
    // init glew (a window must be open to do this):
    
#ifdef WIN32
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "glewInit Error\n");
    }
    else
        fprintf(stderr, "GLEW initialized OK\n");
    fprintf( stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif
    
}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList()
void InitLists() {
    glutSetWindow(MainWindow);
    
    // create the axes
    AxesList = glGenLists(1);
    glNewList(AxesList, GL_COMPILE);
    glLineWidth(AXES_WIDTH);
    Axes(1.5);
    glLineWidth(1.);
    glEndList();
}


// the keyboard callback:
void Keyboard(unsigned char c, int x, int y) {
    if (DebugOn)
        fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );
    
    switch (c) {
        case 'v': case 'V':
            VisualizerOn = !VisualizerOn;
            break;
            
        case 'p': case 'P':
            ParticlesOn = !ParticlesOn;
            break;
            
        case 's': case 'S':
            StageOn = !StageOn;
            break;
            
        case 'm': case 'M':
            switchPaused();
            break;
            
        case 'q': case 'Q': case ESCAPE:
            DoMainMenu(QUIT);	// will not return here
            break;				// happy compiler
            
        case 'f': case 'F':
            Frozen = !Frozen;
            if (Frozen) glutIdleFunc(NULL);
            else glutIdleFunc(Animate);
            break;
            
        case 'r': case 'R':
            RotateOn = !RotateOn;
            break;
            
        case '0':
            Light0On = !Light0On;
            break;
        case '1':
            Light1On = !Light1On;
            break;
        case '2':
            Light2On = !Light2On;
            break;
            
        default:
            fprintf(stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c);
    }
    
    // force a call to Display():
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}



// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene
void Reset() {
    ActiveButton = 0;
    AxesOn = false;
    DistortOn = false;
    TextureOn = false;
    DebugOn = false;
    DepthCueOn = false;
    Scale  = 1.0;
    WhichColor = WHITE;
    WhichProjection = ORTHO;
    Xrot = Yrot = 0.;
    Frozen = false;
    Light0On = true;
    Light1On = true;
    Light2On = true;
    ParticlesOn = false;
    VisualizerOn = true;
    StageOn = false;
    bounceMult = 8;
    RotateOn = false;
}


