#pragma once

typedef std::function<float()> controller_t;
typedef std::function<float(float)> mapper_t;
typedef std::function<float(float,float,float,float,float)> evalit_t;

///////////////////////////////////////////////////////////////////////////////

struct FPARAM
{
    FPARAM();
    void keyOn( int ikey, int ivel );
    float eval(bool dump=false);

    controller_t _C1;
    controller_t _C2;
    float _coarse;
    evalit_t _evaluator;
    float _keyTrack=0;
    float _velTrack=0;
    float _keyOff = 0.0f;
    float _velOff = 0.0f;
};

