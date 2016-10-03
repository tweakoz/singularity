#pragma once

#include <cmath>
#include <cfloat>
#include <assert.h>
#include <memory.h>
#include <stdlib.h>
#include <complex>

static const float SR = 48000.0f;
static const float ISR = 1.0f/SR;
static const float PI2ISR = pi2*ISR;

float BW2Q(float fc,float BWoct);

enum eFilterMode
{
    EM_LPF = 0,
    EM_BPF,
    EM_HPF,
    EM_NOTCH,
};

struct TrapSVF
{
    float v0,v1,v2,v3;
    float a1,a2,a3;
    float m0,m1,m2;
    float ic1eq, ic2eq;
    float output;


    TrapSVF();

    void Clear();
    void SetWithQ(eFilterMode m, float center, float Q);
    void SetWithRes(eFilterMode m, float center, float res);
    void SetWithBWoct(eFilterMode m, float center, float bwOct);
    void Tick(float input);
private:
    void compute(float input);
};

struct TrapAllpass
{
    float s1,s2;
    float g1,g2,g3;
    float y0,y1,y2;
    float damping;

    TrapAllpass();

    void Clear();

    //set coefficients
    void Set(float cutoff);
    float  Tick(float input);
};

struct BiQuad
{
    BiQuad();

    float compute( float input );
    void SetLpfReson(float kfco, float krez);
    void SetBpfWithQ( float kfco, float Q, float peakGain );
    void SetBpfWithBWoct( float kfco, float BWoct, float peakGain );
    void SetNotchWithQ( float kfco, float Q, float peakGain );
    void SetNotchWithBWoct( float kfco, float BWoct, float peakGain );
    void SetLpfNoQ( float kfco );

    float _xm1, _xm2;
    float _ym1, _ym2;
    float _mfa1, _mfa2;
    float _mfb0, _mfb1, _mfb2;
};
