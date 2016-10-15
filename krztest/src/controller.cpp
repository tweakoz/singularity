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

void FPARAM::keyOn( int ikey, int ivel )
{
    _keyOff = float(ikey-60);
    _velOff = float(ivel)/127.0f; 
    printf( "_keyOff<%f>\n", _keyOff );
    printf( "_velOff<%f>\n", _velOff );
}

///////////////////////////////////////////////////////////////////////////////

float FPARAM::eval(bool dump)
{
    float c1 = _C1();
    float c2 = _C2();
    controlevalctx ctx;
    ctx._coarse = _coarse;
    ctx._fine = _fine;
    ctx._src1 = c1;
    ctx._src2 = c2;
    ctx._keyOff = _keyOff;
    ctx._velOff = _velOff;
    float tot = _evaluator(ctx);
    if( dump )
        printf( "coarse<%g> c1<%g> c2<%g> tot<%g>\n", _coarse, c1, c2, tot );

    return tot;
}

///////////////////////////////////////////////////////////////////////////////

FPARAM layer::initFPARAM(const FBlockData& fbd)
{
    FPARAM rval;
    rval._coarse = fbd._coarse;
    rval._fine = fbd._fine;
    rval._C1 = getSRC1(fbd._mods);
    rval._C2 = getSRC2(fbd._mods);
    rval._evaluator = fbd._mods._evaluator;

    rval._keyTrack = fbd._keyTrack;
    rval._velTrack = fbd._velTrack;

    return rval;
}

///////////////////////////////////////////////////////////////////////////////
