//
//  sphere.cpp
//  CS450 Final Project
//
//  Created by Kyler Stole on 12/1/16.
//  Copyright © 2016 Kyler Stole. All rights reserved.
//

#include "sphere.hpp"

struct point {
    float x, y, z;		// coordinates
    float nx, ny, nz;	// surface normal
    float s, t;         // texture coords
};

int		NumLngs, NumLats;
float   radius;
struct point*	Pts;
int bounceMult;

struct point* PtsPointer(int lat, int lng) {
    if (lat < 0)	lat += (NumLats-1);
    if (lng < 0)	lng += (NumLngs-1);
    if (lat > NumLats-1)	lat -= (NumLats-1);
    if (lng > NumLngs-1)	lng -= (NumLngs-1);
    return &Pts[NumLngs*lat + lng];
}

void DrawPoint(struct point *p) {
    glNormal3f(p->nx, p->ny, p->nz);
    glTexCoord2f(p->s, p->t);
    glVertex3f(p->x, p->y, p->z);
}

void MjbSphere(float rad, int slices, int stacks, float** spec) {
    struct point top, bot;		// top, bottom points
    struct point *p;
    
    // set the globals:
    NumLngs = (slices > 3) ? slices : 3;
    NumLats = (stacks > 3) ? stacks : 3;
    
    // allocate the point data structure:
    Pts = new struct point[NumLngs * NumLats];
    
    // fill the Pts structure:
    for (int ilat = 0; ilat < NumLats; ilat++) {
        float lat = -M_PI/2.  +  M_PI * (float)ilat / (float)(NumLats-1);
        float xz = cos(lat);
        float y = sin(lat);
        for (int ilng = 0; ilng < NumLngs; ilng++) {
            if (!spec) {
                radius = rad;
            } else if (ilat > NumLats/2) {
                float newLat = rerange(ilat, NumLats/2, NumLats, -M_PI, M_PI);
                float bulge = spec[0][ilng];
                radius = rad + (cosf(newLat) + 1) * bounceMult * bulge;
            } else {
                int newLng = ilng + NumLngs/2;
                if (newLng >= NumLngs) newLng -= NumLngs;
                float newLat = rerange(ilat, NumLats/2, NumLats, -M_PI, M_PI);
                float bulge = spec[1][newLng];
                radius = rad + (cosf(newLat) + 1) * bounceMult * bulge;
            }
            
            float lng = -M_PI  +  2. * M_PI * (float)ilng / (float)(NumLngs-1);
            float x =  xz * cos(lng);
            float z = -xz * sin(lng);
            
            p = PtsPointer(ilat, ilng);
            p->x = radius * x;  p->y = radius * y;  p->z = radius * z;
            p->nx = x;          p->ny = y;          p->nz = z;
            if (DistortOn) {
                p->s = (lng + M_PI) / (2.*M_PI);
                p->t = (lat + sinf(2*M_PI*(TimeCycle + (float)ilng/(float)NumLngs)) + M_PI/2.) / M_PI;
            } else {
                p->s = (lng + M_PI) / (2.*M_PI);
                p->t = (lat + M_PI/2.) / M_PI;
            }
        }
    }
    
    top.x =  0.;		top.y  = radius;	top.z = 0.;
    top.nx = 0.;		top.ny = 1.;		top.nz = 0.;
    top.s  = 0.;		top.t  = 1.;
    
    bot.x =  0.;		bot.y  = -radius;	bot.z = 0.;
    bot.nx = 0.;		bot.ny = -1.;		bot.nz = 0.;
    bot.s  = 0.;		bot.t  =  0.;
    
    
    // connect the north pole to the latitude NumLats-2:
    glBegin(GL_QUADS);
    for (int ilng = 0; ilng < NumLngs-1; ilng++) {
        p = PtsPointer(NumLats-1, ilng);
        DrawPoint(p);
        p = PtsPointer(NumLats-2, ilng);
        DrawPoint(p);
        p = PtsPointer(NumLats-2, ilng+1);
        DrawPoint(p);
        p = PtsPointer(NumLats-1, ilng+1);
        DrawPoint(p);
    }
    glEnd();
    
    // connect the south pole to the latitude 1:
    glBegin(GL_QUADS);
    for (int ilng = 0; ilng < NumLngs-1; ilng++) {
        p = PtsPointer(0, ilng);
        DrawPoint(p);
        p = PtsPointer(0, ilng+1);
        DrawPoint(p);
        p = PtsPointer(1, ilng+1);
        DrawPoint(p);
        p = PtsPointer(1, ilng);
        DrawPoint(p);
    }
    glEnd();
    
    // connect the other 4-sided polygons:
    glBegin(GL_QUADS);
    for (int ilat = 2; ilat < NumLats-1; ilat++) {
        for (int ilng = 0; ilng < NumLngs-1; ilng++) {
            p = PtsPointer(ilat-1, ilng);
            DrawPoint(p);
            p = PtsPointer(ilat-1, ilng+1);
            DrawPoint(p);
            p = PtsPointer(ilat, ilng+1);
            DrawPoint(p);
            p = PtsPointer(ilat, ilng);
            DrawPoint(p);
        }
    }
    glEnd();
    
    delete [] Pts;
    Pts = NULL;
}
