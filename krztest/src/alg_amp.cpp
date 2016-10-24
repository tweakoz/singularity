#include "krztest.h"
#include <assert.h>
#include "filters.h"
#include "alg.h"

///////////////////////////////////////////////////////////////////////////////

struct panLR
{
    float lmix,rmix;
};

panLR panBlend(float inp)
{
    panLR rval;
    rval.lmix = (inp>0) 
               ? lerp(0.5,0,inp)
               : lerp(0.5,1,-inp);
    rval.rmix = (inp>0) 
               ? lerp(0.5,1,inp)
               : lerp(0.5,0,-inp);
    return rval;
}
///////////////////////////////////////////////////////////////////////////////

const float kmaxclip = 16.0f; // 18dB
const float kminclip = -16.0f;// 18dB

AMP::AMP( const DspBlockData& dbd )
    : DspBlock(dbd)
{   _numParams = 1;  
}

void AMP::compute(dspBlockBuffer& obuf) //final
{
    float gain = _ctrl[0].eval();//,0.01f,100.0f);


    int inumframes = obuf._numframes;
    float* lbuf = obuf._lowerBuffer;
    float* ubuf = obuf._upperBuffer;

    float* aenv = _layer->_AENV;

    auto u_lrmix = panBlend(_upan);
    auto l_lrmix = panBlend(_lpan);

    const auto& layd = _layer->_layerData;
    const auto& F3 = layd->_f3Block;
    const auto& F4 = layd->_f4Block;

    bool f3ena = true;;//F3._dspBlock.length()!=0;

    //printf( "frq<%f> _phaseInc<%lld>\n", frq, _phaseInc );
    if(numInputs()==1) 
    {   
        float* inpbuf = getInpBuf1(obuf); 
        float SingleLinG = outputGainSINGLE();

        for( int i=0; i<inumframes; i++ )
        {
            _filt = 0.995*_filt + 0.005*gain;
            float linG = decibel_to_linear_amp_ratio(_filt);
            linG *= SingleLinG;
            float inp = inpbuf[i];
            float ae = aenv[i];
            float mono = clip_float(inp*linG*_dbd._pad*ae,kminclip,kmaxclip);
            ubuf[i] = mono*l_lrmix.lmix;
            lbuf[i] = mono*l_lrmix.rmix;
        }
    }
    else if(numInputs()==2)
    {
        float UpperLinG = outputGainU();
        float LowerLinG = outputGainL();

        for( int i=0; i<inumframes; i++ )
        {
            //printf("AMPSTER\n");
            _filt = 0.995*_filt + 0.005*gain;
            float linG = decibel_to_linear_amp_ratio(_filt);
            float ae = aenv[i];
            float totG = linG*ae*_dbd._pad;

           float inpU = ubuf[i]*totG;
           float inpL = lbuf[i]*totG;
           inpU *= UpperLinG;
           inpL *= LowerLinG;

            ubuf[i] = clip_float(inpU*u_lrmix.lmix+inpL*l_lrmix.lmix,kminclip,kmaxclip);
            lbuf[i] = clip_float(inpU*u_lrmix.rmix+inpL*l_lrmix.rmix,kminclip,kmaxclip);
        }
    }
    _fval[0] = _filt;
}

void AMP::doKeyOn(const DspKeyOnInfo& koi) //final
{   _filt = 0.0f;
    auto LD = koi._layer->_layerData;
    auto& f3 = LD->_f3Block;
    auto& f4 = LD->_f4Block;
    float fpanu = float(f3._pan)/7.0f;
    float fpanl = float(f4._pan)/7.0f;
    _upan = fpanu;
    _lpan = fpanl;
}

///////////////////////////////////////////////////////////////////////////////

PLUSAMP::PLUSAMP( const DspBlockData& dbd )
    : DspBlock(dbd)
{   _numParams = 1;  
}

void PLUSAMP::compute(dspBlockBuffer& obuf) //final
{
    float gain = _ctrl[0].eval();//,0.01f,100.0f);

    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float* lbuf = obuf._lowerBuffer;

    float* aenv = _layer->_AENV;

    const auto& layd = _layer->_layerData;
    const auto& F3 = layd->_f3Block;
    const auto& F4 = layd->_f4Block;

    float LinG = outputGainSINGLE();

    //printf( "frq<%f> _phaseInc<%lld>\n", frq, _phaseInc );
    if(1) for( int i=0; i<inumframes; i++ )
    {
        _filt = 0.999*_filt + 0.001*gain;
        float linG = decibel_to_linear_amp_ratio(_filt);
        float inU = ubuf[i]*_dbd._pad*LinG;
        float inL = lbuf[i]*_dbd._pad*LinG;
        float ae = aenv[i];
        float res = (inU+inL)*0.5*linG*ae*2.0;
        res = clip_float(res,-2,2);
        ubuf[i] = res;
        lbuf[i] = res;
        //ubuf[i] = res;
    }
    _fval[0] = _filt;
}

void PLUSAMP::doKeyOn(const DspKeyOnInfo& koi) //final
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

    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float* lbuf = obuf._lowerBuffer;

    float* aenv = _layer->_AENV;
    float LinG = outputGainSINGLE();

    //printf( "frq<%f> _phaseInc<%lld>\n", frq, _phaseInc );
    if(1) for( int i=0; i<inumframes; i++ )
    {   
        _filt = 0.999*_filt + 0.001*gain;
        float linG = decibel_to_linear_amp_ratio(_filt);
        float inU = ubuf[i]*_dbd._pad;
        float inL = lbuf[i]*_dbd._pad;
        float ae = aenv[i];
        float res = (inU*inL)*linG*ae*LinG;
        //res = clip_float(res,-4,4);
        lbuf[i] = res;
        ubuf[i] = res;
    }
    _fval[0] = _filt;
}

void XAMP::doKeyOn(const DspKeyOnInfo& koi) //final
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
    float* inpbuf = getInpBuf1(obuf); 

    //printf( "frq<%f> _phaseInc<%lld>\n", frq, _phaseInc );
    if(1) for( int i=0; i<inumframes; i++ )
    {   float inp = inpbuf[i]*_dbd._pad;
        output1(obuf,i, inp*linG );
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
        float inputU = ubuf[i]*_dbd._pad;
        float inputL = lbuf[i]*_dbd._pad;
        _plmix = _plmix*0.995f+lmix*0.005f;
        _pumix = _pumix*0.995f+umix*0.005f;

        float mixed = (inputU*_pumix)+(inputL*_plmix);
        output1(obuf,i, mixed );
        //ubuf[i] = inputU+inputL;//(inputU*_pumix)+(inputL*_plmix);
    }
}

void XFADE::doKeyOn(const DspKeyOnInfo& koi) //final
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

    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float* lbuf = obuf._lowerBuffer;

    if(1) for( int i=0; i<inumframes; i++ )
    {   
        _filt = 0.999*_filt + 0.001*gain;
        float linG = decibel_to_linear_amp_ratio(_filt);
        float inU = ubuf[i]*_dbd._pad;
        float inL = lbuf[i]*_dbd._pad;
        float res = (inU*inL)*linG;
        res = clip_float(res,-1,1);
        //lbuf[i] = res;

        output1(obuf,i, res );
    }
    _fval[0] = _filt;
}
void XGAIN::doKeyOn(const DspKeyOnInfo& koi) //final
{   _filt = 0.0f;
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



    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float* lbuf = obuf._lowerBuffer;

    auto u_lrmix = panBlend(_upan);
    auto l_lrmix = panBlend(_lpan);

    float* aenv = _layer->_AENV;

    const auto& layd = _layer->_layerData;
    const auto& F3 = layd->_f3Block;
    const auto& F4 = layd->_f4Block;
    float LowerLinG = decibel_to_linear_amp_ratio(F4._v14Gain);
    float UpperLinG = decibel_to_linear_amp_ratio(F3._v14Gain);

    //printf( "frq<%f> _phaseInc<%lld>\n", frq, _phaseInc );
    if(1) for( int i=0; i<inumframes; i++ )
    {   
        _filtU = 0.999*_filtU + 0.001*gainU;
        _filtL = 0.999*_filtL + 0.001*gainL;
        float linGU = decibel_to_linear_amp_ratio(_filtU);
        float linGL = decibel_to_linear_amp_ratio(_filtL);
        float inU = ubuf[i]*_dbd._pad;
        float inL = lbuf[i]*_dbd._pad;
        float ae = aenv[i];
        float resU = inU*linGU*ae*UpperLinG;
        float resL = inL*linGL*ae*LowerLinG;

        ubuf[i] = clip_float(resU*u_lrmix.lmix+resL*l_lrmix.lmix,kminclip,kmaxclip);
        lbuf[i] = clip_float(resU*u_lrmix.rmix+resL*l_lrmix.rmix,kminclip,kmaxclip);
    }
    _fval[0] = _filtU;
    _fval[1] = _filtL;
}

void AMPU_AMPL::doKeyOn(const DspKeyOnInfo& koi) //final
{   _filtU = 0.0f;
    _filtL = 0.0f;
    auto LD = koi._layer->_layerData;
    auto& f3 = LD->_f3Block;
    auto& f4 = LD->_f4Block;
    float fpanu = float(f3._pan)/7.0f;
    float fpanl = float(f4._pan)/7.0f;
    _upan = fpanu;
    _lpan = fpanl;
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
void BAL_AMP::doKeyOn(const DspKeyOnInfo& koi) //final
{
    
}
///////////////////////////////////////////////////////////////////////////////

PANNER::PANNER( const DspBlockData& dbd )
    :DspBlock(dbd)
{   _numParams = 1;
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
        float input = ubuf[i]*_dbd._pad;
        _plmix = _plmix*0.995f+lmix*0.005f;
        _prmix = _prmix*0.995f+rmix*0.005f;

        ubuf[i] = input*_plmix;
        lbuf[i] = input*_prmix;
    }
}
void PANNER::doKeyOn(const DspKeyOnInfo& koi) //final
{   _plmix = 0.0f;
    _prmix = 0.0f;
}

///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////

BANGAMP::BANGAMP( const DspBlockData& dbd )
    : DspBlock(dbd)
{   _numParams = 1;
}

void BANGAMP::compute(dspBlockBuffer& obuf) //final
{
    float gain = _ctrl[0].eval();//,0.01f,100.0f);

    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float* lbuf = obuf._lowerBuffer;

    float* aenv = _layer->_AENV;
    const auto& layd = _layer->_layerData;
    const auto& F3 = layd->_f3Block;
    const auto& F4 = layd->_f4Block;
    float LinG = outputGainSINGLE();

    //printf( "frq<%f> _phaseInc<%lld>\n", frq, _phaseInc );
    if(1) for( int i=0; i<inumframes; i++ )
    {   
        _smooth = 0.999*_smooth + 0.001*gain;
        float linG = decibel_to_linear_amp_ratio(_smooth);
        float inU = ubuf[i]*_dbd._pad;
        float inL = lbuf[i]*_dbd._pad;
        float ae = aenv[i];
        float res = (inU+inL);
        res = shaper(res,.25)*linG*ae*LinG;
        lbuf[i] = res;
        ubuf[i] = res;
    }
    _fval[0] = _smooth;
}

void BANGAMP::doKeyOn(const DspKeyOnInfo& koi) //final
{   _smooth = 0.0f;
}


