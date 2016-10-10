#include <audiofile.h>
#include <string>
#include <assert.h>
#include <unistd.h>
#include <math.h>

#include "krzdata.h"
#include "krztest.h"

///////////////////////////////////////////////////////////////////////////////

FPARAM::FPARAM()
    : _coarse(0.0f)
{
   _C1 = [](){return 0.0f;};
   _C2 = [](){return 0.0f;};

}

///////////////////////////////////////////////////////////////////////////////

float FPARAM::eval(bool dump)
{
    float c1 = _C1();
    float c2 = _C2();
    float tot = _evaluator(_coarse,c1,c2);
    if( dump )
        printf( "coarse<%g> c1<%g> c2<%g> tot<%g>\n", _coarse, c1, c2, tot );

    return tot;
}

///////////////////////////////////////////////////////////////////////////////

FPARAM layer::initFPARAM(const FBlockData& fbd)
{
    FPARAM rval;
    rval._coarse = fbd._coarse;
    rval._C1 = getSRC1(fbd._mods);
    rval._C2 = getSRC2(fbd._mods);
    rval._evaluator = fbd._mods._evaluator;
    return rval;
}

///////////////////////////////////////////////////////////////////////////////