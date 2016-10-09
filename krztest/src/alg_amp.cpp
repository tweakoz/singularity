#include "krztest.h"
#include <assert.h>
#include "filters.h"
#include "alg.h"

///////////////////////////////////////////////////////////////////////////////

AMP::AMP( const DspBlockData& dbd )
    : DspBlock(dbd)
{   _numParams = 1;  
}

void AMP::compute(dspBlockBuffer& obuf) //final
{
    float gain = _ctrl[0].eval();//,0.01f,100.0f);

    _filt = 0.995*_filt + 0.005*gain;
    float linG = decibel_to_linear_amp_ratio(_filt);
    _fval[0] = _filt;


    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;

    float* aenv = _layer->_AENV;
    //printf( "frq<%f> _phaseInc<%lld>\n", frq, _phaseInc );
    if(1) for( int i=0; i<inumframes; i++ )
    {   float inp = ubuf[i];
        float ae = aenv[i];
        ubuf[i] = inp*linG*ae;
    }
}

void AMP::doKeyOn(layer*l) //final
{   _filt = 0.0f;
}

///////////////////////////////////////////////////////////////////////////////

PLUSAMP::PLUSAMP( const DspBlockData& dbd )
    : DspBlock(dbd)
{   _numParams = 1;  
}

void PLUSAMP::compute(dspBlockBuffer& obuf) //final
{
    float gain = _ctrl[0].eval();//,0.01f,100.0f);

    _filt = 0.995*_filt + 0.005*gain;
    float linG = decibel_to_linear_amp_ratio(_filt);
    _fval[0] = _filt;


    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float* lbuf = obuf._lowerBuffer;

    float* aenv = _layer->_AENV;
    //printf( "frq<%f> _phaseInc<%lld>\n", frq, _phaseInc );
    if(1) for( int i=0; i<inumframes; i++ )
    {   float inU = ubuf[i];
        float inL = lbuf[i];
        float ae = aenv[i];
        float res = (inU+inL)*0.5*linG*ae*2.0;
        res = clip_float(res,-1,1);
        lbuf[i] = res;
        ubuf[i] = res;
    }
}

void PLUSAMP::doKeyOn(layer*l) //final
{   _filt = 0.0f;
}

///////////////////////////////////////////////////////////////////////////////

XAMP::XAMP( const DspBlockData& dbd )
    : DspBlock(dbd)
{   _numParams = 1;  
}

void XAMP::compute(dspBlockBuffer& obuf) //final
{
    float gain = _ctrl[0].eval();//,0.01f,100.0f);

    _filt = 0.995*_filt + 0.005*gain;
    float linG = decibel_to_linear_amp_ratio(_filt);
    _fval[0] = _filt;


    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float* lbuf = obuf._lowerBuffer;

    float* aenv = _layer->_AENV;
    //printf( "frq<%f> _phaseInc<%lld>\n", frq, _phaseInc );
    if(1) for( int i=0; i<inumframes; i++ )
    {   float inU = ubuf[i];
        float inL = lbuf[i];
        float ae = aenv[i];
        float res = (inU*inL)*linG*ae;
        res = clip_float(res,-1,1);
        lbuf[i] = res;
        ubuf[i] = res;
    }
}

void XAMP::doKeyOn(layer*l) //final
{   _filt = 0.0f;
}

///////////////////////////////////////////////////////////////////////////////

GAIN::GAIN( const DspBlockData& dbd )
    : DspBlock(dbd)
{   _numParams = 1;                
}

void GAIN::compute(dspBlockBuffer& obuf) //final
{
    float gain = _ctrl[0].eval();//,0.01f,100.0f);
    _fval[0] = gain;

    float linG = decibel_to_linear_amp_ratio(gain);

    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;

    //printf( "frq<%f> _phaseInc<%lld>\n", frq, _phaseInc );
    if(1) for( int i=0; i<inumframes; i++ )
    {   float inp = ubuf[i];
        ubuf[i] = inp*linG;
    }
}

///////////////////////////////////////////////////////////////////////////////

XFADE::XFADE( const DspBlockData& dbd )
    : DspBlock(dbd)
{   _numParams = 1;                
}

void XFADE::compute(dspBlockBuffer& obuf) //final
{
    float index = _ctrl[0].eval();//,0.01f,100.0f);
    _fval[0] = index;

    float mix = index*0.01f;
    float lmix = (mix>0) 
               ? lerp(0.5,0,mix)
               : lerp(0.5,1,-mix);
    float umix = (mix>0) 
               ? lerp(0.5,1,mix)
               : lerp(0.5,0,-mix);

    int inumframes = obuf._numframes;
    float* lbuf = obuf._lowerBuffer;
    float* ubuf = obuf._upperBuffer;

    //printf( "frq<%f> _phaseInc<%lld>\n", frq, _phaseInc );
    if(1) for( int i=0; i<inumframes; i++ )
    {        
        float inputU = ubuf[i];
        float inputL = lbuf[i];
        _plmix = _plmix*0.995f+lmix*0.005f;
        _pumix = _pumix*0.995f+umix*0.005f;

        ubuf[i] = (inputU*_pumix)+(inputL*_plmix);
        //ubuf[i] = inputU+inputL;//(inputU*_pumix)+(inputL*_plmix);
    }
}

void XFADE::doKeyOn(layer*l) //final
{   _plmix = 0.0f;
    _pumix = 0.0f;
}


///////////////////////////////////////////////////////////////////////////////

XGAIN::XGAIN( const DspBlockData& dbd )
    : DspBlock(dbd)
{   _numParams = 1;                
}

void XGAIN::compute(dspBlockBuffer& obuf) //final
{
    float gain = _ctrl[0].eval();//,0.01f,100.0f);
    _fval[0] = gain;

    float linG = decibel_to_linear_amp_ratio(gain);

    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;

    //printf( "frq<%f> _phaseInc<%lld>\n", frq, _phaseInc );
    if(1) for( int i=0; i<inumframes; i++ )
    {   float inp = ubuf[i];
        ubuf[i] = inp*linG;
    }
}

///////////////////////////////////////////////////////////////////////////////

AMPU_AMPL::AMPU_AMPL( const DspBlockData& dbd )
    : DspBlock(dbd)
{   _numParams = 2;                
}

void AMPU_AMPL::compute(dspBlockBuffer& obuf) //final
{
    float gainU = _ctrl[0].eval();//,0.01f,100.0f);
    float gainL = _ctrl[1].eval();//,0.01f,100.0f);

    _filtU = 0.995*_filtU + 0.005*gainU;
    _filtL = 0.995*_filtL + 0.005*gainL;
    float linGU = decibel_to_linear_amp_ratio(_filtU);
    float linGL = decibel_to_linear_amp_ratio(_filtL);
    _fval[0] = _filtU;
    _fval[1] = _filtL;


    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float* lbuf = obuf._lowerBuffer;

    float* aenv = _layer->_AENV;
    //printf( "frq<%f> _phaseInc<%lld>\n", frq, _phaseInc );
    if(1) for( int i=0; i<inumframes; i++ )
    {   float inU = ubuf[i];
        float inL = lbuf[i];
        float ae = aenv[i];
        float resU = inU*linGU*ae;
        float resL = inL*linGU*ae;
        //resU = clip_float(resU,-1,1);
        //resL = clip_float(resL,-1,1);
        ubuf[i] = resU;
        lbuf[i] = resL;
    }
}

void AMPU_AMPL::doKeyOn(layer*l) //final
{   _filtU = 0.0f;
    _filtL = 0.0f;
}

///////////////////////////////////////////////////////////////////////////////

BAL_AMP::BAL_AMP( const DspBlockData& dbd )
    : DspBlock(dbd)
{   _numParams = 1;                
}

void BAL_AMP::compute(dspBlockBuffer& obuf) //final
{
    float gain = _ctrl[0].eval();//,0.01f,100.0f);
    _fval[0] = gain;

    float linG = decibel_to_linear_amp_ratio(gain);

    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float* lbuf = obuf._lowerBuffer;

    //printf( "frq<%f> _phaseInc<%lld>\n", frq, _phaseInc );
    if(1) for( int i=0; i<inumframes; i++ )
    {   float inp = ubuf[i];
        ubuf[i] = inp*linG;
    }
}

///////////////////////////////////////////////////////////////////////////////

PANNER::PANNER( const DspBlockData& dbd )
    :DspBlock(dbd)
{   _numParams = 1;
    _numOutputs = 2;
}
void PANNER::compute(dspBlockBuffer& obuf) //final
{
    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float* lbuf = obuf._lowerBuffer;
    float pos = _ctrl[0].eval();
    float pan = pos*0.01f;
    float lmix = (pan>0) 
               ? lerp(0.5,0,pan)
               : lerp(0.5,1,-pan);
    float rmix = (pan>0) 
               ? lerp(0.5,1,pan)
               : lerp(0.5,0,-pan);

    _fval[0] = pos;
    //_fval[1] = lmix;
    //_fval[2] = rmix;
    //printf( "pan<%f> lmix<%f> rmix<%f>\n", pan, lmix, rmix );
    if(1)for( int i=0; i<inumframes; i++ )
    {
        float input = ubuf[i];
        _plmix = _plmix*0.995f+lmix*0.005f;
        _prmix = _prmix*0.995f+rmix*0.005f;

        ubuf[i] = input*_plmix;
        lbuf[i] = input*_prmix;
    }
}
void PANNER::doKeyOn(layer*l) //final
{   _plmix = 0.0f;
    _prmix = 0.0f;
}

