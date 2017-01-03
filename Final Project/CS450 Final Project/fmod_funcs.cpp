//
//  fmod_funcs.cpp
//  CS450 Final Project
//
//  Created by Kyler Stole on 11/25/16.
//  Copyright © 2016 Kyler Stole. All rights reserved.
//

#include "fmod_funcs.hpp"

FMOD::System     *fmod_system;
FMOD::Sound      *sound;
FMOD::Channel    *channel = 0;
FMOD::DSP        *fftdsp;
FMOD_RESULT       result;
unsigned int      version;
float **spec;

void ERRCHECK_fn(FMOD_RESULT result, const char *file, int line) {
    if (result != FMOD_OK) {
        printf("%s(%d): FMOD error %d - %s", file, line, result, FMOD_ErrorString(result));
        exit(-1);
    }
}

void cleanFMOD() {
    puts("Cleaning FMOD resources");
    
    delete [] spec[1];
    delete [] spec[0];
    delete [] spec;
    
    ERRCHECK(sound->release());
    ERRCHECK(fftdsp->release());
    ERRCHECK(fmod_system->release());
}

void switchPaused() {
    bool isPaused;
    channel->getPaused(&isPaused);
    channel->setPaused(!isPaused);
}


// ================================================================================================
// Application-independent initialization
// ================================================================================================
void InitFMOD(int res) {
    /*
     Create a System object and initialize.
     */
    result = FMOD::System_Create(&fmod_system);
    ERRCHECK(result);
    
    result = fmod_system->getVersion(&version);
    ERRCHECK(result);
    if (version < FMOD_VERSION) {
        printf("FMOD lib version %08x doesn't match header version %08x", version, FMOD_VERSION);
    }
    
    /* Disable sound if there are no sound drivers */
    int numDrivers;
    result = fmod_system->getNumDrivers(&numDrivers);
    ERRCHECK(result);
    if (numDrivers == 0) {
        result = fmod_system->setOutput(FMOD_OUTPUTTYPE_NOSOUND);
        ERRCHECK(result);
    }
    
    result = fmod_system->init(512, FMOD_INIT_NORMAL, NULL);
    ERRCHECK(result);
    
//    result = fmod_system->createStream("stairway-to-heaven.mp3", FMOD_DEFAULT, 0, &sound);
//    result = fmod_system->createStream("rise.mp3", FMOD_DEFAULT, 0, &sound);
    result = fmod_system->createStream("delta-zone.mp3", FMOD_DEFAULT, 0, &sound);
    ERRCHECK(result);

    // Play the sound
    result = fmod_system->playSound(sound, 0, false, &channel);
    ERRCHECK(result);
    
    fmod_system->createDSPByType(FMOD_DSP_TYPE::FMOD_DSP_TYPE_FFT, &fftdsp);
    //fftdsp->setParameterInt(FMOD_DSP_FFT_WINDOWTYPE, FMOD_DSP_FFT_WINDOW_RECT);
    fftdsp->setParameterInt(FMOD_DSP_FFT_WINDOWSIZE, 2048);
    channel->addDSP(FMOD_DSP_PARAMETER_DATA_TYPE_FFT, fftdsp);
    
    spec = new float*[2*sizeof(float*)];
    spec[0] = new float[res*sizeof(float)];
    spec[1] = new float[res*sizeof(float)];
    
    
    atexit(cleanFMOD);
}

float** freq_analysis(int res) {
    /* Per-frame update code */
    fmod_system->update();
    
//    unsigned int len;
//    char s[256];
//    int val;
//    
//    fftdsp->getParameterInt(FMOD_DSP_FFT_WINDOWSIZE, &val, s, 256);
//    printf("Window size: %d. %s\n", val, s);
    
    FMOD_DSP_PARAMETER_FFT *fftdata;
    result = fftdsp->getParameterData(FMOD_DSP_FFT_SPECTRUMDATA, (void **)&fftdata, NULL, NULL, 0);
    ERRCHECK(result);
    
//    for (int channel = 0; channel < 2; channel++)
//        for (int bin = 0; bin < res; bin++)
//            spec[channel][bin] = (fftdata->spectrum[channel][bin] +
//                                  fftdata->spectrum[channel][bin+1] +
//                                  fftdata->spectrum[channel][bin+2] +
//                                  fftdata->spectrum[channel][bin+3]) / 4;
    
    if (fftdata->length < res) return NULL;
    
    for (int channel = 0; channel < 2; channel++) {
        for (int bin = 0; bin < res-3; bin += 4) {
            spec[channel][bin] = fftdata->spectrum[channel][bin/4];
            spec[channel][bin+2] = (fftdata->spectrum[channel][bin/4] +
                                    fftdata->spectrum[channel][bin/4+1]) / 2;
            spec[channel][bin+1] = (spec[channel][bin] +
                                    spec[channel][bin+2]) / 2;
            spec[channel][bin+3] = (spec[channel][bin+2] +
                                    fftdata->spectrum[channel][bin/4+1]) / 2;
            
        }
        spec[channel][res-1] = spec[channel][0];
        spec[channel][res-2] = 0.33*spec[channel][res-4] + 0.67*spec[channel][res-1];
        spec[channel][res-3] = 0.67*spec[channel][res-4] + 0.33*spec[channel][res-1];
    }
    spec[1][res/2-1] = spec[1][res/2];
    spec[1][res/2-2] = 0.33*spec[1][res/2-4] + 0.67*spec[1][res/2-1];
    spec[1][res/2-3] = 0.67*spec[1][res/2-4] + 0.33*spec[1][res/2-1];
    
    

    
//    // Find max volume
//    auto maxIterator = std::max_element(&fftdata->spectrum[0][0], &fftdata->spectrum[0][fftdata->length]);
//    float maxVol = *maxIterator;
//    
//    // Normalize
//    if (maxVol != 0) {
//        std::transform(&fftdata->spectrum[0][0], &fftdata->spectrum[0][fftdata->length], &fftdata->spectrum[0][0], [maxVol] (float dB) -> float { return dB / maxVol; });
//        std::transform(&fftdata->spectrum[1][0], &fftdata->spectrum[1][fftdata->length], &fftdata->spectrum[1][0], [maxVol] (float dB) -> float { return dB / maxVol; });
//    }
    
    return spec;
}
