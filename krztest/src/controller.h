#pragma once

///////////////////////////////////////////////////////////////////////////////

struct controlevalctx
{
    float _coarse = 0.0f;
    float _fine = 0.0f;
    float _src1 = 0.0f;
    float _src2 = 0.0f;
    float _keyOff = 0.0f;
    float _velOff = 0.0f;
};

///////////////////////////////////////////////////////////////////////////////

typedef std::function<float()> controller_t;
typedef std::function<float(float)> mapper_t;
typedef std::function<float(const controlevalctx& cec)> evalit_t;

///////////////////////////////////////////////////////////////////////////////

struct FPARAM
{
    FPARAM();
    void keyOn( int ikey, int ivel );
    float eval(bool dump=false);

    controller_t _C1;
    controller_t _C2;
    float _coarse = 0.0f;
    float _fine = 0.0f;
    float _keyTrack=0;
    float _velTrack=0;
    float _keyOff = 0.0f;
    float _velOff = 0.0f;

    evalit_t _evaluator;
};

