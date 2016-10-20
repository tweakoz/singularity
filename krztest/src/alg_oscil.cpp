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
    float lyrcents = _layer->_curcentsOSC;
    float cin = (lyrcents+centoff)*0.01;
    float frq = midi_note_to_frequency(cin);
    //printf("lc<%f> coff<%f> cin<%f> frq<%f>\n", lyrcents, centoff, cin, frq );
    float SR = _layer->_syn._sampleRate;
    _pblep.setFrequency(frq);

    //printf( "frq<%f> _phaseInc<%lld>\n", frq, _phaseInc );
    if(1) for( int i=0; i<inumframes; i++ )
    {   float saw = _pblep.getAndInc();
        //saw *= _layer->_AENV[i];
        ubuf[i] = saw;
    }
}
void SINE::doKeyOn(const DspKeyOnInfo& koi) //final
{
}

///////////////////////////////////////////////////////////////////////////////

SAW::SAW( const DspBlockData& dbd )
    : DspBlock(dbd)
    , _pblep(48000,PolyBLEP::SAWTOOTH)
{   _numParams = 1;                
}

void SAW::compute(dspBlockBuffer& obuf) //final
{
    float centoff = _ctrl[0].eval();//,0.01f,100.0f);
    _fval[0] = centoff;

    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float lyrcents = _layer->_curcentsOSC;
    float cin = (lyrcents+centoff)*0.01;
    float frq = midi_note_to_frequency(cin);
    float SR = _layer->_syn._sampleRate;
    _pblep.setFrequency(frq);

    //printf( "frq<%f> _phaseInc<%lld>\n", frq, _phaseInc );
    if(1) for( int i=0; i<inumframes; i++ )
    {   float saw = _pblep.getAndInc();
        //saw *= _layer->_AENV[i];
        ubuf[i] = saw;
    }
}
void SAW::doKeyOn(const DspKeyOnInfo& koi) //final
{
}

///////////////////////////////////////////////////////////////////////////////

SQUARE::SQUARE( const DspBlockData& dbd )
    : DspBlock(dbd)
    , _pblep(48000,PolyBLEP::SQUARE)
{   _numParams = 1;                
}

void SQUARE::compute(dspBlockBuffer& obuf) //final
{
    float centoff = _ctrl[0].eval();//,0.01f,100.0f);
    _fval[0] = centoff;

    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float lyrcents = _layer->_curcentsOSC;
    float cin = (lyrcents+centoff)*0.01;
    float frq = midi_note_to_frequency(cin);
    float SR = _layer->_syn._sampleRate;
    _pblep.setFrequency(frq);

    //printf( "frq<%f> _phaseInc<%lld>\n", frq, _phaseInc );
    if(1) for( int i=0; i<inumframes; i++ )
    {   float saw = _pblep.getAndInc();
        //saw *= _layer->_AENV[i];
        ubuf[i] = saw;
    }
}
void SQUARE::doKeyOn(const DspKeyOnInfo& koi) //final
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
    float lyrcents = _layer->_curcentsOSC;
    float cin = (lyrcents+centoff)*0.01;
    float frq = midi_note_to_frequency(cin);
    float SR = _layer->_syn._sampleRate;
    _pblep.setFrequency(frq);
    float pad = _dbd._pad;

    //printf( "frq<%f> _phaseInc<%lld>\n", frq, _phaseInc );
    if(1) for( int i=0; i<inumframes; i++ )
    {   float input = ubuf[i]*pad;
        float saw = _pblep.getAndInc();
        //saw *= _layer->_AENV[i];
        float swplus = input+saw;
        ubuf[i] = swplus;
    }
}
void SINEPLUS::doKeyOn(const DspKeyOnInfo& koi) //final
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
    float lyrcents = _layer->_curcentsOSC;
    float cin = (lyrcents+centoff)*0.01;
    float frq = midi_note_to_frequency(cin);
    float SR = _layer->_syn._sampleRate;
    _pblep.setFrequency(frq);
    float pad = _dbd._pad;

    //printf("lc<%f> coff<%f> cin<%f> frq<%f>\n", lyrcents, centoff, cin, frq );
    //printf( "saw+ pad<%f>\n", pad );

    //printf( "frq<%f> _phaseInc<%lld>\n", frq, _phaseInc );
    if(1) for( int i=0; i<inumframes; i++ )
    {   float input = ubuf[i]*pad;
        float saw = _pblep.getAndInc();
        //saw *= _layer->_AENV[i];
        float swplus = input+(saw);
        ubuf[i] = swplus;
    }
}
void SAWPLUS::doKeyOn(const DspKeyOnInfo& koi) //final
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
    float centoff = _ctrl[0].eval();//,0.01f,100.0f);
    _fval[0] = centoff;

    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float lyrcents = _layer->_curcentsOSC;
    float cin = (lyrcents+centoff)*0.01;
    float frq = midi_note_to_frequency(cin);
    float SR = _layer->_syn._sampleRate;
    _pblep.setFrequency(frq);
    float pad = _dbd._pad;

    //printf( "_dbd._pad<%f>\n", _dbd._pad );


    if(1) for( int i=0; i<inumframes; i++ )
    {   float input = clip_float(ubuf[i]*pad,-1,1);//*_dbd._pad;///_layer->_AENV[i];
        float saw = _pblep.getAndInc();
        //saw *= _layer->_AENV[i];
        float xxx = wrap(input+saw,1.0);
        float swplusshp = shaper(xxx,.25);
        ubuf[i] = (swplusshp);//*_layer->_AENV[i];
    }
}
void SWPLUSSHP::doKeyOn(const DspKeyOnInfo& koi) //final
{
}

///////////////////////////////////////////////////////////////////////////////

SHAPEMODOSC::SHAPEMODOSC( const DspBlockData& dbd )
    : DspBlock(dbd)
    , _pblep(48000,PolyBLEP::SINE)
{   _numParams = 2;    
    _pblep.setAmplitude(1.0f);            
}

void SHAPEMODOSC::compute(dspBlockBuffer& obuf) //final
{
    float centoff = _ctrl[0].eval();//,0.01f,100.0f);
    float depth = _ctrl[1].eval();//,0.01f,100.0f);

    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float* lbuf = obuf._lowerBuffer;
    float lyrcents = _layer->_curcentsOSC;
    float cin = (lyrcents+centoff)*0.01;
    float frq = midi_note_to_frequency(cin);
    float SR = _layer->_syn._sampleRate;
    _pblep.setFrequency(frq);
    float pad = _dbd._pad;

    //printf( "_dbd._pad<%f>\n", _dbd._pad );

    float depg = decibel_to_linear_amp_ratio(depth);


    const float kc1 = 1.0f/128.0f;
    const float kc2 = 1.0f/32;

    if(1) for( int i=0; i<inumframes; i++ )
    {   
        float inU = ubuf[i]*pad;
        float inL = lbuf[i]*pad;
        float inp = (inU)*depg;

        //First, the SINE value is multiplied by 
        // the sample input value, then multiplied 
        // by a constant—any samples exceeding full scale 
        // will wrap around.

        float sine = _pblep.getAndInc();
        float xxx = wrap(inp*sine*kc1,1.0);

        //The result is added to the wrapped product 
        // of the SINE value times a constant.

        float yyy = xxx + wrap(sine*kc2,1.0);

        //The entire resulting waveform is then passed 
        // through the SHAPER, whose Adjust value is 
        // set by the level of the sample input.

        float swplusshp = shaper(yyy,inp);
        ubuf[i] = (swplusshp);

    }

    _fval[0] = centoff;
    _fval[1] = depth;
}
void SHAPEMODOSC::doKeyOn(const DspKeyOnInfo& koi) //final
{
}

///////////////////////////////////////////////////////////////////////////////

PLUSSHAPEMODOSC::PLUSSHAPEMODOSC( const DspBlockData& dbd )
    : DspBlock(dbd)
    , _pblep(48000,PolyBLEP::SINE)
{   _numParams = 2;    
    _pblep.setAmplitude(0.25f);            
}

void PLUSSHAPEMODOSC::compute(dspBlockBuffer& obuf) //final
{
    float centoff = _ctrl[0].eval();//,0.01f,100.0f);
    float depth = _ctrl[1].eval();//,0.01f,100.0f);

    int inumframes = obuf._numframes;
    float* ubuf = obuf._upperBuffer;
    float* lbuf = obuf._lowerBuffer;
    float lyrcents = _layer->_curcentsOSC;
    float cin = (lyrcents+centoff)*0.01;
    float frq = midi_note_to_frequency(cin);
    float SR = _layer->_syn._sampleRate;
    _pblep.setFrequency(frq);
    float pad = _dbd._pad;

    //printf( "_dbd._pad<%f>\n", _dbd._pad );

    float depg = decibel_to_linear_amp_ratio(depth);


    const float kc1 = 1.0f/128.0f;
    const float kc2 = 1.0f/32;

    if(1) for( int i=0; i<inumframes; i++ )
    {   
        float inU = ubuf[i]*pad;
        float inL = lbuf[i]*pad;
        float inp = (inU+inL)*depg;

        //First, the SINE value is multiplied by 
        // the sample input value, then multiplied 
        // by a constant—any samples exceeding full scale 
        // will wrap around.

        float sine = _pblep.getAndInc();
        float xxx = wrap(inp*sine*kc1,1.0);

        //The result is added to the wrapped product 
        // of the SINE value times a constant.

        float yyy = xxx + wrap(sine*kc2,1.0);

        //The entire resulting waveform is then passed 
        // through the SHAPER, whose Adjust value is 
        // set by the level of the sample input.

        float swplusshp = shaper(yyy,inp);
        ubuf[i] = (swplusshp);


        /*
        x SHAPE MOD OSC
        this function is
         similar to SHAPE MOD OSC, except that it multiplies
         its two input signals and uses that result as its input.

        + SHAPE MOD OSC

         + SHAPE MOD OSC is similar to x SHAPE MOD OSC, 
         except that it adds its two input signals and uses 
         that sum as its input. 
*/
    }

    _fval[0] = centoff;
    _fval[1] = depth;
}
void PLUSSHAPEMODOSC::doKeyOn(const DspKeyOnInfo& koi) //final
{
}



