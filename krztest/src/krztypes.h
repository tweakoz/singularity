#pragma once
///////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <map>
#include <set>
#include <string>
#include <queue>
#include <functional>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>
#include <math.h>
#include <memory.h>
///////////////////////////////////////////////////////////////////////////////
typedef int16_t s16;
typedef uint32_t u32;
typedef std::function<void()> void_lamda_t;
///////////////////////////////////////////////////////////////////////////////
struct programData;
struct layerData;
struct keymap;
struct kmregion;
struct sample;
struct multisample;
struct synth;
struct layer;
struct outputBuffer;
struct RateLevelEnvData;
struct natenvseg;
struct VastObjectsDB;
///////////////////////////////////////////////////////////////////////////////
s16* getSoundBlock();
///////////////////////////////////////////////////////////////////////////////
static const double pi = 3.141592654;
static const double pi2 = 3.141592654*2.0;
static const double sqrt2 = sqrt(2.0);

///////////////////////////////////////////////////////////////////////////////

struct outputBuffer
{
    outputBuffer();
    void resize(int inumframes);

    float* _leftBuffer;
    float* _rightBuffer;
    int _maxframes;
    int _numframes;
};
