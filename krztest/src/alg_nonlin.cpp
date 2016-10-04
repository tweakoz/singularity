#include "krztest.h"
#include <assert.h>
#include "filters.h"
#include "alg.h"

///////////////////////////////////////////////////////////////////////////////

SAWPLUS::SAWPLUS( const DspBlockData& dbd )
    : DspBlock(dbd)
    , _pblep(48000,PolyBLEP::SAWTOOTH)
{   _numParams = 1;                
}

void SAWPLUS::compute(dspBlockBuffer& obuf) //final
{
    float centoff = _ctrl[0].eval();//,0.01f,100.0f);
    _fval[0] = centoff;

    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float lyrcents = _layer->_curcents;
    float frq = midi_note_to_frequency((centoff+lyrcents)*0.01);
    float SR = _layer->_syn._sampleRate;
    _pblep.setFrequency(frq);

    //printf( "frq<%f> _phaseInc<%lld>\n", frq, _phaseInc );
    if(1) for( int i=0; i<inumframes; i++ )
    {   float input = ubuf[i];
        float saw = _pblep.getAndInc();
        saw *= _layer->_AENV[i];
        float swplus = input+saw;
        ubuf[i] = swplus;
    }
}
void SAWPLUS::doKeyOn(layer* l) //final
{
}

///////////////////////////////////////////////////////////////////////////////

SWPLUSSHP::SWPLUSSHP( const DspBlockData& dbd )
    : DspBlock(dbd)
    , _pblep(48000,PolyBLEP::SAWTOOTH)
{   _numParams = 1;                
}

void SWPLUSSHP::compute(dspBlockBuffer& obuf) //final
{
    float centoff = _ctrl[0].eval();//,0.01f,100.0f);
    _fval[0] = centoff;

    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float lyrcents = _layer->_curcents;
    float frq = midi_note_to_frequency((centoff+lyrcents)*0.01);
    float SR = _layer->_syn._sampleRate;
    //_phaseInc = (65536.0*frq)/SR;
    _pblep.setFrequency(frq);

    if(1) for( int i=0; i<inumframes; i++ )
    {   float input = ubuf[i];
        float saw = _pblep.getAndInc();
        //saw *= _layer->_AENV[i];
        float swplusshp = shaper(input+saw,.25);
        ubuf[i] = swplusshp*_layer->_AENV[i];
    }
}
void SWPLUSSHP::doKeyOn(layer* l) //final
{
}

///////////////////////////////////////////////////////////////////////////////

SHAPER::SHAPER( const DspBlockData& dbd )
    : DspBlock(dbd)
{   _numParams = 1;                
}

void SHAPER::compute(dspBlockBuffer& obuf) //final
{
    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float amt = _ctrl[0].eval();//,0.01f,100.0f);
    _fval[0] = amt;
    if(1) for( int i=0; i<inumframes; i++ )
    {   float s1 = shaper(ubuf[i],amt);
        ubuf[i] = s1;
    }
}

///////////////////////////////////////////////////////////////////////////////

SHAPE2::SHAPE2( const DspBlockData& dbd )
    :DspBlock(dbd)
{   _numParams = 1;
}

void SHAPE2::compute(dspBlockBuffer& obuf) //final
{
    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float amt = _ctrl[0].eval();
    if(1) for( int i=0; i<inumframes; i++ )
    {   float s1 = shaper(ubuf[i],amt);
        float s2 = shaper(s1,amt*0.75);
        ubuf[i] = s2;
    }
}

///////////////////////////////////////////////////////////////////////////////

TWOPARAM_SHAPER::TWOPARAM_SHAPER( const DspBlockData& dbd )
    :DspBlock(dbd)
{   _numParams = 2;  
}

void TWOPARAM_SHAPER::compute(dspBlockBuffer& obuf) //final
{
    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float evn = _ctrl[0].eval();
    float odd = _ctrl[1].eval();
    if(1) for( int i=0; i<inumframes; i++ )
    {   float e = shaper(ubuf[i],evn);
        float o = shaper(ubuf[i],odd);
        float r = (e+o)*0.5f;
        r = wrap(r,-30);
        ubuf[i] = r;
    }
}

///////////////////////////////////////////////////////////////////////////////

WRAP::WRAP( const DspBlockData& dbd )
    : DspBlock(dbd)
{   _numParams = 1;                
}

void WRAP::compute(dspBlockBuffer& obuf) //final
{
    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float rpoint = _ctrl[0].eval();//,-100,100.0f);
    _fval[0] = rpoint;
    if(1) for( int i=0; i<inumframes; i++ )
    {   ubuf[i] = wrap(ubuf[i],rpoint);
    }
}

///////////////////////////////////////////////////////////////////////////////


DIST::DIST( const DspBlockData& dbd )
    : DspBlock(dbd)
{   _numParams = 1;                
}

void DIST::compute(dspBlockBuffer& obuf) //final
{
    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float adj = _ctrl[0].eval();
    _fval[0] = adj;
    float ratio = decibel_to_linear_amp_ratio(adj-30.0);

    if(1) for( int i=0; i<inumframes; i++ )
    {   
        float v = ubuf[i]*ratio;
        v = clip_float(v,-1,1);
        ubuf[i] = v;
    }
}
