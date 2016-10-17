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
    _keyOff = float(ikey-_kstartNote);
    _unitVel = float(ivel)/127.0f; 

    if( false == _kstartBipolar )
    {
        if( _keyOff<0 )
            _keyOff = 0;

        //printf( "ikey<%d> ksn<%d> ko<%d>\n", ikey, _kstartNote, int(_keyOff) );
    }

    printf( "_kstartNote<%d>\n", _kstartNote );
    printf( "_keyOff<%f>\n", _keyOff );
    printf( "_unitVel<%f>\n", _unitVel );
}

///////////////////////////////////////////////////////////////////////////////

float FPARAM::eval(bool dump)
{
    float tot = _evaluator(*this);
    if( dump )
        printf( "coarse<%g> c1<%g> c2<%g> tot<%g>\n", _coarse, _C1(), _C2(), tot );

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
    rval._kstartNote = fbd._keystartNote;
    rval._kstartBipolar = fbd._keystartBipolar;

    return rval;
}

///////////////////////////////////////////////////////////////////////////////
