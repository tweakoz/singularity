#include "krztest.h"
#include <assert.h>
#include "filters.h"
#include "alg.h"

///////////////////////////////////////////////////////////////////////////////

SINE::SINE( const DspBlockData& dbd )
    : DspBlock(dbd)
    , _pblep(48000,PolyBLEP::SINE)
{   _numParams = 1;                
}

void SINE::compute(dspBlockBuffer& obuf) //final
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
    {   float saw = _pblep.getAndInc();
        saw *= _layer->_AENV[i];
        ubuf[i] = saw;
    }
}
void SINE::doKeyOn(layer* l) //final
{
}

///////////////////////////////////////////////////////////////////////////////

SINEPLUS::SINEPLUS( const DspBlockData& dbd )
    : DspBlock(dbd)
    , _pblep(48000,PolyBLEP::SINE)
{   _numParams = 1;                
}

void SINEPLUS::compute(dspBlockBuffer& obuf) //final
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
void SINEPLUS::doKeyOn(layer* l) //final
{
}

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
    , _pblep(48000,PolyBLEP::RAMP)
{   _numParams = 1;    
    _pblep.setAmplitude(1.0f);            
}

void SWPLUSSHP::compute(dspBlockBuffer& obuf) //final
{
    float centoff = _ctrl[0].eval()*100;//,0.01f,100.0f);
    _fval[0] = centoff;

    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float lyrcents = _layer->_curcents;
    float frq = midi_note_to_frequency((centoff+lyrcents)*0.01);
    float SR = _layer->_syn._sampleRate;
    _pblep.setFrequency(frq);

    printf( "_dbd._pad<%f>\n", _dbd._pad );


    if(1) for( int i=0; i<inumframes; i++ )
    {   float input = ubuf[i];//*_dbd._pad;///_layer->_AENV[i];
        float saw = _pblep.getAndInc();
        //saw *= _layer->_AENV[i];
        float xxx = wrap(input+saw,1.0);
        float swplusshp = shaper(xxx,.25);
        ubuf[i] = (swplusshp)*_layer->_AENV[i];
    }
}
void SWPLUSSHP::doKeyOn(layer* l) //final
{
}