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

    if( gain > 42 )
        gain = 42;

    _fval[0] = fc;
    _fval[1] = gain;

    _biquad.SetLowShelf(fc,gain);
    if( gain<0.0f )
        _svf.SetWithQ(EM_HPF,fc,0.5);
    else
        _svf.SetWithQ(EM_LPF,fc,0.5);

    float ling = decibel_to_linear_amp_ratio(gain);

    if(1)for( int i=0; i<inumframes; i++ )
    {
        float input = ubuf[i]*pad;
        float outp = _biquad.compute(input);
        _svf.Tick(input);
        outp = _svf.output;
        ubuf[i] = outp;
    }

    //printf( "ff<%f> wid<%f>\n", ff, wid );

}
void PARABASS::doKeyOn(const DspKeyOnInfo& koi) // final
{
    _svf.Clear();
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

    auto ld = _layer->_layerData;
    const auto& F2 = ld->_f2Block;
    const auto& F3 = ld->_f2Block;
    float pad2 = F2._pad;
    float pad3 = F3._pad;

    _fval[0] = fc;
    _fval[1] = wid;
    _fval[2] = gain;

    //pad = pad*pad2*pad3;

    _biquad.SetParametric2(fc,wid,gain);
    _peq1.Set(fc,wid,gain);

    if(1)for( int i=0; i<inumframes; i++ )
    {
        float inp = ubuf[i]*pad;
        //float outp = _biquad.compute2(inp);
        float outp = _peq1.compute(inp);
        //outp = _peq.proc(1,ubuf,fc/48000.0f,wid,gain);
        ubuf[i] = outp;
    }

}
void PARAMETRIC_EQ::doKeyOn(const DspKeyOnInfo& koi) // final
{
    _biquad.Clear();
    _peq.init();
    _peq1.Clear();
}
