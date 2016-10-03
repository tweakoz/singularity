#pragma once

typedef std::function<float()> controller_t;
typedef std::function<float(float)> mapper_t;

///////////////////////////////////////////////////////////////////////////////

struct FPARAM
{
    FPARAM();
    float eval(bool dump=false);

    controller_t _C1;
    controller_t _C2;
    float _coarse;
};

