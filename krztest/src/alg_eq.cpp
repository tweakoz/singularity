#include "krztest.h"
#include <assert.h>
#include "alg.h"

///////////////////////////////////////////////////////////////////////////////

PARABASS::PARABASS( const DspBlockData& dbd )
    :DspBlock(dbd)
{
    _numParams = 2;
}

void PARABASS::compute(dspBlockBuffer& obuf) //final
{
    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;

    float fc = _ctrl[0].eval();
    float gain = _ctrl[1].eval();
    float pad = _dbd._pad;
    //bool isneg = gain<0.0;
    //gain = sqrtf(fabs(gain));
    //if(isneg)
    //    gain *= -1.0;

    _fval[0] = fc;
    _fval[1] = gain;

    _biquad.SetLowShelf(fc,gain);
    //_filter.SetWithBWoct(EM_BPF,fc,wid);

    float ling = decibel_to_linear_amp_ratio(gain);

    if(1)for( int i=0; i<inumframes; i++ )
    {
        float biq = _biquad.compute(ubuf[i]*pad);

        //ubuf[i] = _filter.output*ling;
        ubuf[i] = biq;
    }

    //printf( "ff<%f> wid<%f>\n", ff, wid );

}
void PARABASS::doKeyOn(const DspKeyOnInfo& koi) // final
{
    _biquad.Clear();
}

///////////////////////////////////////////////////////////////////////////////

PARAMID::PARAMID( const DspBlockData& dbd )
    :DspBlock(dbd)
{
    _numParams = 2;
}

void PARAMID::compute(dspBlockBuffer& obuf) //final
{
    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;

    float fc = _ctrl[0].eval();
    float gain = _ctrl[1].eval();
    float pad = _dbd._pad;
    _fval[0] = fc;
    _fval[1] = gain;

    _biquad.SetBpfWithBWoct(fc,2.2,gain);

    float ling = decibel_to_linear_amp_ratio(gain);

    if(1)for( int i=0; i<inumframes; i++ )
    {
        float biq = _biquad.compute(ubuf[i]*pad);
        ubuf[i] = biq;
    }

    //printf( "ff<%f> wid<%f>\n", ff, wid );

}
void PARAMID::doKeyOn(const DspKeyOnInfo& koi) // final
{
    _biquad.Clear();
}

///////////////////////////////////////////////////////////////////////////////

PARATREBLE::PARATREBLE( const DspBlockData& dbd )
    :DspBlock(dbd)
{
    _numParams = 2;
}

void PARATREBLE::compute(dspBlockBuffer& obuf) //final
{
    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;

    float fc = _ctrl[0].eval();
    float gain = _ctrl[1].eval();
    float pad = _dbd._pad;

    _fval[0] = fc;
    _fval[1] = gain;

    _biquad.SetHighShelf(fc,gain);

    float ling = decibel_to_linear_amp_ratio(gain);

    if(1)for( int i=0; i<inumframes; i++ )
    {
        float biq = _biquad.compute(ubuf[i]*pad);
        ubuf[i] = biq;
    }

    //printf( "ff<%f> wid<%f>\n", ff, wid );

}
void PARATREBLE::doKeyOn(const DspKeyOnInfo& koi) // final
{    _biquad.Clear();
}

///////////////////////////////////////////////////////////////////////////////

PARAMETRIC_EQ::PARAMETRIC_EQ( const DspBlockData& dbd )
    :DspBlock(dbd)
{
    _numParams = 3;
}

void PARAMETRIC_EQ::compute(dspBlockBuffer& obuf) //final
{
    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;

    float fc = _ctrl[0].eval();
    float wid = _ctrl[1].eval();        
    float gain = _ctrl[2].eval();
    float pad = _dbd._pad;

    _fval[0] = fc;
    _fval[1] = wid;
    _fval[2] = gain;

    _biquad.SetParametric(fc,wid,gain);

    if(1)for( int i=0; i<inumframes; i++ )
    {
        float biq = _biquad.compute(ubuf[i]*pad);
        ubuf[i] = biq;
    }

}
void PARAMETRIC_EQ::doKeyOn(const DspKeyOnInfo& koi) // final
{
    _biquad.Clear();
}
