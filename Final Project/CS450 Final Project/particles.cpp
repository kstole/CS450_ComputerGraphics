//
//  particles.cpp
//  CS450 Final Project
//
//  Created by Kyler Stole on 12/3/16.
//  Copyright © 2016 Kyler Stole. All rights reserved.
//

#include "particles.hpp"


#ifdef _WIN32
#define drand48() ((float)rand()/RAND_MAX)
#endif

#define PS_GRAVITY -0.8

typedef struct {
    float x, y, z;
    float radius;
} PSsphere;

typedef struct {
    float position[3]; // current position
    float previous[3]; // previous position
    float velocity[3]; // velocity (mag & direction)
    float dampening;   // % energy lost on collision
    int alive;         // is this particle alive?
    
    void pos(float x, float y, float z) {
        position[0] = x;
        position[1] = y;
        position[2] = z;
    }
} PSparticle;

PSparticle* particles = NULL;

int numParticles = 10000;
int particleSize = 20;
float frame_time = 0;
float flow = 500;
float sphereRadius = 1;

void setSphereRadius(float rad) {
    sphereRadius = rad;
}

/* timedelta: returns the number of seconds that have elapsed since
 the previous call to the function. */
#if defined(_WIN32)
#include <sys/timeb.h>
#else
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/times.h>
#endif

#ifndef CLK_TCK
#define CLK_TCK 1000
#endif

float timedelta(void) {
    static long begin = 0;
    static long finish, difference;
    
#if defined(_WIN32)
    static struct timeb tb;
    ftime(&tb);
    finish = tb.time*1000+tb.millitm;
#else
    static struct tms tb;
    finish = times(&tb);
#endif
    
    difference = finish - begin;
    begin = finish;
    
    return (float)difference/(float)CLK_TCK;
}

int fequal(float a, float b) {
    float epsilon = 0.1;
    float f = a - b;
    
    if (f < epsilon && f > -epsilon)
        return 1;
    else
        return 0;
}

void psTimeStep(PSparticle* p, float dt) {
    if (p->alive == 0)
        return;
    
    p->velocity[0] += 0;
    p->velocity[1] += PS_GRAVITY*dt;
    p->velocity[2] += 0;
    
    p->previous[0] = p->position[0];
    p->previous[1] = p->position[1];
    p->previous[2] = p->position[2];
    
    p->position[0] += p->velocity[0]*dt;
    p->position[1] += p->velocity[1]*dt;
    p->position[2] += p->velocity[2]*dt;
}

void psNewParticle(PSparticle* p, float dt) {
    p->pos(0, 9, 0);
    p->previous[0] = p->position[0];
    p->previous[1] = p->position[1];
    p->previous[2] = p->position[2];
    p->velocity[0] = 2*(drand48()-0.5);
    p->velocity[1] = 2*(drand48()-0.5);
    p->velocity[2] = 2*(drand48()-0.5);
    p->dampening = 0.45*drand48();
    p->alive = 1;
    
    psTimeStep(p, 2*dt*drand48());
}

/* psBounce: the particle has gone past (or exactly hit) the ground plane, so calculate the time at which the particle actually intersected the ground plane (s). essentially, this just rolls back time to when the particle hit the ground plane, then starts time again from then.
 
 - -   o A (previous position)
 | |    \
 | s     \   o (position it _should_ be at) -
 t |      \ /                               | t - s
 | - ------X--------                        -
 |          \
 -           o B (new position)
 
 A + V*s = 0 or s = -A/V
 
 to calculate where the particle should be:
 
 A + V*t + V*(t-s)*d
 
 where d is a damping factor which accounts for the loss
 of energy due to the bounce. */
void psBounce(PSparticle* p, float dt) {
    float s;
    
    // return if the particle is dead
    if (!p->alive) return;
    
    /* since we know it is the ground plane, we only need to calculate s for a single dimension. */
    s = -p->previous[1]/p->velocity[1];
    
    p->position[0] = (p->previous[0] + p->velocity[0] * s +
                      p->velocity[0] * (dt-s) * p->dampening);
    p->position[1] = -p->velocity[1] * (dt-s) * p->dampening; /* reflect */
    p->position[2] = (p->previous[2] + p->velocity[2] * s +
                      p->velocity[2] * (dt-s) * p->dampening);
    
    /* dampen the reflected velocity (since the particle hit something, it lost energy) */
    p->velocity[0] *= p->dampening;
    p->velocity[1] *= -p->dampening;
    p->velocity[2] *= p->dampening;
}

void psCollideSphere(PSparticle* p) {
    float sphere_x = 0, sphere_y = 0, sphere_z = 0;
    float vx = p->position[0] - sphere_x;
    float vy = p->position[1] - sphere_y;
    float vz = p->position[2] - sphere_z;
    float distance;
    
    if (p->alive == 0) return;
    
    distance = sqrt(vx*vx + vy*vy + vz*vz);
    
    if (distance < sphereRadius) {
        p->position[0] = sphere_x+(vx/distance)*sphereRadius;
        p->position[1] = sphere_y+(vy/distance)*sphereRadius;
        p->position[2] = sphere_z+(vz/distance)*sphereRadius;
        p->previous[0] = p->position[0];
        p->previous[1] = p->position[1];
        p->previous[2] = p->position[2];
        p->velocity[0] = vx/distance;
        p->velocity[1] = vy/distance;
        p->velocity[2] = vz/distance;
    }
}


void reshape(int width, int height) {
    float black[] = { 0, 0, 0, 0 };
    
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
    glPointSize(particleSize);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHT0);
    
    timedelta();
}

void drawParticles() {
    static float c;
    
    glPushMatrix();
    
        glBegin(GL_POINTS);
        for (int i = 0; i < numParticles; i++) {
            if (!particles[i].alive) continue;
            c = particles[i].position[1]/2.1*255;
            float height = fabs(particles[i].position[1]);
            
            glColor4ub(height*128, 128, 128, 80);
            glVertex3fv(particles[i].position);
        }
        glEnd();
    
    glPopMatrix();
}

void idleParticles(void) {
    static int i;
    static int living = 0;  /* index to end of live particles */
    static float dt;
    
    dt = timedelta();
    frame_time += dt;
    
#if 1
    /* slow the simulation if we can't keep the frame rate up around 10 fps */
    static float slow_down = 1;
    if (dt > 0.1) {
        slow_down = 1.0/(100*dt);
    } else if (dt < 0.1) {
        slow_down = 1;
    }
    dt *= slow_down;
#endif
    
    /* resurrect a few particles */
    for (i = 0; i < flow * dt; i++) {
        psNewParticle(&particles[living], dt);
        living++;
        if (living >= numParticles)
            living = 0;
    }
    
    for (i = 0; i < numParticles; i++) {
        psTimeStep(&particles[i], dt);
        
        /* collision with sphere? */
        psCollideSphere(&particles[i]);
                
        /* collision with ground? */
        if (particles[i].position[1] <= -2) {
            psBounce(&particles[i], dt);
        }
        
        /* dead particle? */
        if (particles[i].position[1] < -1.9 &&
            fequal(particles[i].velocity[1], 0)) {
            particles[i].alive = 0;
        }
    }
    
    glutPostRedisplay();
}

void DoParticleMenu(int id) {
    switch (id) {
            
        case '+':
            // increase flow
            flow += 100;
            if (flow > numParticles)
                flow = numParticles;
            printf("%g particles/second\n", flow);
            break;
            
        case '-':
            // decrease flow
            flow -= 100;
            if (flow < 0)
                flow = 0;
            printf("%g particles/second\n", flow);
            break;
    }
}

void menustate(int state) {
    /* hook up a fake time delta to avoid jumping when menu comes up */
    if (state == GLUT_MENU_NOT_IN_USE)
        timedelta();
}

void cleanParticles() {
    puts("Cleaning particles resources");
    
    free(particles);
}

void InitParticles() {
    particles = (PSparticle*)malloc(sizeof(PSparticle) * numParticles);
    
    atexit(cleanParticles);
}
