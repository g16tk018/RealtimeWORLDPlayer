//-----------------------------------------------------------------------------
// Copyright 2012-2016 Masanori Morise. All Rights Reserved.
// Author: mmorise [at] yamanashi.ac.jp (Masanori Morise)
//
// Test program for WORLD 0.1.2 (2012/08/19)
// Test program for WORLD 0.1.3 (2013/07/26)
// Test program for WORLD 0.1.4 (2014/04/29)
// Test program for WORLD 0.1.4_3 (2015/03/07)
// Test program for WORLD 0.2.0 (2015/05/29)
// Test program for WORLD 0.2.0_1 (2015/05/31)
// Test program for WORLD 0.2.0_2 (2015/06/06)
// Test program for WORLD 0.2.0_3 (2015/07/28)
// Test program for WORLD 0.2.0_4 (2015/11/15)
// Test program for WORLD in GitHub (2015/11/16-)
// Latest update: 2016/02/02

// test.exe input.wav outout.wav f0 spec
// input.wav  : Input file
// output.wav : Output file
// f0         : F0 scaling (a positive number)
// spec       : Formant scaling (a positive number)
//-----------------------------------------------------------------------------
#include "header/ctest_c.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#if (defined (__WIN32__) || defined (_WIN32)) && !defined (__MINGW32__)
#include <conio.h>
#include <windows.h>
#pragma comment(lib, "winmm.lib")
#pragma warning(disable : 4996)
#endif
#if (defined (__linux__) || defined(__CYGWIN__) || defined(__APPLE__))
#include <stdint.h>
#include <sys/time.h>
#endif

// For .wav input/output functions.
#include "header/audioio.h"

// WORLD core functions.
#include "header/d4c.h"
#include "header/dio.h"
#include "header/matlabfunctions.h"
#include "header/cheaptrick.h"
#include "header/stonemask.h"
#include "header/synthesis.h"
#include "header/synthesisrealtime.h"

#if (defined (__linux__) || defined(__CYGWIN__) || defined(__APPLE__))
// Linux porting section: implement timeGetTime() by gettimeofday(),
#ifndef DWORD
#define DWORD uint32_t
#endif
DWORD timeGetTime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    DWORD ret = (DWORD)(tv.tv_usec / 1000 + tv.tv_sec * 1000);
    return ret;
}
#endif

static void DisplayInformation(int fs, int nbit, int x_length) {
    printf("File information\n");
    printf("Sampling : %d Hz %d Bit\n", fs, nbit);
    printf("Length %d [sample]\n", x_length);
    printf("Length %f [sec]\n", (double)(x_length) / fs);
}

static void F0Estimation(double *x, int x_length, WorldParameters *world_parameters) {
    DioOption option = {0};
    InitializeDioOption(&option);
    
    // Modification of the option
    // When you You must set the same value.
    // If a different value is used, you may suffer a fatal error because of a
    // illegal memory access.
    option.frame_period = world_parameters->frame_period;
    
    // Valuable option.speed represents the ratio for downsampling.
    // The signal is downsampled to fs / speed Hz.
    // If you want to obtain the accurate result, speed should be set to 1.
    option.speed = 1;
    
    // You should not set option.f0_floor to under world_kFloorF0.
    // If you want to analyze such low F0 speech, please change world_kFloorF0.
    // Processing speed may sacrify, provided that the FFT length changes.
    option.f0_floor = 71.0;
    
    // You can give a positive real number as the threshold.
    // Most strict value is 0, but almost all results are counted as unvoiced.
    // The value from 0.02 to 0.2 would be reasonable.
    option.allowed_range = 0.1;
    
    // Parameters setting and memory allocation.
    world_parameters->f0_length = GetSamplesForDIO(world_parameters->fs,
                                                   x_length, world_parameters->frame_period);
    world_parameters->f0 = (double*) malloc(sizeof(double) * (world_parameters->f0_length));
    world_parameters->time_axis = (double*) malloc(sizeof(double) * (world_parameters->f0_length));
    double *refined_f0 = (double*) malloc(sizeof(double) * (world_parameters->f0_length));
    
    printf("\nAnalysis\n");
    DWORD elapsed_time = timeGetTime();
    Dio(x, x_length, world_parameters->fs, &option, world_parameters->time_axis,
        world_parameters->f0);
    printf("DIO: %d [msec]\n", timeGetTime() - elapsed_time);
    
    // StoneMask is carried out to improve the estimation performance.
    elapsed_time = timeGetTime();
    StoneMask(x, x_length, world_parameters->fs, world_parameters->time_axis,
              world_parameters->f0, world_parameters->f0_length, refined_f0);
    printf("StoneMask: %d [msec]\n", timeGetTime() - elapsed_time);
    
    for (int i = 0; i < world_parameters->f0_length; ++i)
        world_parameters->f0[i] = refined_f0[i];
    
    if(refined_f0 != NULL) {
        free(refined_f0);
        refined_f0 = NULL;
    }
    return;
}

static void SpectralEnvelopeEstimation(double *x, int x_length,
                                       WorldParameters *world_parameters) {
    CheapTrickOption option = {0};
    InitializeCheapTrickOption(&option);
    
    // This value may be better one for HMM speech synthesis.
    // Default value is -0.09.
    option.q1 = -0.15;
    
    // Important notice (2016/02/02)
    // You can control a parameter used for the lowest F0 in speech.
    // You must not set the f0_floor to 0.
    // It will cause a fatal error because fft_size indicates the infinity.
    // You must not change the f0_floor after memory allocation.
    // You should check the fft_size before excucing the analysis/synthesis.
    // The default value (71.0) is strongly recommended.
    // On the other hand, setting the lowest F0 of speech is a good choice
    // to reduce the fft_size.
    option.f0_floor = 71.0;
    
    // Parameters setting and memory allocation.
    world_parameters->fft_size =
    GetFFTSizeForCheapTrick(world_parameters->fs, &option);
    world_parameters->spectrogram = (double **) malloc(sizeof(double *) * (world_parameters->f0_length));
    for (int i = 0; i < world_parameters->f0_length; ++i) {
        world_parameters->spectrogram[i] =
        (double*) malloc(sizeof(double) * (world_parameters->fft_size / 2 + 1));
    }
    
    DWORD elapsed_time = timeGetTime();
    CheapTrick(x, x_length, world_parameters->fs, world_parameters->time_axis,
               world_parameters->f0, world_parameters->f0_length, &option,
               world_parameters->spectrogram);
    printf("CheapTrick: %d [msec]\n", timeGetTime() - elapsed_time);
}

static void AperiodicityEstimation(double *x, int x_length,
                                   WorldParameters *world_parameters) {
    D4COption option = {0};
    InitializeD4COption(&option);
    
    // Parameters setting and memory allocation.
    world_parameters->aperiodicity = (double **) malloc(sizeof(double *) * (world_parameters->f0_length));
    for (int i = 0; i < world_parameters->f0_length; ++i) {
        world_parameters->aperiodicity[i] =
        (double*) malloc(sizeof(double) * (world_parameters->fft_size / 2 + 1));
    }
    
    DWORD elapsed_time = timeGetTime();
    // option is not implemented in this version. This is for future update.
    // We can use "NULL" as the argument.
    D4C(x, x_length, world_parameters->fs, world_parameters->time_axis,
        world_parameters->f0, world_parameters->f0_length,
        world_parameters->fft_size, &option, world_parameters->aperiodicity);
    printf("D4C: %d [msec]\n", timeGetTime() - elapsed_time);
}

static void ParameterModification(int argc, char *argv[], int fs, int f0_length,
                                  int fft_size, double *f0, double **spectrogram) {
    // F0 scaling
    if (argc >= 4) {
        double shift = atof(argv[3]);
        for (int i = 0; i < f0_length; ++i) f0[i] *= shift;
    }
    if (argc < 5) return;
    
    // Spectral stretching
    double ratio = atof(argv[4]);
    double *freq_axis1 = (double*) malloc(sizeof(double) * (fft_size));
    double *freq_axis2 = (double*) malloc(sizeof(double) * (fft_size));
    double *spectrum1 = (double*) malloc(sizeof(double) * (fft_size));
    double *spectrum2 = (double*) malloc(sizeof(double) * (fft_size));
    
    for (int i = 0; i <= fft_size / 2; ++i) {
        freq_axis1[i] = ratio * i / fft_size * fs;
        freq_axis2[i] = (double)(i) / fft_size * fs;
    }
    for (int i = 0; i < f0_length; ++i) {
        for (int j = 0; j <= fft_size / 2; ++j)
            spectrum1[j] = log(spectrogram[i][j]);
        interp1(freq_axis1, spectrum1, fft_size / 2 + 1, freq_axis2,
                fft_size / 2 + 1, spectrum2);
        for (int j = 0; j <= fft_size / 2; ++j)
            spectrogram[i][j] = exp(spectrum2[j]);
        if (ratio >= 1.0) continue;
        for (int j = (int)(fft_size / 2.0 * ratio);
             j <= fft_size / 2; ++j)
            spectrogram[i][j] =
            spectrogram[i][(int)(fft_size / 2.0 * ratio) - 1];
    }
    if(spectrum1 != NULL) {
        free(spectrum1);
        spectrum1 = NULL;
    }
    if(spectrum2 != NULL) {
        free(spectrum2);
        spectrum2 = NULL;
    }
    if(freq_axis1 != NULL) {
        free(freq_axis1);
        freq_axis1 = NULL;
    }
    if(freq_axis2 != NULL) {
        free(freq_axis2);
        freq_axis2 = NULL;
    }
}

static void WaveformSynthesis(WorldParameters *world_parameters, int fs,
                              int y_length, double *y) {
    DWORD elapsed_time;
    // Synthesis by the aperiodicity
    printf("\nSynthesis\n");
    elapsed_time = timeGetTime();
    Synthesis(world_parameters->f0, world_parameters->f0_length,
              world_parameters->spectrogram, world_parameters->aperiodicity,
              world_parameters->fft_size, world_parameters->frame_period, fs,
              y_length, y);
    printf("WORLD: %d [msec]\n", timeGetTime() - elapsed_time);
}

static void DestroyMemory(WorldParameters *world_parameters) {
    if(world_parameters->time_axis != NULL) {
        free(world_parameters->time_axis);
        world_parameters->time_axis = NULL;
    }
    if(world_parameters->f0 != NULL) {
        free(world_parameters->f0);
        world_parameters->f0 = NULL;
    }
    for (int i = 0; i < world_parameters->f0_length; ++i) {
        if(world_parameters->spectrogram[i] != NULL) {
            free(world_parameters->spectrogram[i]);
            world_parameters->spectrogram[i] = NULL;
        }
        if(world_parameters->aperiodicity[i] != NULL) {
            free(world_parameters->aperiodicity[i]);
            world_parameters->aperiodicity[i] = NULL;
        }
    }
    if(world_parameters->spectrogram != NULL) {
        free(world_parameters->spectrogram);
        world_parameters->spectrogram = NULL;
    }
    if(world_parameters->aperiodicity != NULL) {
        free(world_parameters->aperiodicity);
        world_parameters->aperiodicity = NULL;
    }
}

//-----------------------------------------------------------------------------
// Test program.
// test.exe input.wav outout.wav f0 spec flag
// input.wav  : argv[1] Input file
// output.wav : argv[2] Output file
// f0         : argv[3] F0 scaling (a positive number)
// spec       : argv[4] Formant shift (a positive number)
//-----------------------------------------------------------------------------
WorldParameters execute_world(const char* inputFile , const char* outputFile,double pitch) {
    int x_length = GetAudioLength(inputFile);
    WorldParameters world_parameters = { 0 };
    if(x_length == 0){
        printf("file not found %s",inputFile);
        return world_parameters;
    }
    double *x = (double*) malloc(sizeof(double) * (x_length));
    // wavread() must be called after GetAudioLength().
    int fs, nbit;
    wavread(inputFile, &fs, &nbit, x);
    DisplayInformation(fs, nbit, x_length);
    
    //---------------------------------------------------------------------------
    // Analysis part
    //---------------------------------------------------------------------------
    // 2016/02/02
    // A new struct is introduced to implement safe program.
    // You must set fs and frame_period before analysis/synthesis.
    world_parameters.fs = fs;
    
    // 5.0 ms is the default value.
    // Generally, the inverse of the lowest F0 of speech is the best.
    // However, the more elapsed time is required.
    world_parameters.frame_period = 5.0;
    
    // F0 estimation
    F0Estimation(x, x_length, &world_parameters);
    
    // Spectral envelope estimation
    SpectralEnvelopeEstimation(x, x_length, &world_parameters);
    
    // Aperiodicity estimation by D4C
    AperiodicityEstimation(x, x_length, &world_parameters);
    printf("%f",pitch);
    if(pitch != -1)
        for(int i = 0;i<world_parameters.f0_length;i++)
        {
            world_parameters.f0[i] = pitch;
        }
    //---------------------------------------------------------------------------
    // Synthesis part
    //---------------------------------------------------------------------------
    // The length of the output waveform
    int y_length = (int)((world_parameters.f0_length - 1) *
                         world_parameters.frame_period / 1000.0 * fs) + 1;
    double *y = (double*) malloc(sizeof(double) * (y_length));
    // Synthesis
    WaveformSynthesis(&world_parameters, fs, y_length, y);
    
    // Output
    printf("%s",outputFile);
    wavwrite(y, y_length, fs, 16, outputFile);
    
    if(x != NULL) {
        free(x);
        x = NULL;
    }
    printf("complete.\n");
    return world_parameters;
}
void execute_Synthesis(WorldParameters world_parameters,const char* outputFile){
    int y_length = (int)((world_parameters.f0_length - 1) *
                         world_parameters.frame_period / 1000.0 * world_parameters.fs) + 1;
    double *y = (double*) malloc(sizeof(double) * (y_length));
    // Synthesis
    WaveformSynthesis(&world_parameters, world_parameters.fs, y_length, y);
    
    // Output
    printf("%s",outputFile);
    wavwrite(y, y_length, world_parameters.fs, 16, outputFile);
    
    printf("complete.\n");
}

WorldSynthesizer *synthesizer;
WorldSynthesizer *synthesizer2;
void *Initializer(WorldParameters *world_parameters,int buffer_size){
    WorldSynthesizer *synth = malloc(sizeof(*synth));
    InitializeSynthesizer(world_parameters->fs, world_parameters->frame_period,
                          world_parameters->fft_size, buffer_size, 24, synth);
    synthesizer = synth;
    /*
     
     ↓Addition
     
     */
    WorldSynthesizer *synth2 = malloc(sizeof(*synth));
    InitializeSynthesizer(world_parameters->fs, world_parameters->frame_period,
                          world_parameters->fft_size, buffer_size, 24, synth2);
    synthesizer2 = synth2;
    return synthesizer;
}

int AddFrames(WorldParameters *world_parameters,int fs,
              int start,int length, double *y,int buffer_size,int synthNum)
{
    int res = 0;
    //start = 79;
    //for (int i = start; i < start+length;) {
    // Add one frame (i shows the frame index that should be added)
    /*
     if (AddParameters(&world_parameters->f0[start], 1,
     &world_parameters->spectrogram[start], &world_parameters->aperiodicity[start],
     synthesizer) == 1){
     ++i;
     }
     */
    
    WorldSynthesizer *currentSynthe;
    if(synthNum == 0){
        currentSynthe = synthesizer;
    }else{
        currentSynthe = synthesizer2;
    }
    while (1) {
        //パラメタの追加
        if(Synthesis2(currentSynthe) != 0){
            for (int j = 0; j < buffer_size; ++j)
                y[j] = currentSynthe->buffer[j];
            res = 1;
            break;
        }
        else{
            AddParameters(&world_parameters->f0[start], 1, &world_parameters->spectrogram[start], &world_parameters->aperiodicity[start], currentSynthe);
            continue;
        }
    }
    
    // Check the "Lock" (Please see synthesisrealtime.h)
    if (IsLocked(currentSynthe) == 1) {
        printf("locked");
        
    }
    //}
    // Synthesize speech with length of buffer_size sample.
    // It is repeated until the function returns 0
    // (it suggests that the synthesizer cannot generate speech).
    
    //for(int i = 0;i<buffer_size;i++)
    //printf("%f,",synthesizer->buffer);
    //return synthesizer->buffer;
    return res;
}
