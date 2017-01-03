//
//  fmod_funcs.hpp
//  CS450 Final Project
//
//  Created by Kyler Stole on 11/25/16.
//  Copyright © 2016 Kyler Stole. All rights reserved.
//

#ifndef fmod_funcs_hpp
#define fmod_funcs_hpp

#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <math.h>

#include "fmod.hpp"
#include "fmod_errors.h"

void InitFMOD(int res);
float** freq_analysis(int res);
void switchPaused();

void ERRCHECK_fn(FMOD_RESULT result, const char *file, int line);
#define ERRCHECK(_result) ERRCHECK_fn(_result, __FILE__, __LINE__)

#endif /* fmod_funcs_hpp */
