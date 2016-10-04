#include "krztest.h"
#include <assert.h>
#include "alg.h"

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

    float fc = _ctrl[0].eval(true);
    float wid = _ctrl[1].eval();        
    float gain = _ctrl[2].eval();        
    _fval[0] = fc;
    _fval[1] = wid;
    _fval[2] = gain;
    _filter.SetWithBWoct(EM_BPF,fc,wid);

    float ling = decibel_to_linear_amp_ratio(gain);

    if(1)for( int i=0; i<inumframes; i++ )
    {
        _filter.Tick(ubuf[i]);
        ubuf[i] = _filter.output*ling;
    }

    //printf( "ff<%f> wid<%f>\n", ff, wid );

}
void PARAMETRIC_EQ::doKeyOn(layer*l) // final
{   _filter.Clear();
}

///////////////////////////////////////////////////////////////////////////////

BANDPASS_FILT::BANDPASS_FILT( const DspBlockData& dbd )
    : DspBlock(dbd)
{   _numParams = 2;
}

void BANDPASS_FILT::compute(dspBlockBuffer& obuf) //final
{
    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;

    float fc = _ctrl[0].eval(true);
    float wid = _ctrl[1].eval();        
    
    _fval[0] = fc;
    _fval[1] = wid;

    _filter.SetWithBWoct(EM_BPF,fc,wid);

    if(1)for( int i=0; i<inumframes; i++ )
    {
        _filter.Tick(ubuf[i]);
        ubuf[i] = _filter.output;
    }

    //printf( "ff<%f> wid<%f>\n", ff, wid );

}

void BANDPASS_FILT::doKeyOn(layer*l) // final
{   _filter.Clear();
}

///////////////////////////////////////////////////////////////////////////////

NOTCH_FILT::NOTCH_FILT( const DspBlockData& dbd )
    :DspBlock(dbd)
{   _numParams = 2;        
}

void NOTCH_FILT::compute(dspBlockBuffer& obuf) //final
{
    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;

    float fc = _ctrl[0].eval();
    float wid = _ctrl[1].eval();
    _fval[0] = fc;
    _fval[1] = wid;
    
    _filter.SetWithBWoct(EM_NOTCH,fc,wid);

    if(1)for( int i=0; i<inumframes; i++ )
    {
        _filter.Tick(ubuf[i]);
        ubuf[i] = _filter.output;
    }

    //printf( "ff<%f> wid<%f>\n", ff, wid );

}

void NOTCH_FILT::doKeyOn(layer*l) // final
{   _filter.Clear();
}

///////////////////////////////////////////////////////////////////////////////

TWOPOLE_ALLPASS::TWOPOLE_ALLPASS( const DspBlockData& dbd )
    : DspBlock(dbd)
{   _numParams = 2;
}

void TWOPOLE_ALLPASS::compute(dspBlockBuffer& obuf) //final
{
    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;

    float fc = _ctrl[0].eval();
    float wid = _ctrl[1].eval(false);
    _fval[0] = fc;
    _fval[1] = wid;

    _filterL.Set(fc);
    _filterH.Set(fc);
    //printf( "fc<%f>\n", fc );
    if(1)for( int i=0; i<inumframes; i++ )
    {
        float f1 = _filterL.Tick(ubuf[i]);
        ubuf[i] = _filterH.Tick(f1);;
    }

    //printf( "ff<%f> res<%f>\n", ff, res );

}

void TWOPOLE_ALLPASS::doKeyOn(layer*l) // final
{   _filterL.Clear();
    _filterH.Clear();
}

///////////////////////////////////////////////////////////////////////////////

TWOPOLE_LOWPASS::TWOPOLE_LOWPASS( const DspBlockData& dbd )
    : DspBlock(dbd)
{   _numParams = 2;
}

void TWOPOLE_LOWPASS::compute(dspBlockBuffer& obuf) //final
{
    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;

    float fc = _ctrl[0].eval();
    float res = _ctrl[1].eval(false);
    _fval[0] = fc;
    _fval[1] = res;

    _filter.SetWithRes(EM_LPF,fc,res);
    //printf( "fc<%f>\n", fc );
    if(1)for( int i=0; i<inumframes; i++ )
    {
        _filter.Tick(ubuf[i]);
        ubuf[i] = _filter.output;
    }

    //printf( "ff<%f> res<%f>\n", ff, res );

}

void TWOPOLE_LOWPASS::doKeyOn(layer*l) // final
{   _filter.Clear();
}

///////////////////////////////////////////////////////////////////////////////
// LOPAS2 = TWOPOLE_LOWPASS (fixed -6dB res)
///////////////////////////////////////////////////////////////////////////////

LOPAS2::LOPAS2( const DspBlockData& dbd )
    :DspBlock(dbd)
{   _numParams = 1;
}

void LOPAS2::compute(dspBlockBuffer& obuf) //final
{
    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float fc = _ctrl[0].eval();
    _fval[0] = fc;
    float res = decibel_to_linear_amp_ratio(-6);
    _filter.SetWithRes(EM_LPF,fc,res);
    if(1)for( int i=0; i<inumframes; i++ )
    {
        _filter.Tick(ubuf[i]);
        ubuf[i] = _filter.output;
    }
}

void LOPAS2::doKeyOn(layer*l) // final
{   _filter.Clear();
}

///////////////////////////////////////////////////////////////////////////////
// LOPAS2 = TWOPOLE_LOWPASS (fixed -6dB res)
///////////////////////////////////////////////////////////////////////////////

LP2RES::LP2RES( const DspBlockData& dbd )
    :DspBlock(dbd)
{   _numParams = 1;
}

void LP2RES::compute(dspBlockBuffer& obuf) //final
{
    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float fc = _ctrl[0].eval();
    _fval[0] = fc;
    float res = decibel_to_linear_amp_ratio(12);
    _filter.SetWithRes(EM_LPF,fc,res);
    if(1)for( int i=0; i<inumframes; i++ )
    {
        _filter.Tick(ubuf[i]);
        ubuf[i] = _filter.output;
    }
}

void LP2RES::doKeyOn(layer*l) // final
{   _filter.Clear();
}

///////////////////////////////////////////////////////////////////////////////

FOURPOLE_LOPASS_W_SEP::FOURPOLE_LOPASS_W_SEP( const DspBlockData& dbd )
    :DspBlock(dbd)
{   _numParams = 3;        
}

void FOURPOLE_LOPASS_W_SEP::compute(dspBlockBuffer& obuf) //final
{
    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;

    float fc = _ctrl[0].eval();
    float res = _ctrl[1].eval();
    float sep = _ctrl[2].eval();
    _fval[0] = fc;
    _fval[1] = res;
    _fval[2] = sep;
    float ratio = cents_to_linear_freq_ratio(sep);

    _filter1.SetWithRes(EM_LPF,fc,res);
    _filter2.SetWithRes(EM_LPF,fc*ratio,res);

    if(1)for( int i=0; i<inumframes; i++ )
    {
        _filter1.Tick(ubuf[i]);
        _filter2.Tick(_filter1.output);
        ubuf[i] = _filter2.output;
    }

    //printf( "ff<%f> res<%f>\n", ff, res );

}

void FOURPOLE_LOPASS_W_SEP::doKeyOn(layer*l) //final
{   _filter1.Clear();
    _filter2.Clear();
}

///////////////////////////////////////////////////////////////////////////////
// LOPASS : 1 pole! lowpass
///////////////////////////////////////////////////////////////////////////////

LOPASS::LOPASS( const DspBlockData& dbd )
    :DspBlock(dbd)
{   _numParams = 1;                
}

void LOPASS::compute(dspBlockBuffer& obuf) //final
{
    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float fc = _ctrl[0].eval();
    _fval[0] = fc;
    _filter.SetWithQ(EM_LPF,fc,0.5);
    if(1)for( int i=0; i<inumframes; i++ )
    {
        _filter.Tick(ubuf[i]);
        ubuf[i] = _filter.output;
    }
}

void LOPASS::doKeyOn(layer*l) //final
{   _filter.Clear();
}

///////////////////////////////////////////////////////////////////////////////

HIPASS::HIPASS( const DspBlockData& dbd )
    :DspBlock(dbd)
{   _numParams = 1;
}

void HIPASS::compute(dspBlockBuffer& obuf) //final
{
    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float fc = _ctrl[0].eval();
    _fval[0] = fc;
    _filter.SetWithRes(EM_HPF,fc,0.0f);
    if(1)for( int i=0; i<inumframes; i++ )
    {
        _filter.Tick(ubuf[i]);
        ubuf[i] = _filter.output;
    }
}

void HIPASS::doKeyOn(layer*l) //final
{   _filter.Clear();
}

///////////////////////////////////////////////////////////////////////////////
// 2pole allpass (for phasers, etc..)
///////////////////////////////////////////////////////////////////////////////

ALPASS::ALPASS( const DspBlockData& dbd )
    :DspBlock(dbd)
{   _numParams = 1;
}

void ALPASS::compute(dspBlockBuffer& obuf) // final
{
    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float fc = _ctrl[0].eval();
    _filter.Set(fc);
    _fval[0] = fc;
    if(1)for( int i=0; i<inumframes; i++ )
    {
        ubuf[i] = _filter.Tick(ubuf[i]);
    }
}

void ALPASS::doKeyOn(layer*l) // final
{   _filter.Clear();
}
