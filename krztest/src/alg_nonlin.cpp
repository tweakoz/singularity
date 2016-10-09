#include "krztest.h"
#include <assert.h>
#include "filters.h"
#include "alg.h"

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
    float la = decibel_to_linear_amp_ratio(amt);
    if(1) for( int i=0; i<inumframes; i++ )
    {   float s1 = shaper(ubuf[i],la);
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
    _fval[0] = amt;
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

/*float shaper(float inp, float adj)
{
    float index = pi*4.0f*inp*adj;
    return sinf(index); ///adj;
}*/

void TWOPARAM_SHAPER::doKeyOn(layer*l)
{
    ph1 = 0.0f;
    ph2 = 0.0f;
}

void TWOPARAM_SHAPER::compute(dspBlockBuffer& obuf) //final
{
    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float evn = _ctrl[0].eval();
    float odd = _ctrl[1].eval();

    //evn = -22;//(0.5f+sinf(ph1+pi)*0.5f)*-60.0f;
    //odd = (sinf(ph1)*30.f)-30.0f;
    ph1 += 0.0003f;

    _fval[0]=evn;
    _fval[1]=odd;
    //printf( "_dbd._pad<%f>\n", _dbd._pad );
    if(1) for( int i=0; i<inumframes; i++ )
    {
        float le = ubuf[i]*decibel_to_linear_amp_ratio(evn);


        //float e = (2.0f*powf(le,2.0f))-1.0f;
        //float e = ((2.0f*powf(le,2.0f))-1.0f)*1000.0f;//decibel_to_linear_amp_ratio(-evn);
        float index = le*le; //clip_float(powf(le,4),-12.0f,12.0f);
        float e = sinf(index*pi2*6); ///adj;
        
        float lo = ubuf[i]*decibel_to_linear_amp_ratio(odd);

        index = clip_float(lo*6,-12.0f,12.0f);
        float o = sinf(index*pi2); ///adj;

        float r = (e+o)*0.5f+_dbd._pad;
        //r = wrap(r,-30);
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
